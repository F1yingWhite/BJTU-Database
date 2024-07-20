#include "net.h"

net::net()
{

}
QString net:: fetchData(const QString &urlStr)
{
    QUrl url(urlStr);
    QNetworkRequest request(url);

    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(request);

    while(!reply->isFinished())
    {
        qApp->processEvents();
    }

    QByteArray response = reply->readAll();
    QString responseData = QString::fromUtf8(response);

    reply->deleteLater();

    return responseData;
}
