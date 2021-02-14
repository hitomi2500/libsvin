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
    list83pal.clear();
    for (int i=0; i<list.size(); i++)
    {
        QFileInfo info(list.at(i));
        QString str = info.fileName();
        QString str2,str3;
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
        //list83.append(str3);
        list83.append(info.baseName());//.append(".nbg"));
        str3 = str2;
        str3.append(".PAL");
        //list83pal.append(str3);
        list83pal.append(info.baseName().append(".pal"));
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    QProcess process;
    QStringList proc_args;
    QImage img;

    if (list.size() > 128)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Oopsie daisy");
        msgBox.setText("Packs with more than 128 files are not supported!");
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
    for (int i=0; i<list83.size(); i++)
    {
        QByteArray _name;
        _name.append("1234567890");//placeholder for : LEFT, TOP, SIZE_X, SIZE_Y, sector
        _name.append(list83.at(i).toLatin1());
        while (_name.size()<64)
            _name.append('\0');
        outfile_pack.write(_name);
    }
    //up to 127 files per pack, that's 8192 bytes = 4 sectors
    //filling rest
    for (int i=list83.size(); i<127; i++)
    {
        QByteArray _name;
        while (_name.size()<64)
            _name.append('\0');
        outfile_pack.write(_name);
    }

    int iCurrentSector = (list.size())/32 + 1;

    //using imagemagick for image transforms
    process.setProgram("C:\\Program Files\\ImageMagick-7.0.11-Q16-HDRI\\magick");

    //pass2 -*/
    for (int iImageNumber=0; iImageNumber<list.size(); iImageNumber++)
    {

        if (ui->comboBox_mode->currentIndex() == 0)
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
        else
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
            QFile::copy("tmp1.png", "tmp2.png");
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
        }
        else
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
        }

        //backuppy
        QFile::copy("tmp2.png", QString("tmp%1b.png").arg(iImageNumber,4,10,QLatin1Char('0')));

        img.load("tmp3.png");

        QByteArray ba;

        int iLeft=0;
        int iTop=0;
        int iSizeX=img.width();
        int iSizeY=img.height();

        //next stage - cropping out transparent data around the image
        //not cropping anymore, tiling instead

        //cropping done, calculating sectors usage
        int iSectorsUsed = (iSizeX*iSizeY - 1 + 256*3)/2048 + 1; //palette is counted as well

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

        if (ui->comboBox_mode->currentIndex() == 0)
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
        else
        {
            //first let's expand image to tile size
           while (img.size().width()%8 != 0)
                img.transformed()
            //let's do some heavy tiling.
            //let's detect a transparent color
            int transp_color = img.pixelIndex(0,0);
            //now we calculate a usage map. since no sprite will be bigger than full screen, maximum usage map is 88x56 for 8x8 tiles.
            int iUsageMap[88][56];
            for (int TileY = 0; TileY < 56; TileY++)
            {
                for (int TileX = 0; TileX < 88; TileX++)
                {
                    bool bEmpty = true;
                    for (int x=0;x<8;x++)
                    {
                        for (int y=0;y<8;y++)
                        {
                            if (img.pixelIndex(TileX*8+x,TileY*8+y) != transp_color)
                                bEmpty = false;
                        }
                    }
                    if (true == bEmpty)
                        iUsageMap[TileX][TileY] = 0;
                    else
                        iUsageMap[TileX][TileY] = 1;

                }
            }

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
                    ba.append(c);
                }
            }

            //now save every tile within usage map
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
            }

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

        QByteArray ba_pal;
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
        else
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

        outfile_pack.write(ba);

        //fill last cluster
        while (outfile_pack.size()%2048 > 0)
            outfile_pack.write("\0",1);

        iCurrentSector += iSectorsUsed;//adding used sectors

        outfile_pack.write(ba_pal);
        //PAL data is 768, rounding
        while (outfile_pack.size()%2048 > 0)
            outfile_pack.write("\0",1);
    }

    outfile_pack.close();

}
