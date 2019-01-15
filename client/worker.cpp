//
// Created by hhx on 2019/1/12.
//

#include "helper.h"
#include "worker.h"
#include "hashmap.h"

#include <string.h>
#include <fstream>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>

namespace LogRec
{

#define USECONDS    20000
#define BLOCK_SIZE  (1024*1024)

bool g_recivered = false;

HashMap g_hashmap;
int* g_key_threadid;
int64_t* g_key_latest;

#define TOTAL_RECORD_SIZE   210000000
LogRecord* g_total_record[TOTAL_RECORD_SIZE];

#define THREAD_RECNUM   2000000
struct ThreadRecord {
	int num;
	LogRecord* records[THREAD_RECNUM];
};

#define MAX_THREAD_NUM  40
ThreadRecord* g_thread_record;

/*
struct RenameOpt {
	int cur_key;
	int new_key;
};
RenameOpt g_rename_operation[TOTAL_RECORD_SIZE];
*/

int g_client_fd;
int64_t g_basic_stamp;

/*
 * recv data from server
 */
void Connect();

void* Reciver(void* param)
{
	CORE_BIND(1);
	TimeUsage tm("ReciverUsage"); tm.Start();
	Connect();

	char g_buffer[BLOCK_SIZE] = {0};
	char g_prev_buffer[BLOCK_SIZE] = {0};
	char g_curr_buffer[BLOCK_SIZE*2] = {0};

	int count = 0;
	ssize_t nsize, prev_size = 0, point = 0;
	while ((nsize = recv(g_client_fd, (void*)g_buffer, BLOCK_SIZE, 0)) != 0) {
		if (g_basic_stamp == 0) {
			for (int i = 0; i < nsize; ++i) {
				if (g_buffer[i] == ' ') break;
				g_basic_stamp = g_basic_stamp*10 + (g_buffer[i] - '0');
			}
			g_basic_stamp = g_basic_stamp - 100;
		}
		if (nsize == 0) break;
		int temp_size = nsize;
		for (point = nsize-1; point >=0; --point) {
			if (g_buffer[point] == '\n') break;
		}
		if (prev_size != 0) {
			memcpy(g_curr_buffer, g_prev_buffer, prev_size);
			memcpy(g_curr_buffer + prev_size, g_buffer, point+1);
			nsize = point + 1 + prev_size;
		} else {
			nsize = point + 1;
			memcpy(g_curr_buffer, g_buffer, nsize);
		}
		prev_size = temp_size - point - 1;
		for (int i = point+1; i < temp_size; ++i) {
			g_prev_buffer[i-point-1] = g_buffer[i];
		}

		int tid = count % g_thread_info.num;
		int old_size = g_thread_info.data_size[tid];
		memcpy(g_thread_info.data_vec[tid] + old_size, g_curr_buffer, nsize);
		g_thread_info.data_size[tid] = old_size + nsize;
		count++;
	}
	close(g_client_fd);
	g_recivered = true;
	tm.Output();
	return NULL;
}


/*
 * Worker thread
 */
const int g_worker_slut[] = {10, 10, 11, 11, 12, 12, 13, 13, 14, 14,
							 15, 15, 16, 16, 17, 17, 18, 18, 19, 19,
							 2, 2, 3, 3, 4, 4, 5, 5,
							 6, 6, 7, 7, 8, 8, 9, 9};

const int g_worker_slut_size = sizeof(g_worker_slut)/sizeof(int);

void AddFileWriter(int tid);
void ParseCommand(int tid);

void* Worker(void* param)
{
	int tid = *((int*)param);
	CORE_BIND(tid+1);

	ParseCommand(tid);
	int start = TOTAL_RECORD_SIZE/g_thread_info.num;

	while (g_thread_info.state[tid] != OPERATE_HASH) {
		usleep(USECONDS);
	}
	for (int i = 0; i < g_thread_record[tid].num; ++i) {
		auto & record = g_thread_record[tid].records[i];
		g_hashmap.ModifyHash(record);
		if (g_key_latest[record->curkey] == record->timestamp) {
			if (record->code == RENAME) {
				g_hashmap.FinishKey(record->newkey);
			} else {
				g_hashmap.FinishKey(record->curkey);
			}
		}
	}
	g_thread_info.state[tid] = WRITE_DATA;

	bool write_data = true;
	while (true) {
		for (int i = 0; i < g_thread_info.num; ++i) {
			if (g_thread_info.state[i] != WRITE_DATA) {
				write_data = false;
				break;
			}
		}
		if (write_data) break;
		usleep(USECONDS);
	}

	TimeUsage tm(std::to_string(tid)+"#AddFileWriter");
	tm.Start();
	if (tid < g_worker_slut_size) AddFileWriter(tid);
	tm.Output();
	return NULL;
}


/*
 * Executer thread
 */
void AllocHashMap()
{
	TimeUsage allocer("Allocer");
	allocer.Start();
	KeyValue* kvarray = (KeyValue*)malloc(sizeof(KeyValue)*MAX_HASH);
	for (int i = 1; i < MAX_HASH; ++i) {
		g_hashmap.hash_map[i] = &kvarray[i];
	}
	g_key_threadid = (int*)malloc(MAX_HASH*sizeof(int));
	g_key_latest = (int64_t*)malloc(MAX_HASH*sizeof(int));
	g_thread_record = (ThreadRecord*)malloc(MAX_THREAD_NUM*sizeof(ThreadRecord));
	allocer.Output();
}

void* Executer(void* param)
{
	CORE_BIND(0);
	AllocHashMap();

	TimeUsage tm("Executer"); tm.Start();

	auto total_num = g_thread_info.num;

	for (int i = 0; i < TOTAL_RECORD_SIZE; ++i) {
		auto record = g_total_record[i];
		if (record != NULL && record->code == RENAME) {
			auto cur_key = record->curkey;
			auto new_key = record->newkey;
			if (g_key_threadid[cur_key] == 0) {
				g_key_threadid[cur_key] = cur_key%total_num;
				g_key_threadid[new_key] = cur_key%total_num;
			} else {
				g_key_threadid[new_key] = g_key_threadid[cur_key];
			}
			g_key_latest[record->newkey] = record->timestamp;
		}
	}

	for (int i = 0; i < TOTAL_RECORD_SIZE; ++i) {
		auto record = g_total_record[i];
		if (record != NULL) {
			auto cur_key = record->curkey;
			auto timestamp = record->timestamp;
			auto tid = cur_key % total_num;
			g_key_latest[cur_key] = timestamp;
			if (g_key_threadid[cur_key] == 0) {
				g_key_threadid[cur_key] = tid;
			} else {
				tid = g_key_threadid[cur_key];
			}
			g_thread_record[tid].records[g_thread_record[tid].num++] = record;
		}
	}
	for (int tid = 0; tid < g_thread_info.num; ++tid) {
		g_thread_info.state[tid] = OPERATE_HASH;
	}
	tm.Output();
	return NULL;
}


/*
 * Hash worker
 */
inline void AddToHashmap(int key, int tid)
{
	auto & k = g_hashmap.hash_map[key];
	if (k != NULL && k->field_num > 0) {
		k->key = key;
		g_thread_info.writer[tid].AddRecord(k);
	}
}

void DFSAddHash(int slut, int tid)
{
	int s = 0, t = 10;
	if (slut < 20) {
		if (tid % 2 == 1) {
			s = 5, t = 10;
		} else {
			s = 0, t = 5;
		}
	}

	for (auto i = s; i < t; ++i) {
		auto key = slut*10 + i;
		if (key >= MAX_HASH) break;
		AddToHashmap(key, tid);
		DFSAddHash(key, tid);
	}
}

void AddFileWriter(int tid)
{
	if (tid == 0) AddToHashmap(1, tid);
	int slut = g_worker_slut[tid];
	if (tid % 2 == 0) AddToHashmap(slut, tid);
	DFSAddHash(slut, tid);
}


/*
 * Parser
 */
inline int64_t ParseTimestmp(int& j, char* addr) {
	int64_t timestamp = 0;
	while (addr[j] != ' ') {
		timestamp = timestamp*10 - '0' + addr[j++];
	}
	j++;
	return timestamp;
}

inline OPTCODE ParseOptcode(int& j, char* addr) {
	int len = 0;
	while (addr[j] != ' ') {
		len++; j++;
	}
	j++;
	return (OPTCODE)len;
}

inline bool NotODStart(int& j, char* addr)
{
	if (addr[j] != 'O') {
		while (addr[j] != '\n') j++;
		j++;
		return true;
	}
	return false;
}

inline int ParseCurKey(int& j, char* addr)
{
	int key = 0;
	j += KEY_PREFIX_SIZE;
	while (addr[j] != ' ' && addr[j] != '\n') {
		key = key*10 - '0' + addr[j++];
	}
	j++;
	return key;
}

inline void ParseHDEL(LogRecord* record, int& j, char* addr)
{
	while (true) {
		j += FIELD_PREFIX_SIZE;
		int fd = 0;
		while (addr[j] != ' ' && addr[j] != '\n') {
			fd = fd*10 - '0' + addr[j++];
		}
		record->field[record->field_num++] = fd;
		if (addr[j] == ' ') j++;
		else { j++; break; }
	}
}

inline void ParseHMSET(LogRecord* record, int& j, char* addr)
{
	while (true) {
		j += FIELD_PREFIX_SIZE;
		int field = 0; int64_t value = 0;
		while (addr[j] != ' ') {
			field = field*10 - '0' + addr[j++];
		}
		j++;
		record->field[record->field_num] = field;
		while (addr[j] != ' ' && addr[j] != '\n') {
			value = value*10 - '0' + addr[j++];
		}
		record->value[record->field_num++] = value;
		if (addr[j] == ' ') j++;
		else { j++; break; }
	}
}

inline int ParseRENAME(int& j, char* addr)
{
	int key = 0;
	j += KEY_PREFIX_SIZE;
	while (addr[j] != '\n') {
		key = key*10 - '0' + addr[j++];
	}
	j++;
	return key;
}

inline void ParseHINCRBY(LogRecord* record, int& j, char* addr)
{
	j += FIELD_PREFIX_SIZE;
	int field = 0; int64_t value = 0;
	while (addr[j] != ' ') {
		field = field*10 - '0' + addr[j++];
	}
	j++;
	record->field[record->field_num] = field;
	while (addr[j] != ' ' && addr[j] != '\n') {
		value = value*10 - '0' + addr[j++];
	}
	record->value[record->field_num++] = value;
	j++;
}


/*
 * parse command
 */
void ParseCommand(int tid)
{
	TimeUsage tm(std::to_string(tid)+"#ParseCommand");
	tm.Start();
	int j = 0;
	char* addr = g_thread_info.data_vec[tid];
	int addr_size = g_thread_info.data_size[tid];
	while (true) {
		while (addr_size - j <= 100 && !g_recivered) {
			usleep(USECONDS);
			addr_size = g_thread_info.data_size[tid];
		}
		if (g_recivered && j == addr_size) break;
		while (j < addr_size) {
			int64_t timestamp = ParseTimestmp(j, addr);
			OPTCODE optcode = ParseOptcode(j, addr);
			if (NotODStart(j, addr)) continue;
			LogRecord* record = new LogRecord;
			record->timestamp = timestamp;
			record->code = optcode;
			auto time_index = timestamp-g_basic_stamp;
			record->curkey = ParseCurKey(j, addr);
			switch (record->code) {
				case HDEL:
					ParseHDEL(record, j, addr);
					break;
				case HMSET:
					ParseHMSET(record, j, addr);
					break;
				case RENAME:
					record->newkey = ParseRENAME(j, addr);
					//g_rename_operation[time_index].cur_key = record->curkey;
					//g_rename_operation[time_index].new_key = record->newkey;
					break;
				case HINCRBY:
					ParseHINCRBY(record, j, addr);
					break;
				default:
					break;
			}
			g_total_record[time_index] = record;
		}
	}
	tm.Output();
}

/*
 * connect helper
 */
#define SERVER_IP   "10.90.51.49"
#define SERVER_PORT 8083

void Connect()
{
	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	addr.sin_addr.s_addr=inet_addr(SERVER_IP);
	g_client_fd = socket(PF_INET, SOCK_STREAM, 0);

	int ret = connect(g_client_fd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret == -1) {
		exit(-1);
	}
}

}
