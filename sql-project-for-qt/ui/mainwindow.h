#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <client.h>
#include <cstdio>
#include <fstream>
#include <QTreeWidgetItem>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct response {
    int style;
    string status_message;
    string tablename;
    int attrnumber;
    QVector<string> attrname;
    QVector<string> attrtype;
    QVector<string> attrconstrain;
    QVector<string> body;
};
//暂时没添加用户功能所以没有写登入界面
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void getClient(Client* temp){
       client = temp;
    }
private slots:
    void Apply_SQL();//获取sql语句
    void Apply_SQL1();
    void moveTabs();
    void openFile();
    void Quit();
    response read_response();//读取响应
    response read_response2(string);//读取响应
    void on_pushButton_clicked();
    void updateTableInfo();
    void ShowResult();
    void ShowResult1();
    void on_pushButton_2_clicked();
    void on_pushButton_4_clicked();
    void onItemClicked(QTreeWidgetItem* item, int column);

protected:
    bool eventFilter(QObject *target, QEvent *event);//事件过滤器
private:
    Client* client;
//    QString total_message;
    bool isinit = false;
    response datatable;
    Ui::MainWindow *ui;
    int g =1;
    QVector<string> tabname;
};
#endif // MAINWINDOW_H
