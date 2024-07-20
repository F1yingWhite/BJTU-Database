#include "mainwindow.h"
#include "neclient.h"
#include <QApplication>
#include <QTextStream>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile f(":/new/prefix1/style.qss");
        if (!f.exists())
        {
            printf("Unable to set stylesheet, file not found\n");
        }
        else
        {
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&f);
            qApp->setStyleSheet(ts.readAll());
        }
      neclient b;
      b.show();
      //MainWindow c;
      //c.show();
     return a.exec();
}
