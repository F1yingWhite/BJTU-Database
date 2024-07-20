//
// Created by BlackCat on 2023/3/10.
//

#ifndef DBMS_SQLCOMPILER_H
#define DBMS_SQLCOMPILER_H
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <regex>
#include <map>
using std::map;
#include "../Catalog/Struct/Attribute.h"
#include "../Catalog/Struct/Table.h"
#include <boost/locale.hpp>
enum state
{
    InsertState,
    DeleteState,
    CreateState,
    UpdateState,
    DropState,
    AlertState,
    SelectState,
    DescTable,
    ShowTables,
    CreateIndex,
    DeleteIndex,
    Commit,
    Backup,
    RollBack,
    UsingDB,
    Recover,
    ShowAllVersions,
    CreateDatabase,
    ShowDatabase,
    DeleteDB,
    UnknownState
};
using namespace std;
class sqlCompiler
{
public:
    int segmentation(const string &s); // sql语句分割后解析返回
    vector<string> preCreate(const string &s);
    vector<string> preInsert(const string &s);
    vector<string> preDelete(const string &s);
    vector<string> preUpdate(const string &s);
    vector<string> preDrop(const string &s);
    vector<string> preAlter(const string &s);
    vector<string> preSelect(const string &s);
    vector<string> createDB(const string &s);
    vector<string> usingDB(const string &s);
    vector<string> showDb(const string &s);
    vector<string> createIndex(const string &s);
    vector<string> delIndex(const string &s);
    string del_space(string);
    vector<string> descTable(const string &s);
    vector<string> commit(const string &s);
    vector<string> recover(const string &s);
    vector<string> rollback(const string &s);
    vector<string> delete_db(const string &s);
    string transfertoUnicode(string s)
    {
        string utf8Text = boost::locale::conv::to_utf<char>(s, "UTF-8");
        return utf8Text;
    }
};

#endif // DBMS_SQLCOMPILER_H
