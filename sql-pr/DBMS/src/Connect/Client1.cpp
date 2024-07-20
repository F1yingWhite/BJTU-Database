// #pragma once

#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include<string>

using namespace std;
using boost::asio::ip::tcp;

class Client {
public:
    Client(const std::string& server_address, int server_port)
        : io_service_(), socket_(io_service_), server_address_(server_address), server_port_(server_port)
    {
        try {
            //(2)通过connect函数连接服务器，打开socket连接。
            tcp::endpoint end_point(boost::asio::ip::address::from_string(server_address_), server_port_);
            socket_.connect(end_point);
        }
        catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }
    
    void send_message(std::string message){
        boost::system::error_code ec;
        socket_.write_some( boost::asio::buffer(message), ec);
        if (ec) {
            std::cout << boost::system::system_error(ec).what() << std::endl;
        }
    }

    void run()
    {
        try {
            for (;;) {
                boost::array<char, 128> buf;
                boost::system::error_code error;

                //(3)通过read_some函数来读数据
                size_t len = socket_.read_some(boost::asio::buffer(buf), error);

                if (error == boost::asio::error::eof) {
                    break;    //connection closed cleadly by peer
                }
                else if (error) {
                    throw boost::system::system_error(error);    //some other error
                }

                std::cout.write(buf.data(), len);
            }
        }
        catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

private:
    boost::asio::io_service io_service_;
    tcp::socket socket_;
    std::string server_address_;
    int server_port_;
};

int main(int argc, char* argv[])
{
    Client client("127.0.0.1", 1001);
    // client.run();
    client.send_message("xyh come in!!");
    return 0;
}
