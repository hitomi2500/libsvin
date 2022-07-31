#include <QFileDialog>
#include <QProcess>
#include <QMessageBox>
#include <QtEndian>
#include <QImage>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "lz.h"


void MainWindow::on_pushButton_openSprite_clicked()
{
    ui->label_sprite_filename->setText(QFileDialog::getOpenFileName(this,
        tr("Open Sprite"), "", tr("Recipe Files (*.*)")));

    //trying to load sprite
    ui->label_master_sprite->setPixmap(QPixmap(ui->label_sprite_filename->text()));
}


void MainWindow::on_pushButton_process_Sprite_clicked()
{
    QString sprite_filename = ui->label_sprite_filename->text();
    sprite_filename = sprite_filename.left(sprite_filename.lastIndexOf(".")).append(".spr");
    //saving sprite in VDP1 mode
    if (false == QFile::exists(ui->label_sprite_filename->text()))
        return;

    QProcess process;
    QStringList proc_args;

    //using imagemagick for image transforms
    if (QFile::exists("C:\\Program Files\\ImageMagick-7.0.11-Q16-HDRI\\magick.exe"))
        process.setProgram("C:\\Program Files\\ImageMagick-7.0.11-Q16-HDRI\\magick");
    else if (QFile::exists("C:\\Program Files\\ImageMagick-7.1.0-Q16-HDRI\\magick.exe"))
        process.setProgram("C:\\Program Files\\ImageMagick-7.1.0-Q16-HDRI\\magick");

    switch (ui->comboBox_sprite_bpp->currentIndex())
    {
    case 0:
        proc_args.clear();
        proc_args.append(ui->label_sprite_filename->text());
        proc_args.append("-colors");
        proc_args.append("255");
        proc_args.append("tmp_spr.png");
        process.setArguments(proc_args);
        process.open();
        process.waitForFinished(30000000);
        break;
    case 1:
        QFile::remove("tmp_spr.png");
        QFile::copy(ui->label_sprite_filename->text(),"tmp_spr.png");
        break;
    }
    QImage img;
    ui->label_processed_sprite->setPixmap(QPixmap("tmp_spr.png"));
    img.load("tmp_spr.png");
    //store header
    //reducing header to 16 bytes, not storing internal name
    //header format : x size (2 bytes), y size (2 bytes), reserved (12 bytes)
    QFile spr_file (sprite_filename);
    spr_file.open(QIODevice::WriteOnly);
    qint16_be s = (qint16_be)img.width();
    spr_file.write((char*)&s,2);
    s = (qint16_be)img.height();
    spr_file.write((char*)&s,2);
    if (img.depth()<=8)
        s = (qint16_be)8;
    else
        s = (qint16_be)16;
    spr_file.write((char*)&s,2);
    while (spr_file.size()<16)
        spr_file.write(QByteArray(1,'\0'));
    //now write data
    QByteArray ba;
    switch (ui->comboBox_sprite_bpp->currentIndex())
    {
    case 0:
        if (img.depth()<=8)
        {
            for (int y=0;y<img.height();y++)
                for (int x=0;x<img.width();x++)
                    ba.append(QByteArray(1,img.pixelIndex(x,y)));
        }
        break;
    case 1:
        if (img.depth()>8)
        {
            QColor clr;
            uint16_t color16;
            uint8_t * pcolor = (uint8_t*)&color16;
            for (int y=0;y<img.height();y++)
                for (int x=0;x<img.width();x++)
                {
                    clr = img.pixelColor(x,y).toRgb();
                    color16 = (0x8000 | (((clr.red()+4)>>3)%0x1F) | ((((clr.green()+4)>>3)%0x1F)<<5) | ((((clr.blue()+4)>>3)%0x1F)<<10) );
                    //we need to replace magenta with transparency
                    if  ( ( (abs(clr.red() - 255) < 4) && (abs(clr.blue() - 255) < 4) && (abs(clr.green()) < 4) ) ||
                          ( (clr.alpha() ) < 4) )
                                color16 = 0;
                    ba.append(QByteArray(1,pcolor[1]));
                    ba.append(QByteArray(1,pcolor[0]));
                    //s = (qint16_be)color16;
                    //spr_file.write((char*)&s,2);
                }
        }
        break;
    }

    //prepare palette
    QByteArray ba_pal;
    switch (ui->comboBox_sprite_bpp->currentIndex())
    {
    case 0:
        for (int j=0;j<img.colorTable().size();j++)
        {
            ba_pal.append(QColor(img.colorTable().at(j)).red());
            ba_pal.append(QColor(img.colorTable().at(j)).green());
            ba_pal.append(QColor(img.colorTable().at(j)).blue());
        }
        while (ba_pal.size()<256*3)
            ba_pal.append('\0');
        break;
    case 1:
        break;
    }

    //save data
    spr_file.write(ba);
    switch (ui->comboBox_sprite_bpp->currentIndex())
    {
    case 0:
        //round to sector's end
        while (spr_file.size()%2048 != 0)
            spr_file.write(QByteArray(1,'\0'));
        //now save palette
        spr_file.write(ba_pal);
        break;
    case 1:
        break;
    }


    spr_file.close();

}
