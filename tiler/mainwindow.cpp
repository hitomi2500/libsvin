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
    ui->comboBox_mode->addItem("HUGExHUGE 8bpp 64x64 quads VDP1 megabackground");
    ui->radioButton_x_auto->setChecked(true);
    ui->radioButton_y_auto->setChecked(true);
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

