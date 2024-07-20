#include "BPlusTree.h"
#include "cstring"
// 我们如何设计index存储文件?
// 首先我们使用filename.index作为索引文件名,然后fileName.type存储索引类型
/* 老规矩,我们直接写成函数,需要传入文件名
 * 1. 前十位记录下root
 * 2. 11-20记录下maxLeft
 * 3. 21-30记录下第一个空位
 * 4. 31-40记录下每个索引的长度,前面40位为索引头
 * 5. 每个索引占4k大小
 * 6. 每个索引内部前10为记录下自己的offset
 * 7. 11-20记录下父节点
 * 8. 21-30记录下左节点
 * 9. 31-40记录下右节点
 * 10. 41记录类型,1为叶子,0为非叶子,42-50记录下有多少个键值对
 * 11. 后面记录索引数据,以键值对的方式存放,值长度为10位int数据
 * */
// 用于索引文件的操作函数,记得随用随释放
#define buffSize 4096

void createIndexFile(vector<Attribute> &types, string tableName)
{
    string fileName, fileType;
    fileName = tableName + "_";
    for (Attribute attr : types)
    {
        fileName += attr.name + "_";
    }
    fileType = fileName;
    fileType += ".type";
    fileName += ".index";
    fstream file(fileName.c_str(), ios::out);
    if (!file)
    {
        cerr << "file open error";
        return;
    }
    char buff[11] = {0};
    buff[0] = '-';
    buff[1] = '1';
    file.write(buff, 10); // root
    file.write(buff, 10); // maxleft
    buff[0] = '0';
    buff[1] = '\0';
    file.write(buff, 10); // 第一个位置
    int len = 0;
    for (Attribute attr : types)
    {
        len += attr.len;
    }
    buff[0] = '\0';
    memset(buff, '\0', sizeof(buff));
    strcpy(buff, to_string(len).c_str());
    file.write(buff, 10); // 每个节点的长度
    file.close();
    // index文件写完,开始写入type文件
    file.open(fileType.c_str(), ios::out);
    if (!file)
    {
        cerr << "file open error";
        return;
    }
    file << types.size() << ' ';
    for (Attribute attr : types)
    {
        file << attr.name << ' ' << attr.type << ' ' << attr.len << ' ';
    }
    file.close();
}

// 用于获取索引文件的类型
vector<Attribute> getIndexType(string fileName)
{
    vector<Attribute> types;
    string indexTypeFile = fileName + ".type";
    fstream file(indexTypeFile.c_str(), ios::in);
    if (!file)
    {
        return types;
    }
    int len;
    file >> len;
    for (int i = 0; i < len; i++)
    {
        string name;
        int length;
        int ty;
        file >> name >> ty >> length;
        types.push_back(Attribute(name, ty, length));
    }
    return types;
}

BNode *getNode(string fileName, int offset)
{
    if (offset == -1)
        return nullptr;
    vector<Attribute> types = getIndexType(fileName);
    string tempFile = fileName;
    fileName += ".index";
    fstream file(fileName.c_str(), ios::out | ios::in | ios::binary);
    if (!file)
    {
        return nullptr;
    }
    // 后续版本,用来优化读取速度
    char buff[4097] = {0};
    file.seekg(30, ios::beg);
    file.read(buff, 10);
    int len = atoi(buff); // 索引长度
    memset(buff, '\0', 10);
    int o = (buffSize - 40) / (len + 10); // 节点的度
    file.seekg(offset * buffSize + 50, ios::beg);
    file.read(buff, buffSize);              // 一次读入一个节点
    int parent = stoi(string(buff, 0, 10)); //?
    int left = stoi(string(buff + 10, 0, 10));
    int right = stoi(string(buff + 20, 0, 10));
    int type = buff[30] == '1' ? 1 : 0;
    int num = stoi(string(buff + 31, 0, 9)); // 一共多少个数据
    int pos = 40;
    //
    BNode *node;
    if (type == 1)
    {
        // 叶子结点
        node = new LeafNode(o, parent, tempFile);
        node->offset = offset;
        node->leftOffset = left;
        node->rightOffset = right;
        // 载入数据
        for (int i = 0; i < num; i++)
        {
            variableType temp;
            temp.type = types;
            for (Attribute attr : types)
            {
                temp.value.emplace_back(string(buff + pos, 0, attr.len));
                pos += attr.len;
            }
            int value = stoi(string(buff + pos, 0, 10));
            pos += 10;
            node->Index.emplace_back(temp);
            ((LeafNode *)node)->Data.emplace_back(value);
        }
    }
    else
    {
        // 卫星节点
        node = new IntervalNode(o, parent, fileName);
        node->offset = offset;
        node->leftOffset = left;
        node->rightOffset = right;
        for (int i = 0; i < num; i++)
        {
            variableType temp;
            temp.type = types;
            for (Attribute attr : types)
            {
                temp.value.emplace_back(string(buff + pos, 0, attr.len));
                pos += attr.len;
            }
            int value = stoi(string(buff + pos, 0, 10));
            pos += 10;
            node->Index.emplace_back(temp);
            ((IntervalNode *)node)->PointersOffset.emplace_back(value);
        }
    }
    file.close();
    node->fileName = tempFile;
    return node;
}

// 获取根节点
BNode *getRoot(string fileName)
{
    string tempFile = fileName;
    fileName += ".index";
    fstream file(fileName.c_str(), ios::in);
    if (!file)
    {
        return nullptr;
    }
    char buff[11] = {0};
    int rootOffset;
    file.read(buff, 10);
    rootOffset = atoi(buff); // 根节点位置
    file.close();
    return getNode(tempFile, rootOffset);
}

// 获取最左节点
BNode *getMaxLeft(string fileName)
{
    string tempFile = fileName;
    fileName += ".index";
    fstream file(fileName.c_str(), ios::in);
    if (!file)
    {
        return nullptr;
    }
    char buff[11] = {0};
    int maxLeftOffset;
    file.seekg(10, ios::beg);
    file.read(buff, 10);
    maxLeftOffset = atoi(buff); // 最左节点位置
    return getNode(tempFile, maxLeftOffset);
}

// 设置根节点
void setRoot(string fileName, int offset)
{
    fileName += ".index";
    fstream file(fileName.c_str(), ios::in | ios::out | ios::binary);
    if (!file)
    {
        return;
    }
    char buff[11] = {0};
    strcpy(buff, to_string(offset).c_str());
    file.write(buff, 10);
    file.close();
}

// 设置最左节点
void setMaxLeft(string fileName, int offset)
{
    fileName += ".index";
    fstream file(fileName.c_str(), ios::in | ios::out | ios::binary);
    if (!file)
    {
        return;
    }
    file.seekp(10, ios::beg);
    char buff[11] = {0};
    memset(buff, '\0', sizeof(buff));
    strcpy(buff, to_string(offset).c_str());
    file.write(buff, 10);
    file.close();
}

//
void deleteNode(string fileName, int offset)
{
    fileName += ".index";
    fstream file(fileName, ios::in | ios::out);
    file.seekp(buffSize * offset + 40, ios::beg);
    for (int i = 0; i < buffSize; i++)
    {
        file.write("\0", 1); // 用空覆盖
    }
    file.close();
    file.open(fileName, ios::in | ios::out | ios::binary);
    char buff[11] = {0};
    file.seekg(20, ios::beg);
    file.read(buff, 10);
    int offset1 = atoi(buff);
    file.close();
    if (offset1 < offset)
        return;
    file.open(fileName, ios::in | ios::out | ios::binary);
    file.seekp(20, ios::beg);
    memset(buff, '\0', sizeof(buff));
    strcpy(buff, to_string(offset).c_str());
    file.write(buff, 10);
    file.close();
}

//
void updateNode(string fileName, BNode *node)
{
    vector<Attribute> types = getIndexType(fileName);
    fileName += ".index";
    fstream file(fileName, ios::in | ios::out | ios::binary);
    file.seekp(buffSize * node->offset + 40, ios::beg);
    char buff[4097] = {0};
    int pos = 0;
    strcpy(buff + pos, to_string(node->offset).c_str());
    pos += 10;
    strcpy(buff + pos, to_string(node->parentOffset).c_str());
    pos += 10;
    strcpy(buff + pos, to_string(node->leftOffset).c_str());
    pos += 10;
    strcpy(buff + pos, to_string(node->rightOffset).c_str());
    pos += 10;
    if (node->getType() == LEAF)
    {
        strcpy(buff + pos, "1");
        pos++;
        strcpy(buff + pos, to_string(node->getNum()).c_str());
        pos += 9;
        for (int i = 0; i < node->getNum(); i++)
        {
            for (int j = 0; j < types.size(); j++)
            {
                Attribute ty = types[j];
                strcpy(buff + pos, node->Index[i].value[j].c_str());
                pos += ty.len;
            }
            strcpy(buff + pos, to_string(((LeafNode *)node)->Data[i]).c_str());
            pos += 10;
        }
    }
    else
    {
        strcpy(buff + pos, "0");
        pos++;
        strcpy(buff + pos, to_string(node->getNum()).c_str());
        pos += 9;
        for (int i = 0; i < node->getNum(); i++)
        {
            for (int j = 0; j < types.size(); j++)
            {
                Attribute ty = types[j];
                strcpy(buff + pos, node->Index[i].value[j].c_str());
                pos += ty.len;
            }
            strcpy(buff + pos, to_string(((IntervalNode *)node)->PointersOffset[i]).c_str());
            pos += 10;
        }
    }
    file.write(buff, buffSize);
    file.close();
}

// 获得一个空位的偏移量
int getNewNodeOffset(string fileName)
{
    fileName += ".index";
    fstream file(fileName.c_str(), ios::in);
    if (!file)
    {
        cerr << "error";
    }
    char buff[4097] = {0};
    file.seekg(20, ios::beg);
    file.read(buff, 10);
    int offset = atoi(buff); // 第一个空位
    int result = offset;
    file.close();
    file.open(fileName, ios::in | ios::out | ios::binary);
    file.seekp(offset * buffSize + 40, ios::beg);
    memset(buff, '\0', sizeof(buff));
    strcpy(buff, to_string(offset).c_str()); // 头部有偏移量表示已经被占用
    file.write(buff, buffSize);
    file.close();
    file.open(fileName, ios::in);
    while (true)
    { // 更新第一个位置
        file.seekg(buffSize * offset + 40, ios::beg);
        char temp = '\0';
        file.read(&temp, 1);
        if (temp == '\0')
        {
            break;
        }
        offset++;
    }
    file.close();
    file.open(fileName, ios::out | ios::in);
    memset(buff, '\0', sizeof(buff));
    strcpy(buff, to_string(offset).c_str());
    file.seekp(20, ios::beg);
    file.write(buff, 10);
    file.close();
    return result;
}

// BPlusTree
void BPlusTree::clearUsed()
{
    cache->clear();
}

BPlusTree::BPlusTree(vector<Attribute> &type, string tableName)
{
    ifRepetition = false;
    this->tableName = tableName;
    this->type = type;
    num = 0;
    MaxLeft = nullptr;
    root = nullptr;
    string file = tableName + "_";
    for (int i = 0; i < type.size(); i++)
    {
        file += type[i].name + "_";
    }
    fileName = file;
    int len = 0;
    for (int i = 0; i < type.size(); i++)
    {
        len += type[i].len;
    }
    fstream f(fileName + ".index", ios::in);
    if (!f)
    {
        createIndexFile(type, tableName);
    }
    order = (buffSize - 40) / (len + 10);
}

BPlusTree::~BPlusTree()
{
    clearUsed();
}

int BPlusTree::find(variableType key)
{
    // 在最后返回的时候清空used
    if (!root)
        root = ::getRoot(fileName);
    if (root)
    {
        root->setTree(this);
    }
    return find(root, key);
}

int BPlusTree::find(BNode *node, variableType key)
{
    if (node == nullptr)
    { // 没找到返回-1
        return -1;
    }
    else if (node->getType() == LEAF)
    {
        int off = node->findKey(key);
        return off;
    }
    else
    {
        int index = node->findIndex(key);
        if (index == node->getNum())
        {
            return -1;
        }
        else
        {
            IntervalNode *temp = (IntervalNode *)cache->get(fileName + to_string(((IntervalNode *)node)->PointersOffset[index]));
            if (temp == nullptr)
                temp = (IntervalNode *)getNode(fileName, ((IntervalNode *)node)->PointersOffset[index]);
            temp->setTree(this);
            temp->setParent(node); // 保持父节点不为空
            return find(temp, key);
        }
    }
}

bool BPlusTree::insert(variableType key, int data)
{
    if (!root)
        root = ::getRoot(fileName);
    if (root)
        root->setTree(this);
    if (root == nullptr)
    {
        root = new LeafNode(order, nullptr, fileName); // 节点数量少,所以直接使用LeafNode<variableType,int>代替
        root->offset = getNewNodeOffset(fileName);
        root->setParent(nullptr);
        root->setTree(this);
        setRoot(root); // 开局先让root和maxleft指向同一个LeafNode<variableType,int>
        setMaxLeft((LeafNode *)root);
    }
    bool off = insert(root, key, data);
    // if (off)
    // {
    //     cout << "Insert" << key.value[0] << " " << data << endl;
    // }
    num++;
    return off;
}

bool BPlusTree::insert(BNode *node, variableType key, int data)
{
    if (node->getType() == INTERNAL)
    {
        int index = ((IntervalNode *)node)->findIndex(key);
        if (index == node->getNum())
        {
            // 表示超过了当前节点最大值,需要更新最大节点的值
            node->setIndex(index - 1, key); // 其实可以删了,保险起见不删
            IntervalNode *temp = (IntervalNode *)cache->get(fileName + to_string(((IntervalNode *)node)->PointersOffset[index - 1]));
            if (temp == nullptr)
                temp = (IntervalNode *)getNode(fileName, ((IntervalNode *)node)->PointersOffset[index - 1]);
            // IntervalNode *temp = (IntervalNode *)getNode(fileName, ((IntervalNode *)node)->PointersOffset[index - 1]);
            temp->setTree(this);
            temp->parent = node;
            return insert(temp, key, data);
        }
        else
        {
            IntervalNode *temp = (IntervalNode *)cache->get(fileName + to_string(((IntervalNode *)node)->PointersOffset[index]));
            if (temp == nullptr)
                temp = (IntervalNode *)getNode(fileName, ((IntervalNode *)node)->PointersOffset[index]);
            temp->setTree(this);
            temp->parent = node;
            return insert(temp, key, data); // 自动更新最大值
        }
    }
    else
    {
        // 插入
        return ((LeafNode *)node)->insertKey(key, data); // 自动向上调用更新
    }
}

vector<int> BPlusTree::rangeSearch(variableType pre, variableType last)
{
    // 范围查找
    BNode *temp = getRoot();
    if (temp == nullptr)
        return {};
    temp->setTree(this);
    while (temp->getType() != LEAF)
    {
        int index = ((IntervalNode *)temp)->findIndex(pre);
        int offset = ((IntervalNode *)temp)->PointersOffset[index];
        if (index == temp->getNum())
            return {}; // 空
        temp = cache->get(fileName + to_string(offset));
        if (temp == nullptr)
            temp = getNode(fileName, offset);
        temp->setTree(this);
    }
    vector<int> result;
    int index = ((LeafNode *)temp)->findIndex(pre);
    // 向左找到第一个满足条件的
    while (temp != nullptr && temp->getIndex(index) <= last)
    {
        result.push_back(((LeafNode *)temp)->getData(index));
        index++;
        if (index == temp->getNum())
        {
            temp = ((LeafNode *)temp)->getRightSibling();
            index = 0;
        }
    }
    return result;
}

vector<int> BPlusTree::EqualSearch(variableType key)
{
    BNode *temp = getRoot();
    if (temp == nullptr)
        return {};
    temp->setTree(this);
    while (temp->getType() != LEAF)
    {
        int index = ((IntervalNode *)temp)->findIndex(key);
        int offset = ((IntervalNode *)temp)->PointersOffset[index];
        if (index == temp->getNum())
            return {}; // 空
        temp = cache->get(fileName + to_string(offset));
        if (temp == nullptr)
            temp = getNode(fileName, offset);
        temp->setTree(this);
    }
    vector<int> result;
    int index = ((LeafNode *)temp)->findIndex(key);
    while (temp != nullptr && temp->getIndex(index) == key)
    {
        result.push_back(((LeafNode *)temp)->getData(index));
        index++;
        if (index == temp->getNum())
        {
            temp = ((LeafNode *)temp)->getRightSibling();
            if (temp != nullptr)
                index = 0;
        }
    }
    return result;
}

vector<int> BPlusTree::BigSearch(variableType key)
{
    BNode *temp = getRoot();
    if (temp == nullptr)
        return {};
    temp->setTree(this);
    while (temp->getType() != LEAF)
    {
        int index = ((IntervalNode *)temp)->findIndex(key);
        int offset = ((IntervalNode *)temp)->PointersOffset[index];
        if (index == temp->getNum())
            return {}; // 空
        temp = cache->get(fileName + to_string(offset));
        if (temp == nullptr)
            temp = getNode(fileName, offset);
        temp->setTree(this);
    }
    vector<int> result;
    int index = ((LeafNode *)temp)->findIndex(key);
    while (temp != nullptr)
    {
        variableType t = temp->getIndex(index);
        bool flag = true;
        for (int i = 0; i < t.type.size(); i++)
        {
            for (int j = 0; j < key.type.size(); j++)
            {
                if (t.type[i].name == key.type[j].name)
                {
                    if (type[i].type == Attribute_Int)
                    {
                        if (stoi(t.value[i]) < stoi(key.value[j]))
                        {
                            flag = false;
                            break;
                        }
                    }
                    else if (type[i].type == Attribute_Double)
                    {
                        if (stod(t.value[i]) < stod(key.value[j]))
                        {
                            flag = false;
                            break;
                        }
                    }
                    else
                    {
                        if (t.value[i] < key.value[j])
                        {
                            flag = false;
                            break;
                        }
                    }
                }
            }
        }
        if (flag)
            result.push_back(((LeafNode *)temp)->getData(index));
        index++;
        if (index == temp->getNum())
        {
            temp = ((LeafNode *)temp)->getRightSibling();
            index = 0;
        }
    }
    return result;
}

vector<int> BPlusTree::SmallSearch(variableType key)
{
    BNode *temp = getRoot();
    if (temp == nullptr)
        return {};
    temp->setTree(this);
    while (temp->getType() != LEAF)
    {
        int index = ((IntervalNode *)temp)->findIndex(key);
        int offset = ((IntervalNode *)temp)->PointersOffset[index];
        if (index == temp->getNum())
            return {}; // 空
        temp = cache->get(fileName + to_string(offset));
        if (temp == nullptr)
            temp = getNode(fileName, offset);
        temp->setTree(this);
    }
    vector<int> result;
    // 向右寻找相等的值
    BNode *rightTemp = temp;
    int index = ((LeafNode *)temp)->findIndex(key);
    int rightIndex = index + 1;
    if (rightIndex == rightTemp->getNum())
    {
        rightTemp = ((LeafNode *)temp)->getRightSibling();
        rightIndex = 0;
    }
    while (rightTemp != nullptr)
    {
        variableType t = rightTemp->getIndex(rightIndex);
        bool flag = true;
        for (int i = 0; i < t.type.size(); i++)
        {
            if (!flag)
                break;
            for (int j = 0; j < key.type.size(); j++)
            {
                if (t.type[i].name == key.type[j].name)
                {
                    if (t.value[i] != key.value[j])
                    {
                        flag = false;
                        break;
                    }
                }
            }
        }
        if (flag)
            result.push_back(((LeafNode *)rightTemp)->getData(rightIndex));
        else
            break;
        rightIndex++;
        if (rightIndex == rightTemp->getNum())
        {
            rightTemp = ((LeafNode *)rightTemp)->getRightSibling();
            rightIndex = 0;
        }
    }
    while (temp != nullptr)
    {
        variableType t = temp->getIndex(index);
        bool flag = true;
        for (int i = 0; i < t.type.size(); i++)
        {
            for (int j = 0; j < key.type.size(); j++)
            {
                if (t.type[i].name == key.type[j].name)
                {
                    if (type[i].type == Attribute_Int)
                    {
                        if (stoi(t.value[i]) > stoi(key.value[j]))
                        {
                            flag = false;
                            break;
                        }
                    }
                    else if (type[i].type == Attribute_Double)
                    {
                        if (stod(t.value[i]) > stod(key.value[j]))
                        {
                            flag = false;
                            break;
                        }
                    }
                    else
                    {
                        if (t.value[i] > key.value[j])
                        {
                            flag = false;
                            break;
                        }
                    }
                }
            }
        }
        if (flag)
            result.push_back(((LeafNode *)temp)->getData(index));
        index--;
        if (index == -1)
        {
            temp = ((LeafNode *)temp)->getLeftSibling();
            if (temp != nullptr)
                index = temp->getNum() - 1;
        }
    }
    return result;
}

int BPlusTree::remove(variableType key)
{ // 直接通过remove返回删除的索引
    root = getRoot();
    if (root == nullptr)
        return -1;
    root->setTree(this);
    int off = remove(root, key);
    --num;
    return off;
}

int BPlusTree::remove(BNode *node, variableType key)
{
    if (node->getType() == INTERNAL)
    { // 卫星节点,需要查找下面的卫星节点,当调用这个函数的时候保证key存在
        int index = ((IntervalNode *)node)->findIndex(key);
        BNode *temp = cache->get(fileName + to_string(((IntervalNode *)node)->PointersOffset[index]));
        if (temp == nullptr)
            temp = getNode(fileName, ((IntervalNode *)node)->PointersOffset[index]);
        temp->setParent(node);
        temp->setTree(this);
        return remove(temp, key);
    }
    else
    {
        // 到达叶子结点,直接删除
        return ((LeafNode *)node)->removeKey(key);
    }
}