#include <QFileDialog>
#include <QProcess>
#include <QMessageBox>
#include <QtEndian>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "lz.h"

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
    if (ui->comboBox_mode->currentIndex() == BG_VDP1_8BPP_TAPESTRY)
        iLimit = 1; //only a single file in tapestry mode
    if (ui->comboBox_mode->currentIndex() == BG_VDP1_8BPP_32X32_QUADS)
        iLimit = 1; //only a single file in huge scroll bg mode
    if ((true == ui->checkBox_PackBG->isChecked())&&(list.size() > iLimit))
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Oopsie daisy");
        msgBox.setText("Packs with more than 127 files and tapestries/superbg with more than 1 file are not supported!");
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
    if (QFile::exists("C:\\Program Files\\ImageMagick-7.0.11-Q16-HDRI\\magick.exe"))
        process.setProgram("C:\\Program Files\\ImageMagick-7.0.11-Q16-HDRI\\magick");
    else if (QFile::exists("C:\\Program Files\\ImageMagick-7.1.0-Q16-HDRI\\magick.exe"))
        process.setProgram("C:\\Program Files\\ImageMagick-7.1.0-Q16-HDRI\\magick");

    TileLibrary.clear();

    //pass2 -*/
    for (int iImageNumber=0; iImageNumber<list.size(); iImageNumber++)
    {

        if (ui->comboBox_mode->currentIndex() == BG_VDP1_8BPP_4QUADS)  //VDP1 backs
        {
            //resize image to at least 704x448 each axis
            /*proc_args.clear();
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
            process.waitForFinished();*/

            //no resize
            QFile::remove("tmp2.png");
            QFile::copy(list.at(iImageNumber), "tmp2.png");
            img.load("tmp2.png");
            //max quad size for highres is 508x256, so 4 quads are 1016x512 max
            if ( (img.size().width() > 1016) || (img.size().height() > 512) )
            {
                QMessageBox msgBox;
                msgBox.setWindowTitle("Oopsie daisy");
                msgBox.setText("The image is too big to fit into 4 VDP1 quads! Max size is 1016x512");
                msgBox.exec();
                return;
            }
        }
        else if (ui->comboBox_mode->currentIndex() == BG_VDP1_8BPP_TAPESTRY) //VDP1 tapestry
        {
            //no resize
        }
        else if (ui->comboBox_mode->currentIndex() == BG_VDP1_16BPP_4QUADS)  //VDP1 backs lowres
        {
            /*//resize image to at least 352x448 each axis
            proc_args.clear();
            proc_args.append(list.at(iImageNumber));
            proc_args.append("-resize");
            proc_args.append("352x448^");
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
            proc_args.append("352x448");
            proc_args.append("-resize");
            proc_args.append("352x448^");
            proc_args.append("tmp2.png");
            process.setArguments(proc_args);
            process.open();
            process.waitForFinished();*/

            //no resize
            QFile::remove("tmp2.png");
            QFile::copy(list.at(iImageNumber), "tmp2.png");
            img.load("tmp2.png");
            //max quad size is 508x256, so 4 quads are 1016x512 max
            if ( (img.size().width() > 1016) || (img.size().height() > 512) )
            {
                QMessageBox msgBox;
                msgBox.setWindowTitle("Oopsie daisy");
                msgBox.setText("The image is too big to fit into 4 VDP1 quads! Max size is 1016x512");
                msgBox.exec();
                return;
            }
        }
        else if (ui->comboBox_mode->currentIndex() == BG_VDP1_8BPP_32X32_QUADS)  //VDP1 huge x huge
        {
            //no resize
            QFile::remove("tmp2.png");
            QFile::copy(list.at(iImageNumber), "tmp2.png");
        }


        if (ui->comboBox_mode->currentIndex() == BG_VDP1_8BPP_4QUADS)
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
        else if (ui->comboBox_mode->currentIndex() == BG_VDP1_8BPP_TAPESTRY)
        {
            //tapestry
            img.load(list.at(iImageNumber));
        }
        else if (ui->comboBox_mode->currentIndex() == BG_VDP1_16BPP_4QUADS)
        {
            //keep all colors
            //backuppy
            QFile::copy("tmp2.png", QString("tmp%1b.png").arg(iImageNumber,4,10,QLatin1Char('0')));
            img.load("tmp2.png");
        }
        else if (ui->comboBox_mode->currentIndex() == BG_VDP1_8BPP_32X32_QUADS)
        {
            //generate palette, sprited VDP1 palette is limited to 254 colours
            proc_args.clear();
            proc_args.append("tmp2.png");
            proc_args.append("-colors");
            proc_args.append("254");
            proc_args.append("tmp3.png");
            process.setArguments(proc_args);
            process.open();
            process.waitForFinished(30000000);
            //backuppy
            QFile::copy("tmp2.png", QString("tmp%1b.png").arg(iImageNumber,4,10,QLatin1Char('0')));
            img.load("tmp3.png");
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

        int iQuad_Size_X = img.size().width()/2;
        int iQuad_Size_Y = img.size().height()/2;

        if (ui->comboBox_mode->currentIndex() == BG_VDP1_8BPP_4QUADS) //VDP1 4-sprites mode
        {
            ba.resize(iQuad_Size_X*iQuad_Size_Y*4);
            ba.fill('\0');
            //new background mode : 4 VDP1 interlaced sprites

            for (int x = 0; x < iQuad_Size_X; x++)
                for (int y = 0; y < iQuad_Size_Y; y++)
                {
                    ba[iQuad_Size_X*iQuad_Size_Y*0 + y*iQuad_Size_X+x] = img.pixelIndex(x,y*2);
                    ba[iQuad_Size_X*iQuad_Size_Y*1 + y*iQuad_Size_X+x] = img.pixelIndex(iQuad_Size_X+x,y*2);
                    ba[iQuad_Size_X*iQuad_Size_Y*2 + y*iQuad_Size_X+x] = img.pixelIndex(x,y*2+1);
                    ba[iQuad_Size_X*iQuad_Size_Y*3 + y*iQuad_Size_X+x] = img.pixelIndex(iQuad_Size_X+x,y*2+1);
                }
        }
        else if (ui->comboBox_mode->currentIndex() == BG_VDP1_8BPP_TAPESTRY) //tapestry
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
                for (int x = 0; x < iQuad_Size_X; x++)
                    {
                        ba_tmp[iQuad_Size_X*0 + x] = img.pixelIndex(x,iLine*2);
                        ba_tmp[iQuad_Size_X*1 + x] = img.pixelIndex(iQuad_Size_X+x,iLine*2);
                        ba_tmp[iQuad_Size_X*2 + x] = img.pixelIndex(x,iLine*2+1);
                        ba_tmp[iQuad_Size_X*3 + x] = img.pixelIndex(iQuad_Size_X+x,iLine*2+1);
                    }
                ba.append(ba_tmp,2048);
            }
        }
        else if (ui->comboBox_mode->currentIndex() == BG_VDP1_16BPP_4QUADS) //VDP1 4-sprites mode, lowres 16bpp
        {
            ba.resize(iQuad_Size_X*iQuad_Size_Y*4);
            ba.fill('\0');
            //new background mode : 4 VDP1 interlaced sprites
            QColor c;
            for (int x = 0; x < iQuad_Size_X; x++)
                for (int y = 0; y < iQuad_Size_Y; y++)
                {
                    c = QColor::fromRgb(img.pixel(x,y*2));
                    ba[iQuad_Size_X*iQuad_Size_Y*2*0 + y*iQuad_Size_X*2+x*2+1] = ((c.red()>>3)&0x1F) | ((c.green()<<2)&0xE0);
                    ba[iQuad_Size_X*iQuad_Size_Y*2*0 + y*iQuad_Size_X*2+x*2] = 0x80 | ((c.blue()>>1)&0x7C) | ((c.green()>>6)&0x3);
                    c = QColor::fromRgb(img.pixel(x,y*2+1));
                    ba[iQuad_Size_X*iQuad_Size_Y*2*2 + y*iQuad_Size_X*2+x*2+1] = ((c.red()>>3)&0x1F) | ((c.green()<<2)&0xE0);
                    ba[iQuad_Size_X*iQuad_Size_Y*2*2 + y*iQuad_Size_X*2+x*2] = 0x80 | ((c.blue()>>1)&0x7C) | ((c.green()>>6)&0x3);
                    c = QColor::fromRgb(img.pixel(x+iQuad_Size_X,y*2));
                    ba[iQuad_Size_X*iQuad_Size_Y*2*1 + y*iQuad_Size_X*2+x*2+1] = ((c.red()>>3)&0x1F) | ((c.green()<<2)&0xE0);
                    ba[iQuad_Size_X*iQuad_Size_Y*2*1 + y*iQuad_Size_X*2+x*2] = 0x80 | ((c.blue()>>1)&0x7C) | ((c.green()>>6)&0x3);
                    c = QColor::fromRgb(img.pixel(x+iQuad_Size_X,y*2+1));
                    ba[iQuad_Size_X*iQuad_Size_Y*2*3 + y*iQuad_Size_X*2+x*2+1] = ((c.red()>>3)&0x1F) | ((c.green()<<2)&0xE0);
                    ba[iQuad_Size_X*iQuad_Size_Y*2*3 + y*iQuad_Size_X*2+x*2] = 0x80 | ((c.blue()>>1)&0x7C) | ((c.green()>>6)&0x3);
                }
        }
        else if (ui->comboBox_mode->currentIndex() == BG_VDP1_8BPP_32X32_QUADS) //VDP1 huge x huge mode
        {
            int x_cells = img.size().width()/64;
            int y_cells = img.size().height()/32;
            ba.resize(x_cells*y_cells*2048);
            ba.fill('\0');
            //new background mode : 4 VDP1 interlaced sprites
            for (int y_cell = 0; y_cell<y_cells; y_cell++)
                for (int x_cell = 0; x_cell<x_cells; x_cell++)
                    for (int x = 0; x < 64; x++)
                        for (int y = 0; y < 32; y++)
                        {
                            if (ui->checkBox_transposed_mega_bg->isChecked())
                                ba[(x_cell*y_cells + y_cell)*2048 + y*64+x] = img.pixelIndex(x_cell*64+x,y_cell*32+y);
                            else
                                ba[(y_cell*x_cells + x_cell)*2048 + y*64+x] = img.pixelIndex(x_cell*64+x,y_cell*32+y);
                        }
        }


        ba_pal.clear();
        if (ui->comboBox_mode->currentIndex() != BG_VDP1_16BPP_4QUADS)
        {
            for (int j=0;j<img.colorTable().size();j++)
            {
                ba_pal.append(QColor(img.colorTable().at(j)).red());
                ba_pal.append(QColor(img.colorTable().at(j)).green());
                ba_pal.append(QColor(img.colorTable().at(j)).blue());
            }
        }
        while (ba_pal.size()<256*3)
            ba_pal.append('\0');

        //doing palette hacks
        if (ui->comboBox_mode->currentIndex() == BG_VDP1_8BPP_4QUADS)
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
        else if (ui->comboBox_mode->currentIndex() == BG_VDP1_8BPP_TAPESTRY) //tapestry
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
        else if (ui->comboBox_mode->currentIndex() == BG_VDP1_16BPP_4QUADS)
        {
            //for 16 bpp do nothing
        }
        else if (ui->comboBox_mode->currentIndex() == BG_VDP1_8BPP_32X32_QUADS)
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
            //compressing prior to saving
            QByteArray ba_compressed;
            if (true == ui->checkBox_bg_rle->isChecked())
            {
                ba_compressed.append("LZ77");

                uint8_t * _in = (uint8_t *)malloc(ba.size());
                uint8_t * _out = (uint8_t *)malloc(ba.size());
                for (int z=0;z<ba.size();z++)
                    _in[z] = ba[z];
                int cmpsize = LZ_Compress(_in,_out,ba.size());
                unsigned char * p8 = (unsigned char *)&cmpsize;
                ba_compressed.append(p8[3]);
                ba_compressed.append(p8[2]);
                ba_compressed.append(p8[1]);
                ba_compressed.append(p8[0]);
                for (int z=0;z<cmpsize;z++)
                    ba_compressed.append(_out[z]);
                free(_in);
                free(_out);

                while (ba_compressed.size()%2048 !=0)
                    ba_compressed.append(1,(char)(0));
                outfile_bg.write(ba_compressed);//saving uncompressed
            }
            else
                outfile_bg.write(ba);//saving uncompressed
            //fill last cluster of bg
            while (written%2048 > 0)
            {
                written++;
                outfile_bg.write("\0",1);
            }
            //now save palette data
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
