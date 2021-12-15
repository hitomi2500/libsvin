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


//every menu is defined by:
//menu start line in unprocessed script
//menu end line in unprocessed script
//menu start line in processed script
//menu end line in processed script
//list of choises (ids)
struct Menu {
    QList<int> choise_id_list;
    int start_line;
    int end_line;
    int start_line_processed_en;
    int start_line_processed_ru;
    int end_line_processed_en;
    int end_line_processed_ru;
};

//every choise is defined by:
//parent menu id
//text
//line to jump to in unprocessed script
//line to jump from to the end of menu in unprocessed script
//line to jump to in processed script
//line to jump from to the end of menu in processed script
struct Menu_Choise {
    int parent_menu_id;
    QByteArray value;
    int start_line;
    int end_line;
    int start_line_processed_en;
    int start_line_processed_ru;
    int end_line_processed_en;
    int end_line_processed_ru;
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
