#ifndef LOGGING_H
#define LOGGING_H

#include <QWidget>
#include <mainwindow.h>
#include "client.h"
namespace Ui {
class logging;
}

class logging : public QWidget
{
    Q_OBJECT

public:
    explicit logging(QWidget *parent = nullptr);
    ~logging();
     bool login(const QString &username, const QString &password);
     void getClient(Client* temp){
        tempclient = temp;
     }

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();


private:
    Ui::logging *ui;
    MainWindow a;
    Client* tempclient;
    int num =2;
};

#endif // LOGGING_H
