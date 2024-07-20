#include "client.h"

void Client::run(){
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

