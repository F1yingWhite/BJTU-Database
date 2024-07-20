#ifndef DBMS_USERMANAGER_H
#define DBMS_USERMANAGER_H
#include <string>
using namespace std;
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "user.h"
using namespace std;

class UserManager
{
public:
    vector<user> users;
    std::string default_file_path = "../build/user.txt";
    fstream file;
    UserManager()
    {
        load();
    }
    void load()
    {
        file.open(default_file_path, ios::in);
        if (file.fail())
        {
            std::cout << "user.txt not found,creating..." << std::endl;
            file.open(default_file_path, ios::out);
            file.close();
        }
        else
        {
            std::cout << "user.txt found,loading..." << std::endl;
        }
        string username;
        string password;
        string net_address;
        user_type type;
        permission p;
        int a1, b1;
        users.clear();
        while (file >> username >> password >> net_address >> a1 >> b1)
        {
            type = (user_type)a1;
            p = (permission)b1;
            users.push_back(user(username, password, net_address, type, p));
        }
        file.close();
    }
    void write()
    {
        file.open(default_file_path, ios::out);
        if (file.fail())
        {
            std::cout << "write faill..." << std::endl;
            return;
        }
        for (auto u : users)
        {
            file << u.username << " " << u.password << " " << u.net_address << " " << u.type << " " << u.p << endl;
        }
        file.close();
    }
    void add_user(string username, string password, string net_address, user_type type, permission p)
    {
        users.push_back(user(username, password, net_address, type, p));
        write();
    }
    void delete_user(string username)
    {
        for (int i = 0; i < users.size(); i++)
        {
            if (users[i].username == username)
            {
                users.erase(users.begin() + i);
                break;
            }
        }
        write();
    }
    bool check_user(string username, string password)
    {
        for (auto u : users)
        {
            if (u.username == username && u.password == password)
            {
                return true;
            }
        }
        return false;
    }
};

#endif // DBMS_USERMANAGER_H