//
// Created by lenovo on 2023/4/28.
//

#ifndef SQL_PR_MYEXCEPTION_H
#define SQL_PR_MYEXCEPTION_H
#include <exception>
#include <string>

class MyException : public std::exception
{
public:
    MyException(const std::string &message) : message_(message) {}
    virtual const char *what() const noexcept override { return message_.c_str(); }

private:
    std::string message_;
};

#endif // SQL_PR_MYEXCEPTION_H
