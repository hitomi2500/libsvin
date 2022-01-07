#include <QFileDialog>
#include <QProcess>
#include <QMessageBox>
#include <QtEndian>
#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::on_pushButton_Select_BGs_clicked()
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
        list83.append(info.baseName());
    }
}

void MainWindow::on_pushButton_process_BGs_clicked()
{
    QProcess process;
    QStringList proc_args;
    QImage img;
    int iSectorsUsed;
    QList<QByteArray> TileLibrary;
    int written;
    QFile outfile_pack;
    QFile outfile_bg;
    QByteArray b;

    int iLimit = 127;
    if (ui->comboBox_mode->currentIndex() == 2)
        iLimit = 1; //only a single file in tapestry mode
    if ((true == ui->checkBox_PackBG->isChecked())&&(list.size() > iLimit))
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Oopsie daisy");
        msgBox.setText("Packs with more than 127 files and tapestries with more than 1 file are not supported!");
        msgBox.exec();
        return;
    }


    if (true == ui->checkBox_PackBG->isChecked())
    {
        //making big bin instead of independent small NBG/PAL
        outfile_pack.setFileName(QString("DATA.PAK"));
        outfile_pack.open(QIODevice::WriteOnly);

        //pass 1 - create filelist table
        //first 64-byte entry is a system header
        outfile_pack.write(QByteArray("SVINPACK"));
        qint16_be s = (qint16_be)list83.size();//files in pack
        outfile_pack.write((char*)&s,2);
        s = 64;//filename size
        outfile_pack.write((char*)&s,2);
        b.clear();
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

        if (true == ui->checkBox_PackBG->isChecked())
        {
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
        }

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


        if (true == ui->checkBox_PackBG->isChecked())
        {
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
        }
        else
        {
            //single file, care less about sectors and clusters
            //open file first
            QByteArray b = list83.at(iImageNumber).toLatin1();
            QByteArray b2 = b;
            if (b2.contains("safe_artist-colon-"))
                b2.replace("safe_artist-colon-","-");
            b2 = b2.left(30);
            b2.append(".bg");
            outfile_bg.setFileName(b2);
            outfile_bg.open(QIODevice::WriteOnly);
            qint16_be s = (qint16_be)iLeft;
            outfile_bg.write((char*)&s,2);
            s = (qint16_be)iTop;
            outfile_bg.write((char*)&s,2);
            s = (qint16_be)(iSizeX);
            outfile_bg.write((char*)&s,2);
            s = (qint16_be)(iSizeY);
            outfile_bg.write((char*)&s,2);
            s = (qint16_be)iCurrentSector;
            outfile_bg.write((char*)&s,2);
            outfile_bg.write(list83.at(iImageNumber).toLatin1());
            while (outfile_bg.size()<2048)
                outfile_bg.write(QByteArray(1,'\0'));
            //now save bg data
            outfile_bg.write(ba);
            //fill last cluster of bg
            while (written%2048 > 0)
            {
                written++;
                outfile_bg.write("\0",1);
            }
            //now save paletteg data
            outfile_bg.write(ba_pal); //write palette
            written = ba_pal.size();
            //fill last cluster
            while (written%2048 > 0)
            {
                written++;
                outfile_bg.write("\0",1);
            }
            outfile_bg.close();
        }

    }

    if (true == ui->checkBox_PackBG->isChecked())
    {
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

}
