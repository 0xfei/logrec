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

namespace LogRec
{

struct TimeUsage {
	TimeUsage(std::string hd): header(hd) {
		beg = clock();
	}
	~TimeUsage() {
		dst = clock();
		std::cout << header << ":" << (dst-beg)/1000 << "ms\n";
	}

	std::string header;
	int64_t beg;
	int64_t dst;
};

using SARR = std::vector<std::string>;

struct Field {
	Field(): empty(true) {}
	Field(std::string f, int64_t v) :
			field(f), bvalue(v), empty(false) {}
    std::string field;
    std::string value;
    int64_t bvalue;
    bool isint;
    bool empty;
};

struct KeyValue {
	KeyValue() : field_num(0) {}
    std::string key;
    int32_t field_num;
    std::vector<Field> fields;

    void sort() {
        for (int i = 0; i < fields.size()-1; ++i) {
        	for (int j = i + 1; j < fields.size(); ++j) {
        		if (fields[i].field > fields[j].field) {
        			auto k = fields[i];
        			fields[i] = fields[j];
        			fields[j] = k;
        		}
        	}
        }
        field_num = 0;
        for (int i = 0; i < fields.size(); ++i) {
        	if (!fields[i].empty) field_num++;
        }
    }
};

enum OPTCODE {
    HMSET = 0,
    HINCRBY = 1,
    HDEL = 2,
    RENAME = 3,
    DEL = 4,
};


struct LogRecord {
	LogRecord(): used_for_sub(false) { }
	int64_t timestamp;
	int64_t secdstamp;
    int32_t mins;
    OPTCODE code;
    std::string curkey;
	std::string newkey;
	bool used_for_sub;
    std::vector<Field> field;
    std::vector<size_t> sub_field;
};

struct MinLogRecord {
    std::vector<LogRecord> records;
};

#define FW_SIZE (512*1024*1024)
struct FileWriter {
	FileWriter() { size = 0; }
    void Alloc() { size = 0; addr = malloc(FW_SIZE); } // TODO: check malloc result
    void Free() { free(addr); }
    void AddRecord(KeyValue *kv) {
    	kv->sort();
    	if (kv->field_num == 0) return;
    	MemCpy(kv->key.c_str(), kv->key.size());
    	for (auto fi : kv->fields) {
		    ((char*)addr)[size++] = ' ';
		    fi.value = std::to_string(fi.bvalue);
		    // TODO: call md5_consumer
		    MemCpy(fi.field.c_str(), fi.field.size());
		    ((char*)addr)[size++] = ' ';
		    MemCpy(fi.value.c_str(), fi.value.size());
    	}
	    ((char*)addr)[size++] = '\n';
    }

    void MemCpy(const char* str, int length) {
		for (int i = 0; i < length; ++i) {
			((char*)addr)[size++] = str[i];
		}
	}

    int32_t size;
    void *addr;
};

struct ThreadInfo {
    int32_t num;
    pthread_t recv_tid;
    pthread_t parse_tid;
    pthread_t merge_tid;
    std::vector<pthread_t> worker;
    std::vector<FileWriter> writer;
	std::vector<MinLogRecord> minlog;
	std::vector<int32_t> minlog_index;
};

extern ThreadInfo g_thread_info;

}

#endif //LOGREC_HELPER_H
