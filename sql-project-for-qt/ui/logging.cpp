#include "logging.h"
#include "ui_logging.h"
#include <QDebug>
#include<string>
#include <QMessageBox>
#include <QScreen>
logging::logging(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::logging)
{
    setWindowFlags(Qt::FramelessWindowHint);
    ui->setupUi(this);
}

logging::~logging()
{
    delete ui;
}
bool logging::login(const QString &username, const QString &password)
{
    tempclient->send_message(username.toStdString()+"_"+password.toStdString());
    if (tempclient->listen()=="1"){
        return true;
    } else {
        return false;
    }
}
void logging::on_pushButton_clicked()
{
    QString username = ui->textEdit->toPlainText();
    QString password = ui->textEdit_1->text();
    if (login(username, password)) {
        //移动窗体到屏幕中央
        QRect rect = a.frameGeometry();
        //QDesktopWidget desktop;
        QScreen* screen = QGuiApplication::primaryScreen();
        QPoint centerPoint = screen->availableGeometry().center();
        rect.moveCenter(centerPoint);
        a.move(rect.topLeft());
        a.show();
        a.getClient(tempclient);
        this->hide();
    } else {
        ui->textEdit->clear();
        ui->textEdit_1->clear();
        QMessageBox::warning(this,"警告","请输入正确的用户名,还有"+QString::number(num)+"次机会");
        num--;
    }
    if(num==-1){
        this->close();
        qApp->quit();

    }

}


void logging::on_pushButton_2_clicked()
{
    this->close();
    tempclient->socket_.close();
    qApp->quit();

}




