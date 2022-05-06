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
    ui->comboBox_mode->addItem("704x448 8bpp 4-sprite VDP1 background");
    ui->comboBox_mode->addItem("704x448 8bpp 8x8 cell VDP2 sprite");
    ui->comboBox_mode->addItem("704xHUGE 8bpp single file VDP1 tapestry");
    ui->comboBox_mode->addItem("352x240 16bpp 4-sprite VDP1 background");
    ui->comboBox_mode->addItem("HUGExHUGE 8bpp 64x64 VDP1 tiles");
}

MainWindow::~MainWindow()
{
    delete ui;
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

