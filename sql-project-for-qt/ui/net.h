#ifndef NET_H
#define NET_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QApplication>

class net: public QObject
{
     Q_OBJECT
public:
    net();
    // 从指定URL获取字符串形式的响应数据，并返回一个QString对象。
   QString fetchData(const QString &urlStr);

};

#endif // NET_H
