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
    ui->comboBox_mode->addItem("8bpp 4-quad VDP1 background");
    ui->comboBox_mode->addItem("8bpp VDP1 tapestry");
    ui->comboBox_mode->addItem("16bpp 4-quad VDP1 background");
    ui->comboBox_mode->addItem("HUGExHUGE 8bpp 32x32 quads VDP1 megabackground");
    ui->comboBox_mode->addItem("8bpp 8x8 quads VDP2 tiles");
    ui->comboBox_mode->addItem("8bpp 1-quad VDP1 background");
    ui->radioButton_x_auto->setChecked(true);
    ui->radioButton_y_auto->setChecked(true);
    ui->comboBox_sprite_bpp->addItem("256 colors (8bpp)");
    ui->comboBox_sprite_bpp->addItem("32768 colors (15bpp)");
    ui->comboBox_sprite_bpp->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

