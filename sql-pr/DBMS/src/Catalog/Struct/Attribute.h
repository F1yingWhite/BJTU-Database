//
// Created by BlackCat on 2023/3/31.
//

#ifndef DBMS_ATTRIBUTE_H
#define DBMS_ATTRIBUTE_H

#include "iostream"
#include "vector"
#include "fstream"
#include "string"

using namespace std;

/* 
 * 仿照MiniDBMS写的数据存储结构
 * Attribute用来存放每一个列的结构信息
 * 存放的信息大致有:name,type,length,primaryKey,unique
 * 为了方便起见就直接把所有类型全部设置为public
 * */

// 数据类型
const int Attribute_Int = 1;
const int Attribute_Char = 2;
const int Attribute_Date = 3;
const int Attribute_Double = 4;

// 默认数据类型长度
const int AttributeIntLen = 11;
const int AttributeDoubleLen = 20;
const int AttributeDateLen = 20;
const int AttributeCharLen = 20;

class Attribute
{
public:
    string name;    // 这个列的名字
    int type;       // 类型
    int primaryKey; // 方便读写
    int unique;
    int len;             // 使用上文的默认数据类型长度
    int rank;            // 记录了当前是第几个属性,从0开始
    string defaultValue; // 默认值,在常杰那部分使用,如果没有输入,就使用默认值
    string foreignKey;   // 外键,定义为表名+字段,如:student_id,当插入的时候,需要检查外键是否存在
    Attribute()
    {
        primaryKey = false;
        unique = false;
        foreignKey = "";
        defaultValue = "";
    } // 默认构造函数
    Attribute(string n, int type, int len)
    {
        Attribute();
        name = n;
        this->type = type;
        primaryKey = false;
        unique = false;
        defaultValue = "";
        foreignKey = "";
        this->len = len;
    }
    Attribute(string &n, int t, int l, bool p, bool u, int r) : name(n), type(t), len(l), primaryKey(p), unique(u), rank(r)
    {
        defaultValue = "";
        foreignKey = "";
    }
};

#endif // DBMS_ATTRIBUTE_H
