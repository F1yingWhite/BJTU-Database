//
// Created by BlackCat on 2023/4/3.
//

#ifndef DBMS_VARIALBETYPE_H
#define DBMS_VARIALBETYPE_H

#include "iostream"
#include "vector"
#include "Attribute.h"
#include "cstring"
using namespace std;

// 可变类型,用于存放不同长度的类型
class variableType
{
public:
    vector<Attribute> type;
    vector<string> value;

public:
    bool operator>(const variableType &v)
    {
        for (int i = 0; i < type.size(); ++i)
        {
            if (type[i].type == Attribute_Int)
            {
                if (stoi(value[i]) > stoi(v.value[i]))
                {
                    return true;
                }
                else if (stoi(value[i]) < stoi(v.value[i]))
                {
                    return false;
                }
            }
            else if (type[i].type == Attribute_Double)
            {
                if (stod(value[i]) > stod(v.value[i]))
                {
                    return true;
                }
                else if (stod(value[i]) < stod(v.value[i]))
                {
                    return false;
                }
            }
            else
            {
                if (value[i] > v.value[i])
                {
                    return true;
                }
                else if (value[i] < v.value[i])
                {
                    return false;
                }
            }
        }
        return false;
    }

    bool operator==(const variableType &v)
    {
        if (type.size() != v.type.size())
        {
            return false;
        }
        for (int i = 0; i < type.size(); ++i)
        {
            if (value[i] != v.value[i] || type[i]. type != v.type[i].type)
            {
                return false;
            }
        }
        return true;
    }

    bool operator<=(const variableType &v)
    {
        return !(*this > v);
    }

    bool operator>=(const variableType &v)
    {
        return !(*this < v);
    }

    variableType &operator=(const variableType &v)
    {
        type.clear();
        value.clear();
        for (int i = 0; i < v.type.size(); ++i)
        {
            type.push_back(v.type[i]);
            value.push_back(v.value[i]);
        }
        return *this;
    }

    bool operator<(const variableType &v)
    {
        for (int i = 0; i < type.size(); ++i)
        {
            if (type[i].type == Attribute_Int)
            {
                if (stoi(value[i]) > stoi(v.value[i]))
                {
                    return false;
                }
                else if (stoi(value[i]) < stoi(v.value[i]))
                {
                    return true;
                }
            }
            else if (type[i].type == Attribute_Double)
            {
                if (stod(value[i]) > stod(v.value[i]))
                {
                    return false;
                }
                else if (stod(value[i]) < stod(v.value[i]))
                {
                    return true;
                }
            }
            else
            {
                if (value[i] > v.value[i])
                {
                    return false;
                }
                else if (value[i] < v.value[i])
                {
                    return true;
                }
            }
        }
        return false;
    }
};

#endif // DBMS_VARIALBETYPE_H
