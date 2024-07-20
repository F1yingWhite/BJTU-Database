#include "BNode.h"
#include "BPlusTree.h"

BNode *BNode::createParent()
{
    if (parentOffset == -1)
        return nullptr;
    BNode *par = cache->get(fileName + to_string(parentOffset));
    if (par == nullptr)
    {
        par = getNode(fileName, parentOffset);
        par->setTree(tree);
    }
    parent = par;
    return par;
}

void BNode::setTree(BPlusTree *tree)
{
    this->tree = tree;
    // for (int i = 0; i < tree->used.size(); i++)
    // {
    //     if (tree->used[i] == this)
    //         return;
    // }
    cache->put(fileName + to_string(offset), this); // 把自己放入缓存
    // tree->used.emplace_back(this);
}

int BNode::findIndex(variableType key)
{
    // 查找插入范围
    for (int i = 0; i < getNum(); i++)
    {
        if (key <= Index[i])
            return i;
    }
    return getNum();
}

int BNode::findKey(variableType key)
{
    // 二分查找寻找对应位置,号召对应的key存在的最左位置
    int left = 0;
    int right = Index.size();
    int middle = 0;
    while (left < right)
    {
        middle = left + ((right - left) / 2);
        if (Index[middle] > key)
        {
            right = middle;
        }
        else if (Index[middle] < key)
        {
            left = middle + 1;
        }
        else
        {
            break;
        }
    }
    // 没找到就返回-1
    if (left >= right)
        return -1;
    else
    {
        while (middle > 0 && Index[middle - 1] == Index[middle])
        {
            middle--;
        }
        return middle;
    }
}

BNode *BNode::getLeftSibling()
{
    if (leftOffset == -1)
        return nullptr;
    BNode *temp = cache->get(fileName + to_string(leftOffset));
    if (temp == nullptr)
        temp = getNode(fileName, leftOffset);
    temp->setTree(this->tree);
    // 获得左孩子的父亲节点
    temp->createParent();
    return temp;
}

BNode *BNode::getRightSibling()
{
    if (rightOffset == -1)
        return nullptr;
    BNode *temp = cache->get(fileName + to_string(rightOffset));
    if (temp == nullptr)
        temp = getNode(fileName, rightOffset);
    temp->setTree(this->tree);
    temp->createParent();
    return temp;
}

// 调用这个函数自动生成左右节点
Sibling BNode::ifSiblingEnough()
{
    leftSibling = getLeftSibling();
    rightSibling = getRightSibling();
    if (leftSibling != nullptr)
    {
        if (leftSibling->parentOffset == parentOffset)
        {
            if (leftSibling->getNum() > this->order - this->order / 2)
                return LEFT;
        }
    }
    if (rightSibling != nullptr)
    {
        if (rightSibling->parentOffset == parentOffset)
        {
            if (rightSibling->getNum() > this->order - this->order / 2)
                return RIGHT;
        }
    }
    return NONEHAVE;
}

void IntervalNode::insertKey(BNode *child)
{
    // 注意,这个算法无法修改最后一个值的大小,所以我们需要通过单独一个函数修改最后一个值的大小
    // 卫星节点从下面过来一个孩子节点数据,我们需要依次在data和pointer中添加
    // 并且如果数据超过了自己的上限,还需要分裂
    child->setParent(this);
    variableType target = child->getLastIndex(); // 孩子节点的最大数据
    bool flag = false;
    for (int i = 0; i < this->Index.size(); i++)
    {
        // 找到插入位置
        if (target < this->Index[i])
        {
            flag = true;
            this->Index.insert(this->Index.begin() + i, target);
            PointersOffset.insert(PointersOffset.begin() + i, child->offset);
            break;
        }
    }
    if (!flag)
    { // 当递归下来的时候保证父节点是存在的
        if (this->parentOffset != -1 && this->getNum() != 0)
        {
            if (this->parent == nullptr)
                this->createParent();
            ((IntervalNode *)this->parent)->updateIndex(target, this->getLastIndex());
        }
        this->Index.push_back(target);
        PointersOffset.push_back(child->offset);
    }
    // 如果当前节点已经过饱和,那么就分裂
    if (this->Index.size() > this->order)
    {
        split();
    }
}

void IntervalNode::split()
{
    // 自己太多啦要分裂,并且我们使用向上层递归的方式分裂
    // 当前节点作为右边节点,新分裂出来左节点,这样可以方便更新
    int left = this->getNum() / 2;
    if (this->parentOffset != -1)
    {
        if (this->parent == nullptr)
            this->createParent();
    }
    IntervalNode *newNode = new IntervalNode(this->order, this->parent, this->tree->fileName);
    newNode->offset = getNewNodeOffset(fileName);
    newNode->setTree(this->tree);
    // 重新构建leftSibling和rightSibling
    newNode->rightOffset = this->offset;
    newNode->leftOffset = this->leftOffset;
    if (this->leftOffset != -1)
    {
        BNode *temp = getLeftSibling(); // 当前节点的左节点
        temp->rightOffset = newNode->offset;
    }
    this->leftSibling = newNode;
    this->leftOffset = newNode->offset;
    // 更新新节点以及孩子节点的父亲节点
    newNode->Index.assign(this->Index.begin(), this->Index.begin() + left);
    newNode->PointersOffset.assign(this->PointersOffset.begin(), this->PointersOffset.begin() + left);
    for (int i = 0; i < left; i++)
    {
        BNode *temp = cache->get(fileName + to_string(PointersOffset[i]));
        if (temp == nullptr)
            temp = getNode(fileName, PointersOffset[i]);
        temp->setTree(this->tree);
        temp->setParent(newNode);
    }
    this->Index.erase(this->Index.begin(), this->Index.begin() + left);
    this->PointersOffset.erase(this->PointersOffset.begin(), this->PointersOffset.begin() + left);
    //
    if (this->parentOffset == -1)
    {
        // 根节点分裂,需要更新tree中的节点
        IntervalNode *newRoot = new IntervalNode(this->order, nullptr, this->fileName);
        newNode->setParent(nullptr);
        newRoot->offset = getNewNodeOffset(fileName);
        newRoot->setTree(this->tree);
        newRoot->insertKey(this);
        newRoot->insertKey(newNode);
        this->tree->setRoot(newRoot);
    }
    else
    {
        // 从上往下递归,则父节点一定存在,在B+树种实现代码
        ((IntervalNode *)this->parent)->insertKey(newNode);
    }
}

void IntervalNode::combineWithSibling(Sibling where)
{
    if (this->parentOffset != -1)
    {
        if (this->parent == nullptr)
            this->createParent();
    }
    if (where == LEFT)
    {
        // 将左xd吸过来,已经看过left和right了,所以不用再访问外存了
        int num = this->leftSibling->getNum();
        for (int i = 0; i < num; i++)
        {
            IntervalNode *temp = (IntervalNode *)(cache->get(fileName + to_string(((IntervalNode *)this->leftSibling)->PointersOffset[i])));
            if (temp == nullptr)
            {
                temp = (IntervalNode *)getNode(fileName, ((IntervalNode *)this->leftSibling)->PointersOffset[i]);
            }
            temp->setTree(tree);
            temp->setParent(this);
            insertKey(temp);
        }
        ((IntervalNode *)this->parent)->removeKey(this->leftSibling);
        IntervalNode *temp = (IntervalNode *)this->leftSibling;
        this->leftOffset = temp->leftOffset;
        this->leftSibling = temp->leftSibling;
        if (temp->leftOffset != -1)
        {
            BNode *temp2 = temp->getLeftSibling();
            temp2->setTree(tree);
            temp2->rightSibling = this;
            temp2->rightOffset = this->offset;
        }
        deleteNode(fileName, temp->offset);
        cache->Remove(fileName + to_string(temp->offset)); // 在缓存中删除左xd
        delete temp;
    }
    else
    {
        // 融入右xd
        int num = this->getNum();
        for (int i = 0; i < num; i++)
        {
            IntervalNode *temp = (IntervalNode *)cache->get(fileName + to_string(PointersOffset[i]));
            if (temp == nullptr)
                temp = (IntervalNode *)getNode(fileName, PointersOffset[i]);
            temp->setTree(tree);
            temp->setParent(rightSibling);
            ((IntervalNode *)rightSibling)->insertKey(temp);
        }
        ((IntervalNode *)this->parent)->removeKey(this); // 父节点中删除自己
        IntervalNode *temp = (IntervalNode *)this->rightSibling;
        temp->leftOffset = this->leftOffset;
        temp->leftSibling = this->leftSibling;
        if (this->leftOffset != -1)
        {
            BNode *temp2 = temp->getLeftSibling();
            temp2->setTree(tree);
            temp2->rightSibling = temp;
            temp2->rightOffset = temp->offset;
        }
        deleteNode(fileName, this->offset);
        cache->Remove(fileName + to_string(this->offset)); // 在缓存中删除自己
        delete this;
    }
}

void IntervalNode::DeleteSelf()
{
    // 先在文件删除自己
    deleteNode(fileName, this->offset);
    // 再删除used中的自己
    cache->Remove(fileName + to_string(this->offset)); // 在缓存中删除自己
    // 在讲所有孩子节点的父亲改为-1
    for (int i = 0; i < PointersOffset.size(); i++)
    {
        BNode *temp = cache->get(fileName + to_string(PointersOffset[i]));
        if (temp == nullptr) // 如果没有被使用过
        {
            temp = getNode(fileName, PointersOffset[i]);
            temp->setTree(tree);
        }
        if (temp->parentOffset == offset)
            temp->setParent(nullptr);
    }
}

void IntervalNode::removeKey(BNode *child)
{
    if (this->parentOffset != -1)
    {
        if (this->parent == nullptr)
            this->createParent();
    }
    int num = this->getNum();
    if (child->parentOffset == this->offset)
    {
        child->parentOffset = -1;
    }
    for (int i = 0; i < num; i++)
    {
        if (child->offset == PointersOffset[i])
        {
            variableType target = this->getLastIndex(); // 旧值
            PointersOffset.erase(PointersOffset.begin() + i);
            this->Index.erase(this->Index.begin() + i);
            if (i == num - 1)
            {
                // 删除了当前节点的最大值,需要向上递归更改索引为当前节点目前最后一个值
                ((IntervalNode *)parent)->updateIndex(this->getLastIndex(), target);
            }
            break;
        }
    }
    if (this->getNum() < this->order - this->order / 2)
    {
        // 如果当前节点不够,需要向兄弟节点借,
        if (this->parent == nullptr)
        {
            // 并且如果自身是root节点,且只剩下一个了,那么就删除自身并且将root节点给自己的唯一的孩子,如果不止一个啥都不干
            if (this->getNum() == 1)
            {
                BNode *temp = cache->get(fileName + to_string(PointersOffset[0]));
                // 查看节点有没有被使用
                // for (int i = 0; i < tree->used.size(); i++)
                // {
                //     if (tree->used[i]->offset == PointersOffset[0])
                //     {
                //         temp = tree->used[i];
                //         break;
                //     }
                // }
                if (temp == nullptr)
                {
                    temp = getNode(fileName, PointersOffset[0]);
                    temp->setTree(this->tree);
                }
                this->tree->setRoot(temp);
                this->tree->setMaxLeft((LeafNode *)temp);
                if (temp != nullptr)
                    temp->setParent(nullptr);
                DeleteSelf();
                delete this;
            }
        }
        else
        {
            Sibling where = this->ifSiblingEnough();
            if (where == NONEHAVE)
            {
                // 考虑合并
                if (this->leftSibling != nullptr && this->leftSibling->getParent() == this->parent)
                {
                    // 与左xd合并
                    // 这里就处理情况4,情况5在IntervalNode<variableType,int>的remove中处理
                    combineWithSibling(LEFT);
                }
                else if (this->rightSibling != nullptr && this->rightSibling->getParent() == this->parent)
                {
                    // 与右xd合并
                    combineWithSibling(RIGHT);
                }
            }
            else if (where == LEFT)
            {
                // 左边节点很富有,借左节点的最后一个节点
                BNode *temp = cache->get(fileName + to_string(((IntervalNode *)(this->leftSibling))->getLastPointerOffset()));
                if (temp == nullptr)
                {
                    temp = getNode(fileName, ((IntervalNode *)(this->leftSibling))->getLastPointerOffset());
                    temp->setTree(tree);
                }
                ((IntervalNode *)this->leftSibling)->removeKey(temp);
                this->insertKey(temp);
            }
            else
            {
                // 右边节点很富有,借右节点的第一个节点
                BNode *temp = cache->get(fileName + to_string(((IntervalNode *)(this->rightSibling))->PointersOffset[0]));
                if (temp == nullptr)
                {
                    temp = getNode(fileName, ((IntervalNode *)(this->rightSibling))->PointersOffset[0]);
                    temp->setTree(tree);
                }
                ((IntervalNode *)this->rightSibling)->removeKey(temp);
                this->insertKey(temp);
            }
        }
    }
}

void IntervalNode::updateIndex(variableType newKey, variableType oldKey)
{
    // 该方法用于递归修改某个值的大小
    if (this->parentOffset != -1)
    {
        if (this->parent == nullptr)
            this->createParent();
    }
    int index = this->findKey(oldKey);
    if (index != -1)
    {
        this->Index[index] = newKey;
        if (this->parentOffset != -1)
            ((IntervalNode *)this->parent)->updateIndex(newKey, oldKey);
    }
}

// LeafNode<variableType,int>
bool LeafNode::insertKey(variableType index, int data)
{
    if (this->parentOffset != -1)
    {
        if (this->parent == nullptr)
            this->createParent();
    }
    bool flag = false;
    int off = -1;
    for (int i = 0; i < this->getNum(); i++)
    {
        if (index == this->Index[i] && tree->ifRepetition == false)
        { // 不可重复并且相等
            return false;
        }
        if (index < this->Index[i])
        {
            flag = true;
            this->Index.insert(this->Index.begin() + i, index);
            this->Data.insert(this->Data.begin() + i, data);
            break;
        }
    }
    if (!flag)
    {
        // 如果插入的是最后一个位置,那么自动向上递归更新索引
        if (this->parentOffset != -1 && this->getNum() != 0)
            ((IntervalNode *)this->parent)->updateIndex(index, this->getLastIndex());
        this->Index.push_back(index);
        this->Data.push_back(data);
    }
    if (this->getNum() > this->order)
    {
        // 太多了,需要分解
        split();
    }
    return true;
}

int LeafNode::removeKey(variableType index)
{ // 只移除一个
    if (this->parentOffset != -1)
    {
        if (this->parent == nullptr)
            this->createParent();
    }
    int num = this->getNum();
    int result = -1;
    for (int i = 0; i < num; i++)
    {
        if (index == this->Index[i])
        {
            variableType tempData = this->getLastIndex();
            this->Index.erase(this->Index.begin() + i);
            result = Data[i];
            this->Data.erase(this->Data.begin() + i);
            if (i == this->getNum())
            {
                // 删除了当前节点的最大值,需要让其父节点向上递归更改索引
                if (this->parentOffset != -1)
                    ((IntervalNode *)this->parent)->updateIndex(this->getLastIndex(), tempData);
            }
            break;
        }
    }
    if (result == -1)
        return -1; // 没有找到,返回-1
    if (this->getNum() == 0)
    {
        if (this->parentOffset == -1)
        {
            // 根节点被删除
            this->tree->setMaxLeft(nullptr);
            this->tree->setRoot(nullptr);
            deleteNode(fileName, this->offset);
            // for (int i = 0; i < tree->used.size(); i++)
            // {
            //     if (this == tree->used[i])
            //     {
            //         tree->used.erase(tree->used.begin() + i);
            //         break;
            //     }
            // }
            cache->Remove(fileName + to_string(this->offset));
            delete this;
        }
        return result;
    }
    if (this->getNum() <= this->order / 2 && this->tree->root != this)
    {
        Sibling where = this->ifSiblingEnough();
        if (where == NONEHAVE)
        {
            // 考虑合并
            if (this->leftSibling != nullptr && this->leftSibling->parentOffset == parentOffset)
            {
                // 与左xd合并
                // 这里就处理情况4,情况5在IntervalNode<variableType,int>的remove中处理
                combineWithSibling(LEFT);
            }
            else if (this->rightSibling != nullptr && this->rightSibling->parentOffset == parentOffset)
            {
                // 与右xd合并
                combineWithSibling(RIGHT);
            }
        }
        else if (where == LEFT)
        {
            // 左边节点很富有,借左节点的最后一个节点
            variableType tempIndex = this->leftSibling->getLastIndex();
            int tempData = ((LeafNode *)this->leftSibling)->getData(leftSibling->getNum() - 1);
            ((LeafNode *)leftSibling)->removeKey(tempIndex);
            this->insertKey(tempIndex, tempData);
        }
        else
        {
            // 右边节点很富有,借右节点的第一个节点
            variableType tempIndex = this->rightSibling->getIndex(0);
            int tempData = ((LeafNode *)this->rightSibling)->getData(0);
            ((LeafNode *)this->rightSibling)->removeKey(tempIndex);
            this->insertKey(tempIndex, tempData);
        }
    }
    return result;
}

void LeafNode::combineWithSibling(Sibling where)
{
    if (this->parentOffset != -1)
    {
        if (this->parent == nullptr)
            this->createParent();
    }
    if (where == LEFT)
    {
        // 将左孩子吸过来
        int num = this->leftSibling->getNum();
        this->Data.insert(this->Data.begin(), ((LeafNode *)leftSibling)->Data.begin(),
                          ((LeafNode *)leftSibling)->Data.end());
        this->Index.insert(this->Index.begin(), leftSibling->Index.begin(), leftSibling->Index.end());
        ((IntervalNode *)this->parent)->removeKey(this->leftSibling);

        LeafNode *temp = (LeafNode *)this->leftSibling;
        temp->leftSibling = temp->getLeftSibling(); // 获得左孩子
        this->leftOffset = temp->leftOffset;
        this->leftSibling = temp->leftSibling;
        if (temp->leftOffset != -1)
        {
            ((LeafNode *)(temp->leftSibling))->rightSibling = this;
            ((LeafNode *)(temp->leftSibling))->rightOffset = this->offset;
        }

        deleteNode(fileName, temp->offset);
        // for (int i = 0; i < tree->used.size(); i++)
        // {
        //     if (temp == tree->used[i])
        //     {
        //         tree->used.erase(tree->used.begin() + i);
        //         break;
        //     }
        // }
        cache->Remove(fileName + to_string(temp->offset));
        delete temp;
    }
    else
    {
        // 融入右孩子
        int num = this->getNum();
        // for (int i = 0; i < num; i++)
        // {
        //     ((LeafNode *)this->rightSibling)->insertKey(this->getIndex(i), this->getData(i));
        // }
        ((LeafNode *)this->rightSibling)->Data.insert(((LeafNode *)this->rightSibling)->Data.begin(), this->Data.begin(), this->Data.end());
        ((LeafNode *)this->rightSibling)->Index.insert(((LeafNode *)this->rightSibling)->Index.begin(), this->Index.begin(), this->Index.end());
        ((IntervalNode *)this->parent)->removeKey(this); // 父节点中删除自己

        LeafNode *temp = (LeafNode *)this->rightSibling;
        temp->ifSiblingEnough();
        temp->leftSibling = this->leftSibling;
        temp->leftOffset = this->leftOffset;
        if (this->leftSibling != nullptr)
        {
            ((LeafNode *)this->leftSibling)->rightSibling = temp;
            ((LeafNode *)this->leftSibling)->rightOffset = temp->offset;
        }
        else
        {
            // 没有左节点,代表自己是第一个
            this->tree->setMaxLeft(temp);
        }
        deleteNode(fileName, this->offset);
        // for (int i = 0; i < tree->used.size(); i++)
        // {
        //     if (this == tree->used[i])
        //     {
        //         tree->used.erase(tree->used.begin() + i);
        //         break;
        //     }
        // }
        cache->Remove(fileName + to_string(this->offset));
        delete this;
    }
}

void LeafNode::split()
{
    if (this->parentOffset != -1)
    {
        if (this->parent == nullptr)
            this->createParent();
    }
    // 自己为右节点,新的为左节点
    int left = this->getNum() / 2;
    LeafNode *newNode = new LeafNode(this->order, this->parent, this->fileName);
    newNode->offset = getNewNodeOffset(fileName);
    newNode->setTree(this->tree);
    // 重建索引
    newNode->rightOffset = this->offset;
    newNode->rightSibling = this;
    newNode->leftOffset = this->leftOffset;
    newNode->leftSibling = this->leftSibling;
    if (this->leftOffset != -1)
    {
        this->getLeftSibling()->rightOffset = newNode->offset;
    }
    else
    {
        this->tree->setMaxLeft(newNode);
    }
    this->leftOffset = newNode->offset;
    this->leftSibling = newNode;
    // 重新赋值
    newNode->Index.assign(this->Index.begin(), this->Index.begin() + left);
    newNode->Data.assign(this->Data.begin(), this->Data.begin() + left);
    this->Index.erase(this->Index.begin(), this->Index.begin() + left);
    this->Data.erase(this->Data.begin(), this->Data.begin() + left);
    if (this->parentOffset == -1)
    {
        // 根节点分裂,需要更新tree中的节点
        IntervalNode *newRoot = new IntervalNode(this->order, nullptr, this->fileName);
        newRoot->offset = getNewNodeOffset(fileName);
        newRoot->setTree(this->tree);
        newRoot->insertKey(this);
        newRoot->insertKey(newNode);
        this->setParent(newRoot);
        newNode->setParent(newRoot);
        this->tree->setRoot(newRoot);
    }
    else
    {
        ((IntervalNode *)this->parent)->insertKey(newNode);
    }
}

LeafNode::~LeafNode()
{
}

IntervalNode::~IntervalNode()
{
}