#include "Log/RedoLogManager.h"
#include "Log/RedoLogManager.cpp"
#include "BPlusTree2/CacheHead.h"
#include "DB8/DB8.h"
#include "Connect/Server.h"

using namespace std;

int main()
{
    int mode;
    cin >> mode;
    if (mode == 1)
    {
        DB8 *db = new DB8();
        // db->te();
        db->start();
    }
    else
    {
        Server s(1001);
        s.run();
    }
}