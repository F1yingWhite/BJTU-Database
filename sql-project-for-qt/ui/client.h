#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QDebug>
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include<string>
using namespace std;
using boost::asio::ip::tcp;

#include <QObject>

namespace Ui {
class Client;
}

class Client : public QObject
{
    Q_OBJECT

public:
    Client(const std::string& server_address = "127.0.0.1", int server_port = 1000,QObject *parent = nullptr)
        : io_service_(), socket_(io_service_), server_address_(server_address), server_port_(server_port)
    {
        try {
            //(2)通过connect函数连接服务器，打开socket连接。
            tcp::endpoint end_point(boost::asio::ip::address::from_string(server_address_), server_port_);
            socket_.connect(end_point);
            isconnected = true;
        }
        catch (std::exception& e) {
            a=e.what();
        }
    }
    void start(){
        if(isconnected == true){

            emit succ_connected();
        }else{
            emit dis_connected();
            emit fail_to_connected(a);
        }
    }

    void send_message(std::string message){
        boost::system::error_code ec;
        socket_.write_some( boost::asio::buffer(message), ec);
        if (ec) {
            std::cout << boost::system::system_error(ec).what() << std::endl;
        }
    }

    std::string listen(){
        try {
                boost::array<char, 128> buf;
                boost::system::error_code error;

                //(3)通过read_some函数来读数据
                size_t len = socket_.read_some(boost::asio::buffer(buf), error);

                if (error == boost::asio::error::eof) {
//                    break;    //connection closed cleadly by peer
                    return "null";
                }
                else if (error) {
                    throw boost::system::system_error(error);    //some other error
                }
                std::cout.write(buf.data(), len);
                std::string message(buf.data(),len);
                return message;
        }
        catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
            return "error";
        }

    }



    void run();

signals:
    void succ_connected();
    void fail_to_connected(string error_message);
    void dis_connected();

public slots:


public:
    boost::asio::io_service io_service_;
    tcp::socket socket_;
    std::string server_address_;
    int server_port_;
    bool isconnected = false;
    string a;
};

//Client client("172.30.75.218", 1001);
////         client.run();
//client.send_message("qqqq");
//client.listen();
//client.send_message("qqqq");
//client.listen();
//client.send_message("qqqq");
//client.listen();

#endif // CLIENT_H
