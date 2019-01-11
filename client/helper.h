//
// Created by hhx on 2019/1/10.
//

#ifndef LOGREC_HELPER_H
#define LOGREC_HELPER_H

#include <cstdint>
#include <string>
#include <vector>
#include <pthread.h>

namespace LogRec
{


using SARR = std::vector<std::string>;

struct Field {
    std::string field;  // TODO: maybe only filed_ID exist
    std::string value;  // TODO: maybe only integer exist
    bool isint;
};

struct KeyValue {
    std::string key;
    std::vector<Field> fields;

    void sort() {
        //TODO: sort here
    }

    void output() {
        //TODO: call md5, and write to file
    }
};

enum OPTCODE {
    HMSET = 0,
    HINCRBY = 1,
    HDEL = 2,
    RENAME = 3,
    DEL = 4,
};

struct LogHeader {
    int64_t timestamp;
    std::string opt;
    std::string key;
};

struct LogRecord {
    int32_t mins;
    OPTCODE code;
    std::string oldkey;
    std::string curkey;
    std::vector<Field> fileds;
};

struct MinLogRecord {
    int32_t mins;
    std::vector<LogRecord> records;
};

struct SingleThreadRecord {
    int current;
    std::vector<MinLogRecord> data; // TODO: remember remove useless data
};

struct KeyThreadId {
    std::string key;
    int32_t hash;
    int32_t mins;
};

enum THREAD_STATE {
    PARSE_DATA = 1,
    WRITE_FILE = 2,
    TIME2_EXIT = 3,
};

struct FileWriter {
    FileWriter() {}

    virtual ~FileWriter() {}

    void AddRecord(KeyValue &kv) {}

    int32_t size;
    void *addr;
};

struct ThreadInfo {
    THREAD_STATE state;
    int32_t num;
    std::vector<pthread_t> tids;
    std::vector<FileWriter> writer;
    std::vector<SingleThreadRecord> thread_record;
    KeyThreadId key_tids;
};

}

#endif //LOGREC_HELPER_H
