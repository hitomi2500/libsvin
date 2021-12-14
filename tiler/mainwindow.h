#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct Image_Link {
    int recipe;
    int image;
    int big_palette;
    int shifted_palette;
};

struct Menu_Choise {
    int menu_id;
    int choise_id;
    QByteArray value;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_Select_BGs_clicked();

    void on_pushButton_process_BGs_clicked();

    void on_pushButton_Set_reciles_List_clicked();

    void on_pushButton_Set_Script_clicked();

    void on_pushButton_Process_Sprites_clicked();

private:
    //void Process_Sprite(QString filename);
    Ui::MainWindow *ui;
    QStringList list;
    QStringList list83;
    //QStringList list83pal;
};
#endif // MAINWINDOW_H
