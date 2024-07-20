//
// Created by BlackCat on 2023/3/31.
//

#ifndef DBMS_DATABASE_H
#define DBMS_DATABASE_H

#include "iostream"
#include "vector"
#include "fstream"
#include "string"
#include "Struct/Attribute.h"

using namespace std;

const string fileName = "schema.db";

/* 用于存储和读取表结构,我们将日志信息存放在schema.db里
 * 我们自己定义schema的存储结构如下
 * TableNum TableName AttributeNum DataNum AttributeName AttributeType length primaryKey unique
 * DB8一直管理这个数据结构,用来了解所有的表结构
 * */
class Table;
// 这个类记录了所有的表结构,表结构为table
class Database
{
public:
        int tableNum;
        vector<Table *> tableRecord;
        string dbName; // schema.bd的名字
        string Name;   // 数据库的名字
public:
        Database(string name)
        {
                Name = name;
#ifdef _WIN32
                dbName = "Database\\" + Name + "\\schema.db";
#elif __linux__
                dbName = "Database/" + Name + "/schema.db";
#elif __APPLE__
                dbName = "Database/" + Name + "/schema.db";
#endif
                tableNum = 0;
                // readCatalog();
        }

        void intilize()
        {
#ifdef _WIN32
                system(("mkdir Database\\" + Name).c_str());
#elif __linux__
                system(("mkdir Database/" + Name).c_str());
#elif __APPLE__
                system(("mkdir Database/" + Name).c_str());
#endif
                fstream fout(dbName.c_str(), ios::out);
                fout << 0 << " ";
                fout.close();
        }

        ~Database();

        void print();

        void backup(); // 用于拷贝文件夹,拷贝的文件夹名称叫做databaseName_time

        void recovery(string backup); // 用于恢复数据库

        bool upDataTable(Table *newTable);

        bool insertTable(Table *newTable);

        bool deleteTable(string name);

        bool existTable(string name);

        Table *getTable(string name);

        void deleteDatabase();
        void readCatalog(); // 当DB8创建的时候自动调用该函数,因为数据表信息比较小,所以直接读入内存
        void readCatalog(string backup);
        void writeCatalog(); // 当DB8调用析构函数的时候自动调用该函数
};

#endif // DBMS_DATABASE_H