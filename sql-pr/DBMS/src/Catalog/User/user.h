//
// Created by BlackCat on 2023/3/31.
//

#ifndef DBMS_USER_H
#define DBMS_USER_H
#include <string>
using namespace std;
#include <iostream>
#include <fstream>

// 先放着吧,以后在加,先把数据库功能完成
enum user_type
{
    ADMIN,
    USER
};

enum permission
{
    create, // 创建删除权限
    Access, // 访问权限
    all     // 都具有
};

class user
{
public:
    string username;
    string password;
    string net_address;
    user_type type;
    permission p;

    user(string username, string password, string net_address, user_type type, permission p)
    {
        this->username = username;
        this->password = password;
        this->net_address = net_address;
        this->type = type;
        this->p = p;
    }
};

#endif // DBMS_USER_H
