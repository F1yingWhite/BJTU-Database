#ifndef NECLIENT_H
#define NECLIENT_H

#include <QObject>
#include "client.h"
#include "logging.h"
namespace Ui {
class neclient;
}

class neclient : public QWidget
{
    Q_OBJECT

public:
    explicit neclient(QWidget *parent = nullptr);
    ~neclient();

    bool connectToHost(const QString& host, quint16 port);
    void disconnectFromHost();
    bool isConnected() const;
    bool sendData(const QString& data);
    QString receiveData();
    void onNewConnection();

signals:
//    void connected(); // 连接成功信号
//    void disconnected(); // 断开连接信号
//    void dataReceived(const QString& data); // 数据接收信号


private slots:
    void onConnected();
    void onDisconnected();
    void onError(string error_message);
    void onTimerTimeout();

    void on_pushButton_clicked();

private:
    int num=0;
    Ui::neclient *ui;
    int port;
    Client* client;
    logging a;
    QTimer *timer;

};

#endif // NECLIENT_H
