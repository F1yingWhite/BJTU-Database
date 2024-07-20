#ifndef VERSIONMANAGER_H
#define VERSIONMANAGER_H

#include <iostream>
#include <fstream>
#include <ostream>
#include <chrono>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;
// 定义日志记录的类型
struct version
{
    uint32_t id;      // 偏移量
    std::string info; // 信息
};

class versionManager
{
public:
    vector<version> versions;
    fstream file;
    const std::string default_file_path = "../build/version.txt"; // 默认的 redo log 文件路径

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

    void load()
    {
        file.open(default_file_path, ios::in);
        if (!file)
        {
            cout << "file not exist,creating..." << endl;
            file.open(default_file_path, ios::out);
            file.close();
            return;
        }
        version v;
        string line;
        versions.clear();
        while (getline(file, line))
        {
            std::vector<std::string> elems = stringSplit(line, '_');
            version v;
            v.id = stoi(elems[0]);
            v.info = elems[1];
            versions.push_back(v);
        }
        file.close();
    }

    void write()
    {
        file.open(default_file_path, ios::out);
        if (!file)
        {
            cout << "file open error" << endl;
            return;
        }
        for (auto v : versions)
        {
            file << v.id << "_" << v.info << endl;
        }
        file.close();
    }

    // 添加一条日志记录
    void add(version v)
    {
        versions.push_back(v);
    }

    void add(int a, string b)
    {
        version v;
        v.id = a;
        v.info = b;
        versions.push_back(v);
    }

    void delete_version(int id)
    {
        for (auto it = versions.begin(); it != versions.end(); it++)
        {
            if (it->id == id)
            {
                versions.erase(it);
                break;
            }
        }
    }
};

#endif
