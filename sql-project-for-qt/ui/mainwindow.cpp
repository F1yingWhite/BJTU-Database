#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFile>
#include <QFileDialog>
#include <QTableWidget>
#include <thread>
#include <QTextStream>
#include <QStringList>
#include <iostream>
#include <strstream>
#include <string>
#include <QString>
#include <vector>
#include <ctime>
#include <QTextEdit>
#include <QKeyEvent>
#include <stdexcept>
#include <QTimer>

#define MAXLENGTHOFTEXT 65536

using namespace std;

vector<string> Execute();
void AddSeperator(char *command);
short int IsComEnd(char *input);
//写个定时器没有数据返回时或格式不正确的时候抛出异常
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    setWindowTitle("sqlplus");
    /*
    QAction *quitAction = new QAction("quit", this);
    quitAction->setShortcut(QKeySequence::Open);
    ui->menu_1->addAction(quitAction);
    connect(quitAction, &QAction::triggered, this, &MainWindow::Quit);*/
    //回车的处理
    ui->setupUi(this);
    //    QPalette pl = ui->textEdit->palette();
    //    pl.setColor(QPalette::Text, Qt::green);    // 设置文本颜色为绿色
    //    ui->textEdit->setPalette(pl);              // 设置回调色板
    QFont font = QFont("Comic Sans MS",11,7);
    font.setBold(true);//粗体
    font.setLetterSpacing(QFont::PercentageSpacing,100);//间距
    ui->textEdit->setFont(font);
    ui->textBrowser->setFont(font);
    ui->textEdit->setFocusPolicy(Qt::StrongFocus);
    ui->textEdit->insertPlainText("#");
    ui->textEdit->setFocus();
    ui->textEdit->installEventFilter(this);//设置完后自动调用其eventFilter函数
    connect(ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(onItemClicked(QTreeWidgetItem*, int)));

    //connect(ui->tableWidget->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(test(QModelIndex,QModelIndex,QVector<int>)));
    //updateTableInfo();
}

MainWindow::~MainWindow()
{
    delete ui;
    client->socket_.close();
}
void MainWindow::onItemClicked(QTreeWidgetItem* item, int column)
{
    if (item->parent() == nullptr) // 仅当它是顶级项目时才执行
        {
            QString text = item->text(column);
            bool tabExists = false;
            for(int i = 0; i < ui->tabWidget->count(); i++) {
                //qDebug()<<ui->tabWidget->tabText(i);
                if(ui->tabWidget->tabText(i) == " "+text) {
                    tabExists = true;
                    break;
                }
            }
            if(tabExists== false){
                ui->textEdit->insertPlainText("\n------------------------");
                 ui->textEdit->insertPlainText("\n#");
                ui->textEdit->insertPlainText("select * from "+text+";");
                Apply_SQL();
                ui->textEdit->insertPlainText("\n------------------------");
                ui->textEdit->insertPlainText("\n#");
            }

        }
}

void MainWindow ::Apply_SQL()
{
    QString str2 = "";//报错提示文本框
    QTextCharFormat redFormat;
    redFormat.setForeground(Qt::red);//设置红色字体格式
    QTextCharFormat whiteFormat;
    whiteFormat.setForeground(Qt::white);//设置黑色字体格式
    ui->textBrowser->clear();

    QString messages = ui->textEdit->toPlainText();
    int epos = messages.lastIndexOf("#");
    messages = messages.right(messages.size()-epos -1);
    messages.replace("\n","");//把空格去了
    //qDebug()<<messages;

    //计时开始
    clock_t start, end;
    start = clock();

    client->send_message(messages.toStdString());//这是一个发送
    //这是一个返回值的获取
    datatable = read_response();

    //计时结束
    end = clock();

    //接受到数据的处理方法
    if(datatable.style == 0){
        str2 = messages;
        str2 += "语句花费的时间" + QString::number(end - start) + "ms.\n";//花费的时间计算
        QTextCursor cursor = ui->textBrowser->textCursor();
        cursor.insertText(str2,whiteFormat);
        cursor.movePosition(QTextCursor::End);
        QString a =QString::fromStdString(datatable.status_message);
        cursor.insertText(a+"\n",redFormat);
        if(messages.contains("create")||messages.contains("alter")||messages.contains("drop")||messages.contains("use")){
            this->updateTableInfo();
        }
    }
    else if(datatable.style == 1){
        str2 = messages;
        str2 += "语句花费的时间" + QString::number(end - start) + "ms.\n";//花费的时间计算
        QTextCursor cursor = ui->textBrowser->textCursor();
        cursor.insertText(str2,whiteFormat);
        cursor.movePosition(QTextCursor::End);
        QString a =QString::fromStdString(datatable.status_message);
        cursor.insertText(a+"\n",whiteFormat);
        if(messages.contains("create")||messages.contains("alter")||messages.contains("drop")||messages.contains("use")){
            //qDebug()<<"hhhh";
            this->updateTableInfo();
        }
    }
    else if(datatable.style == 3){
        str2 = messages;
        str2 += "语句花费的时间" + QString::number(end - start) + "ms.\n";//花费的时间计算
        QTextCursor cursor = ui->textBrowser->textCursor();
        cursor.insertText(str2,whiteFormat);
        cursor.movePosition(QTextCursor::End);
        QString a =QString::fromStdString(datatable.status_message);
        cursor.insertText(a+"\n",whiteFormat);
        ShowResult1();
        if(messages.contains("desc")){
            tabname.append(messages.toStdString());
        }
        if(messages.contains("create")||messages.contains("alter")||messages.contains("drop")||messages.contains("use")){
            this->updateTableInfo();
        }
    }
    else if(datatable.style == 2){
        str2 = messages;
        str2 += "语句花费的时间" + QString::number(end - start) + "ms.\n";//花费的时间计算
        QTextCursor cursor = ui->textBrowser->textCursor();
        cursor.insertText(str2,whiteFormat);
        cursor.movePosition(QTextCursor::End);
        QString a =QString::fromStdString(datatable.status_message);
        cursor.insertText(a+"\n",whiteFormat);
        ShowResult();
        if(messages.contains("select")){
            tabname.append(messages.toStdString());
        }
    }else if(datatable.style == 4){
        str2 = messages;
        str2 += "语句花费的时间" + QString::number(end - start) + "ms.\n";//花费的时间计算
        QTextCursor cursor = ui->textBrowser->textCursor();
        cursor.insertText(str2,whiteFormat);
        cursor.movePosition(QTextCursor::End);
        QString a = "";
        for(int i = 0;i<datatable.body.count();i++){
            a = a+"table name:"+QString::fromStdString(datatable.body.at(i))+"\n";
        }
        cursor.insertText(a+"\n",whiteFormat);

    }




}
void MainWindow ::Apply_SQL1()
{
    QString str2 = "";//报错提示文本框
    QTextCharFormat redFormat;
    redFormat.setForeground(Qt::red);//设置红色字体格式
    QTextCharFormat whiteFormat;
    whiteFormat.setForeground(Qt::white);//设置黑色字体格式
    QString messages = ui->textEdit->toPlainText();
    int epos = messages.lastIndexOf("#");
    messages = messages.right(messages.size()-epos -1);
    messages.replace("\n","");//把空格去了

    //计时开始
    clock_t start, end;
    start = clock();

    client->send_message(messages.toStdString());//这是一个发送
    //这是一个返回值的获取
    datatable = read_response();

    //计时结束
    end = clock();

    //接受到数据的处理方法
    if(datatable.style == 0){
        str2 = messages;
        str2 += "语句花费的时间" + QString::number(end - start) + "ms.\n";//花费的时间计算
        QTextCursor cursor = ui->textBrowser->textCursor();
        cursor.insertText(str2,whiteFormat);
        cursor.movePosition(QTextCursor::End);
        QString a =QString::fromStdString(datatable.status_message);
        cursor.insertText(a+"\n",redFormat);
    }
    else if(datatable.style == 1||datatable.style == 4){
        if(messages.contains("create")||messages.contains("alter")||messages.contains("drop")||messages.contains("use")){
            this->updateTableInfo();
        }
    }
    else if(datatable.style == 3){
        ShowResult1();
        if(messages.contains("desc")){
            tabname.append(messages.toStdString());
        }
        if(messages.contains("create")||messages.contains("alter")||messages.contains("drop")||messages.contains("use")){
            this->updateTableInfo();
        }
    }
    else if(datatable.style == 2){
        ShowResult();
        if(messages.contains("select")){
            tabname.append(messages.toStdString());
        }
    }

}


//接受xyh的方法直接写文件欸黑
//读取响应//投机取巧了
response MainWindow::read_response2(string x) noexcept(false){
    response res;
    try{
        qDebug()<<"使用文件了哦";
        boost::system::error_code error;
        boost::array<char, 5000> buf;
        std::ofstream file("read.txt");
        if(file.is_open()){
            QString a = QString::fromStdString(x);
            a.replace("\\r","\r");
            a.replace("\\n","\n");
            string change = a.toStdString();
            file<<change;
            size_t len=0;
            len = client->socket_.read_some(boost::asio::buffer(buf), error);
            while(len ==5000){
                std::string message(buf.data(),len);
                QString a = QString::fromStdString(message);
                a.replace("\\r","\r");
                a.replace("\\n","\n");
                //qDebug()<<a;
                string change = a.toStdString();
                file<<change;
                len = client->socket_.read_some(boost::asio::buffer(buf), error);
            }
            std::string message(buf.data(),len);
            a = QString::fromStdString(message);
            a.replace("\\r","\r");
            a.replace("\\n","\n");
            //qDebug()<<a;
            change = a.toStdString();
            file<<change;

            file.close();
        }
        char buf1[5000];
        std::ifstream file1("read.txt");
        string line;
        int num =0;
        int pp = 0;
        while(file1.getline(buf1,5000)){
            line += buf1;
            if(line.back()=='\r'){
                line.erase(line.length()-1);
                if(num==0){
                    res.style = stoi(line);
                    line.clear();
                    num++;
                    if(res.style==4){
                        num=7;
                    }
                }
                else if(num==1){
                    if (res.style==0||res.style == 1||res.style == 2 || res.style == 3){
                        res.status_message=line;
                        line.clear();
                        num++;
                    }//
                }
                else if(num ==2){
                    if (res.style == 2 || res.style == 3)
                    {
                        //
                        res.tablename=line;
                        line.clear();
                        num++;
                    }
                }
                else if(num ==3){
                    if (res.style == 2 || res.style == 3)
                    {
                        //
                        res.attrnumber = stoi(line);
                        line.clear();
                        num++;
                    }
                }else if(num ==4){
                    if (res.style == 2 || res.style == 3)
                    {
                        pp++;
                        if(pp==res.attrnumber){
                            num++;
                            pp=0;
                        }
                        res.attrname.append(line);
                        line.clear();
                    }
                }else if(num ==5){
                    if (res.style == 2 || res.style == 3)
                    {
                        pp++;
                        if(pp==res.attrnumber){
                            num++;
                            pp=0;
                        }
                        res.attrtype.append(line);
                        line.clear();
                    }
                }else if(num == 6){
                    if (res.style == 2 || res.style == 3)
                    {
                        pp++;
                        if(pp==res.attrnumber){
                            num++;
                            pp=0;
                        }
                        res.attrconstrain.append(line);
                        line.clear();
                    }
                }else if(num==7){
                    res.body.append(line);
                    line.clear();
                }

            }

        }
        file1.close();
        remove("read.txt");
        return res;

    }catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

}
response MainWindow::read_response() noexcept(false){
    response res;
    try{
        //boost::asio::streambuf buf;//换成了流
        boost::system::error_code error;
        boost::array<char, 5000> buf;
        //(3)通过read_some函数来读数据,异步好难写啊
        //size_t len = client->socket_.read_some(boost::asio::buffer(buf.prepare(10000000)), error);//整个老大的内存
        size_t len = client->socket_.read_some(boost::asio::buffer(buf), error);
        //buf.commit(len);
        //boost::asio::read_until(client->socket_, buf, "\r\n");
        //std::istream input1(&buf);
        std::string message1(buf.data(),len);
        //qDebug()<<len;
        if(len==5000){
            res=read_response2(message1);
            return res;
        }

        //std::getline(input1, message1);

        QString a = QString::fromStdString(message1);
        a.replace("\\r","\r");
        a.replace("\\n","\n");
        qDebug()<<a;
        string change = a.toStdString();
        //QString messages = ui->textEdit->toPlainText();
        //ui->textEdit->setText(messages.append(QString::fromStdString("\n")).append(QString::fromStdString(message1)).append(QString::fromStdString("\ndb8>")));
        if (error == boost::asio::error::eof) {
            throw std::runtime_error("Connection closed by peer.");
        }
        else if (error) {
            throw boost::system::system_error(error);    //some other error
        }
        // 解析数据
        std::stringstream input(change);
        string style,attrnumber;
        string attrnames, attrtypes, constraints, data;
        //
        std::getline(input, style,'\r');
        input.ignore(1); // 忽略换行符
        res.style = stoi(style);
        //qDebug()<<res.style;
        //qDebug()<<QString::fromStdString(style);
        //
        if (res.style==0||res.style == 1||res.style == 2 || res.style == 3){
            std::getline(input, res.status_message,'\r');
            input.ignore(1); // 忽略换行符
            //qDebug()<<QString::fromStdString(res.status_message);
        }
        //
        if(res.style == 4){
            int num = 0;
            while( num==0 ){
                std::getline(input, data, '\r');
                if (input.eof() || input.fail()) {
                    break;
                    cout<<"finish";// 到达了输入流的结尾或者发生了读取错误
                }
                input.ignore(1);
                res.body.append(data);
                //qDebug()<<QString::fromStdString(data);

            }
        }
        //
        if (res.style == 2 || res.style == 3)
        {
            //
            std::getline(input, res.tablename, '\r');
            input.ignore(1);
            //qDebug()<<QString::fromStdString(res.tablename);
            //
            std::getline(input, attrnumber, '\r');
            input.ignore(1);
            res.attrnumber = stoi(attrnumber);
            //qDebug()<<QString::fromStdString(attrnumber);
            //
            int num =0;
            while(num < res.attrnumber){
                std::getline(input, attrnames, '\r');
                input.ignore(1);
                res.attrname.append(attrnames);
                //qDebug()<<QString::fromStdString(attrnames);
                num++;
            }

            //
            num = 0;
            while(num < res.attrnumber){
                std::getline(input, attrtypes, '\r');
                input.ignore(1);
                res.attrtype.append(attrtypes);
                //qDebug()<<QString::fromStdString(attrtypes);
                num++;
            }
            //
            num = 0;
            while(num < res.attrnumber){
                std::getline(input, constraints, '\r');
                input.ignore(1);
                res.attrconstrain.append(constraints);
                //qDebug()<<QString::fromStdString(constraints);
                num++;
            };

            //
            if( res.style == 2 ){
                num = 0;
                while( num==0 ){
                    std::getline(input, data, '\r');
                    if (input.eof() || input.fail()) {
                        break;
                        cout<<"finish";// 到达了输入流的结尾或者发生了读取错误
                    }
                    res.body.append(data);  // 输出结果
                    //qDebug()<<QString::fromStdString(data); // 输出结果
                    input.ignore(1);

                }
            }

        }

        return res;
    }catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

}
/*//这里写个read util版本的
 * response MainWindow::read_response() noexcept(false){
        response res;
           try{
               boost::asio::streambuf buf;//换成了流
               boost::system::error_code error;


               return res;
        }catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }

}
 *
 */

//打开文件//解决
void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("open file"), "",  tr("SQL File(*.sql*);;All files(*)"));
    if(fileName=="") return;
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QTextStream in(&file);
    QString b ="";
    //加了个提示按钮

    QMessageBox *m_box = new QMessageBox(QMessageBox::Information,QString("通知"),QString("sql文件正在读取中请等待"));
    QTimer::singleShot(1000,m_box,SLOT(accept()));
    m_box->exec();
    while (!in.atEnd()) {
        QString line = in.readLine();
        if(line.contains(";")){  // 如果读取到了 ";" 字符串，
            QStringList a = line.split(";");
            for(int i=0;i<a.count();i++){
                if(i==0){
                    ui->textEdit->insertPlainText(b);
                    b="";
                    ui->textEdit->insertPlainText(a.at(i));
                    //qDebug()<<b+a.at(i);
                    ui->textEdit->insertPlainText(";");
                    Apply_SQL1();
                    ui->textEdit->insertPlainText("\n------------------------");
                    ui->textEdit->insertPlainText("\n#");
                }
                else if(i==a.count()-1){
                    b=a.at(i);
                }else{
                    ui->textEdit->insertPlainText(a.at(i));
                    //qDebug()<<a.at(i);
                    ui->textEdit->insertPlainText(";");
                    Apply_SQL1();
                    ui->textEdit->insertPlainText("\n------------------------");
                    ui->textEdit->insertPlainText("\n#");

                }
            }
        }else{
            b =b+line;
        }

    }
    QMessageBox *m_box1 = new QMessageBox(QMessageBox::Information,QString("通知"),QString("文件读取成功"));
    QTimer::singleShot(1000,m_box1,SLOT(accept()));
    m_box1->exec();
    file.close();


}
//退出
void MainWindow::Quit()
{
    exit(0);
}


void MainWindow::on_pushButton_clicked()
{

    ui->textEdit->clear();
    ui->textEdit->insertPlainText("#");

}
//建树状图，丑后面优化下
void MainWindow::updateTableInfo()
{
    ui->treeWidget->clear();
    //这里要单独有一个处理的方法
    client->send_message("show table;");//这是一个发送
    //这是一个返回值的获取
    datatable = read_response();
    int num = datatable.body.size();//有几张表

    QVector<string> name=datatable.body;//目录的名
    static const int indexlong =0;//索引数量 注意在这里写的时候,因为是同时分配变量值的,况且变量并不是一个定值,
    //数组的长度(涉及到内存的分配)不可以使用变量来分配

    //需要获得目录信息随便瞎写的后面改一下

    for(int i=0;i< num/*目录长度*/;i++){
        QTreeWidgetItem *Table = new QTreeWidgetItem(QStringList()<<QString::fromStdString(name[i]));//aaaa是目录的名字
        ui->treeWidget->addTopLevelItem(Table);//加入了第一层目录
        //再对每个表进行数据的处理
        client->send_message("desc "+name[i]+";");//这是一个发送
        //这是一个返回值的获取
        datatable = read_response();
        for(int j =0;j<datatable.attrnumber/*属性长度*/;j++){
            string attr =datatable.attrname[j];

            attr +="\t"+datatable.attrtype[j];
            QTreeWidgetItem *Attribute = new QTreeWidgetItem(QStringList()<<QString::fromStdString(attr));//约束条件


            QTreeWidgetItem *attr_pk = new QTreeWidgetItem(QStringList()<<QString::fromStdString(datatable.attrconstrain[j]));
            Attribute->addChild(attr_pk);

            Table->addChild(Attribute);

        }


    }

}
//建表
void MainWindow::ShowResult1(){
    //获得数据
    int attrlong=datatable.attrnumber;//表格属性的个数这个不可以为0，小bug

    string name=datatable.tablename ;
    //
    QVector<string> attrname= datatable.attrname;
    //
    QVector<string> attrtype = datatable.attrtype;
    QVector<string> constrain = datatable.attrconstrain;
    //不管给啥样的格式，我都按string写了
    //假设按元组获得一组属性
    //QVector<string> data = datatable.body;

    //新建了一个页面
    QTableWidget *newTable = new QTableWidget();
    ui->tabWidget->addTab(newTable, tr(name.c_str()));
    ui->tabWidget->setCurrentWidget(newTable);
    //新建了一个按钮用来关闭页面
    QPushButton* button = new QPushButton("*");
    button->setProperty("tabIndex", g); // 将该按钮与第2个tab关联
    g++;
    //qDebug()<<g;
    QTabBar* tab_bar = ui->tabWidget->tabBar();
    tab_bar->setTabButton(tab_bar->count()-1, QTabBar::RightSide, button);

    // 连接按钮的点击信号到响应函数
    connect(button, &QPushButton::clicked, this, &MainWindow::moveTabs);

    newTable->setColumnCount(3);//hhhhhh
    QStringList header;
    header.append(QString::fromStdString("attrname"));
    header.append(QString::fromStdString("type"));
    header.append(QString::fromStdString("constrain"));
    newTable->setHorizontalHeaderLabels(header);
    newTable->show();

    //给表格里添加内容，不知道获得啥格式
    newTable->setRowCount(attrlong);
    for(int i=0;i<attrlong;i++){
        for(int j=0;j<3;j++){
            //qDebug()<<QString::fromStdString(data.at(i*attrlong+j));
            //QTableWidgetItem *item = new QTableWidgetItem(QString::fromStdString(data.at(i*attrlong+j))
            if(j==0) newTable->setItem(i, j, new QTableWidgetItem(QString::fromStdString(attrname.at(i))));
            if(j==1) newTable->setItem(i, j, new QTableWidgetItem(QString::fromStdString(attrtype.at(i))));
            if(j==2) newTable->setItem(i, j, new QTableWidgetItem(QString::fromStdString(constrain.at(i))));
            //qDebug()<<QString::fromStdString(constrain.at(i));
        }
    }



}
//建表
void MainWindow::ShowResult(){
    //获得数据
    int attrlong=datatable.attrnumber;//表格属性的个数这个不可以为0，小bug

    string name=datatable.tablename ;
    //
    QVector<string> attrname= datatable.attrname;
    //
    //QVector<string> attrtype = datatable.attrtype;
    //
    //不管给啥样的格式，我都按string写了
    //假设按元组获得一组属性
    QVector<string> data = datatable.body;
    if(data.size() == 0){
        //新建了一个页面
        QTableWidget *newTable = new QTableWidget();
        ui->tabWidget->addTab(newTable, tr(name.c_str()));
        ui->tabWidget->setCurrentWidget(newTable);
        //新建了一个按钮用来关闭页面
        QPushButton* button = new QPushButton("*");
        button->setProperty("tabIndex", g); // 将该按钮与第2个tab关联
        g++;
        qDebug()<<g;
        QTabBar* tab_bar = ui->tabWidget->tabBar();
        tab_bar->setTabButton(tab_bar->count()-1, QTabBar::RightSide, button);

        // 连接按钮的点击信号到响应函数
        connect(button, &QPushButton::clicked, this, &MainWindow::moveTabs);

        newTable->setColumnCount(attrlong);
        //qDebug()<<attrlong;
        QStringList header;
        header.clear();
        for(int i=0;i<attrlong;i++){
            //qDebug()<<i;
            header.append(QString::fromStdString(attrname.at(i)));
        }
        newTable->setHorizontalHeaderLabels(header);
        newTable->show();
        return;

    }

    //新建了一个页面
    QTableWidget *newTable = new QTableWidget();
    ui->tabWidget->addTab(newTable, tr(name.c_str()));
    ui->tabWidget->setCurrentWidget(newTable);
    //新建了一个按钮用来关闭页面
    QPushButton* button = new QPushButton("*");
    button->setProperty("tabIndex", g); // 将该按钮与第2个tab关联
    g++;
    qDebug()<<g;
    QTabBar* tab_bar = ui->tabWidget->tabBar();
    tab_bar->setTabButton(tab_bar->count()-1, QTabBar::RightSide, button);

    // 连接按钮的点击信号到响应函数
    connect(button, &QPushButton::clicked, this, &MainWindow::moveTabs);

    newTable->setColumnCount(attrlong);
    QStringList header;
    for(int i=0;i<attrlong;i++){
        header.append(QString::fromStdString(attrname[i]));
    }
    newTable->setHorizontalHeaderLabels(header);
    newTable->show();

    //给表格里添加内容，不知道获得啥格式
    newTable->setRowCount(data.size());
    //newTable->setColumnCount(attrlong);
    for(int i=0;i<data.size();i++){
        size_t pos = 0;  // 当前查找到的空格的位置
        size_t o_pos = 0;
        int num =0;
        QVector<string> result;   // 截取到的子串
        result.clear();
        //qDebug()<<QString::fromStdString(data.at(i));
        while(((pos = data.at(i).find(' ', pos))!= string::npos) && num<attrlong) {
            if (pos > 0) {
                //qDebug()<<pos;
                result.append(data.at(i).substr(o_pos, pos-o_pos));  // 截取空格前面的子串
                //qDebug()<<QString::fromStdString(data.at(i).substr(o_pos, pos-o_pos)); // 输出结果
                o_pos = pos++;
                num++;
            }
            ++pos;  // 继续查找下一个空格
            //qDebug()<<pos;
        };
        if(num < attrlong){
            while(num < attrlong){
                result.append("null");
                num++;
            }

        }
        for(int j=0;j<attrlong;j++){
            //qDebug()<<QString::fromStdString(data.at(i*attrlong+j));
            //QTableWidgetItem *item = new QTableWidgetItem(QString::fromStdString(data.at(i*attrlong+j))
            newTable->setItem(i, j, new QTableWidgetItem(QString::fromStdString(result.at(j))));
        }
    }



}

void MainWindow::moveTabs() {//删除表tab的操作但我不知道如何找到按钮所在的index，我只能找到当前页面的然后就容易把sql界面删了
    //int current_index = ui->tabWidget->currentIndex();
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button) {
        int index = button->property("tabIndex").toInt();
        int a =ui->tabWidget->count();
        //qDebug()<<index;
        g--;
        // 执行与该按钮索引相关的操
        ui->tabWidget->removeTab(index);
        tabname.remove(index-1);
        if(index!=a-1){
            for(int i = 1; i < ui->tabWidget->count(); i++) {
                qDebug()<<"hhhh";
                QTabBar* tab_bar = ui->tabWidget->tabBar();
                QWidget* button = tab_bar->tabButton(i, QTabBar::RightSide);
                button->setProperty("tabIndex", i);
            }

        }
    }

}

bool MainWindow::eventFilter(QObject *target, QEvent *event)
{

    if(target == ui->textEdit)
    {
        if(event->type() == QEvent::KeyPress)//回车键
        {
            QString messages = ui->textEdit->toPlainText();
            int epos = messages.lastIndexOf("#");//太危险了这个符号
            messages = messages.right(messages.size()-epos -1);
            //qDebug()<<"transforming data: "<<messages;
            messages.replace("\n"," ");
            //qDebug()<<messages;
            QKeyEvent *k = static_cast<QKeyEvent *>(event);
            if(k->key() == Qt::Key_Return)
            {   //qDebug()<< messages.right(1);
                if( messages.right(1) == ";"){
                    Apply_SQL();
                    ui->textEdit->insertPlainText("\n------------------------");
                    ui->textEdit->insertPlainText("\n#");
                }else{
                    ui->textEdit->insertPlainText("\n  ");//我打算处理的时候给这玩意换了
                }
                return true;
            }
        }
    }
    return QWidget::eventFilter(target,event);
}

void MainWindow::on_pushButton_2_clicked()//sql语句导入
{
    openFile();
}


void MainWindow::on_pushButton_4_clicked()
{
    int k = g;
    g-1;
    int current_index = ui->tabWidget->currentIndex();
    while(g!=0){
        ui->tabWidget->removeTab(g);
        g--;
    }
    g++;
    for(int i = 0;i<k-1;i++){
        client->send_message(tabname.at(i));//这是一个发送
        //这是一个返回值的获取
        datatable = read_response();
        if(datatable.style==2){
            ShowResult();
        }
        if(datatable.style == 3){
            ShowResult1();
        }
    }
    ui->tabWidget->setCurrentIndex(current_index);
    //qDebug()<<current_index;
}

