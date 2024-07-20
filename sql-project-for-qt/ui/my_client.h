#ifndef MY_CLIENT_H
#define MY_CLIENT_H

#include <QObject>

class MY_CLIENT : public QObject
{
    Q_OBJECT
public:
    explicit MY_CLIENT(QObject *parent = nullptr);

signals:

public slots:
};

#endif // MY_CLIENT_H
