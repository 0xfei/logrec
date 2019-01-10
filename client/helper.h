//
// Created by hhx on 2019/1/10.
//

#ifndef LOGREC_HELPER_H
#define LOGREC_HELPER_H

#include <cstdint>
#include <string>
#include <vector>
#include <pthread.h>

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

struct LogRecord {
	int64_t timestamp;
	int32_t mins;
	std::string opt;
	std::string oldkey;
	std::string curkey;
	std::vector<Field> fileds;
};

struct MinLogRecord {
	int32_t mins;
	std::vector<LogRecord> records;
};

struct ThreadInfo {
	int32_t num;
	std::vector<pthread_t> tids;
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

struct FileWriter {
	FileWriter() {}
	virtual ~FileWriter() {}
	void AddRecord(KeyValue& kv) {}

	int32_t size;
	void* addr;
};

extern KeyThreadId g_key_tid;
extern ThreadInfo g_thread_info;
extern std::vector<SingleThreadRecord> g_thread_record;


#endif //LOGREC_HELPER_H
