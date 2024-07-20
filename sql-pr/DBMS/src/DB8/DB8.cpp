//
// Created by BlackCat on 2023/3/10.
//

#include "DB8.h"
#include "iostream"
#include "dirent.h"
#include "MyException.h"
using std::memset;

string DB8::start(string command, bool whether_log)
{
    if (command.find(';') != string::npos) // 如果找到了分号
    {
        // 执行代码,后续添加方法

        // 退出设置
        int pre = 0;
        // 清楚开头的字符串
        while (command[pre] == ' ')
        {
            pre++;
        }
        string commandge = command.substr(pre, command.length() - pre);
        command = processStringSpace(commandge);
        if (whether_log)
        {
            logManager.write_record(command);
            logManager.set_checkpoint();
        }
        if (commandge == "exit;")
        {
            logManager.remove_checkpoint();
            cache->clear(); // 清空缓存区
            if (catalog != nullptr)
                catalog->writeCatalog(); // 某个析构函数
            exit(0);
        }

        // 判断当前输入值是否合法
        regex prejudge("\\s{2,}");
        smatch csma;
        if (regex_search(command, csma, prejudge))
        {
            return R"(0\r\nDO NOT ENTRE TOO MANY SPACE\r\n)";
        }

        int select = test.segmentation(command);
        vector<string> ans; // 用于接收解析语句
        Table *newtable = nullptr;
        vector<Attribute> attributes;
        int begin = 2;

        if (catalog == nullptr && select < 14)
            return R"(0\r\nselect your database first\r\n)";
        try
        {
            switch (select)
            {
            case 0:
                ans = test.preInsert(command);

                if (!ans.empty() && find(ans.begin(), ans.end(), "3!@") != ans.end())
                {
                    // select snow,cno from sc where sno = 1 having

                    vector<string> turple;
                    int i = 0;
                    int begin_cols = find(ans.begin(), ans.end(), "2!@") - ans.begin(); // 属性的开始下标
                    int begin_vals = find(ans.begin(), ans.end(), "3!@") - ans.begin(); // 值的开始下标

                    Table *t = judExit(ans[1]);
                    if (t != nullptr)
                    {
                        if (find(ans.begin(), ans.end(), "2!@") == ans.end() && ans.size() - 1 - begin_vals < t->attributeNum)
                            return R"(0\r\nvalue missed\r\n)";
                        else if (find(ans.begin(), ans.end(), "2!@") == ans.end() && ans.size() - 1 - begin_vals > t->attributeNum)
                            return R"(0\r\n too many values! \r\n)";
                        if (find(ans.begin(), ans.end(), "2!@") != ans.end() && ans.size() - begin_vals < begin_vals - begin_vals)
                            return R"(0\r\nvalue missed\r\n)";
                        else if (find(ans.begin(), ans.end(), "2!@") != ans.end() && ans.size() - begin_vals > begin_vals - begin_cols)
                            return R"(0\r\n too many values! \r\n)";
                        if (begin_cols != ans.size()) // ans中包含columns
                        {
                            int col_exit[begin_vals - begin_cols - 1]; // 每一个位置的值代表当前输入的列名
                            memset(col_exit, 1, sizeof(col_exit));     // 将数组初始化为1，代表输入的列名全部都存在
                            int count = 0;                             // 记录attributes的下标
                            for (Attribute a_f : t->attributes)
                            {
                                for (int j = begin_cols + 1, k = begin_vals + 1; j < begin_vals && k < ans.size(); j++, k++)
                                {
                                    if (a_f.name == ans[j])
                                    {
                                        int leng = ans[k].size();
                                        if (t->attributes[count].type == 2 || t->attributes[count].type == 3)
                                            leng -= 2;
                                        if (leng > t->attributes[count].len && t->attributes[count].type != 3)
                                            return R"(0\r\nOverlength.\r\n)";
                                        turple.emplace_back(match_type_column(t, ans[k], count)); // 将对应的colum值压入vector中
                                        col_exit[j - begin_cols - 1] = 0;                         // 将找到对应列的值设为0；
                                    }
                                }
                                count++;
                            }
                            string lack_name = "";
                            for (int j = 0; j < begin_vals - begin_cols - 1; j++)
                                if (col_exit[j] != 0)
                                    lack_name += R"(0\r\n columns:)" + ans[j + begin_cols + 1] + R"(not exist!  \r\n)"; // 代表该列并没有被找到
                            if (!lack_name.empty())
                                return lack_name;
                        }
                        else
                        {
                            int count = 0;
                            for (int k = begin_vals + 1; k < ans.size(); k++)
                            {
                                int leng = ans[k].size();
                                if (t->attributes[count].type == 2 || t->attributes[count].type == 3)
                                    leng -= 2;
                                if (leng > t->attributes[count].len && t->attributes[count].type != 3)
                                    return R"(0\r\nOverlength.\r\n)";
                                turple.emplace_back(match_type_column(t, ans[k], count++));
                            }
                        }
                        if (t->insert(turple))
                        {
                            return R"(1\r\ninsert success fully!\r\n)";
                        }
                        break;
                    }
                    else
                        return R"(0\r\nerror when do )" + ans[1] + R"(\r\n)";
                }
                else
                    return R"(0\r\nenter apppropriate value\r\n)";
                break;
            case 1:
                ans = test.preDelete(command);
                if (!ans.empty())
                {
                    Table *t = judExit(ans[0]);
                    if (t != nullptr) // 查找对应的表
                    {
                        vector<string> submit; // 用于传输给selectequal查询值
                        submit.emplace_back("1!@");
                        submit.emplace_back("*");
                        submit.emplace_back("2!@");
                        submit.emplace_back(ans[0]);

                        if (ans.size() == 1)
                        { // 判断类型是否为delete from tablename;
                            // todo 删除全部的值
                            t->removeAll();
                        }
                        else
                        {
                            // todo 特定的值
                            vector<string> del;
                            vector<string> get_after_clean;
                            submit.emplace_back("3!@");
                            for (int i = 1; i < ans.size(); i++)
                            {
                                if (ans[i].find(">=") != -1)
                                {
                                    get_after_clean = clean(ans[i], ">=");
                                    submit.emplace_back(get_after_clean[0] + ">=" + get_after_clean[1]);
                                }
                                else if (ans[i].find("<=") != -1)
                                {
                                    get_after_clean = clean(ans[i], "<=");
                                    submit.emplace_back(get_after_clean[0] + "<=" + get_after_clean[1]);
                                }
                                else if (ans[i].find("=") != -1)
                                {
                                    get_after_clean = clean(ans[i], "=");
                                    submit.emplace_back(get_after_clean[0] + "=" + get_after_clean[1]);
                                }
                                else if (ans[i].find("<") != -1)
                                {
                                    get_after_clean = clean(ans[i], "<");
                                    submit.emplace_back(get_after_clean[0] + "<" + get_after_clean[1]);
                                }
                                else if (ans[i].find(">") != -1)
                                {
                                    get_after_clean = clean(ans[i], ">");
                                    submit.emplace_back(get_after_clean[0] + ">" + get_after_clean[1]);
                                }
                            }

                            vector<vector<string>> ans_from_select = select_one_table(submit);
                            if (!ans_from_select.empty())
                                ans_from_select.erase(ans_from_select.begin()); // 擦除第一行
                            else
                                return R"(0/r/nno values.\r\n)";
                            if (!ans.empty())
                            {
                                for (vector<string> t1 : ans_from_select)
                                    if (!t->remove(t1))
                                        return t1[0] + R"(0\r\nremove fale\r\n)";
                                return R"(1\r\nremove successfullt\r\n)";
                            }
                            else
                                return R"(0\r\nerror no values\r\n)";
                        }
                        break;
                    }
                    else
                        return R"(0\r\nccreate table first\r\n)";
                }
                break;
            case 2:
                ans = test.preCreate(command);
                if (!ans.empty())
                {
                    Table *t = judExit(ans[1]);
                    if (t == nullptr)
                    {
                        int rank = 0;
                        newtable = new Table(ans[1], 0, stoi(ans[0]), 0);
                        for (int i = 0; i < stoi(ans[0]); i++)
                        {
                            Attribute attribute;
                            int j = begin;
                            while (j < ans.size() && ans[j] != "|")
                            {
                                attribute.name = ans[j++];
                                string type = "";
                                string len = "";
                                for (int k = 0; k < ans[j].length(); k++)
                                {
                                    if (ans[j][k] >= 'a' && ans[j][k] <= 'z')
                                        type += ans[j][k];
                                    else if (ans[j][k] >= '0' && ans[j][k] <= '9')
                                    {
                                        len += ans[j][k];
                                    }
                                }
                                j++;
                                attribute.rank = rank++;
                                if (type != "date")
                                    attribute.len = stoi(len);
                                if (type == "int")
                                    attribute.type = 1;
                                if (type == "char")
                                    attribute.type = 2;
                                if (type == "date")
                                {
                                    attribute.type = 3;
                                    attribute.len = 20;
                                }
                                if (type == "double")
                                    attribute.type = 4;
                                if (j < ans.size() && ans[j] == "primary")
                                {
                                    attribute.primaryKey = 1;
                                    j += 2;
                                }
                                else if (j < ans.size() && ans[j] == "unique")
                                {
                                    attribute.unique = 1;
                                    j++;
                                }
                                else if (j < ans.size() && ans[j] == "default")
                                {
                                    attribute.defaultValue = ans[j + 1];
                                    j += 2;
                                }
                                else if (j < ans.size() && ans[j] == "references")
                                {
                                    attribute.foreignKey = ans[j + 1].substr(0, ans[j + 1].find("(")) + "_" + ans[j + 1].substr(ans[j + 1].find("(") + 1, ans[j + 1].find(")") - ans[j + 1].find("(") - 1);
                                }
                                else
                                    continue;
                            }
                            attributes.emplace_back(attribute);
                            begin = j + 1;
                        }
                        newtable->attributes = attributes;
                        catalog->insertTable(newtable);
                        catalog->writeCatalog();
                    }
                    else
                        return R"(0\r\ntable already exists\r\n)";
                }
                else
                    return R"(0\r\nspaces in sentence\r\n)";
                break;
            case 3:
                ans = test.preUpdate(command);
                if (!ans.empty())
                {
                    int jud = 0;
                    for (Table *t : catalog->tableRecord)
                    {
                        if (t->tableName == ans[0])
                        {
                            vector<string> submit;
                            submit.emplace_back("1!@");
                            submit.emplace_back("*");
                            submit.emplace_back("2!@");
                            submit.emplace_back(ans[0]);
                            vector<string> after_clean_value = clean(ans[1], "="); // set部分的值

                            int index_col = 0; // 获取需要修改的属性的下标
                            for (int k = 0; k < t->attributes.size(); k++)
                                if (t->attributes[k].name == after_clean_value[0])
                                {
                                    index_col = k;
                                    break;
                                }

                            if (find(ans.begin(), ans.end(), "|") == ans.end())
                            {
                                vector<vector<string>> oldValues = select_one_table(submit);
                                vector<string> newValues;
                                oldValues.erase(oldValues.begin());
                                if (!oldValues.empty())
                                {
                                    oldValues.erase(oldValues.begin()); // 擦除第一列
                                    for (vector<string> tt : oldValues)
                                    {
                                        for (int j = 0; j < t->attributes.size(); j++)
                                        {
                                            if (j == index_col)
                                                newValues.emplace_back(after_clean_value[1]);
                                            else
                                                newValues.emplace_back(tt[j]);
                                        }
                                        t->update(tt, newValues);
                                    }
                                    return R"(0\r\nsuccessfully\r\n)";
                                }
                                else
                                    return R"(0\r\n0 columns\r\n)";
                            }
                            else
                            {
                                // 指定更新
                                vector<string> after_clean_2; // where部分的值
                                vector<vector<string>> oldvalues;
                                vector<string> newvalues;
                                vector<vector<string>> get_select;
                                int begin_where = find(ans.begin(), ans.end(), "|") - ans.begin();
                                submit.emplace_back("3!@");
                                for (int i = begin_where + 1; i < ans.size(); i++)
                                {
                                    if (ans[i].find("=") != -1)
                                    {
                                        after_clean_2 = clean(ans[i], "=");
                                        submit.emplace_back(after_clean_2[0] + "=" + after_clean_2[1]);
                                    }
                                    else if (ans[i].find(">=") != -1)
                                    {
                                        after_clean_2 = clean(ans[i], ">=");
                                        submit.emplace_back(after_clean_2[0] + ">=" + after_clean_2[1]);
                                    }
                                    else
                                    {
                                        after_clean_2 = clean(ans[i], "<=");
                                        submit.emplace_back(after_clean_2[0] + "<=" + after_clean_2[1]);
                                    }
                                }
                                oldvalues = select_one_table(submit);
                                oldvalues.erase(oldvalues.begin());
                                if (!oldvalues.empty())
                                {
                                    oldvalues.erase(oldvalues.begin());
                                    for (vector<string> tt : oldvalues)
                                    {
                                        for (int j = 0; j < t->attributes.size(); j++)
                                        {
                                            if (j == index_col)
                                                newvalues.emplace_back(after_clean_value[1]);
                                            else
                                                newvalues.emplace_back(tt[j]);
                                        }
                                        string priName = ans[0] + "_";
                                        string uniName = ans[0] + "_";
                                        variableType pri, uni;
                                        for (int i = 0; i < attributes.size(); i++)
                                        {
                                            if (attributes[i].primaryKey == true)
                                            {
                                                priName += attributes[i].name + "_";
                                                pri.type.push_back(attributes[i]);
                                                pri.value.push_back(newvalues[i]);
                                            }
                                            if (attributes[i].unique == true)
                                            {
                                                uniName += attributes[i].name + "_";
                                                uni.type.push_back(attributes[i]);
                                                uni.value.push_back(newvalues[i]);
                                            }
                                        }
                                        if (t->BPlusTreeMap.find(priName) != t->BPlusTreeMap.end() &&
                                            t->BPlusTreeMap[priName]->find(pri) != -1)
                                        {
                                            return R"(0\r\nPrimary key duplication!\r\n)";
                                        }
                                        if (t->BPlusTreeMap.find(uniName) != t->BPlusTreeMap.end() &&
                                            t->BPlusTreeMap[uniName]->find(uni) != -1)
                                        {
                                            return R"(0\r\n unique key duplication\r\n)";
                                        }

                                        return R"(1\r\n))"+t->update(tt, newvalues)+R"(\r\n)"; // 调用函数，进行更新，需要的参数 oldvalues，newvalues
                                        newvalues.clear();
                                    }
                                }
                                else
                                {
                                    return R"(0\r\n0 columns!\r\n)";
                                }
                            }
                            break;
                        }
                        else
                            jud++;
                    }
                    if (jud == catalog->tableRecord.size())
                        return R"(0\r\ncreate table first!\r\n)";
                }
                else
                    return R"(0\r\nenter appporpriate values\r\n)";
                break;
            case 4:
                ans = test.preDrop(command);
                {
                    int i = 0;
                    for (; i < catalog->tableRecord.size(); i++)
                    {
                        if (catalog->tableRecord[i]->tableName == ans[0])
                        {
                            catalog->tableRecord[i]->drop();
                            delete catalog->tableRecord[i];
                            catalog->tableRecord.erase(catalog->tableRecord.begin() + i);
                            catalog->writeCatalog();
                            return R"(0\r\nDone successfully.)";
                            break;
                        }
                    }
                    return R"(0\r\nThis table don`t exist.)";
                }
                break;
            case 5:
                ans = test.preAlter(command);
                if (!ans.empty())
                {
                    Table *t = judExit(ans[0]);
                    int judNum = 0;
                    vector<Attribute> perAlter;
                    if (t != nullptr)
                    {
                        switch (judAlter[ans[1]])
                        {
                        case 1:
                            for (Attribute attr : t->attributes)
                            {
                                if (attr.name == ans[2])
                                {
                                    if (ans[3] == "set")
                                    {
                                        switch (judConstraint[ans[4]])
                                        {
                                        case 1:
                                            attr.defaultValue = ans[5];
                                            break;
                                        case 2:
                                            attr.primaryKey = 1;
                                            if (t->createPrimaryKey(attr))
                                            {
                                                return R"(1\r\nDone successfully.)";
                                            }
                                            else
                                            {
                                                return R"(0\r\n Error,Please check your command.)";
                                            }
                                            break;
                                        case 3:
                                            attr.foreignKey = ans[5].substr(0, ans[5].find("(") + 1) + "_" + ans[5].substr(ans[5].find("(") + 1, ans[5].find(")") - ans[5].find("(") - 1);
                                        case 5:
                                            attr.unique = 1;
                                            if (t->createUnique(attr))
                                            {
                                                return R"(1\r\nDone successfully.)";
                                            }
                                            else
                                                return R"(0\r\n Error,Please check your command.)";
                                        }
                                    }
                                    else if (ans[3] == "drop")
                                    {
                                        switch (judConstraint[ans[4]])
                                        {
                                        case 1:
                                            attr.defaultValue = "";
                                            break;
                                        case 2:
                                            attr.primaryKey = 0;
                                            break;
                                        case 4:
                                            attr.foreignKey = "";
                                        case 5:
                                            attr.unique = 0;
                                        }
                                    }
                                    perAlter.emplace_back(attr);
                                    judNum++;
                                }
                                else
                                {
                                    perAlter.emplace_back(attr);
                                }
                            }
                            if (judNum == 0)
                                return R"(0\r\nThis column doesn`t exist.\r\n)";

                            t->alter(perAlter);
                            catalog->writeCatalog();
                            break;
                        case 2: // modify column tablename char(8);
                            for (Attribute attr : t->attributes)
                            {
                                if (attr.name == ans[2])
                                {
                                    string type = ans[3].substr(0, ans[3].find("("));
                                    int len = stoi(ans[3].substr(ans[3].find("(") + 1, ans[3].find(")") - ans[3].find("(") + 1));

                                    type == "int" ? attr.type = 1 : (type == "char" ? attr.type = 2 : (type == "date" ? attr.type = 3 : (type == "double" ? attr.type = 4 : attr.type = 0)));
                                    attr.len = len;
                                    if (attr.type == 0)
                                        return R"(0\r\nThere is not this type.\r\n)";

                                    perAlter.emplace_back(attr);
                                    judNum++;
                                }
                                else
                                {
                                    perAlter.emplace_back(attr);
                                }
                            }
                            // 接口
                            if (judNum == 0)
                                return R"(0\r\nThis column doesn`t exist.\r\n)";
                            t->alter(perAlter);
                            break;
                        case 3: // add new column
                            for (Attribute attr : t->attributes)
                            {
                                if (attr.name == ans[2])
                                {
                                    return R"(0\r\ncolumn already exists\r\n)";
                                }
                                else
                                    perAlter.emplace_back(attr);
                            }
                            if (true)
                            {
                                string type = ans[3].substr(0, ans[3].find("("));
                                int type_int = 0;
                                type == "int" ? type_int = 1 : (type == "char" ? type_int = 2 : (type == "date" ? type_int = 3 : (type == "double" ? type_int = 4 : type_int = 0)));
                                if (type_int == 0)
                                    return R"(0\r\nerror type.\r\n)";
                                int len = stoi(ans[3].substr(ans[3].find("(") + 1, ans[3].find(")") - ans[3].find("(") + 1));
                                if(type_int == 3) len = 20;
                                Attribute newAttr(ans[2], type_int, len);
                                // 接口
                                perAlter.emplace_back(newAttr);
                                t->alter(perAlter);
                            }
                            break;
                        case 4: // drop 列
                            for (Attribute attr : t->attributes)
                            {
                                if (attr.name != ans[2])
                                {
                                    perAlter.emplace_back(attr);
                                }
                            }
                            if (perAlter.size() == t->attributeNum - 1)
                            {
                                t->alter(perAlter);
                                return R"(1\r\nDelete successfully\r\n)";
                            }
                            return R"(0\r\n This column doesn`t exist!\r\n)";
                            break;
                        default:
                            return R"(0\r\n Unknow operator!\r\n)";
                        }
                    }
                    else
                        return R"(0\r\n This table doesn`t exist\r\n)";
                    judNum = 0;
                    perAlter.clear();
                }
                else
                    return R"(0\r\nPlease check your command \r\n)";
                break;
            case 6:
            {
                ans = test.preSelect(command);
                std::string return_name = " ";
                if (!ans.empty())
                {
                    int index_of_from = find(ans.begin(), ans.end(), "2!@") - ans.begin();
                    int index_of_tableanme = index_of_from + 1;
                    int end_of_tablename = index_of_tableanme;
                    shared_ptr<SelectStatement> ast;                                                          // 定义抽象语法树
                    while (end_of_tablename < ans.size() && ans[end_of_tablename].find("!@") == string::npos) // 到达下一个分隔符退出循环
                        end_of_tablename++;
                    vector<vector<string>> getSelect;                         // 记录获得的数据
                    vector<string> all_column_name;                           // 用于存储表的属性
                    vector<int> index_tables_col;                             // 多表查询投影的列在all_column_name中的下标
                    int num_of_table = end_of_tablename - index_of_tableanme; // 计算表的数量
                    if (num_of_table == 1)
                    { // 单表查询
                        getSelect = select_one_table(ans);
                        return_name += ans[index_of_tableanme];
                    }
                    else
                    { // 多表查询
                        // todo sql语法树
                        ast = init_ast(ans); // 生成SQL语句对应的ast
                        ast = match_where(ast);
                        // todo 多表查询接口
                        vector<vector<string>> get_after_union;
                        vector<string> temp;
                        for (int i = 0; i < ast->fromTable.size(); i++)
                        {
                            temp.emplace_back("1!@");
                            temp.emplace_back("*");
                            temp.emplace_back("2!@");
                            temp.emplace_back(ast->fromTable[i]->tableName);
                            return_name += ast->fromTable[i]->tableName;
                            return_name += " ";
                            if (!ast->fromTable[i]->where.empty())
                            {
                                temp.emplace_back("3!@");
                                for (shared_ptr<Comparison> wheres : ast->fromTable[i]->where)
                                {
                                    temp.emplace_back(wheres->left + wheres->comparison + wheres->right);
                                }
                            }
                            if (get_after_union.empty())
                            {
                                get_after_union = select_one_table(temp);
                                if (!get_after_union.empty())
                                    get_after_union.erase(get_after_union.begin());
                            }
                            else
                            {
                                vector<vector<string>> temp_values_table = select_one_table(temp); // 用于存储第二，三，四...的表返回的数据
                                if (!temp_values_table.empty())
                                    temp_values_table.erase(temp_values_table.begin());             // 擦除第一列
                                get_after_union = union_tables(get_after_union, temp_values_table); // 做表的连接
                            }
                            temp.clear();

                            for (Table *t : catalog->tableRecord)
                                if (t->tableName == ast->fromTable[i]->tableName)
                                    for (Attribute attr_temp : t->attributes)
                                        all_column_name.emplace_back(attr_temp.name);
                        }

                        for (shared_ptr<Comparison> cpn : ast->whereClause)
                        {
                            if (cpn->double_column)
                            {
                                union_again(get_after_union, ast);
                                break;
                            }
                        }

                        // 为了方便就按顺序将所有参与的表的属性全部放到一个vector中
                        if (!get_after_union.empty() && ast->columns[0]->columnName != "*")         //指定投影
                        {
                            vector<string> temp_projection;                     // 投影选择的列
                            vector<int> index_col;                              // 存储下标
                            for (shared_ptr<Column> column_temp : ast->columns) // 获取下标
                            {
                                if (column_temp->columnName.find(".") != string::npos)
                                {
                                    string table_name_1 = column_temp->columnName.substr(0, column_temp->columnName.find("."));                                                                   // 获取表名
                                    string col_1 = column_temp->columnName.substr(column_temp->columnName.find(".") + 1, column_temp->columnName.size() - column_temp->columnName.find(".") - 1); // 在检查一下  get the column`s name
                                    Table *table_1 = judExit(table_name_1);
                                    if (table_1 == nullptr)
                                        throw MyException("table not exists");
                                    int jud = 0; // 判断表的位置
                                    int count = 0;
                                    for (Attribute attr : table_1->attributes)
                                        if (attr.name != col_1)
                                            count++;
                                        else
                                            break;
                                    for (shared_ptr<TableName> table_name : ast->fromTable)
                                    {
                                        if (table_name->tableName == table_name_1)
                                        {

                                            for (int i = 0; i < jud; i++)
                                            {
                                                count += judExit(ast->fromTable[i]->tableName)->attributeNum;
                                            }
                                            index_col.emplace_back(count);
                                            break;
                                        }
                                        jud++;
                                    }
                                }
                                else
                                {
                                    index_col.emplace_back(find(all_column_name.begin(), all_column_name.end(), column_temp->columnName) - all_column_name.begin());
                                }
                            }

                            index_tables_col = index_col;
                            // 进行投影
                            for (vector<string> temp_values : get_after_union)
                            {
                                for (int j : index_col)
                                {
                                    temp_projection.emplace_back(temp_values[j]);
                                }
                                getSelect.emplace_back(temp_projection);
                                temp_projection.clear();
                            }
                        }
                        else
                            getSelect = get_after_union;
                    }

                    if (!getSelect.empty() && num_of_table == 1) // 单表查询
                    {
                        vector<int> index_col;
                        for (string t : getSelect[0])
                        {
                            index_col.emplace_back(stoi(t));
                        }
                        if (index_col.empty())
                        {
                            return R"(0\r\n0 columns! \r\n)";
                        }
                        int index_table_name = find(ans.begin(), ans.end(), "2!@") - ans.begin() + 1;
                        Table *t = judExit(ans[index_of_tableanme]);
                        string ret = R"(2\r\nsearch successfully \r\n)" + return_name + R"(\r\n)";
                        int len = getSelect.size();
                        int num_attribute = getSelect[0].size();
                        ret += std::to_string(num_attribute);
                        ret += R"(\r\n)";
                        // 加入属性对应的编号
                        for (int i : index_col)
                        {
                            ret += t->attributes[i].name;
                            ret += R"(\r\n)";
                        }
                        for (int i : index_col)
                        {
                            if (t->attributes[i].type == 1)
                            {
                                ret += "int";
                            }
                            else if (t->attributes[i].type == 2)
                            {
                                ret += "char";
                            }
                            else if (t->attributes[i].type == 3)
                            {
                                ret += "date";
                            }
                            else if (t->attributes[i].type == 4)
                            {
                                ret += "double";
                            }
                            ret += R"(\r\n)";
                        }
                        for (int i : index_col)
                        {
                            int a = 0;
                            if (t->attributes[i].primaryKey == 1)
                            {
                                ret += "primarykey ";
                                a++;
                            }
                            if (t->attributes[i].unique == 1)
                            {
                                ret += "unique ";
                                a++;
                            }
                            if (a == 0)
                            {
                                ret += "null ";
                            }
                            ret += R"(\r\n)";
                        }
                        for (int i = 1; i < getSelect.size(); i++)
                        {
                            for (int k = 0; k < getSelect[i].size(); k++)
                            {
                                ret += getSelect[i][k];
                                ret += " ";
                            }
                            ret += R"(\r\n)";
                        }
                        return ret;
                    }
                    else if (!getSelect.empty())
                    { // 多表查询显示结果
                        string ret = R"(2\r\nsearch successfully \r\n)";
                        vector<Table *> tables;
                        Table *t;
                        vector<int> temp_for_index;          // 存储表的下标和列的下标
                        vector<vector<int>> index_table_col; // 存储表的下标和对应的列的下标
                        for (shared_ptr<TableName> Tname : ast->fromTable)
                        {
                            t = judExit(Tname->tableName);
                            if (t != nullptr)
                            {
                                tables.emplace_back(t);
                                ret += Tname->tableName + " ";
                            }else return R"(0\r\ntable don`t exist\r\n)";
                        }
                        t = nullptr;
                        if (index_tables_col.empty()) // 全投影
                            for (int k = 0; k < all_column_name.size(); k++)
                                index_tables_col.emplace_back(k);
                        ret += R"(\r\n)" + to_string(index_tables_col.size()) + R"(\r\n)";
                        for (int i : index_tables_col)
                        {
                            int count = -1;
                            int sum = 0;
                            while (sum < i + 1)
                            {
                                sum += tables[++count]->attributeNum;
                            }
                            t = tables[count];
                            temp_for_index.emplace_back(count);                                 // 存储tables中表的下标
                            temp_for_index.emplace_back(i - sum + tables[count]->attributeNum); // 存储对应表中的列的下标
                            ret += t->tableName + "." + t->attributes[temp_for_index[1]].name;
                            ret += R"(\r\n)";
                            t = nullptr;
                            index_table_col.emplace_back(temp_for_index);
                            temp_for_index.clear();
                        }
                        for (vector<int> i : index_table_col)
                        {
                            if (tables[i[0]]->attributes[i[1]].type == 1)
                            {
                                ret += "int";
                            }
                            else if (tables[i[0]]->attributes[i[1]].type == 2)
                            {
                                ret += "char";
                            }
                            else if (tables[i[0]]->attributes[i[1]].type == 3)
                            {
                                ret += "date";
                            }
                            else if (tables[i[0]]->attributes[i[1]].type == 4)
                            {
                                ret += "double";
                            }
                            ret += R"(\r\n)";
                        }

                        for (vector<int> j : index_table_col)
                        {
                            int a = 0;
                            if (tables[j[0]]->attributes[j[1]].primaryKey == 1)
                            {
                                ret += "primarykey ";
                                a++;
                            }
                            if (tables[j[0]]->attributes[j[1]].unique == 1)
                            {
                                ret += "unique ";
                                a++;
                            }
                            if (a == 0)
                            {
                                ret += "null ";
                            }

                            ret += R"(\r\n)";
                        }
                        int i = 0;  
                        if (num_of_table == 1)
                            i = 1;
                        for (; i < getSelect.size(); i++)
                        {
                            for (int k = 0; k < getSelect[i].size(); k++)
                            {
                                ret += getSelect[i][k];
                                ret += " ";
                            }
                            ret += R"(\r\n)";
                        }
                        return ret;
                    }
                    else
                        return R"(0\r\n0 columns\r\n)";
                }
                break;
            }

            case 7:
                ans = test.descTable(command);
                if (!ans.empty())
                {
                    Table *table = judExit(ans[0]);
                    if (table != nullptr)
                    {
                        string ret = "";
                        ret += R"(3\r\n search successfully \r\n)";
                        // ret +=R"(\r\n)";
                        ret += table->tableName;
                        ret += R"(\r\n)";
                        ret += to_string(table->attributeNum);
                        ret += R"(\r\n)";
                        for (Attribute a : table->attributes)
                        {
                            ret += a.name;
                            ret += R"(\r\n)";
                        }
                        for (Attribute a : table->attributes)
                        {
                            if (a.type == 1)
                            {
                                ret += "int";
                            }
                            else if (a.type == 2)
                            {
                                ret += "char";
                            }
                            else if (a.type == 3)
                            {
                                ret += "date";
                            }
                            else if (a.type == 4)
                            {
                                ret += "double";
                            }
                            ret += R"(\r\n)";
                        }

                        if (!table->attributes.empty())
                        {
                            string constraint = "";
                            for (Attribute a : table->attributes)
                            {
                                if (a.unique)
                                    constraint = "unique";
                                else if (a.primaryKey)
                                    constraint = "primary key";
                                else if (!a.foreignKey.empty())
                                    constraint = "foreign key";
                                else
                                    constraint = "null";
                                ret += constraint;
                                ret += R"(\r\n)";
                            }
                        }
                        return ret;
                    }
                    else
                        return R"(0\r\ntable don`t exist\r\n)";
                }
                else
                    return R"(0\r\nPlease check your command.\r\n)";
                break;

            case 8:
                if (!catalog->tableRecord.empty())
                {
                    string ret = R"(4\r\n)";
                    for (Table *table : catalog->tableRecord)
                    {
                        ret += table->tableName + R"(\r\n)"; // 表名加换行
                    }
                    return ret;
                }
                else
                    return R"(0\r\nThere are not tables in the current database.)";
                break;

            case 9:
                ans = test.createIndex(command);
                if (!ans.empty())
                {
                    Table *t = judExit(ans[0]);
                    vector<int> exist_col;
                    if (t != nullptr)
                    {
                        vector<Attribute> newColumn;
                        for (int i = 1; i < ans.size(); i++) // 判断输入的列是否存在
                            for (Attribute attr : t->attributes)
                                if (attr.name == ans[i])
                                {
                                    exist_col.emplace_back(1);
                                    newColumn.emplace_back(attr);
                                }
                        if (exist_col.size() != ans.size() - 1)
                            return R"(0\r\ncolumn not exist\r\n)";
                        return t->createBPlusTree(newColumn);
                    }
                    else
                        return R"(0\r\ntable not exist\r\n)";
                }
                else
                    return R"(0\r\nPlease check your command\r\n)";
                break;

            case 10:
                ans = test.delIndex(command);
                if (!ans.empty())
                {
                    Table *t = judExit(ans[0]);
                    vector<int> exist_col;
                    if (t != nullptr)
                    {
                        vector<Attribute> newColumn;
                        for (int i = 1; i < ans.size(); i++) // 判断输入的列是否存在
                            for (Attribute attr : t->attributes)
                                if (attr.name == ans[i])
                                {
                                    exist_col.emplace_back(1);
                                    newColumn.emplace_back(attr);
                                }
                        if (exist_col.size() != ans.size() - 1)
                            return R"(0\r\ncolumn not exist\r\n)";
                        if (t->removeBPlusTree(newColumn))
                            return R"(1\r\ndelete index successfully\r\n)";
                    }
                    else
                        return R"(0\r\ntable not exist\r\n)";
                }
                else
                    return R"(0\r\nPlease check your command\r\n)";
                break;

            case 11:
                ans = test.commit(command);
                logManager.commit(ans[0]);
                return R"(1\r\n commit successfully \r\n )";
                break;

            case 12:
                logManager.commit();
                cache->clear();          // 清空缓存区
                catalog->writeCatalog(); // 某个析构函数
                catalog->backup();
                return R"(1\r\n backup successfully \r\n )";
                break;

            case 13:
                ans = test.rollback(command);
                rollback(ans[0], stoi(ans[1]));
                return R"(1\r\n rollback successfully \r\n )";
                break;
            case 15:
                ans = test.recover(command);
                recovery(ans[0]);
                return R"(1\r\n recover successfully \r\n )";
                break;
            case 16:
            {
                string message = logManager.show_all_version();
                return message;
                break;
            }

            case 14:
                ans = test.usingDB(command);
                if (!ans.empty())
                {
                    for (Database *d : databases)
                    {
                        if (d->Name == ans[0])
                        {
                            catalog = d;
                            cache->clear();
                            d->readCatalog();
                            return R"(1\r\ndatavbase:)" + d->Name + R"( is ready\r\n)";
                            break;
                        }
                    }
                    return R"(0\r\ndatabase:)" + ans[0] + R"( not exists\r\n)";
                }
                else
                {
                    return R"(0\r\n error message.\r\n)";
                }

            case 17:
                ans = test.createDB(command);
                if (!ans.empty())
                {
                    for (Database *d : databases)
                    {
                        if (d->Name == ans[0])
                        {
                            return R"(0\r\ndatabase already exists\r\n)";
                            break;
                        }
                    }

                    Database *newDB = new Database(ans[0]);
                    newDB->intilize();
                    databases.emplace_back(newDB);
                    return R"(1\r\create database successfully \r\n)" + ans[0] + R"(\r\n)";
                }
                else
                    return R"(0\r\nerror sentencesr\n)";
                break;

            case 18:
                ans = test.showDb(command);
                if (!ans.empty())
                {
                    string ret = R"(1\r\n)";
                    for (Database *d : databases)
                    {
                        ret += d->Name + R"(\n)";
                    }
                    if (!ret.empty())
                    {
                        ret = ret.substr(0, ret.length() - 2);
                        ret += R"(\r\n)";
                    }
                    if (databases.empty())
                        return R"(0\r\nno databases\r\n)";
                    return ret;
                }
                break;
            case 19:
                ans = test.delete_db(command);
                if (!ans.empty())
                {
                    int i = 0;
                    for (Database *db : databases)
                    {
                        if (db->Name == ans[0])
                        {
                            // delete database
                            db->deleteDatabase();
                            databases.erase(databases.begin() + i);
                            return R"(1\r\nDelete database successfully.\r\n)";
                        }
                        i++;
                    }
                    return R"(0\r\nDatabase does not exist.\r\n)";
                }
                else
                    return R"(0\r\nPlease check your command.\r\n)";
                break;
            case 20:
                return R"(0\r\nERROE: Unkown message:)" + command + R"(\r\n)";
                break;
            }
        }
        catch (const MyException &e)
        {
            return R"(0\r\n)" + string(e.what()) + R"(\r\n)";
        }
        catch (const std::invalid_argument &)
        {
            return R"(0\r\n invalid argument, it is not the type of integer.\r\n)";
        }
        catch (const std::out_of_range &e)
        {
            return R"(0\r\n)" + string(e.what()) + R"(\r\n)";
        }
        command.clear();
    }

    return R"(1\r\n done successfuly \r\n)";
}

DB8::~DB8()
{
    catalog->writeCatalog(); // 修改日志标记
    logManager.remove_checkpoint();
}

void DB8::start()
{
    string temp;    //
    string command; //
    while (true)
    {
        if (command == "")
            cout << "DB8 >";
        else
            cout << "    >";
        getline(cin, temp);
        command += temp;
        temp = "";
        if (command.find(';') != -1)
        {
            cout << start(command) << endl;
            command = "";
        }
    }
}

vector<string> DB8::clean(const string str, string sign) // 用于清理带等号的数据
{
    vector<string> ret; // judge whether there`s an equal sign here
    int begin = 0;      // 获取左边的值
    int end = str.find(sign) - 1;
    for (int i = begin, j = end; i != -5 || j != -5; i++, j--)
    {
        if (j != -5 && (str[j] == ' ' || str[j] == '\''))
        {
            end--;
        }
        if (i != -5 && (str[i] == ' ' || str[i] == '\''))
        {
            begin++;
        }
        if (str[j] != ' ' && str[j] != '\'')
            j = -5;
        if (str[i] != ' ' && str[i] != '\'')
            i = -5;
        if (i == -5)
            i--;
        if (j == -5)
            j++;
    }
    ret.emplace_back(str.substr(begin, end - begin + 1));

    begin = str.find(sign) + sign.length(); // 获取等号右边的值
    end = str.length() - 1;
    for (int k = begin, j = end; k != -5 || j != -5; k++, j--)
    {
        if (j != -5 && (str[j] == ' ' || str[j] == '\''))
        {
            end--;
        }
        if (k != -5 && (str[k] == ' ' || str[k] == '\''))
        {
            begin++;
        }
        if (str[j] != ' ' && str[j] != '\'')
            j = -5;
        if (str[k] != ' ' && str[k] != '\'')
            k = -5;
        if (j == -5)
            j++;
        if (k == -5)
            k--;
    }
    ret.emplace_back(str.substr(begin, end - begin +
                                           1)); // push the value on the right-hand side of the equal sign to the vector
    return ret;
}

Table *DB8::judExit(string name)
{
    for (Table *t : catalog->tableRecord)
    {
        if (t->tableName == name)
            return t;
    }
    return nullptr;
}

vector<vector<string>> DB8::union_tables(vector<vector<string>> table_1, vector<vector<string>> table_2)
{
    // todo 多表连接
    if (table_1.empty() || table_2.empty())
        return {};
    vector<string> temp;
    vector<vector<string>> ret;
    for (vector<string> turple_1 : table_1)
    {
        for (vector<string> turple_2 : table_2)
        {
            temp.insert(temp.end(), turple_1.begin(), turple_1.end());
            temp.insert(temp.end(), turple_2.begin(), turple_2.end());
            ret.emplace_back(temp);
            temp.clear();
        }
    }
    return ret;
}

DB8::DB8()
{
    judsign.insert(pair<string, int>("=", 1));
    judsign.insert(pair<string, int>("<=", 2));
    judsign.insert(pair<string, int>(">=", 3));
    judsign.insert(pair<string, int>("<", 4));
    judsign.insert(pair<string, int>(">", 5));

    judAlter.insert(pair<string, int>("modify", 1));
    judAlter.insert(pair<string, int>("modify_column", 2));
    judAlter.insert(pair<string, int>("add", 3));
    judAlter.insert(pair<string, int>("drop", 4));

    judConstraint.insert(pair<string, int>("default", 1));
    judConstraint.insert(pair<string, int>("primary", 2));
    judConstraint.insert(pair<string, int>("references", 3));
    judConstraint.insert(pair<string, int>("foreign", 4));
    judConstraint.insert(pair<string, int>("unique", 5));

    catalog = nullptr;
    DIR *dp;          // 定义一个指向目录的指针
    dirent *filename; // 定义一个指向dirent结构的指针
    dp = opendir("Database");
    if (!dp)
    {
        system("mkdir Database");
    }
    else
    {
        int i = 0;
        while (filename = readdir(dp))
        {
            // 避免将目录中的 . 和 .. 文件加入 databases
            i++;
            if (i >= 3)
            {
                string name = filename->d_name;
                Database *newDB = new Database(name);
                databases.emplace_back(newDB);
            }
        }
    }
}

string DB8::processStringSpace(string input)
{
    string output;
    bool hasSpace = false;
    for (char c : input)
    {
        if (c == ' ')
        {
            hasSpace = true;
        }
        else
        {
            if (hasSpace)
            {
                output.push_back(' ');
                hasSpace = false;
            }
            output.push_back(c);
        }
    }
    if (output.size() > 0 && output.back() == ' ')
    {
        output.pop_back();
    }
    return output;
}

vector<vector<string>> DB8::select_one_table(vector<string> ans)
{
    Table *t = judExit(ans[find(ans.begin(), ans.end(), "2!@") - ans.begin() + 1]);
    if (t != nullptr)
    { // 当前表名存在
        // 记录属性的开始下标
        int begin_col = find(ans.begin(), ans.end(), "1!@") - ans.begin() + 1;
        int end_col = find(ans.begin(), ans.end(), "2!@") - ans.begin();

        vector<vector<string>> getSelect;

        // 将所有的where条件打包
        int index_of_where = find(ans.begin(), ans.end(), "3!@") - ans.begin();
        int index_of_condition = index_of_where + 1;
        int end_of_condition = index_of_condition;
        while (end_of_condition < ans.size() && ans[end_of_condition].find("!@") == string::npos) // 计算条件个数
            end_of_condition++;
        vector<string> where_condition;
        for (int i = index_of_condition; i < end_of_condition; i++)
        {
            if (ans[i] != "and" && ans[i] != "or")
                where_condition.emplace_back(ans[i]);
            else
                ans[i] == "and" ? where_condition.emplace_back("and") : where_condition.emplace_back("or");
        }
        vector<vector<string>> allValues;
        bool needDel = false;         // 判断是否需要再加工
        if (!where_condition.empty()) // 判断是否为条件查询
        {
            vector<string> after_clean = get_after_clean(where_condition[0]);
            bool judExistCol = false;            // 判断属性是否存在
            for (Attribute attr : t->attributes) // get attribute that coresspond to after_clean[0]
            {
                if (attr.name == after_clean[0]) // judsign 1 : = , 2 : <=, 3:>=,4:<,5:>.
                {
                    judExistCol = true;
                    switch (judsign[after_clean[2]])
                    {
                    case 1:
                        allValues = t->SelectEqual({after_clean[1]}, {attr});
                        break;
                    case 2:
                        allValues = t->SelectSmall({after_clean[1]}, {attr});
                        break;
                    case 3:
                        allValues = t->SelectBig({after_clean[1]}, {attr});
                        break;
                    case 4:
                        allValues = t->SelectSmall({after_clean[1]}, {attr});
                        needDel = true;
                        break;
                    case 5:
                        allValues = t->SelectBig({after_clean[1]}, {attr});
                        needDel = true;
                        break;
                    }
                }
            }
            if (!judExistCol)
                throw MyException("The name of column doesn`t exist.");
            else if (needDel)
            {
                allValues = where_del("and", after_clean, allValues, t);
            }
        }
        else
            allValues = t->selectAll(); // 否则为非条件查询
        if (where_condition.size() > 1)
        {
            where_condition.erase(where_condition.begin()); // 擦除第一个条件
            vector<string> after_clean;
            for (int i = 0; i < where_condition.size(); i += 2)
            {
                after_clean = get_after_clean(where_condition[i + 1]);
                allValues = where_del(where_condition[i], after_clean, allValues, t);
            }
        }

        getSelect = allValues;

        if (ans[1] != "*" && !getSelect.empty() && !getSelect.empty()) // 若输入的类型为select ...... from tablename where ......
        {
            vector<vector<string>> newGet;
            vector<string> temp;
            int jud[end_col - begin_col]; // 用于决定输出顺序
            memset(jud, -1, sizeof(jud));
            for (int i = begin_col; i < end_col; i++) // 按查询的顺序输出，并判断当前投影的列是否存在
            {
                for (int j = 0; j < t->attributeNum; j++)
                    if (ans[i] == t->attributes[j].name)
                    {
                        jud[i - begin_col] = j; // select后面的第i+1个列在attributes中的位置
                    }
            }

            for (int index : jud) // 返回需要的列的下标
            {
                temp.emplace_back(to_string(index));
            }
            if (!temp.empty())
                newGet.emplace_back(temp);
            temp.clear();

            for (int j = 0; j < getSelect.size(); j++)
            {
                for (int k = 0; k < sizeof(jud) / sizeof(jud[0]); k++)
                {
                    if (jud[k] != -1)
                        temp.emplace_back(getSelect[j][jud[k]]);
                    else
                        throw MyException("column not exist");
                }
                newGet.emplace_back(temp);
                temp.clear();
            }

            if (newGet.empty())
                throw MyException("column not exist"); // R"(0\r\n当前列不存在！\r\n)";
            else if (end_col - begin_col == newGet[1].size())
                getSelect = newGet;
            else
                throw MyException("column not exist"); // R"(0\r\n未发现行！\r\n)";
        }
        else if (!getSelect.empty())
        {
            vector<string> temp_index_col;            // 没有投影
            for (int i = 0; i < t->attributeNum; i++) // 需要投影的列的下表
                temp_index_col.emplace_back(to_string(i));
            allValues.insert(allValues.begin(), temp_index_col);
            getSelect = allValues;
        }

        if (!getSelect.empty())
        {
            return getSelect;
        }
        else
        {
            throw MyException("There is not column."); //"未发现行";
        }
    }
    else
        throw MyException("table not exists");
}

vector<vector<string>> DB8::where_del(string union_type, vector<string> condition, vector<vector<string>> allValues, Table *t)
{

    // 调用stl库的std::unordered_map方法
    std::unordered_map<std::string, std::function<bool(int, int)>> op_map = {
        {"<", std::less<int>()},
        {">", std::greater<int>()},
        {"<=", std::less_equal<int>()},
        {">=", std::greater_equal<int>()},
        {"=", std::equal_to<int>()},
        {"!=", std::not_equal_to<int>()}};

    if (union_type == "and")
    {
        for (int k = 0;
             k < t->attributeNum; k++) // 将数据按照where条件处理   =:1  <=:2  >=:3 <:4 >:5
            if (t->attributes[k].name == condition[0])
            {
                switch (judsign[condition[2]])
                {
                case 1:
                    for (int i = 0; i < allValues.size(); i++)
                        if (allValues[i][k] !=
                            condition[1]) // 判断所有的数据中的对应属性attrs[j]的值与attrs对应的的values[j]是否相等，不等则删除
                        {
                            allValues.erase(allValues.begin() + i);
                            i--;
                        }
                    break;
                case 2:
                    for (int i = 0; i < allValues.size(); i++)
                        if (allValues[i][k] >
                            condition[1]) // 判断所有的数据中的对应属性attrs[j]的值是否小于等于attrs对应的的values[j]，否则删除
                        {
                            allValues.erase(allValues.begin() + i);
                            i--;
                        }
                    break;
                case 3:
                    for (int i = 0; i < allValues.size(); i++)
                        if (allValues[i][k] <
                            condition[1]) // 判断所有的数据中的对应属性attrs[j]的值是否大于等于attrs对应的的values[j]，否则删除
                        {
                            allValues.erase(allValues.begin() + i);
                            i--;
                        }
                    break;
                case 4:
                    for (int i = 0; i < allValues.size(); i++)
                        if (allValues[i][k] >=
                            condition[1]) // 判断所有的数据中的对应属性attrs[j]的值是否小于attrs对应的的values[j]，否则删除
                        {
                            allValues.erase(allValues.begin() + i);
                            i--;
                        }
                    break;
                case 5:
                    for (int i = 0; i < allValues.size(); i++)
                        if (allValues[i][k] <=
                            condition[1]) // 判断所有的数据中的对应属性attrs[j]的值是否大于attrs对应的的values[j]，否则删除
                        {
                            allValues.erase(allValues.begin() + i);
                            i--;
                        }
                    break;
                }
            }
    }
    else if (union_type == "or")
    {
        int index[2]{-1, -1};
        for (int k = 0; k < t->attributeNum; k++) // 将数据按照where条件处理   =:1  <=:2  >=:3 <:4 >:5
            if (t->attributes[k].name == condition[0])
            {
                index[0] = k;
            }
            else if (t->attributes[k].name == condition[3])
            {
                index[1] = k;
            }
        if (index[0] != -1 && index[1] != -1)
        {
            auto cmp_2 = op_map.at(condition[2]);
            auto cmp_5 = op_map.at(condition[5]);
            for (int i = 0; i < allValues.size(); i++)
            {
                if (cmp_2(stoi(condition[1]), stoi(allValues[i][index[0]])) || cmp_5(stoi(condition[1]), stoi(allValues[i][index[1]])))
                {
                    allValues.erase(allValues.begin() + i);
                    i--;
                }
            }
        }
        else
            throw MyException("There is not column.");
    }
    return allValues;
}

shared_ptr<SelectStatement> DB8::init_ast(vector<string> ans)
{
    using std::shared_ptr;
    shared_ptr<SelectStatement> selectStatement = make_shared<SelectStatement>();
    // 投影
    int i = 1;
    for (; i < find(ans.begin(), ans.end(), "2!@") - ans.begin(); i++)
    {
        shared_ptr<Column> column1 = make_shared<Column>();
        column1->columnName = ans[i];
        selectStatement->columns.push_back(column1);
    }
    // 表名
    int j = i + 1;
    for (; j < ans.size() && ans[j].find("!@") == string::npos; j++) // string.find函数返回值若没有找到时返回一个很大的数字，在这里==表示没有发现那就继续往下运行  string::npos = 18446744073709551615
    {
        shared_ptr<TableName> table = make_shared<TableName>();
        table->tableName = ans[j];
        selectStatement->fromTable.emplace_back(table);
    }
    // where语句
    int k = 0;
    if (find(ans.begin(), ans.end(), "3!@") != ans.end())
    {
        k = j + 1;
        for (; k < ans.size() && ans[k].find("!@") == string::npos; k++)
        {
            shared_ptr<Comparison> comparsion = make_shared<Comparison>();
            string sign_temp;
            for (char ch : ans[k]) // 将比较符分隔出来
                if (ch == '=' || ch == '<' || ch == '>')
                    sign_temp += ch;
            vector<string> after_clean = clean(ans[k], sign_temp);
            comparsion->left = after_clean[0];
            comparsion->right = after_clean[1];
            comparsion->visited = -1;
            comparsion->comparison = sign_temp;
            selectStatement->whereClause.emplace_back(comparsion);
        }
    }
    // groupby
    int x = 0;
    if (find(ans.begin(), ans.end(), "4!@") != ans.end())
    {
        k == 0 ? x = j + 1 : x = k + 1;
        shared_ptr<GroupBy> group_by = make_shared<GroupBy>();
        for (; x < ans.size() && ans[x].find("!@") == string::npos; x++)
        {
            group_by->columns.emplace_back(ans[x]);
        }
        selectStatement->groupByClause = group_by;
    }
    // having
    int y = 0;
    if (find(ans.begin(), ans.end(), "5!@") != ans.end())
    {
        k != 0 ? y = k + 1 : (x == 0 ? y = j + 1 : y = x + 1);
        shared_ptr<Having> having = make_shared<Having>();
        for (; y < ans.size() && ans[y].find("!@") == string::npos; y++)
        {
            having->condition.emplace_back(ans[y]);
        }
        selectStatement->havingClause = having;
    }
    // orderby
    int z = 0;
    if (find(ans.begin(), ans.end(), "6!@") != ans.end())
    {
        y != 0 ? z = y + 1 : (x == 0 ? (k == 0 ? z = j + 1 : z = k + 1) : z = x + 1);
        shared_ptr<OrderBy> order_by = make_shared<OrderBy>();
        for (; z < ans.size() && ans[z].find("!@") == string::npos; z++)
        {
            order_by->columns.emplace_back(ans[z]);
        }
        selectStatement->orderByClause = order_by;
    }
    return selectStatement;
}

shared_ptr<SelectStatement> DB8::match_where(shared_ptr<SelectStatement> &ast)
{
    for (int i = 0; i < ast->fromTable.size(); i++)
    {
        Table *t = judExit(ast->fromTable[i]->tableName);
        if (t != nullptr)
        {
            for (shared_ptr<Comparison> where : ast->whereClause)
                for (Attribute attr : t->attributes)
                {
                    if (where->left == attr.name)
                    {
                        if (where->visited == -1)
                        {
                            ast->fromTable[i]->where.emplace_back(where);
                            where->visited = 1;
                        }
                        else
                            throw MyException("column name is collide!" + where->left); // 列名冲突，在两张表中出现了同名的表
                    }
                }
        }
        else
            throw MyException("table not exist");
        // 处理表名.列的情况
        for (shared_ptr<Comparison> where : ast->whereClause)
        {
            if (where->left.find(".") != string::npos && where->right.find(".") == string::npos)
            {
                string table_name = where->left.substr(0, where->left.find("."));                                           // 获取表名
                string col = where->left.substr(where->left.find(".") + 1, where->left.size() - where->left.find(".") - 1); // 在检查一下  get the column`s name
                Table *table = judExit(table_name);
                if (table != nullptr)
                {
                    for (shared_ptr<TableName> temp_table : ast->fromTable)
                    {
                        if (temp_table->tableName == table->tableName)
                            for (Attribute attr : table->attributes)
                                if (attr.name == col)
                                {
                                    where->left = table_name; // 将表名.列重新改为列，并放入对应的table.where中
                                    temp_table->where.emplace_back(where);
                                    where->visited = 1;
                                }
                    }
                }else throw MyException("table not exists");
            }
            if (where->left.find(".") != string::npos && where->right.find(".") != string::npos && where->visited == -1) // 处理表名.列compare表名.列的情况
            {
                string table_name_1 = where->left.substr(0, where->left.find("."));                                           // 获取表名
                string col_1 = where->left.substr(where->left.find(".") + 1, where->left.size() - where->left.find(".") - 1); // 在检查一下  get the column`s name
                Table *table_1 = judExit(table_name_1);

                string table_name_2 = where->right.substr(0, where->right.find("."));                                             // 获取表名
                string col_2 = where->right.substr(where->right.find(".") + 1, where->right.size() - where->right.find(".") - 1); // 在检查一下  get the column`s name
                Table *table_2 = judExit(table_name_2);
                if (table_1 != nullptr && table_2 != nullptr)
                {
                    where->double_column = true;
                    int i = 0;
                    int k = 0;
                    for (shared_ptr<TableName> temp_table : ast->fromTable)
                    {
                        if (temp_table->tableName == table_1->tableName)
                        {
                            int j = 0;
                            for (Attribute attr : table_1->attributes)
                            {
                                if (attr.name == col_1)
                                {
                                    where->index_table_col_1.emplace_back(i); // 将表名.列重新改为表的下标和列的下标
                                    where->index_table_col_1.emplace_back(j);
                                    where->visited = 1;
                                }
                                j++;
                            }
                        }
                        i++;
                        if (temp_table->tableName == table_2->tableName)
                        {
                            int j = 0;
                            for (Attribute attr : table_2->attributes)
                            {
                                if (attr.name == col_2)
                                {
                                    where->index_table_col_2.emplace_back(k); // 将表名.列重新改为表的下标和列的下标
                                    where->index_table_col_2.emplace_back(j);
                                }
                                j++;
                            }
                        }
                        k++;
                    }
                    if (where->index_table_col_2.empty())
                        throw MyException(where->left + where->comparison + where->right + "don`t exist.");
                }
                else
                    throw MyException("table not exists");
            }
        }
    }
    string error = "";
    for (shared_ptr<Comparison> where : ast->whereClause)
        if (where->visited == -1)
            error += where->left + where->comparison + where->right + " ";
    if (!error.empty())
        throw MyException(error + "don`t exist.");
    return ast;
}

vector<string> DB8::get_after_clean(string ans)
{
    vector<string> after_clean;
    if (ans.find("<=") != string::npos) // 比较类型为<=
    {
        after_clean = clean(ans, "<=");
        after_clean.emplace_back("<=");
    }
    else if (ans.find(">=") != string::npos) // 比较类型为>=
    {
        after_clean = clean(ans, ">=");
        after_clean.emplace_back(">=");
    }
    else if (ans.find("=") != string::npos) // 比较类型为=
    {
        after_clean = clean(ans, "=");
        after_clean.emplace_back("=");
    }
    else if (ans.find("<") != string::npos) // 比较类型为<
    {
        after_clean = clean(ans, "<");
        after_clean.emplace_back("<");
    }
    else if (ans.find(">") != string::npos) // 比较类型为>
    {
        after_clean = clean(ans, ">");
        after_clean.emplace_back(">");
    }
    else
        throw MyException("There is not comparing sign.");
    return after_clean;
}

void DB8::union_again(vector<vector<string>> &allValues, shared_ptr<SelectStatement> &ast)
{
    int first_col = 0;
    int second_col = 0;
    for (shared_ptr<Comparison> temp_ptr : ast->whereClause)
    {
        if (temp_ptr->double_column)
        {
            first_col = temp_ptr->index_table_col_1[1];
            for (int i = 0; i < temp_ptr->index_table_col_1[0]; i++)
            {
                Table *t = judExit(ast->fromTable[i]->tableName);
                if (t == nullptr)
                    throw MyException("table not exists");
                first_col += t->attributeNum;
            }

            second_col = temp_ptr->index_table_col_2[1];
            for (int i = 0; i < temp_ptr->index_table_col_2[0]; i++)
            {
                Table *t = judExit(ast->fromTable[i]->tableName);
                if (t == nullptr)
                    throw MyException("table not exists");
                second_col += t->attributeNum;
            }

            for (int j = 0; j < allValues.size(); j++)
            {
                if (allValues[j][first_col] != allValues[j][second_col])
                {
                    allValues.erase(allValues.begin() + j);
                    j--;
                }
            }
        }
    }
}