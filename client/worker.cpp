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

#define USECONDS    10000
#define BLOCK_SIZE  (1024*1024)

char g_buffer[BLOCK_SIZE] = {0};
char g_prev_buffer[BLOCK_SIZE] = {0};
char g_curr_buffer[BLOCK_SIZE*2] = {0};

bool g_execcuted = false;
int g_client_fd;


/*
 * connect helper
 */
void LoadIpPort(std::string& ip, int& port)
{
	std::ifstream conf("client.conf");
	conf >> ip >> port;
	conf.close();
}

void Connect()
{
	std::string ip; int port;
	LoadIpPort(ip, port);

	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr=inet_addr(ip.c_str());
	g_client_fd = socket(PF_INET, SOCK_STREAM, 0);

	int ret = connect(g_client_fd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret == -1) {
		exit(-1);
	}
}

/*
 * recv data from server
 */
void* Reciver(void* param)
{
	CORE_BIND(1);
	TimeUsage tm("ReciverUsage");
	tm.Start();
	Connect();

	int count = 0;
	ssize_t nsize, prev_size = 0, point = 0;
	while ((nsize = recv(g_client_fd, (void*)g_buffer, BLOCK_SIZE, 0)) != 0) {
		if (g_basic_stamp == 0) {
			for (int i = 0; i < nsize; ++i) {
				if (g_buffer[i] == ' ') break;
				g_basic_stamp = g_basic_stamp*10 + (g_buffer[i] - '0');
			}
			g_basic_stamp = g_basic_stamp - 10000;
			std::cout << g_basic_stamp << std::endl;
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

		int tid = (count / 350) % g_thread_info.num;
		if (tid != 0) {
			tid = tid;
		}
		int old_size = g_thread_info.data_size[tid];
		memcpy(g_thread_info.data_vec[tid] + old_size, g_curr_buffer, nsize);
		g_thread_info.data_size[tid] = old_size + nsize;
		count++;
	}
	close(g_client_fd);
	tm.Output();
	return NULL;
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

inline void ParseCurKey(LogRecord* record, int& j, char* addr)
{
	j += KEY_PREFIX_SIZE;
	while (addr[j] != ' ' && addr[j] != '\n') {
		record->curkey = record->curkey*10 - '0' + addr[j++];
	}
	j++;
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

inline void ParseRENAME(LogRecord* record, int& j, char* addr)
{
	j += KEY_PREFIX_SIZE;
	while (addr[j] != '\n') {
		record->newkey = record->newkey*10 - '0' + addr[j++];
	}
	j++;
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

void Modifier(int tid)
{
	TimeUsage tm(std::to_string(tid)+"#Modifier");
	tm.Start();
	char* addr = g_thread_info.data_vec[tid];
	int addr_size = g_thread_info.data_size[tid];
	int j = 0;
	while (j < addr_size) {
		auto timestamp = ParseTimestmp(j, addr);
		auto optcode = ParseOptcode(j, addr);
		if (NotODStart(j, addr)) continue;
		LogRecord* record = new LogRecord;
		record->timestamp = timestamp;
		record->code = optcode;
		ParseCurKey(record, j, addr);
		switch (record->code) {
			case HDEL:
				ParseHDEL(record, j, addr);
				break;
			case HMSET:
				ParseHMSET(record, j, addr);
				break;
			case RENAME:
				ParseRENAME(record, j, addr);
				break;
			case HINCRBY:
				ParseHINCRBY(record, j, addr);
				break;
			default:
				break;
		}
		g_total_record[timestamp-g_basic_stamp] = record;
		if (timestamp > g_final_stamp) g_final_stamp = timestamp;
	}
	tm.Output();
}


/*
 * Hash worker
 */
const int g_worker_slut[] = {10, 11, 12, 13, 14, 15, 2, 3, 4, 5, 6, 7, 8, 9};
const int g_worker_slut_size = 14;

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
	if (slut*10 > MAX_HASH) return;
	for (auto i = 0; i < 10; ++i) {
		auto key = slut*10 + i;
		AddToHashmap(key, tid);
		DFSAddHash(key, tid);
	}
}

void AddFileWriter(int tid)
{
	g_thread_info.writer[tid].Alloc();
	if (tid == 0) AddToHashmap(1, tid);
	int slut = g_worker_slut[tid];
	AddToHashmap(slut, tid);
	DFSAddHash(slut, tid);
}

/*
 * Worker thread
 */
void* Worker(void* param)
{
	int tid = *((int*)param);
	CORE_BIND(tid+1);

	Modifier(tid);
	g_thread_info.state[tid] = FINIS_PARS;

	while (!g_execcuted) {
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
void* Executer(void* param)
{
	CORE_BIND(0);
	{
		TimeUsage allocer("Allocer");
		allocer.Start();

		KeyValue* kvarray = (KeyValue*)malloc(sizeof(KeyValue)*MAX_HASH);
		for (int i = 1; i < MAX_HASH; ++i) {
			g_hashmap.hash_map[i] = &kvarray[i];
		}
		allocer.Output();
	}

	while (true) {
		bool finish = true;
		for (int i = 0; i < g_thread_info.num; ++i) {
			if (g_thread_info.state[i] != FINIS_PARS) {
				finish = false;
				break;
			}
		}
		if (finish) break;
		else usleep(USECONDS);
	}

	TimeUsage tm("Executer");
	tm.Start();

	for (int i = 0; i < TOTAL_RECORD_SIZE; ++i) {
		if (g_total_record[i] != NULL) {
			g_hashmap.ModifyHash(g_total_record[i]);
		}
	}
	g_execcuted = true;
	tm.Output();
	return NULL;
}

}
