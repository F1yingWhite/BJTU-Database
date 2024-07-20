#ifndef REDOLOG222_H
#define REDOLOG222_H


#include <iostream>
#include <fstream>
#include <ostream>
#include <chrono>
#include <string>
#include <vector>
#include <algorithm>
#include "versionManager.h"


using namespace std;

// 定义日志记录的类型
struct LogRecord {
    uint32_t offset; // 偏移量
    std::string time;
    //char* 400字节的数组
    std::string info; // 信息
    // uint64_t prev_value; // 原值
    // uint64_t new_value; // 新值
    LogRecord() {
        offset = 0;
        time = "null";
        info = "null";
    };
};

const uint32_t record_size = 2*sizeof(uint32_t)+400; // 一条日志记录的最大大小
const uint32_t buffer_size = 1024; // 缓冲区大小
const uint32_t max_record_num = 2; // 缓冲区最多能存放的日志记录数
const uint64_t capacity_size = 40000000; // redo log 的容量


class RedoLogManager {
private:
    uint64_t capacity_; // redo log 的容量
    uint64_t cur_offset_; // 当前写入位置的偏移量 实现binlog的反复写回滚写
    uint64_t buffer_record_num_; // 缓冲区中当前存放的日志记录数
    fstream  out_stream_;
    fstream check_stream;
    ifstream  in_stream_;
    LogRecord* buffer = new LogRecord[max_record_num]; // 缓冲区
    const std::string default_file_path = "../build/redolog.txt"; // 默认的 redo log 文件路径
    uint64_t cur_final_pos; //当前文件的最后位置
    versionManager vm;
    

public:
    RedoLogManager() {
        capacity_ = capacity_size;
        buffer_record_num_ = 0;
        in_stream_.open(default_file_path, ios::in);
        check_stream.open(default_file_path,ios::out|ios::in);
        out_stream_.open(default_file_path,ios::out|ios::app);
         std::string line;
         getline(in_stream_,line);
         cur_offset_ = atoi(line.c_str());
         getline(in_stream_,line);
         cur_final_pos = atoi(line.c_str());
         cout<<cur_offset_<<" "<<cur_final_pos<<endl;
//        cout<<"cur_offset_:"<<cur_offset_<<endl;
    }

    ~RedoLogManager() {
        //update the cur
        commit();//提交缓冲区,可以不提交
        out_stream_.close();
        out_stream_.open(default_file_path,ios::out | ios::in);
        out_stream_<<0<<endl;
        out_stream_<<cur_final_pos<<endl;
        if (out_stream_.is_open()) {
            out_stream_.close();
        }
        if(in_stream_.is_open()) {
            in_stream_.close();
        }
        delete [] buffer;
    }    // 打开 redo log 文件，以追加写入的方式打开

    bool write_record(const LogRecord& record);  // 将一条记录写入 redo log
    bool write_record(string info);  // 将多条记录写入 redo log
    LogRecord read_record(uint64_t offset);//读一条,读指定位置
//    LogRecord read_record();//读一条,先读换红曲，再读文件
    vector<LogRecord> read_all_records(uint64_t offset);//读取日志记录,先读取缓冲区，再读取文件
    bool rollback(uint64_t offset);   // 回滚到指定位置的日志
    bool commit();  // 提交缓冲区中的日志记录

    void set_checkpoint() {
        check_stream.seekp(0,ios::beg);
        check_stream<<1<<endl;
        check_stream<<cur_final_pos<<endl;
    }
    void remove_checkpoint() {
//        check_stream.open(default_file_path,ios::out|ios::in);
        check_stream.seekp(0,ios::beg);
        check_stream<<0<<endl;
        check_stream<<cur_final_pos<<endl;
    }

    string show_all_version(){
        vm.load();
        string info = R"(1\r\n)";
        for(auto v : vm.versions){
            info += std::to_string(v.id)+" "+v.info+R"(\n)";
        }
        info +=R"(\r\n)";
        return info;
    }

    void commit(string info){
        vm.load();
        version v;
        v.id = cur_final_pos;
        v.info = info;
        vm.add(v);
        vm.write();
    }

    // 关闭 redo log 文件
    bool close() {
        if (!out_stream_.is_open()) {
            return false;
        }
        out_stream_.close();
        return true;
    }

 // 清空 redo log
    bool clear() {
        if (!out_stream_.is_open()) {
            return false;
        }
        out_stream_.seekp(0, ios::beg);
        // out_stream_.truncate(0);  //截断函数,貌似版本不适用了
        cur_offset_ = 0;
        buffer_record_num_ = 0;
        return true;
    }
    // 获取当前写入位置的偏移量
    uint64_t get_cur_offset() {
        return cur_offset_;
    }
    // 获取 redo log 的容量
    uint64_t get_capacity() {
        return capacity_;
    }
    // 获取缓冲区中当前存放的日志记录数
    uint64_t get_buffer_record_num() {
        return buffer_record_num_;
    }
    // 获取缓冲区的大小
    uint64_t get_buffer_size() {
        return buffer_size;
    }
    // 获取一条日志记录的大小
    uint64_t get_record_size() {
        return record_size;
    }
    
    void open() {
        //打开文件夹,如果打开失败,就创建一个文件夹就创建
        out_stream_.open("build/redolog.txt",ios::out|ios::app);
        if(out_stream_.is_open()){
        }
        else{
            cout<<"open fail"<<endl;
        }
    }
    
};

#endif
