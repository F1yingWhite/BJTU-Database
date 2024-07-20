//
// Created by BlackCat on 2023/3/31.
//

#include "Database.h"
#include "Struct/Table.h"
#include <dirent.h>
#include <fstream>
#include <filesystem>

// 分割字符串的工具方法
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

void Database::deleteDatabase()
{
    string lujin = "";
#ifdef _WIN32
    lujin = "Database\\" + Name;
    system(("rd /s /q " + lujin).c_str());
#elif __linux__
    lujin = "Database/" + Name;
    system(("rm -rf " + lujin).c_str());
#elif __APPLE__
    lujin = "Database/" + Name;
    system(("rm -rf " + lujin).c_str());
#endif
    for (auto table : tableRecord)
    {
        delete table;
    }
}

void Database::readCatalog()
{
    fstream fin;
    fin.open(dbName.c_str(), ios::in);
    tableRecord.clear();
    if (!fin.is_open())
    {
        cout << "schema.db not exist" << endl;
        return;
    }
    else
    {
        fin >> tableNum;
        for (int i = 0; i < tableNum; i++)
        {
            Table *newTable = new Table;
            newTable->database = this;
            fin >> newTable->tableName;
#ifdef _WIN32
            newTable->tableName2 = "Database\\" + Name + "\\" + newTable->tableName;
#elif __linux__
            newTable->tableName2 = "Database/" + Name + "/" + newTable->tableName;
#elif __APPLE__
            newTable->tableName2 = "Database/" + Name + "/" + newTable->tableName;
#endif
            fin >> newTable->attributeNum;
            fin >> newTable->dataNum;
            fin >> newTable->indexNum;
            tableRecord.push_back(newTable);
            int rank = 0;
            for (int j = 0; j < newTable->attributeNum; j++)
            {
                Attribute newAttribute;
                fin >> newAttribute.name;
                int flag = 0;
                fin >> flag;
                if (flag == 0)
                {
                    newAttribute.defaultValue = "";
                }
                else
                {
                    fin >> newAttribute.defaultValue;
                }
                fin >> flag;
                if (flag == 0)
                {
                    newAttribute.foreignKey = "";
                }
                else
                {
                    fin >> newAttribute.foreignKey;
                }
                fin >> newAttribute.type;
                fin >> newAttribute.len;
                fin >> newAttribute.primaryKey;
                fin >> newAttribute.unique;
                string foreign;
                fin >> foreign;
                if (foreign != "0")
                {
                    newAttribute.foreignKey = foreign;
                }
                newAttribute.rank = rank++;
                newTable->attributes.push_back(newAttribute);
            }
            for (int j = 0; j < newTable->indexNum; j++)
            {
                string indexName;
                int repeat;
                fin >> indexName;
                fin >> repeat;
                vector<string> name;
                int pre = 0;
                for (int i = 0; i < indexName.length(); i++)
                { // 按名字分割字符串
                    if (indexName[i] == '_')
                    {
                        name.push_back(indexName.substr(pre, i - pre));
                        pre = i + 1;
                    }
                }
                vector<Attribute> attributes;
                for (int i = 1; i < name.size(); i++)
                {
                    for (int j = 0; j < newTable->attributes.size(); j++)
                    {
                        if (name[i] == newTable->attributes[j].name)
                        {
                            attributes.push_back(newTable->attributes[j]);
                            break;
                        }
                    }
                }
                BPlusTree *tree = new BPlusTree(attributes, newTable->tableName2);
                tree->ifRepetition = repeat == 0 ? false : true;
                newTable->insertBPlusTree(indexName, tree);
            }
        }
        fin.close();
    }
}

void Database::readCatalog(string backup)
{
    fstream fin;
    string tempName;
#ifdef _WIN32
    tempName = "Backup\\" + backup + "\\schema.db";
#elif __linux__
    tempName = "Backup/" + backup + "/schema.db";
#elif __APPLE__
    tempName = "Backup/" + backup + "/schema.db";
#endif
    fin.open(tempName.c_str(), ios::in);
    if (!fin.is_open())
    {
        cout << "schema.db not exist" << endl;
        return;
    }
    else
    {
        fin >> tableNum;
        for (int i = 0; i < tableNum; i++)
        {
            Table *newTable = new Table;
            newTable->database = this;
            fin >> newTable->tableName;
#ifdef _WIN32
            newTable->tableName2 = "Database\\" + Name + "\\" + newTable->tableName;
#elif __linux__
            newTable->tableName2 = "Database/" + Name + "/" + newTable->tableName;
#elif __APPLE__
            newTable->tableName2 = "Database/" + Name + "/" + newTable->tableName;
#endif
            fin >> newTable->attributeNum;
            fin >> newTable->dataNum;
            fin >> newTable->indexNum;
            tableRecord.push_back(newTable);
            int rank = 0;
            for (int j = 0; j < newTable->attributeNum; j++)
            {
                Attribute newAttribute;
                fin >> newAttribute.name;
                int flag = 0;
                fin >> flag;
                if (flag == 0)
                {
                    newAttribute.defaultValue = "";
                }
                else
                {
                    fin >> newAttribute.defaultValue;
                }
                fin >> flag;
                if (flag == 0)
                {
                    newAttribute.foreignKey = "";
                }
                else
                {
                    fin >> newAttribute.foreignKey;
                }
                fin >> newAttribute.type;
                fin >> newAttribute.len;
                fin >> newAttribute.primaryKey;
                fin >> newAttribute.unique;
                string foreign;
                fin >> foreign;
                if (foreign != "0")
                {
                    newAttribute.foreignKey = foreign;
                }
                newAttribute.rank = rank++;
                newTable->attributes.push_back(newAttribute);
            }
            for (int j = 0; j < newTable->indexNum; j++)
            {
                string indexName;
                int repeat;
                fin >> indexName;
                fin >> repeat;
                vector<string> name;
                int pre = 0;
                for (int i = 0; i < indexName.length(); i++)
                { // 按名字分割字符串
                    if (indexName[i] == '_')
                    {
                        name.push_back(indexName.substr(pre, i - pre));
                        pre = i + 1;
                    }
                }
                vector<Attribute> attributes;
                for (int i = 1; i < name.size(); i++)
                {
                    for (int j = 0; j < newTable->attributes.size(); j++)
                    {
                        if (name[i] == newTable->attributes[j].name)
                        {
                            attributes.push_back(newTable->attributes[j]);
                            break;
                        }
                    }
                }
                BPlusTree *tree = new BPlusTree(attributes, newTable->tableName2);
                tree->ifRepetition = repeat == 0 ? false : true;
                newTable->insertBPlusTree(indexName, tree);
            }
        }
        fin.close();
    }
}

void Database::writeCatalog()
{
    fstream fout(dbName.c_str(), ios::out);
    if (!fout.is_open())
    {
        cerr << "open filed,please check your device!" << endl;
        return;
    }
    else
    {
        // TableNum TableName AttributeNum DataNum AttributeName AttributeType length primaryKey unique foreignKey
        fout << tableRecord.size() << " ";
        for (auto table : tableRecord)
        {
            fout << table->tableName << " ";
            fout << table->attributeNum << " ";
            fout << table->dataNum << " ";
            fout << table->indexNum << " ";
            for (auto attribute : table->attributes)
            {
                fout << attribute.name << " ";
                if (attribute.defaultValue == "")
                {
                    fout << 0 << " ";
                }
                else
                {
                    fout << 1 << " ";
                    fout << attribute.defaultValue << " ";
                }
                if (attribute.foreignKey == "")
                {
                    fout << 0 << " ";
                }
                else
                {
                    fout << 1 << " ";
                    fout << attribute.foreignKey << " ";
                }
                fout << attribute.type << " ";
                fout << attribute.len << " ";
                fout << attribute.primaryKey << " ";
                fout << attribute.unique << " ";
                if (attribute.foreignKey == "")
                {
                    fout << 0 << " ";
                }
                else
                {
                    fout << attribute.foreignKey << " ";
                }
            }
            for (auto index : table->BPlusTreeMap)
            {
                fout << index.first << " ";
                if (index.second->ifRepetition)
                {
                    fout << 1 << " ";
                }
                else
                {
                    fout << 0 << " ";
                }
            }
        }
        fout.close();
    }
}

Table *Database::getTable(string name)
{
    for (auto table : tableRecord)
    {
        if (table->tableName == name)
            return table;
    }
    return nullptr;
}

bool Database::existTable(string name)
{
    for (auto table : tableRecord)
    {
        if (table->tableName == name)
            return true;
    }
    return false;
}

void Database::print()
{
    // TableNum TableName AttributeNum DataNum AttributeName AttributeType length primaryKey unique
    cout << "tableNum " << tableNum << endl;
    for (auto table : tableRecord)
    {
        cout << "tableName " << table->tableName << endl;
        cout << "AttributeNum " << table->attributeNum << endl;
        cout << "DataNum " << table->dataNum << endl;
        cout << "IndexNum " << table->indexNum << endl;
        for (auto attribute : table->attributes)
        {
            cout << "AttributeName " << attribute.name << endl;
            cout << "AttributeType " << attribute.type << endl;
            cout << "length " << attribute.len << endl;
            cout << "primaryKey " << attribute.primaryKey << endl;
            cout << "unique " << attribute.unique << endl;
        }
        cout << endl;
    }
}

string getTime()
{
    time_t timep;
    time(&timep);
    char tmp[64];
    strftime(tmp, sizeof(tmp), "%Y-%m-%d_%H:%M:%S", localtime(&timep));
    return tmp;
}

void Database::backup()
{
    //  会覆盖之前的备份库,只保留最新的备份?
    //  拷贝该文件夹下的所有文件到新的文件夹
    cache->clear();
    string time = getTime();
    fstream file;
    file.open("../build/redolog.txt", ios::out | ios::in);
    // 查看file最后一行的数据
    file.seekg(0, ios::beg);
    int a;
    file >> a;
    file >> a;
    cout << a;
    DIR *dp;          // 定义一个指向目录的指针
    dirent *filename; // 定义一个指向dirent结构的指针
    string prefix;
#ifdef _WIN32
    prefix = "Database\\" + Name;
#elif __linux__
    prefix = "Database/" + Name;
#elif __APPLE__
    prefix = "Database/" + Name;
#endif
    dp = opendir(prefix.c_str()); // 读取该数据库文件夹下的所有文件
    vector<string> files;
    int i = 0;
    while (filename = readdir(dp))
    {
        // 避免将目录中的 . 和 .. 文件加入 databases
        i++;
        if (i >= 3)
        {
            string name = filename->d_name;
            files.emplace_back(name); // 存放当前文件夹中的所有文件名称
        }
    }
    string newDBPath;
#ifdef _WIN32
    newDBPath = "Backup\\";
    prefix += "\\";
#elif __linux__
    prefix += "/";
    newDBPath = "Backup/";
#elif __APPLE__
    prefix += '/';
    newDBPath = "Backup/";
#endif
    dp = opendir("Backup");
    if (!dp)
    {
        system("mkdir Backup"); // 先创建backup目录
    }
    else
    {
        // 删除原来的备份,使用一份新的
        int i = 0;
        while (filename = readdir(dp))
        {
            i++;
            if (i >= 3)
            {
                string name = filename->d_name;
                if (name.substr(0, Name.size()) == Name)
                {
#ifdef _WIN32
                    system(("rd /s /q " + newDBPath + "" + name).c_str());
#elif __linux__
                    system(("rm -rf " + newDBPath + "" + name).c_str());
#elif __APPLE__
                    system(("rm -rf " + newDBPath + "" + name).c_str());
#endif

                    break;
                }
            }
        }
    }
    newDBPath += Name + "_" + to_string(a); // 新的目录名称
    system(("mkdir " + newDBPath).c_str());
#ifdef _WIN32
    newDBPath += "\\";
#elif __linux__
    newDBPath += "/";
#elif __APPLE__
    newDBPath += "/";
#endif
    for (string n : files)
    {
#ifdef _WIN32
        system(("copy " + prefix + n + " " + newDBPath + n).c_str());
#elif __linux__
        system(("cp " + prefix + n + " " + newDBPath + n).c_str());
#elif __APPLE__
        system(("cp " + prefix + n + " " + newDBPath + n).c_str());
#endif
    }
}

void Database::recovery(string backup)
{
    //  拷贝该文件夹下的所有文件到新的文件夹
    cache->clear();
    fstream file;
    file.open("../build/redolog.txt", ios::out | ios::in);
    // 查看file最后一行的数据
    vector<string> elems = stringSplit_(backup, '_');
    string Name = elems[0];
    int a = stoi(elems[1]);
    file.seekg(a, ios::beg);
    DIR *dp;          // 定义一个指向目录的指针
    dirent *filename; // 定义一个指向dirent结构的指针
    string prefix;
#ifdef _WIN32
    prefix = "Backup\\" + backup;
#elif __linux__
    prefix = "Backup/" + backup;
#elif __APPLE__
    prefix = "Backup/" + backup;
#endif
    dp = opendir(prefix.c_str()); // 读取该数据库文件夹下的所有文件
    vector<string> files;
    int i = 0;
    while (filename = readdir(dp))
    {
        // 避免将目录中的 . 和 .. 文件加入 databases
        i++;
        if (i >= 3)
        {
            string name = filename->d_name;
            files.emplace_back(name); // 存放当前文件夹中的所有文件名称
        }
    }
    string newDBPath;
#ifdef _WIN32
    newDBPath = "Database\\";
    prefix += "\\";
#elif __linux__
    prefix += "/";
    newDBPath = "Database/";
#elif __APPLE__
    prefix += '/';
    newDBPath = "Database/";
#endif
    newDBPath += Name; // 新的目录名称
#ifdef _WIN32
    newDBPath += "\\";
#elif __linux__
    newDBPath += "/";
#elif __APPLE__
    newDBPath += "/";
#endif
    for (string n : files)
    {
#ifdef _WIN32
        system(("copy  " + prefix + n + " " + newDBPath + n).c_str());
#elif __linux__
        system(("cp -f" + prefix + n + " " + newDBPath + n).c_str());
#elif __APPLE__
        system(("cp -f " + prefix + n + " " + newDBPath + n).c_str());
#endif
    }
}

bool Database::deleteTable(string name)
{
    if (!existTable(name))
        return false;
    else
    {
        for (int i = 0; i < tableRecord.size(); i++)
        {
            Table *table = tableRecord[i];
            if (table->tableName == name)
            {
                table->drop();
                tableRecord.erase(tableRecord.begin() + i);
            }
        }
    }
    return true;
}

bool Database::upDataTable(Table *newTable)
{
    if (!existTable(newTable->tableName))
        return false;
    else
    {
        for (int i = 0; i < tableRecord.size(); i++)
        {
            Table *table = tableRecord[i];
            if (table->tableName == newTable->tableName)
            {
                tableRecord[i] = newTable;
            }
        }
    }
    return true;
}

bool Database::insertTable(Table *newTable)
{
    if (existTable(newTable->tableName))
        return false;
    else
    {
#ifdef _WIN32
        newTable->tableName2 = "Database\\" + Name + "\\" + newTable->tableName;
#elif __linux__
        newTable->tableName2 = "Database/" + Name + "/" + newTable->tableName;
#elif __APPLE__
        newTable->tableName2 = "Database/" + Name + "/" + newTable->tableName;
#endif
        tableRecord.emplace_back(newTable);
        vector<Attribute> pri;
        for (auto attribute : newTable->attributes)
        {
            if (attribute.unique != 0)
            {
                newTable->createBPlusTree({attribute}, false);
            }
            if (attribute.primaryKey != 0)
            {
                pri.push_back(attribute);
            }
        }
        if (pri.size() != 0)
            newTable->createBPlusTree(pri, false);
    }
    return true;
}

Database::~Database()
{
    writeCatalog();
    for (int i = 0; i < tableNum; i++)
    {
        delete tableRecord[i];
    }
}