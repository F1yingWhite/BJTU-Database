//
// Created by BlackCat on 2023/3/31.
//

#ifndef DBMS_TABLE_H
#define DBMS_TABLE_H

#include <cstdio>
#include "iostream"
#include "vector"
#include "fstream"
#include "string"
#include "Attribute.h"
#include "../Database.h"
#include "../../BPlusTree2/BPlusTree.h"
#include "map"
#include "../../DB8/MyException.h"

using namespace std;

class Table
{
public:
    vector<Attribute> attributes;          // 表结构存放信息,后续的节点都需要调用这个
    Database *database;                    // 数据库的指针
    string tableName;                      // 数据库的名称
    string tableName2;                     // schema的名称
    int dataNum;                           // 存放当前表的数据个数
    int attributeNum;                      // 有多少个列
    int indexNum;                          // 索引的个数
    map<string, BPlusTree *> BPlusTreeMap; // B+树的集合
    ~Table()
    {
        for (auto it = BPlusTreeMap.begin(); it != BPlusTreeMap.end(); it++)
        {
            delete it->second;
        }
        BPlusTreeMap.clear();
    }

public:
    string insertBPlusTree(string indexName, BPlusTree *bPlusTree)
    {
        // 插入一个已有的B+树
        if (BPlusTreeMap.find(indexName) != BPlusTreeMap.end())
        {
            cout << "索引重复" << endl;
            throw MyException("索引重复");
            return "error";
        }
        BPlusTreeMap[indexName] = bPlusTree;
        indexNum = BPlusTreeMap.size();
        return "successful";
    }

    string createBPlusTree(vector<Attribute> att, bool repeat = true)
    {
        // 新建一个B+树,传入需要的索引字段
        string indexName = tableName2 + "_";
        for (auto at : att)
        {
            indexName += at.name + "_";
        }
        if (BPlusTreeMap.find(indexName) != BPlusTreeMap.end())
        {
            cout << "索引重复" << endl;
            throw MyException("索引重复");
            return "索引重复";
        }
        BPlusTree *bPlusTree = new BPlusTree(att, tableName2);
        BPlusTreeMap[indexName] = bPlusTree;
        bPlusTree->ifRepetition = repeat;
        // 新建索引
        int i = 0, count = 0;
        while (count < dataNum)
        {
            variableType temp;
            temp.type = att;
            vector<string> ts = getData(attributes, i);
            bool flag = false;
            for (int i = 0; i < ts.size(); i++)
            {
                if (ts[i] != "")
                {
                    flag = true;
                    break;
                }
            }
            if (flag)
            {
                for (auto t : att)
                { // 投影对应的列
                    temp.value.push_back(ts[t.rank]);
                }
                count++;
                if (!bPlusTree->insert(temp, i))
                {
                    // 插入失败,证明有重复,删除对应的B+树索引和文件
                    ::remove((bPlusTree->fileName + ".index").c_str());
                    ::remove((bPlusTree->fileName + ".type").c_str());
                    delete bPlusTree;
                    BPlusTreeMap.erase(indexName);
                    throw MyException("存在重复,无法创建索引!");
                    return "存在重复,无法创建索引!";
                }
            }
            i++;
        }
        indexNum = BPlusTreeMap.size();
        return R"(1\r\ncreate index successfully\r\n)";
    }

    bool createUnique(Attribute att)
    {
          for (auto &at : attributes)
        {
            if (at.name == att.name)
            {
                at.unique = true;
                break;
            }
        }
        try
        {
            createBPlusTree({att}, false);
        }
        catch (MyException e)
        {
            return false;
        }
        return true;
    }

    bool createPrimaryKey(Attribute att)
    {
        // 把要加的传给我
        vector<Attribute> pri;
        // 先找到主键
        for (auto at : attributes)
        {
            if (at.primaryKey == true)
            {
                pri.push_back(at);
            }
        }
        // 再修改主键
        for (auto &at : attributes)
        {
            if (at.name == att.name)
            {
                at.primaryKey = true;
                break;
            }
        }
        pri.push_back(att);
        try
        {
            createBPlusTree(pri, false);
        }
        catch (MyException e)
        {
            return false;
        }
        // 删除旧索引
        pri.pop_back();
        try
        {
            removeBPlusTree(pri);
        }
        catch (MyException e)
        {
            return true;
        }

        return true;
    }

    bool removeBPlusTree(vector<Attribute> att)
    {
        string indexName = tableName2 + "_";
        for (auto at : att)
        {
            indexName += at.name + "_";
        }
        if (BPlusTreeMap.find(indexName) == BPlusTreeMap.end())
        {
            cout << "索引不存在" << endl;
            throw MyException("索引不存在");
            return "索引不存在";
        }
        BPlusTree *bPlusTree = BPlusTreeMap[indexName];
        ::remove((bPlusTree->fileName + ".index").c_str());
        ::remove((bPlusTree->fileName + ".type").c_str());
        BPlusTreeMap.erase(indexName);
        delete bPlusTree;
        return true;
    }

    bool removeAll()
    {
        dataNum = 0;
        for (auto bpt : BPlusTreeMap)
        {
            ::remove((bpt.second->fileName + ".index").c_str());
            ::remove((bpt.second->fileName + ".type").c_str());
            createIndexFile(bpt.second->type, tableName2);
        }
        // 删除所有的数据
        ::remove(tableName2.c_str());
        fstream file;
        file.open(tableName2 + ".db", ios::out);
        char buff[11] = {0};
        buff[0] = '0';
        file.write(buff, 10);
        file.close();
        // 写入空文件
        return true;
    }

    vector<vector<string>> selectAll()
    {
        vector<vector<string>> result;
        int num = 0, i = 0;
        while (num < dataNum)
        {
            vector<string> temp = getData(attributes, i);
            for (int j = 0; j < temp.size(); j++)
            {
                if (temp[0] != "")
                {
                    num++;
                    result.push_back(temp);
                    break;
                }
            }
            i++;
        }
        return result;
    }

    bool drop()
    {
        // 需要手动删除catalog中的tablerecord
        //  删除所有的B+树
        for (auto bpt : BPlusTreeMap)
        {
            ::remove((bpt.second->fileName + ".index").c_str());
            ::remove((bpt.second->fileName + ".type").c_str());
            delete bpt.second;
        }
        // 删除所有的数据
        BPlusTreeMap.clear();
        ::remove((tableName2 + ".db").c_str());
        return true;
    }

    bool remove(vector<string> a)
    {
        int offset = -1;
        // 首先查看B+树是否存在,如果有,那么offset就从b+树中获取
        for (auto bpt : BPlusTreeMap)
        {
            // 先找到对应B+树的索引,确保传入的顺序是严格按照table中来的
            variableType temp;
            for (int i = 0; i < bpt.second->type.size(); i++)
            {
                for (int j = 0; j < attributes.size(); j++)
                {
                    if (bpt.second->type[i].name == attributes[j].name)
                    {
                        temp.value.push_back(a[j]);
                        temp.type.push_back(attributes[j]);
                        break;
                    }
                }
            }
            offset = bpt.second->remove(temp);
            if (offset == -1)
                return false; // 表示没找到
        }
        if (offset == -1)
        {
            // 顺序查找删除
            int i = 0;
            int count = 0;
            while (count < dataNum)
            {
                vector<string> temp = getData(attributes, i);
                for (int i = 0; i < temp.size(); i++)
                {
                    if (temp[i] != "")
                    {
                        count++;
                        break;
                    }
                }
                if (temp == a)
                {
                    deleteData(attributes, i);
                    dataNum--;
                    return true;
                }
                i++;
            }
        }
        else
        {
            deleteData(attributes, offset);
            dataNum--;
            return true;
        }
        return false;
    }

    string update(vector<string> oldData, vector<string> newData)
    {
        if (!remove(oldData))
        {
            return "更新失败,不存在该数据!";
        }
        if (!insert(newData))
        {
            return "更新失败";
        }
        return "更新成功";
    }

    vector<vector<string>> SelectEqual(vector<string> a, vector<Attribute> att)
    {
        // 比如条件为select * from table where a = 1 and b = 2;那么就传入1,2和a,b,注意这里ab必须按照table中的顺序来
        vector<int> offset;
        string name = tableName2 + "_";
        vector<vector<string>> result;
        vector<string> temp;
        bool flag = false;
        for (int i = 0; i < att.size(); i++)
        {
            name += att[i].name + "_";
        }
        for (auto bpt : BPlusTreeMap)
        {
            // 先找到对应的b+树
            if (bpt.second->fileName == name)
            {
                variableType temp;
                temp.value = a;
                for (int i = 0; i < bpt.second->type.size(); i++)
                {
                    for (int j = 0; j < attributes.size(); j++)
                    {
                        if (bpt.second->type[i].name == attributes[j].name)
                        {
                            temp.type.push_back(attributes[j]);
                        }
                    }
                }
                offset = bpt.second->EqualSearch(temp);
                flag = true;
                break;
            }
        }
        if (!flag)
        {
            // 顺序查找
            int i = 0;
            int count = 0;
            variableType t;
            t.type = att;
            t.value = a;
            while (count < dataNum)
            {
                vector<string> temp = getData(attributes, i);
                for (int i = 0; i < temp.size(); i++)
                {
                    if (temp[i] != "")
                    {
                        count++;
                        break;
                    }
                }
                variableType t2;
                t2.type = att;
                for (int i = 0; i < att.size(); i++)
                {
                    for (int j = 0; j < attributes.size(); j++)
                    {
                        if (att[i].name == attributes[j].name)
                        {
                            t2.value.push_back(temp[j]);
                            break;
                        }
                    }
                }
                if (t2 == t) // 投影出对应的列
                {
                    result.push_back(temp);
                }
                i++;
            }
        }
        else
        {
            for (int i = 0; i < offset.size(); i++)
            {
                result.push_back(getData(attributes, offset[i]));
            }
        }
        return result;
    }

    vector<vector<string>> SelectBig(vector<string> a, vector<Attribute> att)
    {
        vector<int> offset;
        string name = tableName2 + "_";
        vector<vector<string>> result;
        bool flag = false;
        for (int i = 0; i < att.size(); i++)
        {
            name += att[i].name + "_";
        }
        for (auto bpt : BPlusTreeMap)
        {
            // 先找到对应的b+树
            if (bpt.second->fileName == name)
            {
                variableType temp;
                temp.value = a;
                for (int i = 0; i < bpt.second->type.size(); i++)
                {
                    for (int j = 0; j < attributes.size(); j++)
                    {
                        if (bpt.second->type[i].name == attributes[j].name)
                        {
                            temp.type.push_back(attributes[j]);
                        }
                    }
                }
                offset = bpt.second->BigSearch(temp);
                flag = true;
                break;
            }
        }
        if (!flag)
        {
            // 顺序查找
            int i = 0;
            int count = 0;
            while (count < dataNum)
            {
                vector<string> temp = getData(attributes, i);
                bool exist = false;
                for (int i = 0; i < temp.size(); i++) // 检查非空
                {
                    if (temp[i] != "")
                    {
                        count++;
                        exist = true;
                        break;
                    }
                }
                if (!exist)
                    break;
                bool flag = true;
                // 比较对应列的大小
                for (int i = 0; i < att.size(); i++)
                {
                    for (int j = 0; j < attributes.size(); j++)
                    {
                        if (att[i].name == attributes[j].name)
                        { // 找到对应的列
                            if (att[i].type == Attribute_Int)
                            {
                                if (stoi(temp[j]) < stoi(a[i]))
                                {
                                    flag = false;
                                    break;
                                }
                            }
                            else if (att[i].type == Attribute_Double)
                            {
                                if (stod(temp[j]) < stod(a[i]))
                                {
                                    flag = false;
                                    break;
                                }
                            }
                            else
                            {
                                if (temp[j] < a[i])
                                {
                                    flag = false;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (flag)
                {
                    result.push_back(temp);
                }
                i++;
            }
        }
        else
        {
            for (int i = 0; i < offset.size(); i++)
            {
                result.push_back(getData(attributes, offset[i]));
            }
        }
        return result;
    }

    vector<vector<string>> SelectSmall(vector<string> a, vector<Attribute> att)
    {
        vector<int> offset;
        string name = tableName2 + "_";
        vector<vector<string>> result;
        bool flag = false;
        for (int i = 0; i < att.size(); i++)
        {
            name += att[i].name + "_";
        }
        for (auto bpt : BPlusTreeMap)
        {
            // 先找到对应的b+树
            if (bpt.second->fileName == name)
            {
                variableType temp;
                temp.value = a;
                for (int i = 0; i < bpt.second->type.size(); i++)
                {
                    for (int j = 0; j < attributes.size(); j++)
                    {
                        if (bpt.second->type[i].name == attributes[j].name)
                        {
                            temp.type.push_back(attributes[j]);
                        }
                    }
                }
                offset = bpt.second->SmallSearch(temp);
                flag = true;
                break;
            }
        }
        if (!flag)
        {
            // 顺序查找
            int i = 0;
            int count = 0;
            while (count < dataNum)
            {
                vector<string> temp = getData(attributes, i);
                bool exist = false;
                for (int i = 0; i < temp.size(); i++) // 检查非空
                {
                    if (temp[i] != "")
                    {
                        count++;
                        exist = true;
                        break;
                    }
                }
                if (!exist)
                    break;
                bool flag = true;
                // 比较对应列的大小
                for (int i = 0; i < att.size(); i++)
                {
                    for (int j = 0; j < attributes.size(); j++)
                    {
                        if (att[i].name == attributes[j].name)
                        { // 找到对应的列
                            if (att[i].type == Attribute_Int)
                            {
                                if (stoi(temp[j]) > stoi(a[i]))
                                {
                                    flag = false;
                                    break;
                                }
                            }
                            else if (att[i].type == Attribute_Double)
                            {
                                if (stod(temp[j]) > stod(a[i]))
                                {
                                    flag = false;
                                    break;
                                }
                            }
                            else
                            {
                                if (temp[j] > a[i])
                                {
                                    flag = false;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (flag)
                {
                    result.push_back(temp);
                }
                i++;
            }
        }
        else
        {
            for (int i = 0; i < offset.size(); i++)
            {
                result.push_back(getData(attributes, offset[i]));
            }
        }
        return result;
    }

    bool insert(vector<string> a)
    {
        // 先要查看主键和唯一键是否重复
        string priName = tableName2 + "_";
        string uniName = tableName2 + "_";
        variableType pri, uni;
        for (int i = 0; i < attributes.size(); i++)
        {
            if (attributes[i].primaryKey == true)
            {
                priName += attributes[i].name + "_";
                pri.type.push_back(attributes[i]);
                pri.value.push_back(a[i]);
            }
            if (attributes[i].unique == true)
            {
                uniName += attributes[i].name + "_";
                uni.type.push_back(attributes[i]);
                uni.value.push_back(a[i]);
            }
        }
        if (BPlusTreeMap.find(priName) != BPlusTreeMap.end() && BPlusTreeMap[priName]->find(pri) != -1)
        {
            cout << "主键重复" << endl;
            throw MyException("主键重复");
            return false;
        }
        if (BPlusTreeMap.find(uniName) != BPlusTreeMap.end() && BPlusTreeMap[uniName]->find(uni) != -1)
        {
            cout << "唯一键重复" << endl;
            throw MyException("唯一键重复");
            return false;
        }
        // 还要查看外键约束
        vector<int> foreignColumnNumber;
        vector<string> foreignTableName;
        vector<string> foreignColumnName;
        for (int i = 0; i < attributes.size(); i++)
        {
            if (attributes[i].foreignKey != "")
            {
                foreignColumnNumber.push_back(i);
                int len = attributes[i].foreignKey.length();
                int pos = 0;
                while (attributes[i].foreignKey[pos] != '_')
                {
                    pos++;
                }
                foreignTableName.push_back(attributes[i].foreignKey.substr(0, pos));
                foreignColumnName.push_back(attributes[i].foreignKey.substr(pos + 1, len - pos - 1));
            }
        }
        if (foreignColumnName.size() != 0)
        {
            for (int i = 0; i < foreignColumnName.size(); i++)
            {
                // 先找到对应的表
                for (auto table : database->tableRecord)
                {
                    if (table->tableName == foreignTableName[i])
                    {
                        // 然后找到对应的列
                        for (auto col : table->attributes)
                        {
                            if (col.name == foreignColumnName[i])
                            {
                                // 直接查找对应值是否存在,不存在直接返回
                                if (table->SelectEqual({a[i]}, {col}).size() == 0)
                                {
                                    cout << "外键约束不满足" << endl;
                                    throw MyException("外键约束不满足");
                                    return false;
                                }
                            }
                        }
                    }
                }
            }
        }
        // 成功加入
        dataNum++;
        int offset = insertData(attributes, a);
        for (auto m : BPlusTreeMap)
        {
            variableType temp;
            for (int i = 0; i < m.second->type.size(); i++)
            {
                for (int j = 0; j < attributes.size(); j++)
                {
                    if (m.second->type[i].name == attributes[j].name)
                    {
                        temp.type.push_back(attributes[j]);
                        temp.value.push_back(a[j]);
                        break;
                    }
                }
            }
            m.second->insert(temp, offset);
        }
        return true;
    }

    void alter(vector<Attribute> att)
    { // 传入的内容为修改之后的类型样式
        // 首先删除所有索引
        for (auto m : BPlusTreeMap)
        {
            // 删除所有B+树索引,然后重建主键和唯一键索引
            string filename = m.second->fileName;
            delete m.second;
            ::remove((filename + ".index").c_str());
            ::remove((filename + ".type").c_str());
        }
        BPlusTreeMap.clear();
        // 然后重新写一遍表
        string file = tableName2 + "Temp.db";
        fstream file1(tableName2 + ".db", ios::in);
        fstream file2(file, ios::out);
        // 新文件初始化
        char buff[11] = {0};
        buff[0] = '0';
        file2.write(buff, 10);
        file2.close();
        int count = 0;
        int i = 0;
        while (count < dataNum)
        {
            bool kong = true;
            vector<string> temp;
            temp = getData(attributes, i);
            for (int j = 0; j < temp.size(); j++)
            {
                if (temp[j] != "")
                { // 不为空,那么加一个
                    count++;
                    kong = false;
                    break;
                }
            }
            if (!kong)
            {
                vector<string> t2; // 投影对应的列并且写入默认值
                for (auto at : att)
                {
                    bool find = false;
                    for (auto j : attributes)
                    {
                        if (at.name == j.name)
                        {
                            find = true;
                            t2.push_back(temp[j.rank]);
                            break;
                        }
                    }
                    if (!find)
                    {
                        t2.push_back(at.defaultValue);
                    }
                }
                insertDataOutSide(att, t2, file);
            }
            i++;
        }
        file1.close();
        file2.close();
        attributes = att; // 更新attributes
        ::remove((tableName2 + ".db").c_str());
        ::rename(file.c_str(), (tableName2 + ".db").c_str());
        // 重建主键和唯一键索引
        vector<Attribute> pri;
        for (auto at : attributes)
        {
            if (at.primaryKey == true)
            {
                pri.push_back(at);
            }
            if (at.unique == true)
            {
                createBPlusTree({at});
            }
        }
        if (pri.size() != 0)
        {
            createBPlusTree(pri);
        }
        this->attributes = att;
        this->attributeNum = att.size();
        for (int i = 0; i < attributeNum; i++)
        { // 重建rank
            attributes[i].rank = i;
        }
    }

    Table()
    {
        dataNum = 0;
        attributeNum = 0;
        indexNum = 0;
    }

    Table(string n, int dataN, int atn, int indN) : tableName(n), dataNum(dataN), attributeNum(atn), indexNum(indN) {}

    // 操作数据文件
    vector<string> getData(vector<Attribute> &type, int offset)
    {
        string filename = tableName2 + ".db";
        vector<string> data;
        fstream file(filename, ios::in);
        if (!file.is_open())
        {
            return {};
        }
        int len = 10;
        for (auto ty : type)
        {
            len += ty.len;
        }
        file.seekg(len * offset + 20, ios::beg); // 前10个用来存放第一个空位置在哪
        for (auto ty : type)
        {
            char *temp = new char[ty.len];
            file.read(temp, ty.len);
            string str = "";
            for (int i = 0; i < ty.len; i++)
            {
                if (temp[i] == '\0')
                    break;
                str += temp[i];
            }
            data.push_back(str);
        }
        file.close();
        return data;
    }

    // 使用覆盖写的方式更新数据
    void updateData(vector<Attribute> &type, int offset, vector<string> data)
    {
        string filename = tableName2 + ".db";
        fstream file(filename, ios::in | ios::out);
        if (!file.is_open())
        {
            cerr << "file open error";
            return;
        }
        int len = 10;
        for (auto ty : type)
        {
            len += ty.len;
        }
        file.seekp(len * offset + 20, ios::beg);
        for (int i = 0; i < data.size(); i++)
        {
            file.write(data[i].c_str(), type[i].len);
        }
        file.close();
    }

    // 删除某个位置的数据
    void deleteData(vector<Attribute> &type, int offset)
    {
        string filename = tableName2 + ".db";
        fstream file(filename, ios::in | ios::out);
        int len = 10;
        for (auto ty : type)
        {
            len += ty.len;
        }
        file.seekp(len * offset + 10, ios::beg);
        for (int i = 0; i < len; i++)
        {
            file.write("\0", 1);
        }
        file.close();
        file.open(filename, ios::in | ios::out | ios::binary);
        char buff[11] = {0};
        file.seekg(0, ios::beg);
        file.read(buff, 10);
        int offset1 = atoi(buff);
        file.close();
        if (offset1 < offset)
            return;
        file.open(filename, ios::in | ios::out | ios::binary);
        file.seekp(0, ios::beg);
        memset(buff, '\0', sizeof(buff));
        strcpy(buff, to_string(offset).c_str());
        file.write(buff, 10);
        file.close();
    }

    // 插入数据,返回offset
    int insertData(vector<Attribute> &type, vector<string> data)
    {
        string filename = tableName2 + ".db";
        fstream file(filename, ios::binary | ios::in | ios::out);
        if (!file.is_open())
        {
            file.close();
            file.open(filename, ios::out);
            char buff[11] = {0};
            buff[0] = '0';
            file.write(buff, 10);
            file.close();
            file.open(filename, ios::binary | ios::in | ios::out);
        }
        int len = 10; // 前10个存放自己的offset
        for (auto ty : type)
        {
            len += ty.len;
        }
        int offset;
        int result;
        file >> offset;
        result = offset;
        file.seekp(len * offset + 10, ios::beg);
        char buff[10];
        memset(buff, '\0', sizeof(buff));
        strcpy(buff, to_string(offset).c_str());
        file.write(buff, 10); // 先把位置数据记录下来
        offset++;
        for (int i = 0; i < data.size(); i++)
        {
            char buff[type[i].len + 1];
            memset(buff, '\0', sizeof(buff));
            strcpy(buff, data[i].c_str());
            file.write(buff, type[i].len);
        }
        file.close();
        file.open(filename, ios::in);
        while (true)
        { // 更新第一个位置
            file.seekg(len * offset + 10, ios::beg);
            char temp = '\0';
            file.read(&temp, 1);
            if (temp == '\0')
            {
                break;
            }
            offset++;
        }
        file.close();
        file.open(filename, ios::out | ios::in);
        memset(buff, '\0', sizeof(buff));
        strcpy(buff, to_string(offset).c_str());
        file.seekp(0, ios::beg);
        file.write(buff, 10);
        file.close();
        return result;
    }

    int insertDataOutSide(vector<Attribute> &type, vector<string> data, string tableName2)
    {
        string filename = tableName2 + ".db";
        fstream file(filename, ios::binary | ios::in | ios::out);
        if (!file.is_open())
        {
            file.close();
            file.open(filename, ios::out);
            char buff[11] = {0};
            buff[0] = '0';
            file.write(buff, 10);
            file.close();
            file.open(filename, ios::binary | ios::in | ios::out);
        }
        int len = 10; // 前10个存放自己的offset
        for (auto ty : type)
        {
            len += ty.len;
        }
        int offset;
        int result;
        file >> offset;
        result = offset;
        file.seekp(len * offset + 10, ios::beg);
        char buff[10];
        memset(buff, '\0', sizeof(buff));
        strcpy(buff, to_string(offset).c_str());
        file.write(buff, 10); // 先把位置数据记录下来
        offset++;
        for (int i = 0; i < data.size(); i++)
        {
            char buff[type[i].len + 1];
            memset(buff, '\0', sizeof(buff));
            strcpy(buff, data[i].c_str());
            file.write(buff, type[i].len);
        }
        file.close();
        file.open(filename, ios::in);
        while (true)
        { // 更新第一个位置
            file.seekg(len * offset + 10, ios::beg);
            char temp = '\0';
            file.read(&temp, 1);
            if (temp == '\0')
            {
                break;
            }
            offset++;
        }
        file.close();
        file.open(filename, ios::out | ios::in);
        memset(buff, '\0', sizeof(buff));
        strcpy(buff, to_string(offset).c_str());
        file.seekp(0, ios::beg);
        file.write(buff, 10);
        file.close();
        return result;
    }
};

#endif // DBMS_TABLE_H
