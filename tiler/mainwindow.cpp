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
    ui->comboBox_mode->addItem("16x16 cell full screen VDP2 background");
    ui->comboBox_mode->addItem("16x16 cell partial VDP2 sprite");
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

    /*if (list.size() > 128)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Oopsie daisy");
        msgBox.setText("Packs with more than 128 files are not supported!");
        msgBox.exec();
        return;
    }*/

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

    //if we're not in BG mode, we need to calculate the superpalette - a single pallete for all the sprites.

    /*if (false == ui->checkBox_background->isChecked())
    {
        //rename all images according to ffmpeg reqirement
        for (int i=0;i<list.size();i++)
        {
            QFile::copy(list.at(i), QString("tmp%1.png").arg(i,4,10,QLatin1Char('0')));
        }

        //merge all images into one video
        proc_args.clear();
        proc_args.append("-y");
        proc_args.append("-i");
        proc_args.append("tmp%04d.png");
        proc_args.append("super.avi");
        process.setProgram("C:\\Saturn\\ES\\ffmpeg");
        process.setArguments(proc_args);
        process.open();
        process.waitForFinished();

        //remove temp frames
        for (int i=0;i<list.size();i++)
        {
            QFile::remove(QString("tmp%1.png").arg(i,4,10,QLatin1Char('0')));
        }

        //avi to gif
        proc_args.clear();
        proc_args.append("-y");
        proc_args.append("-i");
        proc_args.append("super.avi");
        proc_args.append("super.apng");
        process.setProgram("C:\\Saturn\\ES\\ffmpeg");
        process.setArguments(proc_args);
        process.open();
        process.waitForFinished();

        //generate superpalette
        proc_args.clear();
        proc_args.append("-y");
        proc_args.append("-i");
        proc_args.append("super.apng");
        proc_args.append("-vf");
        proc_args.append("palettegen=max_colors=256");
        proc_args.append("superpal.png");
        process.setArguments(proc_args);
        process.open();
        process.waitForFinished();

        //remove avi file
        QFile::remove("super.avi");

    }*/

    int iCurrentSector = (list.size())/32 + 1;

    //pass2 -*/
    for (int iImageNumber=0; iImageNumber<list.size(); iImageNumber++)
    {
        //downscale image to at least 704x448
        proc_args.clear();
        /*proc_args.append("-y");
        proc_args.append("-i");
        proc_args.append(list.at(iImageNumber));
        proc_args.append("-vf");
        proc_args.append("scale=-1:448");
        proc_args.append("tmp1.png");
        process.setProgram("C:\\Saturn\\ES\\ffmpeg");*/
        proc_args.append(list.at(iImageNumber));
        proc_args.append("-resize");
        proc_args.append("704x448^");
        proc_args.append("tmp1.png");
        process.setProgram("C:\\Program Files\\ImageMagick-7.0.11-Q16-HDRI\\magick");
        process.setArguments(proc_args);
        process.open();
        process.waitForFinished();

        //checking if image is wider than 704
        /*QImage img("tmp1.png");
        if (img.size().width() > 704)
        {
            //cut image to 704x448
            proc_args.clear();
            proc_args.append("-y");
            proc_args.append("-i");
            proc_args.append("tmp1.png");
            proc_args.append("-vf");
            proc_args.append("crop=704:448");
            proc_args.append("tmp2.png");
        }
        else
        {
            //no crop required, plain copy
            QFile f("tmp1.png");
            f.open(QIODevice::ReadOnly);
            QByteArray b2 = f.readAll();
            f.close();
            f.setFileName("tmp2.png");
            f.open(QIODevice::WriteOnly);
            f.write(b2);
            f.close();
        }*/

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

        if (ui->comboBox_mode->currentIndex() == 0)
        {
            //generate palette, sprited palette is limited to 254 colours
            /*proc_args.clear();
            proc_args.append("-y");
            proc_args.append("-i");
            proc_args.append("tmp2.png");
            proc_args.append("-vf");
            proc_args.append("palettegen=max_colors=255:stats_mode=full");
            proc_args.append("tmppal.png");
            process.setArguments(proc_args);
            process.open();
            process.waitForFinished();*/
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
            //generate palette, NBG palette is limited to 255 colours
            proc_args.clear();
            proc_args.append("-y");
            proc_args.append("-i");
            proc_args.append("tmp2.png");
            proc_args.append("-vf");
            proc_args.append("palettegen=max_colors=256:reserve_transparent=1:transparency_color=black:stats_mode=full");
            proc_args.append("tmppal.png");
            process.setArguments(proc_args);
            process.open();
            process.waitForFinished();

            //use superpalette
            //QFile::copy("superpal.png","tmppal.png");
        }


        //squish image by palette
        /*proc_args.clear();
        proc_args.append("-y");
        proc_args.append("-i");
        proc_args.append("tmp2.png");
        proc_args.append("-i");
        proc_args.append("tmppal.png");
        proc_args.append("-lavfi");
        proc_args.append("paletteuse");
        proc_args.append("tmp3.png");
        process.setArguments(proc_args);
        process.open();
        process.waitForFinished();*/

        //backuppy
        QFile::copy("tmp2.png", QString("tmp%1b.png").arg(iImageNumber,4,10,QLatin1Char('0')));

        img.load("tmp3.png");

        QByteArray ba;

        int iLeft=0;
        int iTop=0;
        int iSizeX=img.width();
        int iSizeY=img.height();

        //next stage - cropping out transparent data around the image
        if (ui->comboBox_mode->currentIndex() == 0)
        {
            //no crop in background mode
        }
        else if (ui->comboBox_mode->currentIndex() == 1)
        {
            //no crop in background mode
        }
        else
        {
            //cropping, detecting a transparent color first
            //most non-bg images will have a transparent color at top left angle
            //so we're doing the stupid thing - using this color as transparent
            QRgb transp_color = img.pixel(0,0);
            //crop top
            bool bCrop = true;
            while (bCrop)
            {
                for (int i=0;i<img.width();i++)
                {
                    if (img.pixel(i,iTop) != transp_color)
                        bCrop = false;
                }
                if (bCrop)
                {
                    iTop++;
                    iSizeY--;
                }
            }
            //crop bot
            bCrop = true;
            while (bCrop)
            {
                for (int i=0;i<img.width();i++)
                {
                    if (img.pixel(i,iTop+iSizeY-1) != transp_color)
                        bCrop = false;
                }
                if (bCrop) iSizeY--;
            }
            //round Y size to cell size (16)
            while (iSizeY % 16 != 0)
            {
                iSizeY++;
                if (iTop+iSizeY > 448)
                    iTop--;
            }
            //crop left
            bCrop = true;
            while (bCrop)
            {
                for (int i=0;i<img.height();i++)
                {
                    if (img.pixel(iLeft,i) != transp_color)
                        bCrop = false;
                }
                if (bCrop)
                {
                    iLeft++;
                    iSizeX--;
                }
            }
            //crop right
            bCrop = true;
            while (bCrop)
            {
                for (int i=0;i<img.height();i++)
                {
                    if (img.pixel(iLeft+iSizeX-1,i) != transp_color)
                        bCrop = false;
                }
                if (bCrop) iSizeX--;
            }
            //round X size to cell size (16)
            while (iSizeX % 16 != 0)
            {
                iSizeX++;
                if (iLeft + iSizeX > 704)
                    iLeft--;
            }
        }

        //sizes are in tiles now
        //iSizeX /= 16;
        //iSizeY /= 16;

        QImage img_c = img.copy(iLeft,iTop,iSizeX,iSizeY);
        img_c.save(QString("tmp%1f.png").arg(iImageNumber,4,10,QLatin1Char('0')));

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
        else if (ui->comboBox_mode->currentIndex() == 1)
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

        }
        else
        {
            ba.resize(iSizeX*iSizeY);
            ba.fill('\0');
            //mess with tiles
            for (int iTileY = 0; iTileY < (iSizeY/16); iTileY++)
                for (int iTileX = 0; iTileX < (iSizeX/16); iTileX++)
                    for (int cell=0;cell<4;cell++)
                        for (int y=0;y<8;y++)
                            for (int x=0;x<8;x++)
                                ba[(iTileY*(iSizeX/16)+iTileX)*16*16+cell*64+y*8+x] = img_c.pixelIndex(iTileX*16+(cell%2)*8+x,iTileY*16+(cell/2)*8+y);

            //replace color 0x01 with color 0x00 because shit
            /*for (int i=0;i<ba.size();i++)
                if (ba.at(i) == 1)
                    ba[i]=0;*/
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
    /*QPixmap pix;
    pix.load(ui->lineEdit->text());
    QPixmap pix2 = pix.scaledToHeight(448);
    QPixmap pix3 = pix2.copy((pix2.width()-704)/2,0,704,448);
    ui->label->setPixmap(pix3);*/
    //now processing time. first transform to 8-bit and optimize palette

}
