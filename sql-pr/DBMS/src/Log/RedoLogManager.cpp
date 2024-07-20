#include "RedoLogManager.h"

bool RedoLogManager::write_record(const LogRecord &record)
{
    if (!out_stream_.is_open())
    {
        return false;
    }
    // 如果缓冲区还有空间，则先写入缓冲区
    if (buffer_record_num_ < max_record_num)
    {
        buffer[buffer_record_num_++] = record;
        return true;
    }
    // 如果缓冲区已满，则先将缓冲区中的日志写入文件
    // 如果当前写入位置加上记录大小超过了容量，则回滚到开头位置继续写入
    if (cur_offset_ + 1 > capacity_)
    {
        out_stream_.seekp(0, ios::beg);
        cur_offset_ = 0;
    }
    // 依次写入记录id,表id与info
    for (int i = 0; i < max_record_num; i++)
    {
        out_stream_ << buffer[i].offset << "_" << buffer[i].time << "_" << buffer[i].info << "_"
                    << " " << endl;
    }
    buffer_record_num_ = 0;
    buffer[buffer_record_num_++] = record;
    cur_offset_ += max_record_num;
    return true;
}

std::vector<std::string> stringSplit(const std::string &str, char delim)
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

bool RedoLogManager::write_record(string info)
{
    LogRecord record;
    record.info = info;
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    // 转换为时间戳
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    // 转换为字符串
    std::string time_str = std::ctime(&now_c);
    time_str.erase(time_str.find_last_not_of("\n") + 1);
    record.time = time_str;
    if (!out_stream_.is_open())
    {
        return false;
    }
    // 如果缓冲区还有空间，则先写入缓冲区
    if (buffer_record_num_ < max_record_num)
    {
        buffer[buffer_record_num_++] = record;
        return true;
    }
    // 如果缓冲区已满，则先将缓冲区中的日志写入文件
    // 如果当前写入位置加上记录大小超过了容量，则回滚到开头位置继续写入
    if (cur_offset_ + 1 > capacity_)
    {
        out_stream_.seekp(0, ios::end);
        cur_offset_ = 0;
    }
    // 依次写入记录id,表id与info
    for (int i = 0; i < max_record_num; i++)
    {
        out_stream_.seekg(0, std::ios::end);
        buffer[i].offset = out_stream_.tellg();
        cur_final_pos = out_stream_.tellg();
        out_stream_ << buffer[i].offset << "_" << buffer[i].time << "_" << buffer[i].info << "_"
                    << " " << endl;
    }
    buffer_record_num_ = 0;
    buffer[buffer_record_num_++] = record;
    cur_offset_ += max_record_num;
    return true;
}

bool RedoLogManager::commit()
{
    if (!out_stream_.is_open())
    {
        return false;
    }
    for (int i = 0; i < buffer_record_num_; i++)
    {
        out_stream_.seekg(0, std::ios::end);
        buffer[i].offset = out_stream_.tellg();
        cur_final_pos = out_stream_.tellg();
        out_stream_ << buffer[i].offset << "_" << buffer[i].time << "_" << buffer[i].info << "_"
                    << " " << endl;
    }
    buffer_record_num_ = 0;
    cur_offset_ += 1;
    return true;
}

bool RedoLogManager::rollback(uint64_t offset)
{
    if (!out_stream_.is_open() || offset >= capacity_)
    {
        return false;
    }
    //    in_stream_.seekg(offset,ios::beg);
    out_stream_.seekp(offset, ios::beg);
    cur_offset_ = offset;
    return true;
}

// 读取从指定offset开始到末尾的所有record的信息
vector<LogRecord> RedoLogManager::read_all_records(uint64_t offset = 0)
{
    vector<LogRecord> records;
    if (!in_stream_.is_open() || offset >= capacity_)
    {
        return records;
    }
    if (offset == 0)
    {
        offset = cur_final_pos;
    }
    in_stream_.seekg(offset, ios::beg);

    string line;
    while (getline(in_stream_, line))
    {
        std::vector<std::string> elems = stringSplit(line, '_');
        LogRecord record;
        record.offset = stoi(elems[0]);
        record.time = elems[1];
        record.info = elems[2];
        records.push_back(record);
    }
    //    in_stream_>>line;
    //    cout<<line<<endl;
    //    in_stream_.read(reinterpret_cast<char*>(&record.offset), sizeof(record.offset));
    //    in_stream_.read(reinterpret_cast<char*>(&record.time), sizeof(record.time));
    //    in_stream_.read(reinterpret_cast<char*>(&record.info), sizeof(record.info));
    // in_stream_.read(reinterpret_cast<char*>(&record.new_value), sizeof(record.new_value));
    return records;
}
// 读取指定位置的一条信息
LogRecord RedoLogManager::read_record(uint64_t offset = 0)
{
    if (!in_stream_.is_open() || offset >= capacity_)
    {
        return LogRecord();
    }
    if (offset == 0)
    {
        offset = cur_final_pos;
    }
    in_stream_.seekg(offset, ios::beg);
    LogRecord record;
    string line;
    getline(in_stream_, line);
    std::vector<std::string> elems = stringSplit(line, '_');
    record.offset = stoi(elems[0]);
    record.time = elems[1];
    record.info = elems[2];
    //    in_stream_>>line;
    //    cout<<line<<endl;
    //    in_stream_.read(reinterpret_cast<char*>(&record.offset), sizeof(record.offset));
    //    in_stream_.read(reinterpret_cast<char*>(&record.time), sizeof(record.time));
    //    in_stream_.read(reinterpret_cast<char*>(&record.info), sizeof(record.info));
    // in_stream_.read(reinterpret_cast<char*>(&record.new_value), sizeof(record.new_value));
    return record;
}

// 读取一条日志记录,先读取缓冲区,缓冲区为零的话,读取文件
// LogRecord RedoLogManager::read_record() {
//     if (buffer_record_num_ > 0) {
//         return buffer[buffer_record_num_-1];
//
//     }
//     if (!in_stream_.is_open()) {
//         return LogRecord();
//     }
//     LogRecord record;
//     std::string line;
//     std::getline(in_stream_, line);
//     cout << "line:" << line << endl;
//     std::vector<std::string> elems = stringSplit(line,'_');
//     record.offset = elems[0][0]-48;
//     record.time = elems[1];
//     record.info = elems[2];
//     cout<<"offset:"<<record.offset<<endl;
//     cout<<"time:"<<record.time<<endl;
//     cout<<"info:"<<record.info<<endl;
//     return record;
// }
