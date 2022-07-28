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
    QString sprite_filename = ui->label_sprite_filename->text().append(".spr");
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

    proc_args.clear();
    proc_args.append(ui->label_sprite_filename->text());
    proc_args.append("-colors");
    proc_args.append("255");
    proc_args.append("tmp_spr.png");
    process.setArguments(proc_args);
    process.open();
    process.waitForFinished(30000000);
    ui->label_processed_sprite->setPixmap(QPixmap("tmp_spr.png"));
    QImage img;
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
    while (spr_file.size()<16)
        spr_file.write(QByteArray(1,'\0'));
    QByteArray ba;
    //now write data
    for (int y=0;y<img.height();y++)
        for (int x=0;x<img.width();x++)
            ba.append(QByteArray(1,img.pixelIndex(x,y)));

    //prepare palette
    QByteArray ba_pal;
    for (int j=0;j<img.colorTable().size();j++)
    {
        ba_pal.append(QColor(img.colorTable().at(j)).red());
        ba_pal.append(QColor(img.colorTable().at(j)).green());
        ba_pal.append(QColor(img.colorTable().at(j)).blue());
    }
    while (ba_pal.size()<256*3)
        ba_pal.append('\0');
    //for backgrounds 0x00 (transparent) and 0xFE(normal shadow) can't be used
    //imagemagick generates palettes from 0x00 to 0xFD, 0xFE being transparent
    //so we only have to move color 0x00 to color 0xFF
    /*for (int i=0;i<ba.size();i++)
        if (ba.at(i) == 0)
            ba[i]=-1;*/
    //hacking palette, moving color 0x00 to color 0xFF
    /*ba_pal[255*3] = ba_pal.at(0*3);
    ba_pal[255*3+1] = ba_pal.at(0*3+1);
    ba_pal[255*3+2] = ba_pal.at(0*3+2);)*/

    //save data
    spr_file.write(ba);
    //round to sector's end
    while (spr_file.size()%2048 != 0)
        spr_file.write(QByteArray(1,'\0'));
    //now save palette
    spr_file.write(ba_pal);

    spr_file.close();

}
