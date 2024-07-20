//
// Created by BlackCat on 2023/3/10.
//

#include "sqlCompiler.h"
#include "../DB8/MyException.h"
using std::regex;
std::smatch subMatch;
using std::map;
// 用于编译代码hyx到此一游
int sqlCompiler::segmentation(const string &s)
{
    vector<string> result;
    string str = s;
    // 删除前缀空格
    int pre = 0;
    while (str[pre] == ' ')
    {
        pre++;
    }
    // 部分数据库创建的时候，支持选择是否大小写敏感，日后可能需要调整该段代码
    transform(str.begin(), str.end(), str.begin(), ::tolower); // 将大写字符全部转换成为小写字母，待定，可能有问题

    str = str.substr(pre, str.length());
    // str.append(" ");
    string temp = "";

    // 利用正则表达式解析信息
    string sqlcreate = "create table.+\\(.+\\n*\\)\\s?;";
    string sqlinsert = "insert into.+;";
    string sqldelete = "delete from\\s.+(;|\\swhere\\s.+;)";
    string sqlupdate = "update\\s.+\\sset.+;"; // update\s[0-9a-z]+\sset\s([0-9a-z]+\s=\s([0-9a-z]+|'[0-9a-zA-Z]+');|[0-9a-z]+=([0-9a-z]+|'[0-9a-zA-Z]+');)
    string sqldrop = "drop table\\s.+;";
    string sqlalter = "alter table\\s.+\\s(modify\\s.+(set\\s+.+|drop\\s+.+)|add.+|drop.+|modify column.+);";
    string sqlselect = "select.+from.+(where.+)?((group by)?|(order by)?|(having)?);";
    regex Pcreate(sqlcreate);
    regex Pdelete(sqldelete);
    regex Pupdate(sqlupdate);
    regex Pdrop(sqldrop);
    regex Palter(sqlalter);
    regex Pselect(sqlselect);
    regex Pinsert(sqlinsert);
    regex create_DB("create database [a-z]+;");
    regex using_DB("use .+;");
    regex show_Db("show database;");
    regex show_tables("show table;");
    regex create_index("create index on\\s.+;");
    regex del_index("delete index\\s.+;");
    regex desc_table("desc .+;");

    regex del_db("delete database.+;");
    if (regex_match(str, del_db))
    {
        return DeleteDB;
    }

    regex commit("commit -m\\s?.+;");
    if (regex_match(str, commit))
    {
        return Commit;
    }

    regex backup("backup;");
    if (regex_match(str, backup))
    {
        return Backup;
    }

    regex rollback("rollback .+_[0-9]+\\s[0-9]+;");
    if (regex_match(str, rollback))
    {
        return RollBack;
    }

    regex recovery("recover\\s.+_[0-9]+;");
    if (regex_match(str, recovery))
    {
        return Recover;
    }

    if (regex_match(str, regex("show versions;")))
        return ShowAllVersions;

    if (regex_match(str, desc_table))
    {
        return DescTable;
    }

    if (regex_match(str, show_Db))
    {
        return ShowDatabase;
    }

    if (regex_match(str, del_index))
    {
        return DeleteIndex;
    }

    if (regex_match(str, create_index))
    {
        return CreateIndex;
    }

    if (regex_match(str, show_tables))
        return ShowTables;

    if (regex_match(str, Pcreate))
    {
        return CreateState;
    }
    else if (regex_match(str, Pinsert))
    {
        return InsertState;
    }
    else if (regex_match(str, Pdelete))
    {
        return DeleteState;
    }
    else if (regex_match(str, Pupdate))
    {
        return UpdateState;
    }
    else if (regex_match(str, Pdrop))
    {
        return DropState;
    }
    else if (regex_match(str, Palter))
    {
        return AlertState;
    }
    else if (regex_match(str, Pselect))
    {
        return SelectState;
    }
    else if (regex_match(str, create_DB))
    {
        return CreateDatabase;
    }
    else if (regex_match(str, using_DB))
    {
        return UsingDB;
    }
    else
        return UnknownState;
}

vector<string> sqlCompiler::preCreate(const string &s)
{
    string str = s;
    string name = "(?:(create table ))(.+?)(?=\\()";
    string text = "(?:\\().+(?=\\))";
    string attr = ",";
    string jud = "(?:.+),(?=\\))";
    regex create_1(name);
    regex create_2(text);
    regex create_3(attr);
    regex judge(jud);
    smatch csm1;
    smatch csm2;
    smatch csm3;
    smatch csm4;
    string table_name = "";
    string parameter = "";
    vector<string> ret;
    if (regex_search(str, csm4, judge))
    {
        //        cout << "','输入错误！！！" << endl;
        throw MyException("','Input error.");
    }
    ret.emplace_back(to_string(distance(std::sregex_iterator(str.begin(), str.end(), create_3), std::sregex_iterator()) + 1));
    if (regex_search(str, csm1, create_1))
    {
        string temp = csm1[0];
        for (int i = 13; i < temp.length(); i++)
        {
            if (temp[i] == ' ' && i + 1 < temp.length())
                return {};
            table_name += temp[i];
        }
        ret.emplace_back(table_name);
    }
    else
    {
        //        cout << "请检查语句是否正确";
        throw MyException("Please check your command.");
    }
    if (regex_search(str, csm2, create_2))
    {
        string temp = csm2[0];
        int begin = 1;
        temp = del_space(temp);
        if (temp[1] == ' ')
            begin++;
        for (int i = begin; i < temp.length(); i++)
        {
            //            if ((temp[i+1]==')'||temp[i] != ' ' || temp[i+1] == '(' || (temp[i+1] > '0' && temp[i+1] < '9')) && temp[i] != ','){
            //                if(temp[i+1] == '(' && temp[i] == ' ') i++;
            //                parameter += temp[i];
            if (temp[i] != ' ' && temp[i] != ',')
            {
                parameter += temp[i];
            }
            else if (!parameter.empty())
            {
                ret.emplace_back(parameter);
                parameter = "";
            }
            if (temp[i] == ',')
                ret.emplace_back("|");
        }
        if (!parameter.empty())
            ret.emplace_back(parameter);
    }
    else
    {
        //        cout << "请检查括号中的数据是否正确";
        throw MyException("Please check that the data in parentheses is correct.");
    }
    return ret;
}

vector<string> sqlCompiler::preAlter(const string &s)
{
    string str = s;
    vector<string> ret;
    regex table_name("(?:table ).+\\s((?=modify)|(?=modify column)|(?=add)|(?=drop))");
    regex column("(?:modify ).+\\s((?=drop)|(?=set))");
    regex attributes("(?:add\\s?\\().+?(?=\\)\\s?;)");
    regex modify_col("(?:modify column).+(?=;)");
    smatch sma0;
    smatch sma1;
    smatch sma2;
    smatch sma3;
    smatch sma4;
    smatch sma5;

    bool if_satisfy = false; // 是否满足modify column_name set/drop ...

    if (regex_search(str, sma1, table_name))
    {
        string temp = sma1[0];
        string name = "";
        for (int i = 6; i < temp.length(); i++)
        {
            if (temp[i] != ' ')
                name += temp[i];
            else if (!name.empty())
            {
                ret.emplace_back(name);
                name = "";
                break;
            }
        }
        if (!name.empty())
            ret.emplace_back(name);
    }
    else
    {
        //        cout << "请检查表达式alter table是否正确";
        throw MyException("Please check alter table statement is correct.");
    }

    // 获取操作类型和表名
    if (regex_search(str, sma2, column))
    {
        string temp = sma2[0];
        if_satisfy = true;
        string columns = "";
        for (int i = 0; i < temp.length(); i++)
        {
            if (temp[i] != ' ' && temp[i] != ',')
                columns += temp[i];
            else if (!columns.empty())
            {
                ret.emplace_back(columns);
                columns = "";
            }
        }
        if (!columns.empty())
            ret.emplace_back(columns);
    }
    // alter的modify column drop操作
    if (if_satisfy)
    {
        regex after_modify("((?:set )|(?:drop )).+(?=;)");
        if (regex_search(str, sma3, after_modify))
        {
            string temp = sma3[0];
            string operate = "";
            for (int i = 0; i < temp.length(); i++)
            {
                if (temp[i] != ' ' && temp[i] != ',')
                    operate += temp[i];
                else if (!operate.empty())
                {
                    ret.emplace_back(operate);
                    operate = "";
                }
            }
            if (!operate.empty())
                ret.emplace_back(operate);
        }
    }
    // alter的添加列
    if (regex_search(str, sma5, attributes))
    {
        string temp = sma5[0];
        string attr = "";
        ret.emplace_back("add");
        temp = del_space(temp);
        for (int j = 4; j < temp.length(); j++)
        {
            if (temp[j] != ' ')
                attr += temp[j];
            else if (!attr.empty())
            {
                ret.emplace_back(attr);
                attr = "";
            }
            if (temp[j] == ',')
                throw MyException("Input error ',' ");
        }
        if (!attr.empty())
            ret.emplace_back(attr);
    }

    // alter的drop列
    regex drop_col("(?:" + ret[0] + " drop).+(?=;)");
    if (regex_search(s, sma0, drop_col))
    {
        int jud_exceed = 0;
        string temp = sma0[0];
        string col_name = "";
        for (int i = ret[0].length(); i < temp.length(); i++)
        {
            if (jud_exceed >= 2)
                throw MyException("Only one value can be entered at a time.");
            if (temp[i] != ' ')
                col_name += temp[i];
            else if (!col_name.empty())
            {
                ret.emplace_back(col_name);
                col_name = "";
                jud_exceed++;
            }
        }
        if (!col_name.empty())
            ret.emplace_back(col_name);
    }

    // modify column
    if (regex_search(s, sma4, modify_col))
    {
        ret.emplace_back("modify_column");
        string temp = del_space(sma4[0]);
        string temp_sma = "";
        for (int i = 13; i < temp.length(); i++)
        {
            if (temp[i] != ' ')
                temp_sma += temp[i];
            else if (!temp_sma.empty())
            {
                ret.emplace_back(temp_sma);
                temp_sma = "";
            }
        }
        if (!temp_sma.empty())
        {
            ret.emplace_back(temp_sma);
        }
    }

    // 删除除主键外的所有索引
    regex del_all_index("drop index");
    if (regex_search(s, sma1, del_all_index))
        ret.emplace_back("drop_index");
    return ret;
}

vector<string> sqlCompiler::preDelete(const string &s)
{
    string str = s;
    vector<string> ret;
    regex table_name("(?:delete from ).+?((?=where)|(?=;))");
    regex wheres("(?:where ).+?(?=;)");
    smatch sma1;
    smatch sma2;
    if (regex_search(str, sma1, table_name))
    {
        string temp = sma1[0];
        string columns = "";
        for (int i = 11; i < temp.length(); i++)
        {
            if ((temp[i] != ' ' || temp[i + 1] == '=') && temp[i] != ',')
                columns += temp[i];
            else if (!columns.empty())
            {
                ret.emplace_back(columns);
                columns = "";
            }
        }
        if (!columns.empty())
            ret.emplace_back(columns);
    }
    else
        return {};
    if (regex_search(str, sma2, wheres))
    {
        string temp = sma2[0];
        string columns = "";
        for (int i = 5; i < temp.length(); i++)
        {
            if ((temp[i] != ' ' || temp[i + 1] == '=' || temp[i - 1] == '=') && temp[i] != ',')
                columns += temp[i];
            else if (!columns.empty())
            {
                ret.emplace_back(columns);
                columns = "";
            }
        }
        if (!columns.empty())
            ret.emplace_back(columns);
    }
    return ret;
}

vector<string> sqlCompiler::preDrop(const string &s)
{
    string str = s;
    vector<string> ret;
    regex table_name("(?:drop table).+?((?=;)|(?=restrict)|(?=cascade))");
    regex cons("(cascade|restrict)");
    smatch sma1;
    smatch sma2;
    if (regex_search(str, sma1, table_name))
    {
        string temp = sma1[0];
        string ans = "";
        for (int i = 10; i < temp.length(); i++)
        {
            if (temp[i] != ' ' && temp[i] != ',')
                ans += temp[i];
            else if (!ans.empty())
            {
                ret.emplace_back(ans);
                ans = "";
            }
        }
        if (!ans.empty())
            ret.emplace_back(ans);
    }
    else
    {
        //        cout << "请检查表达式drop table是否正确";
        throw MyException("Please check drop table statement is correct.");
    }
    if (regex_search(s, sma2, cons))
    {
        ret.emplace_back(sma2[0]);
    }
    return ret;
}

vector<string> sqlCompiler::preInsert(const string &s) // 返回类型为 1 tablename 2 columns 3 values
{
    string str = s;
    vector<string> ret;
    regex table_name("(?:into).+?((?=\\()|(?=values))");
    regex values_1("((?=values\\()).+?(?=\\))");
    regex values_2("((?=values \\()).+?(?=\\))");
    smatch sma1;
    smatch sma2;
    smatch sma3;
    smatch sma4;
    string tablename = "";
    // 获取表名
    if (regex_search(str, sma1, table_name))
    {
        string temp = sma1[0];
        string ans = "";
        ret.emplace_back("1!@");
        for (int i = 4; i < temp.length(); i++)
        {
            if (temp[i] != ' ' && temp[i] != ',')
                ans += temp[i];
            else if (!ans.empty())
            {
                ret.emplace_back(ans);
                tablename = ans;
                ans = "";
            }
        }
        if (!ans.empty())
        {
            ret.emplace_back(ans);
            tablename = ans;
        }
    }
    else
    {
        //        cout << "请检查表达式insert into是否正确";
        throw MyException("Please check insert into statement is correct.");
    }
    string col = "(?:" + tablename + ")\\s?\\(.+?(?=\\))";
    regex columns(col);

    // 获取columns
    if (regex_search(str, sma2, columns) && !tablename.empty())
    {
        string temp = sma2[0];
        string column = "";
        ret.emplace_back("2!@");
        bool jud = false;
        for (int i = 0; i < temp.length(); i++)
        {
            if (jud)
            {
                if (temp[i] != ' ' && temp[i] != ',')
                    column += temp[i];
                else if (!column.empty())
                {
                    ret.emplace_back(column);
                    column = "";
                }
            }
            if (temp[i] == '(')
                jud = true;
        }
        if (!column.empty())
            ret.emplace_back(column);
    }
    // 获取values
    if (regex_search(str, sma3, values_1))
    {
        string temp = sma3[0];
        string ans = "";
        ret.emplace_back("3!@");
        for (int i = 7; i < temp.length(); i++)
        {
            if (temp[i] != ' ' && temp[i] != ',')
                ans += temp[i];
            else if (!ans.empty())
            {
                ret.emplace_back(ans);
                ans = "";
            }
        }
        if (!ans.empty())
            ret.emplace_back(ans);
    }
    else
    {
        //        cout << "请检查表达式values是否正确";
        throw MyException("Please check values statement is correct.");
    }

    // 获取values
    if (regex_search(str, sma4, values_2))
    {
        string temp = sma4[0];
        string value = "";
        ret.emplace_back("3!@");
        for (int i = 7; i < temp.length(); i++)
        {
            if (temp[i] != ' ' && temp[i] != ',')
                value += temp[i];
            else if (!value.empty())
            {
                ret.emplace_back(value);
                value = "";
            }
        }
        if (!value.empty())
            ret.emplace_back(value);
    }
    return ret;
}

vector<string> sqlCompiler::preSelect(const string &s)
{
    // 返回类型1 属性名 2 表名 3 where子句 4 group by子句 5 order by子句
    string str = s;
    // 设立正则表达式
    vector<string> ret;
    string column_name = "(?:select\\s?).+?(?=\\s?from)";
    string table_name = "((?:from ).+?(?= where))|((?:from ).+?(?=;)|(?= group by)|(?= order by))";
    string where = "(?:where ).+?((?=;)|(?=group by)|(?=order by))";
    string group_by = "(?:group by).+?((?=;)|(?= order by))";
    string order_by = "(?:order by).+?(?=;)";
    // 初始化正则
    regex columnname(column_name);
    regex tablename(table_name);
    regex wheres(where);
    regex groupby(group_by);
    regex orderby(order_by);

    // 设立匹配
    smatch sma1;
    smatch sma2;
    smatch sma3;
    smatch sma4;
    smatch sma5;
    // 获取属性名
    if (regex_search(s, sma1, columnname))
    {
        string temp = sma1[0];
        string columns = "";
        ret.emplace_back("1!@");
        for (int i = 6; i < temp.length(); i++)
        {
            if (temp[i] != ' ' && temp[i] != ',')
                columns += temp[i];
            else if (!columns.empty())
            {
                ret.emplace_back(columns);
                columns = "";
            }
        }
        if (!columns.empty())
            ret.emplace_back(columns);
    }
    // 获取表名
    if (regex_search(s, sma2, tablename))
    {
        string temp = sma2[0];
        string name = "";
        ret.emplace_back("2!@");
        for (int i = 5; i < temp.length(); i++)
        {
            if (temp[i] != ' ' && temp[i] != ',')
                name += temp[i];
            else if (!name.empty())
            {
                ret.emplace_back(name);
                name = "";
            }
        }
        if (!name.empty())
            ret.emplace_back(name);
    }
    // 获取where子句
    if (regex_search(s, sma3, wheres))
    {
        string temp = sma3[0];
        string name = "";
        ret.emplace_back("3!@");
        for (int i = 6; i < temp.length(); i++)
        {
            if ((temp[i] != ' ' || temp[i + 1] == '=' || temp[i - 1] == '=' || temp[i + 1] == '<' || temp[i - 1] == '<' || temp[i + 1] == '>' || temp[i - 1] == '>') && temp[i] != ',' && name != "and" && name != "or")
            {
                if (temp[i] == ' ')
                    i++;
                name += temp[i];
            }
            else if (!name.empty())
            {
                ret.emplace_back(name);
                name = "";
            }
        }
        if (!name.empty())
            ret.emplace_back(name);
    }
    // 获取group by子句
    if (regex_search(s, sma4, groupby))
    {
        string temp = sma4[0];
        string name = "";
        ret.emplace_back("4!@");
        for (int i = 8; i < temp.length(); i++)
        {
            if (temp[i] != ' ' && temp[i] != ',')
                name += temp[i];
            else if (!name.empty())
            {
                ret.emplace_back(name);
                name = "";
            }
        }
        if (!name.empty())
            ret.emplace_back(name);
    }
    // 获取order by子句
    if (regex_search(s, sma5, orderby))
    {
        string temp = sma5[0];
        string name = "";
        ret.emplace_back("6!@");
        for (int i = 8; i < temp.length(); i++)
        {
            if (temp[i] != ' ' && temp[i] != ',')
                name += temp[i];
            else if (!name.empty())
            {
                ret.emplace_back(name);
                name = "";
            }
        }
        if (!name.empty())
            ret.emplace_back(name);
    }

    return ret;
}

vector<string> sqlCompiler::createDB(const string &s)
{
    vector<string> ret;
    regex createdb("database .+(?=;)");
    smatch csma;
    if (regex_search(s, csma, createdb))
    {
        string temp = csma[0];
        string dbname = "";
        for (int i = 8; i < temp.length(); i++)
        {
            if (temp[i] != ' ')
                dbname += temp[i];
            else if (!dbname.empty())
            {
                ret.emplace_back(dbname);
                dbname = "";
            }
        }
        if (!dbname.empty())
            ret.emplace_back(dbname);
    }
    return ret;
}

vector<string> sqlCompiler::preUpdate(const string &s)
{
    string str = s;
    vector<string> ret;
    string table_name = "(?:update ).+?(?= set)";
    string values = "(?:set).+?((?=where)|(?=;))";
    string wheres = "(?:where).+?(?=;)";
    regex tablename(table_name);
    regex value(values);
    regex where(wheres);
    smatch sma1;
    smatch sma2;
    smatch sma3;

    // 获取表名
    if (regex_search(s, sma1, tablename))
    {
        string temp = sma1[0];
        string name = "";
        for (int i = 7; i < temp.length(); i++)
        {
            if (temp[i] != ' ')
                name += temp[i];
            else if (!name.empty())
            {
                ret.emplace_back(name);
                name = "";
            }
        }
        if (!name.empty())
            ret.emplace_back(name);
    }
    else
    {
        //        cout << "请检查表达式update是否正确";
        throw MyException("Please check update statement is correct.");
    }
    // 获取new values
    if (regex_search(s, sma2, value))
    {
        string temp = sma2[0];
        string name = "";
        for (int i = 3; i < temp.length(); i++)
        {
            if ((temp[i] != ' ' || temp[i + 1] == '=' || temp[i - 1] == '=') && temp[i] != ',')
                name += temp[i];
            else if (!name.empty())
            {
                ret.emplace_back(name);
                name = "";
            }
        }
        if (!name.empty())
            ret.emplace_back(name);
    }
    else
    {
        //        cout << "请检查表达式set是否正确";
        throw MyException("Please check your set statement.");
    }
    // 获取where后面的条件
    if (regex_search(s, sma3, where))
    {
        string temp = sma3[0];
        string name = "";
        ret.emplace_back("|"); // 用于分隔set和where
        for (int i = 5; i < temp.length(); i++)
        {
            if (temp[i] != ' ' || temp[i + 1] == '=' || temp[i - 1] == '=')
                name += temp[i];
            else if (!name.empty())
            {
                ret.emplace_back(name);
                name = "";
            }
        }
        if (!name.empty())
            ret.emplace_back(name);
    }
    else
    {
        //        cout << "请检查表达式where是否正确";
        throw MyException("Please check your expression of where is correct.");
    }
    return ret;
}

vector<string> sqlCompiler::usingDB(const string &s)
{
    vector<string> ret;
    regex usingdb("use .+(?=;)");
    smatch csma;
    if (regex_search(s, csma, usingdb))
    {
        string temp = csma[0];
        string dbname = "";
        for (int i = 4; i < temp.length(); i++)
        {
            if (temp[i] != ' ')
                dbname += temp[i];
            else if (!dbname.empty())
            {
                ret.emplace_back(dbname);
                dbname = "";
            }
        }
        if (!dbname.empty())
            ret.emplace_back(dbname);
    }
    return ret;
}

vector<string> sqlCompiler::showDb(const string &s)
{
    vector<string> ret;
    regex showdb("show .+(?=;)");
    smatch csma;
    if (regex_search(s, csma, showdb))
    {
        string temp = csma[0];
        string dbname = "";
        for (int i = 4; i < temp.length(); i++)
        {
            if (temp[i] != ' ')
                dbname += temp[i];
            else if (!dbname.empty())
            {
                ret.emplace_back(dbname);
                dbname = "";
            }
        }
        if (!dbname.empty())
            ret.emplace_back(dbname);
    }
    return ret;
}

string sqlCompiler::del_space(string result)
{
    for (int i = 0; i < result.length(); i++)
        if (result[i] == ' ')
        {
            if (result[i - 1] == '(' || result[i + 1] == ')' || result[i + 1] == '(')
                result.erase(i, 1);
        }
    return result;
}

vector<string> sqlCompiler::createIndex(const string &s)
{
    vector<string> ret;
    regex create_index_name("(?:on).+(?=\\()");
    regex create_index_col("(?:\\().+(?=\\))");
    smatch scma1;
    smatch scma2;
    // 获取表名
    if (regex_search(s, scma1, create_index_name))
    {
        string temp = scma1[0];
        string tablename = "";
        for (int i = 2; i < temp.length(); i++)
        {
            if (temp[i] != ' ')
                tablename += temp[i];
        }
        ret.emplace_back(tablename);
    }
    // 获取列表
    if (regex_search(s, scma2, create_index_col))
    {
        int num = ret.size(); // 标明现在的ret向量的大小
        string temp = scma2[0];
        temp = del_space(temp);                   // 去除多余空格
        temp = temp.substr(1, temp.length() - 1); // 去除左括号
        if (temp.find(',') != string::npos)
        {
            regex re(","); // 按逗号进行分割
            sregex_token_iterator iter(temp.begin(), temp.end(), re, -1), end;
            string column = "";
            while (iter != end)
            {
                column = *iter++;
                string::iterator index_pos = remove(column.begin(), column.end(), ' ');
                column.erase(index_pos, column.end());
                if (!column.empty())
                    ret.emplace_back(column);
            }
            if (ret.size() - num != count(temp.begin(), temp.end(), ',') + 1)
                throw MyException("There are too many or  ‘,’.");
        }
        else
        {
            ret.emplace_back(temp);
        }
    }
    return ret;
}

vector<string> sqlCompiler::delIndex(const string &s)
{
    vector<string> ret;
    regex del_index_name("(?=delete index ).+(?=\\()");
    regex del_index_col("(?=\\(\\s?).+(?=\\))");
    smatch csma1;
    smatch csma2;
    if (regex_search(s, csma1, del_index_name))
    {
        string temp = csma1[0];
        string tablename = "";
        for (int i = 12; i < temp.length(); i++)
        {
            if (temp[i] != ' ')
                tablename += temp[i];
        }
        ret.emplace_back(tablename);
    }
    else
        throw MyException("Please check your command.");
    if (regex_search(s, csma2, del_index_col))
    {
        string temp = csma2[0];
        string col = "";
        for (int i = 1; i < temp.length(); i++)
        {
            if (temp[i] != ' ' && temp[i] != ',')
                col += temp[i];
            if (temp[i] == ',' && !col.empty())
            {
                ret.emplace_back(col);
                col = "";
            }
        }
        if (!col.empty())
            ret.emplace_back(col);
    }
    return ret;
}

vector<string> sqlCompiler::descTable(const string &s)
{
    string tableName = "";
    vector<string> ret;
    regex desc_name("(?:desc).+(?=;)");
    smatch scma;
    if (regex_search(s, scma, desc_name))
    {
        string temp = scma[0];
        for (int i = 4; i < temp.length(); i++)
        {
            if (temp[i] != ' ')
                tableName += temp[i];
        }
    }
    if (tableName.empty())
        throw MyException("Please check your command");
    ret.emplace_back(tableName);
    return ret;
}

vector<string> sqlCompiler::commit(const string &s)
{
    vector<string> ret;
    regex commit_("(?=\"\\s?).+(?=\")");
    smatch csma;
    if (regex_search(s, csma, commit_))
    {
        string temp = csma[0];
        string temp_content = "";
        ret.emplace_back(temp.substr(1, temp.length() - 1));
    }
    return ret;
}

vector<string> sqlCompiler::recover(const string &s)
{
    vector<string> ret;
    regex recover("(?:\\s).+_[0-9]+(?=\\s?;)");
    smatch scma;
    if (regex_search(s, scma, recover))
    {
        string temp = scma[0];
        ret.emplace_back(temp.substr(1, temp.length() - 1));
    }
    return ret;
}

vector<string> sqlCompiler::rollback(const string &s)
{
    vector<string> ret;
    regex roll_back("(?=\\s).+_[0-9]+(?=\\s)");
    regex version("[0-9]+(?=\\s?;)");
    smatch csma;
    string temp = "";
    if (regex_search(s, csma, roll_back))
    {
        temp = csma[0];
        ret.emplace_back(temp.substr(1, temp.length() - 1));
    }
    else
        throw MyException("Please check your command.");
    if (regex_search(s, csma, version))
        ret.emplace_back(csma[0]);
    else
        throw MyException("Please check your command.");
    return ret;
}

vector<string> sqlCompiler::delete_db(const string &s)
{
    regex db_name("(?:database).+(?=;)");
    smatch csma;
    vector<string> ret;
    if (regex_search(s, csma, db_name))
    {
        string temp = csma[0];
        string name = "";
        for (int i = 8; i < temp.length(); i++)
        {
            if (temp[i] != ' ')
                name += temp[i];
            else if (!name.empty())
            {
                ret.emplace_back(name);
                name = "";
            }
        }
        if (!name.empty())
            ret.emplace_back(name);
    }
    return ret;
}
