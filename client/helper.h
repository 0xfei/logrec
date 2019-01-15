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
#include "md5/time_consuming_md5.h"

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
	FileWriter() { size = 0; addr = malloc(FW_SIZE); }

    void AddRecord(KeyValue *kv) {
    	std::string key(KEY_PREFIX);
    	key = key + std::to_string(kv->key);
    	MemCpy(key.c_str(), key.size());
    	AddField(kv, 1);
    	AddField(kv, 10);
	    AddField(kv, 100);
    	for (int i = 1; i < 10; ++i) {
    		AddField(kv, 10 + i);
    	}
    	for (int i = 2; i < 10; ++i) {
    		AddField(kv, i);
    		for (int j = 0; j < 10; ++j) {
    			AddField(kv, i*10 + j);
    		}
    	}
	    ((char*)addr)[size++] = '\n';
    }

    void AddField(KeyValue *kv, int fd) {
	    if (kv->fields[fd] <= 0) return;
	    ((char*)addr)[size++] = ' ';
	    std::string field(FIELD_PREFIX);
	    field = field + std::to_string(fd);
	    MemCpy(field.c_str(), field.size());
	    ((char*)addr)[size++] = ' ';
	    std::string value = time_consuming_md5(std::to_string(kv->fields[fd]));
	    // TODO: call md5_consumer
	    MemCpy(value.c_str(), value.size());
	}

    void MemCpy(const char* str, int length) {
		for (auto i = 0; i < length; ++i) {
			((char*)addr)[size++] = str[i];
		}
	}

    int32_t size;
    void *addr;
};



enum THREAD_STATE {
	PARSE_DATA = 1,
	FINIS_PARS = 2,
};

#define DATA_BLOCK_SIZE (1024*1024*1024)

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

#define TOTAL_RECORD_SIZE   210000000

extern ThreadInfo g_thread_info;

}

#endif //LOGREC_HELPER_H
