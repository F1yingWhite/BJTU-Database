#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include "../DB8/DB8.h"
#include "../Catalog/User/UserManager.h"
#include "../Catalog/User/user.h"
using boost::asio::ip::tcp;



class Server {
public:
    Server(int port) : acceptor_(io_service_, tcp::endpoint(tcp::v4(), port)) {
        start_accept();
    
    }

    Server() : acceptor_(io_service_, tcp::endpoint(tcp::v4(), 1000)) {
        start_accept();
     
    }

    void run() {
        io_service_.run();
    }
    std::vector<std::string> stringSplit(const std::string &str, char delim)
    {
        std::size_t previous = 0;
        std::size_t current = str.find(delim);
        std::vector<std::string> elems;
        while (current != std::string::npos)
        {
            if (current > previous)
            {
                elems.push_back(str.substr(previous, current - previous));
            }
            previous = current + 1;
            current = str.find(delim, previous);
        }
        if (previous != str.size())
        {
            elems.push_back(str.substr(previous));
        }
        return elems;
    }

    bool login(tcp::socket* socket){
        for(int i =0 ;i<3;i++) {
            char data[128];
            boost::system::error_code error;
            size_t length = socket->read_some(boost::asio::buffer(data), error);
            if (error == boost::asio::error::eof) {
                std::cout << "Connection closed by client" << std::endl;
                return false;
            } else if (error) {
                // throw boost::system::system_error(error);
                return false;
            }
            std::string message(data, length);
            std::cout << "clinet info: " << message << std::endl;

            vector<string> elems = stringSplit(message, '_');
            UserManager *um = new UserManager();
            if (um->check_user(elems[0], elems[1])) {
                std::string message = "1";
                socket->write_some(boost::asio::buffer(message));
            } else {
                std::string message = "0";
                socket->write_some(boost::asio::buffer(message));
            }
            return true;
        }
        return false;
    }

    void start_accept(){
         tcp::socket*  socket = new tcp::socket(io_service_);
        
        //异步接受客户端连接，当有客户端连接时，调用回调函数   
        //语法糖：[this, socket]，表示在回调函数中可以使用 this 和 socket
        
        acceptor_.async_accept(*socket,[this, socket,db=db](boost::system::error_code ec) {
            if (!ec) {
                std::cout << "New client connected: " << socket->remote_endpoint().address() << std::endl;
                if(login(socket)){
                    std::thread t([this, socket,db=db]() {
                    try {
                        while (true) {
                            char data[128];
                            boost::system::error_code error;
                            size_t length = socket->read_some(boost::asio::buffer(data), error);
                            if (error == boost::asio::error::eof) {
                                std::cout << "Connection closed by client" << std::endl;
                                break;
                            } else if (error) {
                                throw boost::system::system_error(error);
                            }
                            std::cout << "Received data from client: " << std::string(data, length) << std::endl;

                            //执行业务逻辑
                            std::string instructions(data, length);
                            std::string result = db->start(instructions);
                            socket->write_some(boost::asio::buffer(result));

                        }
                    } catch (std::exception& e) {
                        std::cout << "Exception in thread: " << e.what() << std::endl;
                    }
                    });
                    t.detach();
                }
                else{
                    //登录失败，关闭socket
//                    return;
                    socket->close();
                }             
            }
            start_accept();
        });
} ;

private:
    DB8* db = new DB8();
    boost::asio::io_service io_service_;
    tcp::acceptor acceptor_;
};



