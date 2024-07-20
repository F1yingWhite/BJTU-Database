#include "neclient.h"
#include "ui_neclient.h"
#include "client.h"
#include <QMessageBox>
#include <QTimer>

neclient::neclient(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::neclient)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    port = 1001;
    //    client = new Client("192.168.43.226", port);
   // client = new Client("192.168.31.44",port);
    ui->textBrowser->setText("请输入服务器的ipv4 地址");
}
void neclient::onTimerTimeout()
{
    // 在定时器到期后连接到主机
    if(num == 0 ){
    ui->textBrowser->setText("conneting...");
    num += 1 ;//so your timer will be on all the time?
    }
    else if(num == 1){
        QString a;
        a=ui->textEdit->toPlainText();
        //qDebug()<<a;
        a.replace("\n","");
        client = new Client(a.toStdString(), port);
        connect(client,SIGNAL(succ_connected()), this, SLOT(onConnected())); // 连接成功信号槽
        connect(client, SIGNAL(dis_connected()), this, SLOT(onDisconnected())); // 断开连接信号槽
//        connect(client, SIGNAL(fail_to_connected()), this, SLOT(onError(string error_message))); // 错误信号槽
        client->start();
        //onConnected();
        num++;
        timer->deleteLater();
        timer->stop();//
        num=0;
    }

}
//测试的服务端连接
//void neclient::onNewConnection()
//{
//    m_socket = m_server->nextPendingConnection();

//    qDebug() << "Client connected:" << m_socket->peerAddress().toString();
//}
neclient::~neclient()
{
    delete ui;
}

bool neclient::connectToHost(const QString& host, quint16 port)
{
//    m_socket->connectToHost(host, port); // 连接服务器
//    return m_socket->waitForConnected(); // 等待连接成功
}

void neclient::disconnectFromHost()
{
//    m_socket->disconnectFromHost(); // 断开连接
}

bool neclient::isConnected() const
{
//    return m_socket->state() == QTcpSocket::ConnectedState; // 判断是否已连接
}

//bool neclient::sendData(const QString& data)
//{
//    QByteArray byteArray = data.toUtf8(); // 将QString转为QByteArray
//    qint64 bytesWritten = m_socket->write(byteArray); // 发送数据
//    return bytesWritten == byteArray.size(); // 返回发送是否成功
//}



void neclient::onConnected()
{
    ui->textBrowser->setText("连接成功");// 发送连接成功信号
    a.show();
    a.getClient(client);
    this->hide();
}

void neclient::onDisconnected()
{
    ui->textBrowser->setText("连接失败,请再次输入或者输入exit退出"); // 输出错误信息
    //this->hide();
}

void neclient::onError(string error_message)
{
    ui->textBrowser->setText("连接失败"); // 输出错误信息
    //this->hide();

}



void neclient::on_pushButton_clicked()
{
    QString a;
    a=ui->textEdit->toPlainText();

    if(a==""){
        ui->textBrowser->setText("请输入正确的ipv4");
    }else if(a=="exit"){
        this->close();
        qApp->quit();
    }
    else{
        ui->textBrowser->setText("正在启动客户端.....");
        // 创建一个 QTimer 实例
        timer = new QTimer(this);
        // 关联 QTimer 到定时器到期的槽函数
        connect(timer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));
        // 启动定时器，设置为 1 秒后到期
        timer->start(1000);
    }

}
