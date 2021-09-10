#include <QFileDialog>
#include <QProcess>
#include <QMessageBox>
#include <QtEndian>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->comboBox_mode->addItem("4-sprite VDP1 background");
    ui->comboBox_mode->addItem("8x8 cell VDP2 sprite");
    ui->comboBox_mode->addItem("704xHUGE single file VDP1 tapestry");
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    QFileDialog dia;
    list.clear();
    list = dia.getOpenFileNames(this,tr("Open Image"),"",tr("Image Files (*.*)"));
    ui->label_2->setText(QString(tr("%1 files selected")).arg(list.size()));
    //generate auto 8.3 names based on longnames
    list83.clear();
    //list83pal.clear();
    for (int i=0; i<list.size(); i++)
    {
        QFileInfo info(list.at(i));
        QString str = info.fileName();
        /*QString str2,str3;
        if(str.startsWith("ext_"))
        {
            str2.append("E_");
            str = str.mid(4);
        }
        else if(str.startsWith("int_"))
        {
            str2.append("I_");
            str = str.mid(4);
        }
        while ( (str.length() > 0) && (str2.length() < 8) )
        {
            if (str.startsWith("of_"))
                str = str.mid(3);
            str2.append(str.left(3).toUpper());
            str2.append("_");
            if (str2.length()>8) str2 = str2.left(8);
            if (str.indexOf('_')> 0)
            {
                str = str.mid(str.indexOf('_')+1);
            }
        }
        str3 = str2;
        str3.append(".NBG");
        //list83.append(str3);*/
        list83.append(info.baseName());//.append(".nbg"));
        //str3 = str2;
        //str3.append(".PAL");
        //list83pal.append(str3);
        //list83pal.append(info.baseName().append(".pal"));
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    QProcess process;
    QStringList proc_args;
    QImage img;
    int iSectorsUsed;
    QList<QByteArray> TileLibrary;
    int written;

    int iLimit = 127;
    if (ui->comboBox_mode->currentIndex() == 2)
        iLimit = 1; //only a single file in tapestry mode
    if (list.size() > iLimit)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Oopsie daisy");
        msgBox.setText("Packs with more than 127 files and tapestries with more than 1 file are not supported!");
        msgBox.exec();
        return;
    }

    //making big bin instead of independent small NBG/PAL
    QFile outfile_pack(QString("DATA.PAK"));
    outfile_pack.open(QIODevice::WriteOnly);

    //pass 1 - create filelist table
    //first 64-byte entry is a system header
    outfile_pack.write(QByteArray("SVINPACK"));
    qint16_be s = (qint16_be)list83.size();//files in pack
    outfile_pack.write((char*)&s,2);
    s = 64;//filename size
    outfile_pack.write((char*)&s,2);
    QByteArray b;
    b.fill('\0',52);
    outfile_pack.write(b);

    //64 bytes per entry, zero-terminated
    written = 64;
    for (int i=0; i<list83.size(); i++)
    {
        QByteArray _name;
        _name.append("1234567890");//placeholder for : LEFT, TOP, SIZE_X, SIZE_Y, sector
        _name.append(list83.at(i).toLatin1());
        while (_name.size()<64)
            _name.append('\0');
        outfile_pack.write(_name);
        written+=64;
    }
    if (ui->comboBox_mode->currentIndex() == 1)
    {
        //for VDP2 format add a special "tile library" file at the end of the pack
        QByteArray _name;
        _name.append("1234567890_svin_tile_library");
        while (_name.size()<64)
            _name.append('\0');
        outfile_pack.write(_name);
        written+=64;
    }
    //up to 127 files per pack, that's 8192 bytes = 4 sectors
    //filling rest
    while (written < 8192)
    {
        QByteArray _name;
        while (_name.size()<64)
            _name.append('\0');
        outfile_pack.write(_name);
        written+=64;
    }

    int iCurrentSector = (list.size())/32 + 1;

    //using imagemagick for image transforms
    process.setProgram("C:\\Program Files\\ImageMagick-7.0.11-Q16-HDRI\\magick");

    TileLibrary.clear();

    //pass2 -*/
    for (int iImageNumber=0; iImageNumber<list.size(); iImageNumber++)
    {

        if (ui->comboBox_mode->currentIndex() == 0)  //VDP1 backs
        {
            //resize image to at least 704x448 each axis
            proc_args.clear();
            proc_args.append(list.at(iImageNumber));
            proc_args.append("-resize");
            proc_args.append("704x448^");
            proc_args.append("tmp1.png");
            process.setArguments(proc_args);
            process.open();
            process.waitForFinished();

            //cut image down to 704x448
            proc_args.clear();
            proc_args.append("tmp1.png");
            proc_args.append("-gravity");
            proc_args.append("center");
            proc_args.append("-extent");
            proc_args.append("704x448");
            proc_args.append("-resize");
            proc_args.append("704x448^");
            proc_args.append("tmp2.png");
            process.setArguments(proc_args);
            process.open();
            process.waitForFinished();
        }
        else if (ui->comboBox_mode->currentIndex() == 1) //VDP2
        {
            //resize sprites for height of 448
            proc_args.clear();
            proc_args.append(list.at(iImageNumber));
            proc_args.append("-resize");
            proc_args.append("8x448^");
            proc_args.append("tmp1.png");
            process.setArguments(proc_args);
            process.open();
            process.waitForFinished();
            QFile::remove("tmp2.png");
            QFile::copy("tmp1.png", "tmp2.png");
        }
        else //VDP1 tapestry
        {
            //no resize
        }


        if (ui->comboBox_mode->currentIndex() == 0)
        {
            //generate palette, sprited VDP1 palette is limited to 254 colours
            proc_args.clear();
            proc_args.append("tmp2.png");
            proc_args.append("-colors");
            proc_args.append("254");
            proc_args.append("tmp3.png");
            process.setArguments(proc_args);
            process.open();
            process.waitForFinished();            
            //backuppy
            QFile::copy("tmp2.png", QString("tmp%1b.png").arg(iImageNumber,4,10,QLatin1Char('0')));
            img.load("tmp3.png");
        }
        else if (ui->comboBox_mode->currentIndex() == 1)
        {
            //generate palette, VDP2 NBG palette is limited to 255 colours
            proc_args.clear();
            proc_args.append("tmp2.png");
            proc_args.append("-colors");
            proc_args.append("255");
            proc_args.append("tmp3.png");
            process.setArguments(proc_args);
            process.open();
            process.waitForFinished();
            //backuppy
            QFile::copy("tmp2.png", QString("tmp%1b.png").arg(iImageNumber,4,10,QLatin1Char('0')));
            img.load("tmp3.png");
        }
        else
        {
            //tapestry
            img.load(list.at(iImageNumber));
        }


        if (ui->comboBox_mode->currentIndex() == 1)
        {
            //let's crop image to tile size
            if ( (img.size().width()%8 != 0) || (img.size().height()%8 != 0) )
                img = img.copy(0,0,(img.size().width()/8)*8,(img.size().height()/8)*8);
        }

        QByteArray ba;
        //QByteArray ba_map;
        QByteArray ba_pal;

        int iLeft=0;
        int iTop=0;
        int iSizeX=img.width();
        int iSizeY=img.height();

        //updating rect in header
        //qint64 SeekBackup = outfile_pack.pos();
        outfile_pack.seek(64+64*iImageNumber);
        qint16_be s = (qint16_be)iLeft;
        outfile_pack.write((char*)&s,2);
        s = (qint16_be)iTop;
        outfile_pack.write((char*)&s,2);
        s = (qint16_be)(iSizeX);
        outfile_pack.write((char*)&s,2);
        s = (qint16_be)(iSizeY);
        outfile_pack.write((char*)&s,2);
        s = (qint16_be)iCurrentSector;
        outfile_pack.write((char*)&s,2);
        outfile_pack.seek(iCurrentSector*2048);

        if (ui->comboBox_mode->currentIndex() == 0) //VDP1 4-sprites mode
        {
            ba.resize(704*448);
            ba.fill('\0');
            //new background mode : 4 VDP1 interlaced sprites

            for (int x = 0; x < 352; x++)
                for (int y = 0; y < 224; y++)
                {
                    ba[352*224*0 + y*352+x] = img.pixelIndex(x,y*2);
                    ba[352*224*1 + y*352+x] = img.pixelIndex(352+x,y*2);
                    ba[352*224*2 + y*352+x] = img.pixelIndex(x,y*2+1);
                    ba[352*224*3 + y*352+x] = img.pixelIndex(352+x,y*2+1);
                }
        }
        /*else if (ui->comboBox_mode->currentIndex() == 1)
        {
            ba.resize(704*448);
            ba.fill('\0');
            //mess with tiles
            //plane 1 processing
            for (int iTileY = 0; iTileY < 28; iTileY++)
                for (int iTileX = 0; iTileX < 32; iTileX++)
                    for (int cell=0;cell<4;cell++)
                        for (int y=0;y<8;y++)
                            for (int x=0;x<8;x++)
                                ba[(iTileY*32+iTileX)*16*16+cell*64+y*8+x] = img.pixelIndex(iTileX*16+(cell%2)*8+x,iTileY*16+(cell/2)*8+y);

            //plane 2 processing
            for (int iTileY = 0; iTileY < 28; iTileY++)
                for (int iTileX = 32; iTileX < 44; iTileX++)
                    for (int cell=0;cell<4;cell++)
                        for (int y=0;y<8;y++)
                            for (int x=0;x<8;x++)
                                ba[32*28*16*16+(iTileY*12+(iTileX-32))*16*16+cell*64+y*8+x] = img.pixelIndex(iTileX*16+(cell%2)*8+x,iTileY*16+(cell/2)*8+y);

        }*/
        else if (ui->comboBox_mode->currentIndex() == 1) //tile mode
        {
            //let's do some heavy tiling.
            //we fill pack-wise tile library. for each tile we check if it exist within library, and add it if not.
            //for each image we prepare a tile-wise recipe.
            //let's detect a transparent color
            int transp_color = img.pixelIndex(0,0);
            //now we calculate a usage map. since no sprite will be bigger than full screen, maximum usage map is 88x56 for 8x8 tiles.
            //calculating tiled size
            int iSizeTiledX = iSizeX/8;
            int iSizeTiledY = iSizeY/8;
            //int iUsageMap[88][56];
            for (int TileY = 0; TileY < iSizeTiledY; TileY++)
            {
                for (int TileX = 0; TileX < iSizeTiledX; TileX++)
                {
                    //is the tile empty?
                    bool bEmpty = true;
                    for (int x=0;x<8;x++)
                    {
                        for (int y=0;y<8;y++)
                        {
                            if (img.pixelIndex(TileX*8+x,TileY*8+y) != transp_color)
                                    bEmpty = false;
                        }
                    }
                    //if the tile is non-empty, add it into recipe
                    if (false == bEmpty)
                    {
                        //we may check if the exact tile like this is already used
                        //this will be useful for singlalayer sprites set
                        //but the probability is too low for multilayer sprites,so not doing it for now
                        //TODO: do it later, maybe add an option
                        QByteArray tiledata;
                        for (int y=0;y<8;y++)
                        {
                            for (int x=0;x<8;x++)
                            {
                                tiledata.append((char)img.pixelIndex(TileX*8+x,TileY*8+y));
                            }
                        }
                        TileLibrary.append(tiledata);
                        //save recipe instead of actual tile data
                        ba.append(TileX);
                        ba.append(TileY);
                        ba.append(TileLibrary.size()%0x100);
                        ba.append(TileLibrary.size()/0x100);
                    }
                }
            }

            /*ba_map.clear();
            //now save usage map into file
            for (int TileY = 0; TileY < 56; TileY++)
            {
                for (int TileX = 0; TileX < 88; TileX+=8)
                {
                    uint8_t c = 0;
                    for (int Tile = 0; Tile<8; Tile++)
                    {
                        if (iUsageMap[TileX+Tile][TileY] != 0)
                            c |= 1<<Tile;
                    }
                    ba_map.append(c);
                }
            }*/

            //now save every tile within usage map
            /*ba.clear();
            for (int TileY = 0; TileY < 56; TileY++)
            {
                for (int TileX = 0; TileX < 88; TileX++)
                {
                    if (iUsageMap[TileX][TileY] == 1)
                    {
                        for (int x=0;x<8;x++)
                        {
                            for (int y=0;y<8;y++)
                            {
                                ba.append(img.pixelIndex(TileX*8+x,TileY*8+y));
                            }
                        }
                    }

                }
            }*/

            /*
            ba.resize(iSizeX*iSizeY);
            ba.fill('\0');
            //mess with tiles
            for (int iTileY = 0; iTileY < (iSizeY/16); iTileY++)
                for (int iTileX = 0; iTileX < (iSizeX/16); iTileX++)
                    for (int cell=0;cell<4;cell++)
                        for (int y=0;y<8;y++)
                            for (int x=0;x<8;x++)
                                ba[(iTileY*(iSizeX/16)+iTileX)*16*16+cell*64+y*8+x] = img_c.pixelIndex(iTileX*16+(cell%2)*8+x,iTileY*16+(cell/2)*8+y);
                                */
        }      
        else //tapestry
        {
            // for tapestries we do something completely different
            // pack every 704x2 quad into 4 quads with size of 352x1
            // and store these packs with CD block offset (2048 bytes)
            QByteArray ba_tmp;
            ba_tmp.resize(2048);
            ba_tmp.fill('\0');
            ba.clear();
            for (int iLine = 0; iLine < iSizeY/2; iLine++)
            {
                for (int x = 0; x < 352; x++)
                    {
                        ba_tmp[352*0 + x] = img.pixelIndex(x,iLine*2);
                        ba_tmp[352*1 + x] = img.pixelIndex(352+x,iLine*2);
                        ba_tmp[352*2 + x] = img.pixelIndex(x,iLine*2+1);
                        ba_tmp[352*3 + x] = img.pixelIndex(352+x,iLine*2+1);
                    }
                ba.append(ba_tmp,2048);
            }
        }

        ba_pal.clear();
        for (int j=0;j<img.colorTable().size();j++)
        {
            ba_pal.append(QColor(img.colorTable().at(j)).red());
            ba_pal.append(QColor(img.colorTable().at(j)).green());
            ba_pal.append(QColor(img.colorTable().at(j)).blue());
        }
        while (ba_pal.size()<256*3)
            ba_pal.append('\0');

        //doing palette hacks
        if (ui->comboBox_mode->currentIndex() == 0)
        {
            //for backgrounds 0x00 (transparent) and 0xFE(normal shadow) can't be used
            //imagemagick generates palettes from 0x00 to 0xFD, 0xFE being transparent
            //so we only have to move color 0x00 to color 0xFF
            for (int i=0;i<ba.size();i++)
                if (ba.at(i) == 0)
                    ba[i]=-1;
            //hacking palette, moving color 0x00 to color 0xFF
            ba_pal[255*3] = ba_pal.at(0*3);
            ba_pal[255*3+1] = ba_pal.at(0*3+1);
            ba_pal[255*3+2] = ba_pal.at(0*3+2);
        }
        else if (ui->comboBox_mode->currentIndex() == 1)
        {
            //for VDP2 sprites 0x00 (transparent) can't be used
            //imagemagick generates palettes from 0x00 to 0xFE, 0xFF being transparent
            //so we only have to move color 0x00 to color 0xFF
            /*for (int i=0;i<ba.size();i++)
                if (ba.at(i) == 0)
                    ba[i]=-1;
            //hacking palette, moving color 0x00 to color 0xFF
            ba_pal[255*3] = ba_pal.at(0*3);
            ba_pal[255*3+1] = ba_pal.at(0*3+1);
            ba_pal[255*3+2] = ba_pal.at(0*3+2);*/
        }
        else //tapestry
        {
            //for backgrounds 0x00 (transparent) and 0xFE(normal shadow) can't be used
            //imagemagick generates palettes from 0x00 to 0xFD, 0xFE being transparent
            //so we only have to move color 0x00 to color 0xFF
            for (int i=0;i<ba.size();i++)
                if (ba.at(i) == 0)
                    ba[i]=-1;
            //hacking palette, moving color 0x00 to color 0xFF
            ba_pal[255*3] = ba_pal.at(0*3);
            ba_pal[255*3+1] = ba_pal.at(0*3+1);
            ba_pal[255*3+2] = ba_pal.at(0*3+2);
        }


        outfile_pack.write(ba); //write recipe
        written = ba.size();
        //fill last cluster
        while (written%2048 > 0)
        {
            written++;
            outfile_pack.write("\0",1);
        }
        iSectorsUsed = (ba.size() - 1)/2048 + 1;
        iCurrentSector += iSectorsUsed;//adding used sectors

        outfile_pack.write(ba_pal); //write palette
        written = ba_pal.size();
        //fill last cluster
        while (written%2048 > 0)
        {
            written++;
            outfile_pack.write("\0",1);
        }
        iCurrentSector++;//adding sector for palette

        /*outfile_pack.write(ba_map);
        //fill last cluster
        while (outfile_pack.size()%2048 > 0)
            outfile_pack.write("\0",1);
        iCurrentSector++;//adding sector for usagemap */
    }

    //saving tile library. it is a last file, everything else is processed, so wa can save it safely
    written = 0;
    for (int i=0;i<TileLibrary.size();i++)
    {
        outfile_pack.write(TileLibrary.at(i)); //write tile
        written += TileLibrary.at(i).size();
    }

    //fill last cluster
    while (written%2048 > 0)
    {
        written++;
        outfile_pack.write("\0",1);
    }

    //don't care about iCurrentSector because it's the last file

    outfile_pack.close();

}

void MainWindow::on_pushButton_Set_reciles_List_clicked()
{
    ui->lineEdit_Recipes_List->setText(QFileDialog::getOpenFileName(this,
        tr("Open Recipe"), "", tr("Recipe Files (*.*)")));
}

void MainWindow::on_pushButton_Set_Script_clicked()
{
    ui->lineEdit_script->setText(QFileDialog::getOpenFileName(this,
        tr("Open Script"), "", tr("Script Files (*.*)")));
}

void MainWindow::on_pushButton_Process_Sprites_clicked()
{
    //load every sprite recipe from script into recipes list
    QFile script_file(ui->lineEdit_script->text());
    script_file.open(QIODevice::ReadOnly);
    QList<QByteArray> Script_Lines;
    QByteArray b;
    do {
        b = script_file.readLine();
        Script_Lines.append(b);
    } while (b.length()>0);
    ui->textEdit->append(QString("Loaded %1 strings from script").arg(Script_Lines.size()));

    QList<QByteArray> Script_Sprite_Recipes;
    QList<int> Script_Sprite_Recipes_Position;
    for (int i=0;i<Script_Lines.size();i++)
    {
        if (Script_Lines.at(i).simplified().startsWith("show"))
        {
            QByteArray b2 = Script_Lines.at(i).simplified();
            b2 = b2.mid(5); //remove "show"
            if (b2.contains("with"))
                b2 = b2.left(b2.indexOf(" with")); //remove appearance effect for now
            if (b2.contains("at"))
                b2 = b2.left(b2.indexOf(" at")); //remove location
            Script_Sprite_Recipes.append(b2);
            Script_Sprite_Recipes_Position.append((i));
        }
    }
    ui->textEdit->append(QString("Script: detected %1 sprite recipes").arg(Script_Sprite_Recipes.size()));

    //now find a corresponding recipe for each entry from recipe list
    QFile recipes_file(ui->lineEdit_Recipes_List->text());
    recipes_file.open(QIODevice::ReadOnly);
    QByteArray Recipes_data = recipes_file.readAll().simplified();
    QList<QByteArray> ImagePaths;
    QList<Image_Link> ImageLinks;
    QList<int> ImageTilesUsage;

    //parse used recipies
    for (int i=0;i<Script_Sprite_Recipes.size();i++)
    {
        //find exact match for our recipe
        QByteArray search = Script_Sprite_Recipes.at(i);
        search.append(" =");
        QByteArray b = Recipes_data.mid(Recipes_data.indexOf(search));
        b = b.left(b.indexOf("image ")-1);//cut until next recipe
        //cutout night and evening conditions for now
        if (b.contains("ConditionSwitch"))
        {
            b = b.mid(b.indexOf("True"));
        }
        int iLayer = 0;
        while (b.contains("\""))
        {
            b = b.mid(b.indexOf("\"")+1);
            QByteArray b2 = b.left(b.indexOf("\""));
            //checking if the image alreaddy exists in the list
            if (ImagePaths.contains(b2))
            {
                //do not add anoter copy, just link it
            }
            else
            {
                //new file, append it
                ImagePaths.append(b2);
            }
            ImageLinks.append({i,ImagePaths.indexOf(b2),iLayer});
            iLayer = 1;
            b = b.mid(b.indexOf("\"")+1);
        }
    }
    ui->textEdit->append(QString("Script: found %1 image files").arg(ImagePaths.size()));

    //using imagemagick for image transforms
    QProcess process;
    QStringList proc_args;
    QImage img;
    process.setProgram("C:\\Program Files\\ImageMagick-7.0.11-Q16-HDRI\\magick");

    //next step - processing every image in the list
    int iTotalTiles = 0;
    QByteArray path = ui->lineEdit_Recipes_List->text().toLatin1();
    while (false == path.endsWith("/"))
        path.chop(1);
    for (int iImageNumber=0; iImageNumber < ImagePaths.size(); iImageNumber++)
    {
        QByteArray b = ImagePaths.at(iImageNumber);
        b.prepend(path);
        QByteArray b2 = b.left(b.indexOf("."));
        QByteArray b3 = b2;
        b2.append("_processed.png");
        b3.append(".tim");
        //check if the file was already processed and a processed copy exist
        if (QFile::exists(b2))
        {
            //exists, do nothing
        }
        else
        {
            //resize sprite for height of 448
            proc_args.clear();
            proc_args.append(b);
            proc_args.append("-resize");
            proc_args.append("8x448^");
            proc_args.append("tmp1.png");
            process.setArguments(proc_args);
            process.open();
            process.waitForFinished();

            //generate palette, VDP2 NBG palette is limited to 255 colours
            proc_args.clear();
            proc_args.append("tmp1.png");
            proc_args.append("-colors");
            proc_args.append("255");
            proc_args.append(b2);
            process.setArguments(proc_args);
            process.open();
            process.waitForFinished();
        }

        img.load(b2);

        if (img.size().width() <= 0 )
            return;

        //let's crop image to tile size
        if ( (img.size().width()%8 != 0) || (img.size().height()%8 != 0) )
            img = img.copy(0,0,(img.size().width()/8)*8,(img.size().height()/8)*8);

        //let's detect a transparent color
        int transp_color = img.pixelIndex(0,0);

        //let's calculate tile usage for image (no tile sharing yet)
        //writing tile usage into file as well
        QFile _tiled_image_file(b3);
        _tiled_image_file.open(QIODevice::WriteOnly|QIODevice::Truncate);
        int iSizeTiledX = img.size().width()/8;
        int iSizeTiledY = img.size().height()/8;
        int iActiveTiles = 0;
        for (int TileY = 0; TileY < iSizeTiledY; TileY++)
        {
            for (int TileX = 0; TileX < iSizeTiledX; TileX++)
            {
                //is the tile empty?
                bool bEmpty = true;
                for (int x=0;x<8;x++)
                {
                    for (int y=0;y<8;y++)
                    {
                        if (img.pixelIndex(TileX*8+x,TileY*8+y) != transp_color)
                                bEmpty = false;
                    }
                }
                //if the tile is non-empty, count its usage
                if (false == bEmpty)
                {
                    iActiveTiles++;
                    _tiled_image_file.write(QByteArray(1,1));
                }
                else
                {
                    _tiled_image_file.write(QByteArray(1,0));
                }
            }
        }
        //pass 2 - writing tile data
        for (int TileY = 0; TileY < iSizeTiledY; TileY++)
        {
            for (int TileX = 0; TileX < iSizeTiledX; TileX++)
            {
                //is the tile empty?
                bool bEmpty = true;
                for (int x=0;x<8;x++)
                {
                    for (int y=0;y<8;y++)
                    {
                        if (img.pixelIndex(TileX*8+x,TileY*8+y) != transp_color)
                                bEmpty = false;
                    }
                }
                //if the tile is non-empty, write it
                if (false == bEmpty)
                {
                    for (int x=0;x<8;x++)
                    {
                        for (int y=0;y<8;y++)
                        {
                            _tiled_image_file.write(QByteArray(1,(char)img.pixelIndex(TileX*8+x,TileY*8+y)));
                        }
                    }

                }
            }
        }
        //now write palette
        QColor _col;
        for (int i=0;i<256;i++)
        {
            _col.fromRgb(img.color(i));
            _tiled_image_file.write(QByteArray(1,_col.red()));
            _tiled_image_file.write(QByteArray(1,_col.green()));
            _tiled_image_file.write(QByteArray(1,_col.blue()));
        }
        ImageTilesUsage.append(iActiveTiles);
        iTotalTiles += iActiveTiles;
        _tiled_image_file.close();
    }
    ui->textEdit->append(QString("Script: detected %1 total tiles").arg(iTotalTiles));


    /*//now squashing files into packs
    QList<QList<int>> Packs;
    QList<int> PacksCellUsage;

    //first allocate a pack for each recipe
    for (int i=0; i<Script_Sprite_Recipes.size(); i++)
    {
        //QList<int> * l = new QList<int>();
        Packs.append(QList<int>());
        PacksCellUsage.append(0);
        for (int j=0; j<ImageLinks.size();j++)
        {
            if (ImageLinks[j].recipe == i)
            {
                Packs[i].append(ImageLinks[j].image);
            }
        }
    }

    //remove duplicated packs
    for (int p1=0; p1<Packs.size(); p1++)
    {
        for (int p2=p1+1; p2<Packs.size(); p2++)
        {
            //processing pair
            bool bDuplicated = true;
            if (Packs[p1].size() != Packs[p1].size())
                bDuplicated = false;
            else
            {
                for (int i=0;i<Packs[p1].size();i++)
                {
                    if (Packs[p1][i] != Packs[p2][i])
                        bDuplicated = false;
                }
            }
            if (true == bDuplicated)
            {
                Packs.removeAt(p2);
                PacksCellUsage.removeAt(p2);
                //start at the beginning
                p1 = 0; p2 = 1;
            }
        }
    }


    bool bMergeSuccessful = true;

    while (true == bMergeSuccessful)
    {
        //calculate total packs cell usage
        int iTotalUsage = 0;
        for (int i=0; i<Packs.size(); i++)
        {
            PacksCellUsage[i] = 0;
            for (int j=0; j<Packs[i].size(); j++)
            {
                iTotalUsage += ImageTilesUsage[Packs[i][j]];
                PacksCellUsage[i] += ImageTilesUsage[Packs[i][j]];
            }
        }

        //parse thru every pack pair to detect merge gain
        //TODO: squish into packs not gain-wise, but script-wise (to minimize loading time)
        int iMaxGain = 0;
        int iCurrentGain = 0;
        int iMaxPack1 = 0;
        int iMaxPack2 = 0;
        for (int p1=0; p1<Packs.size(); p1++)
        {
            for (int p2=p1+1; p2<Packs.size(); p2++)
            {
                //processing pair
                iCurrentGain = 0;
                //search if any image in the packs match, and add it to the gain
                for (int i=0; i<Packs[p1].size(); i++)
                    for (int j=0; j<Packs[p2].size(); j++)
                    {
                        if (Packs[p1][i] == Packs[p2][j])
                           iCurrentGain += ImageTilesUsage[Packs[p1][i]];
                    }
                //if a) gain is bigger than max, and b) packs won't overflow 128 KB, use the gain
                if ( (iCurrentGain >= iMaxGain) && (PacksCellUsage[p1]+PacksCellUsage[p2]-iCurrentGain <= 2048) )
                {
                    iMaxGain = iCurrentGain;
                    iMaxPack1 = p1;
                    iMaxPack2 = p2;
                }
            }
        }

        bMergeSuccessful = false; //we're being a bit of pessimistic here.

        //do we have a champion?
        if (iMaxPack1 != iMaxPack2)
        {
            bMergeSuccessful = true; //yay!
            //do an actual merge, move all unique from pack2 to pack1 first
            for (int i=0; i<Packs[iMaxPack2].size(); i++)
                if (false == Packs[iMaxPack1].contains(Packs[iMaxPack2].at(i)))
                    Packs[iMaxPack1].append(Packs[iMaxPack2].at(i));
            //now kill pack2
            Packs.removeAt(iMaxPack2); //memory leak here? aw, who cares
            PacksCellUsage.removeAt(iMaxPack2);
        }
    }
    ui->textEdit->append(QString("Script: squished into %1 packs").arg(Packs.size()));
    */

    /*
    //now that we've got pack data, update script with exact pack links instead of recipes
    QFile script_outfile("SCRIPT.TXT");
    int iCurrentRecipe;
    script_outfile.open(QIODevice::WriteOnly|QIODevice::Truncate);
    for (int i=0;i<Script_Lines.size();i++)
    {
        if (Script_Lines.at(i).simplified().startsWith("show"))
        {
            //this is a recipe, searching the exact recipe number
            iCurrentRecipe = -1;
            for (int rec =0; rec < Script_Sprite_Recipes.size(); rec++)
            {
                if (Script_Lines.at(i).simplified().contains(Script_Sprite_Recipes.at(rec)))
                    iCurrentRecipe = rec;
            }
            if (iCurrentRecipe != -1)
            {
                //valid recipe? write its contents into output file
                for (int f=0;f<ImageLinks.size();f++)
                {
                    //if we have a match. add image. search thru every pack until we find it somewhere
                    if (ImageLinks.at(f).recipe == iCurrentRecipe)
                    {
                        int pa=0;
                        bool bFound = false;
                        while (false == bFound)
                        {
                            for (int k=0;k<Packs[pa].size();k++)
                            {
                                if (Packs[pa][k] == ImageLinks.at(f).image)
                                {
                                    bFound = true;
                                }
                            }
                        }
                        script_outfile.write(QString("SPRITE PACK %1 FILE %2").arg(pa).arg(QString(ImagePaths[f])).toLatin1());
                        script_outfile.write("\r");
                    }
                }

            }
        }
        else
        {
            //write script data as commentary for now
            script_outfile.write(Script_Lines.at(i).simplified().prepend("REM "));
            script_outfile.write("\r");
        }
    }
    script_outfile.close();


    //now create all ze pack files
    for (int pa=0;pa<Packs.size();pa++)
    {
        QFile pack_outfile(QString("SPR%1.PAK").arg(pa,3));
        pack_outfile.open(QIODevice::WriteOnly|QIODevice::Truncate);
        //adding data to pack
        for (int spr=0;spr<Packs[pa].size();spr++)
        {
            QString sprite_name = ImagePaths.at(Packs[pa][spr]);
            //sprites are already pre-processed, using processed files instead of original
            sprite_name = sprite_name.mid(sprite_name.indexOf(".")).append("_processed.png");
            Process_Sprite(sprite_name);
        }

        pack_outfile.close();
    }*/

    //now packing every image file into tiled image
    //preprocessing should've compressed and paletted the image
    //we need to save this data:
    // 1) tile usage map, for full image size (46x56 for 373x448 sprites, 88x56 for full 704x448 sprites
    // 2) tiles data with unused tiles skipped
    // 3) palette, 768-byte fixed format
    for (int iImage=0;iImage<ImagePaths.size(); iImage++)
    {
        QString _tiled_filename = ImagePaths[iImage];
        _tiled_filename.chop(3);
        _tiled_filename.append("tim");

    }

    //creating simplified script file
    QFile script_outfile("SCRIPT.TXT");
    int iCurrentRecipe;
    script_outfile.open(QIODevice::WriteOnly|QIODevice::Truncate);
    for (int i=0;i<Script_Lines.size();i++)
    {
        if (Script_Lines.at(i).simplified().startsWith("show"))
        {
            //this is a recipe, searching the exact recipe number
            iCurrentRecipe = -1;
            int iPosition = 0;
            if (Script_Lines.at(i).simplified().contains("at left"))
                iPosition = -1;
            else if (Script_Lines.at(i).simplified().contains("at right"))
                iPosition = 1;
            for (int rec =0; rec < Script_Sprite_Recipes.size(); rec++)
            {
                if (Script_Lines.at(i).simplified().contains(Script_Sprite_Recipes.at(rec)))
                    iCurrentRecipe = rec;
            }
            if (iCurrentRecipe != -1)
            {
                //valid recipe? write its contents into output file, searching thru entire list
                for (int f=0;f<ImageLinks.size();f++)
                {
                    //we found a match. add image.
                    if (ImageLinks.at(f).recipe == iCurrentRecipe)
                    {
                        QString _tiled_filename = QString(ImagePaths[f]);
                        _tiled_filename.chop(3);
                        _tiled_filename.append("tim");
                        script_outfile.write(QString("SPRITE LAYER %1 POSITION %2 FILE %3").arg(iPosition).arg(ImageLinks.at(f).layer).arg(_tiled_filename).toLatin1());
                        script_outfile.write("\r");
                    }
                }

            }
        }
        else if (Script_Lines.at(i).simplified().startsWith("\""))
        {
            script_outfile.write(Script_Lines.at(i).simplified().prepend("TEXT ACTOR=0 "));
            script_outfile.write("\r");
        }
        else if (Script_Lines.at(i).simplified().startsWith("th \""))
        {
            QByteArray _tmp = Script_Lines.at(i).simplified();
            _tmp = _tmp.mid(_tmp.indexOf(' ')+1);
            script_outfile.write(_tmp.prepend("TEXT ACTOR=1 "));
            script_outfile.write("\r");
        }
        else if (Script_Lines.at(i).simplified().startsWith("me \""))
        {
            QByteArray _tmp = Script_Lines.at(i).simplified();
            _tmp = _tmp.mid(_tmp.indexOf(' ')+1);
            script_outfile.write(_tmp.prepend("TEXT ACTOR=2 "));
            script_outfile.write("\r");
        }
        else if (Script_Lines.at(i).simplified().startsWith("sl \""))
        {
            QByteArray _tmp = Script_Lines.at(i).simplified();
            _tmp = _tmp.mid(_tmp.indexOf(' ')+1);
            script_outfile.write(_tmp.prepend("TEXT ACTOR=3 "));
            script_outfile.write("\r");
        }
        else if (Script_Lines.at(i).simplified().startsWith("scene bg"))
        {
            QByteArray _tmp = Script_Lines.at(i).simplified();
            _tmp = _tmp.mid(_tmp.indexOf(' ')+1);
            _tmp = _tmp.mid(_tmp.indexOf(' ')+1);
            script_outfile.write(_tmp.prepend("BG "));
            script_outfile.write("\r");
        }

        else
        {
            //write script data as commentary for now
            script_outfile.write(Script_Lines.at(i).simplified().prepend("REM "));
            script_outfile.write("\r");
        }
    }
    script_outfile.write("END\r");
    script_outfile.close();
}

/*void MainWindow::Process_Sprite(QString filename)
{
    //preprocessing should've compressed and paletted the image
    //we need to save this data:
    // 1) tile usage map, for full image size (46x56 for 373x448 sprites, 88x56 for full 704x448 sprites
    // 2) tiles data with unused tiles skipped
    // 3) palette, 768-byte fixed format

}*/
