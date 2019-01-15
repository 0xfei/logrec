//
// Created by hhx on 2019/1/10.
//

#ifndef LOGREC_HELPER_H
#define LOGREC_HELPER_H

#include <cstdint>
#include <string>
#include <vector>
#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

namespace LogRec
{

#define MAX_HASH    16000000

#define FIELD_PREFIX        "field_"
#define FIELD_PREFIX_SIZE   6
#define KEY_PREFIX          "OD_"
#define KEY_PREFIX_SIZE     3

struct TimeUsage {
	TimeUsage(std::string hd): header(hd) { }

	void Start() {
		beg = GetTime();
	}
	void Output() {
		dst = GetTime();
		sprintf(s, "%s: %lldms\n", header.c_str(), dst-beg);
		write(2, s, strlen(s));
	}

	int64_t GetTime() {
		struct timeval tv;
		gettimeofday(&tv,NULL);
		return tv.tv_sec*1000 + tv.tv_usec/1000;
	}

	std::string header;
	char s[100];
	int64_t beg;
	int64_t dst;
};

struct KeyValue {
	KeyValue(): field_num(0) {}
    int32_t key;
    int32_t field_num;
    int64_t fields[101];
    std::string buffer;
};

enum OPTCODE {
    HMSET = 5,
    HINCRBY = 7,
    HDEL = 4,
    RENAME = 6,
    DEL = 3,
};

struct LogRecord {
	LogRecord() {
		timestamp = 0;
		field_num = 0;
		curkey = 0;
		newkey = 0;
	}
	int64_t timestamp;
    OPTCODE code;
    int curkey;
	int newkey;
	int field_num;
	int field[5];
	int64_t value[5];
};


#define FW_SIZE (512*1024*1024)
struct FileWriter {
	FileWriter() { size = 0; addr = (char*)malloc(FW_SIZE); }

    void AddRecord(KeyValue *kv) {
		memcpy(addr, kv->buffer.c_str(), kv->buffer.size());
		size += kv->buffer.size();
    }

    int32_t size;
    char *addr;
};


enum THREAD_STATE {
	PARSE_DATA = 1,
	OPERATE_HASH = 2,
	WRITE_DATA = 3,
};


struct ThreadInfo {
    int32_t num;
    pthread_t recv_tid;
    pthread_t exec_tid;
	std::vector<THREAD_STATE> state;
	std::vector<char*> data_vec;
	std::vector<int> data_size;
    std::vector<pthread_t> worker;
    std::vector<FileWriter> writer;
	std::vector<int> tids;
};

extern ThreadInfo g_thread_info;

}

#endif //LOGREC_HELPER_H
