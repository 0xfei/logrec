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

	void Start() { beg = GetTime(); }
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

struct WField {
	int weight;
	int field;
	int64_t value;
};

struct KeyValue {
	KeyValue(): field_num(0) {}
    int key;
    int field_num;
	int index_num;
	int field_index[101];
	WField  field_info[101];

	void Sort() {
		for (int i = 0; i < index_num - 1; ++i) {
			for (int j = i+1; j < index_num; ++j) {
				auto a = field_info[i];
				auto b = field_info[j];
				if (a.weight > b.weight) {
					field_info[j] = a;
					field_info[i] = b;
				}
			}
		}
	}
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
		char buffer[4096] = {0};
		sprintf(buffer, "OD_%d", kv->key);
    	kv->Sort();
    	for (int i = 0; i < kv->index_num; ++i) {
    		if (kv->field_info[i].value > 0) {
    			auto& fd = kv->field_info[i];
    			// TODO: call md5 consume
    			char field[128] = {0};
    			sprintf(field, " field_%d %s", fd.field, std::to_string(fd.value).c_str());
    			strcat(buffer, field);
    		}
    	}
    	strcat(buffer, "\n");
    	int len = strlen(buffer);
    	memcpy(addr+size, buffer, len);
    	size += len;
    }

    int32_t size;
    char *addr;
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
	std::vector<char*> moved_data_vec;
	std::vector<int> data_size;
    std::vector<pthread_t> worker;
    std::vector<FileWriter> writer;
	std::vector<int> tids;
};

#define TOTAL_RECORD_SIZE   210000000

extern ThreadInfo g_thread_info;

}

#endif //LOGREC_HELPER_H
