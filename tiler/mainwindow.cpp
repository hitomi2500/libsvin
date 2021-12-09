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

