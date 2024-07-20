//
// Created by BlackCat on 2023/3/10.
//

#ifndef DBMS_DB8_H
#define DBMS_DB8_H

#include "iostream"
#include "../sqlCompiler/sqlCompiler.h"
#include "../Log/RedoLogManager.h"
#include "string"
#include "vector"
#include "map"
#include "time.h"
#include "stack"
#include "../Catalog/Database.h"
#include <map>
#include <chrono>
#include <iomanip>
#include "sstream"
using namespace std;

class SQLStatement
{
public:
    virtual ~SQLStatement() {}
};

class Column : public SQLStatement
{
public:
    std::string columnName; // 列名
};

class Comparison : public SQLStatement
{
public:
    std::string left;       // 左表达式
    std::string right;      // 右表达式
    std::string comparison; // 比较运算符
    int visited;            // 判断是否背表引用  -1 表示没有  1 表示已经被引用
    bool double_column;      //用于判断是否是表名.列 compare 表名.列
    vector<int> index_table_col_1;
    vector<int> index_table_col_2;
};

class TableName : public SQLStatement
{
public:
    std::string tableName; // 表名
    vector<shared_ptr<Comparison>> where;
};

class OrderBy : public SQLStatement
{
public:
    std::vector<std::string> columns; // ORDER BY子句中的列名
};

class GroupBy : public SQLStatement
{
public:
    std::vector<std::string> columns; // GROUP BY子句中的列名
};

class Having : public SQLStatement
{
public:
    vector<string> condition; // HAVING子句中的条件表达式
};

class SelectStatement : public SQLStatement
{
public:
    // 使用智能指针，防止内存泄漏,会自动初始化为空
    std::vector<std::shared_ptr<Column>> columns;      // SELECT语句中的列
    std::vector<std::shared_ptr<TableName>> fromTable; // FROM子句中的表
    std::vector<shared_ptr<Comparison>> whereClause;   // WHERE子句
    std::shared_ptr<GroupBy> groupByClause;            // GROUP BY子句
    std::shared_ptr<Having> havingClause;              // HAVING子句
    std::shared_ptr<OrderBy> orderByClause;            // ORDER BY子句
};

class DB8
{
public:
    string start(string temp, bool whether_log = true);

    ~DB8();

    DB8();

    shared_ptr<SelectStatement> init_ast(vector<string> ans);

public:
    vector<Database *> databases; // 数据库所有管理的数据库
    Database *catalog;            // 目前管理使用的数据库
    void start();
    // 符号与数字对应
    map<string, int> judsign;
    map<string, int> judConstraint;
    map<string, int> judAlter;
    RedoLogManager logManager;

    sqlCompiler test; // 语法解析树
    shared_ptr<SelectStatement> match_where(shared_ptr<SelectStatement> &ast);
    string processStringSpace(string str);               // 用于处理字符串中的空格
    vector<string> clean(const string str, string sign); // 用于返回 一个用于判断表达式子之间清洗后的值 比方说 0号是左边的,1号是右边的
    vector<vector<string>> union_tables(vector<vector<string>> table_1, vector<vector<string>> table_2);
    vector<vector<string>> select_one_table(vector<string>);
    vector<vector<string>> where_del(string union_type, vector<string>, vector<vector<string>>, Table *t); // 用于删除where语句中不满足条件的数据；
    vector<string> get_after_clean(string);
    void union_again(vector<vector<string>> &s,shared_ptr<SelectStatement> &ast);
    string match_type_column(Table *t,string value,int k) {
        switch(t->attributes[k].type)
        {
        case 1:
            stoi(value); // int类型匹配
            return value;
            break;
        case 2:
            if ((value[0] == '\'' && value[value.length() - 1] == '\'') || (value[0] == '"' && value[value.length() - 1] == '"'))
            {
                return value.substr(1, value.length() - 2);
            }
            else
            {
                throw MyException("Please check your type " + value + ".");
            }
            break;
        case 3:
            if (!value.empty())
            {
                if ((value[0] == '\'' && value[value.length() - 1] == '\'') || (value[0] == '"' && value[value.length() - 1] == '"'))
                {
                    value = value.substr(1, value.length() - 2);
                    if (is_date(value)) // date类型匹配
                    {
                        tm dt = {};
                        istringstream iss(value);
                        iss >> get_time(&dt, "%Y-%m-%d");
                        std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(std::mktime(&dt));
                        std::time_t tt = std::chrono::system_clock::to_time_t(tp);
                        stringstream ss;
                        ss << put_time(&dt, "%Y-%m-%d");
                        return ss.str();
                    }
                    else
                        throw MyException("Invaild enter type )" + value + R"(\r\n)");
                }
                else
                {
                    throw MyException("Please check your type " + value + ".");
                }
            }
            else
                throw MyException("Please check your type.");
            break;
        case 4:
            stod(value); // double类型匹配
            return value;
            break;
        default:
            throw MyException("Please check your type.");
        }
    }
    bool is_date(const string &str)
    {
        istringstream iss(str);
        tm dt = {};
        iss >> get_time(&dt, "%Y-%m-%d");
        return !iss.fail();
    }

    Table *judExit(string name);

    void te()
    { // 用于测试的函数
        cout << start("create database IndexText;") << endl;
        cout << start("use IndexText;") << endl;
        cout << start("create table sc(sno int(8),cno int(8),grade int(8));") << endl;
        for (int i = 0; i < 100000; i++)
        {
            cout << i << start("insert into sc values(" + to_string(rand() % 1000) + "," + to_string(rand() % 1000) + "," + to_string(rand() % 1000) + ");") << endl;
        }
        // clock_t start = clock(); // 程序开始计时
        // this->start("select * from sc where grade = 998;");
        // this->start("select * from sc where grade>998;");
        // this->start("select * from sc where grade<5;");
        // clock_t end = clock(); // 程序结束用时
        // double endtime = (double)(end - start) / CLOCKS_PER_SEC;
        // cout << "Total time:" << endtime * 1000 << "ms" << endl;    // 不使用索引的用时ms
        // cout << this->start("create index on  sc(grade);") << endl; // 在grade字段创建索引
        // start = clock();                                            // 程序开始计时
        // this->start("select * from sc where grade = 998;");
        // this->start("select * from sc where grade>998;");
        // this->start("select * from sc where grade<5;");
        // end = clock(); // 程序结束用时,创建索引之后的用时
        // endtime = (double)(end - start) / CLOCKS_PER_SEC;
        // cout << "Total time:" << endtime * 1000 << "ms" << endl; // ms
        cout << this->start("exit;");
    }

    std::vector<std::string> stringSplit_(const std::string &str, char delim)
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

    void recovery(string backup)
    {
        //        cout<<1;
        //        catalog->writeCatalog();
        catalog->recovery(backup);
        //        catalog->readCatalog();

        //        catalog->readCatalog(backup);
        vector<string> elems = stringSplit_(backup, '_');
        cout << start("use " + elems[0] + ";") << endl;
        vector<LogRecord> records = logManager.read_all_records(stoi(elems[1]));
        for (auto n : records)
        {
            string head1("recover");
            string head2("rollback");
            string head3("backup");
            bool startwith1 = n.info.compare(0, head2.size(), head2) == 0;
            bool startwith2 = n.info.compare(0, head1.size(), head1) == 0;
            bool startwith3 = n.info.compare(0, head3.size(), head3) == 0;
            if (!startwith3 && !startwith1 && !startwith2)
            {
                cout << start(n.info, false) << endl;
            }
        }
    }

    void rollback(string backup, int pos)
    {
        catalog->writeCatalog();
        catalog->recovery(backup);
        catalog->readCatalog();
        //        catalog->readCatalog(backup);
        vector<string> elems = stringSplit_(backup, '_');
        cout << start("use " + elems[0] + ";") << endl;
        if (pos < stoi(elems[1]))
        {
            cout << "The version number you specify needs to be larger than your backup record " << endl;
        }
        vector<LogRecord> records = logManager.read_all_records(stoi(elems[1]));
        for (auto n : records)
        {
            cout << n.offset << endl;
            if (pos >= n.offset)
            {
                string head1("recover");
                string head2("rollback");
                string head3("backup");
                bool startwith1 = n.info.compare(0, head2.size(), head2) == 0;
                bool startwith2 = n.info.compare(0, head1.size(), head1) == 0;
                bool startwith3 = n.info.compare(0, head3.size(), head3) == 0;

                if (!startwith3 && !startwith1 && !startwith2)
                {
                    cout << n.info << endl;
                    cout << start(n.info, false) << endl;
                }
            }
            else
            {
                return;
            }
        }
    }

private:
    // 解释器
    sqlCompiler compiler;
};

#endif // DBMS_DB8_H
