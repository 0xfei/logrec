//
// Created by hhx on 2019/1/12.
//

#include "helper.h"
#include "worker.h"
#include "parser.h"
#include "hashmap.h"

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
std::vector<void*> g_data_vec;
std::vector<int> g_data_vec_size;

bool g_reciver_finish = false;
bool g_parsed_finish = false;
bool g_merged_finish = false;

Parser g_parser_manager;

int g_client_fd;


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
	//int flags = fcntl(g_client_fd, F_GETFL, 0);
	//fcntl(g_client_fd, F_SETFL, flags|O_NONBLOCK);

	int ret = connect(g_client_fd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret == -1) {
		exit(-1);
	}
}


void* Reciver(void* param)
{
	CORE_BIND(0);

	TimeUsage tm("ReciverUsage");
	int nopcount = 0;
	Connect();

	ssize_t nsize;
	while ((nsize = recv(g_client_fd, (void*)g_buffer, BLOCK_SIZE, 0)) != 0) {
		if (nsize == 0) {
			break;
		} else {
			void* data = malloc(nsize);
			memcpy(data, g_buffer, nsize);
			g_data_vec.push_back(data);
			g_data_vec_size.push_back(nsize);
		}
	}
	g_reciver_finish = true;
	close(g_client_fd);
	return NULL;
}

void* Modifier(void* param)
{
	CORE_BIND(1);

	TimeUsage tm("ModifierUsage");
	int  data_index = 0;
	std::string log_record;
	while (true) {
		if (data_index == g_data_vec.size()) {
			if (g_reciver_finish) {
				break;
			}
			usleep(USECONDS);
			continue;
		}
		char* current_point = (char*)g_data_vec[data_index];
		int current_size = g_data_vec_size[data_index];
		for (int i = 0; i < current_size; ++i) {
			if (current_point[i] == 0) break;
			if (current_point[i] == '\n') {
				g_parser_manager.Parse(log_record);
				log_record = "";
			} else {
				log_record = log_record + current_point[i];
			}
		}
		free(g_data_vec[data_index]);
		data_index++;
	}
	if (log_record != "") g_parser_manager.Parse(log_record);
	g_parser_manager.AddLastIndex();
	g_parsed_finish = true;

	return NULL;
}


void MergeData(MinLogRecord& minlog);

void* Merger(void* param)
{
	CORE_BIND(2);

	TimeUsage tm("MergerITEM");
	int log_index = 0;
	while (true) {
		if (log_index == g_thread_info.minlog_index.size()) {
			if (g_parsed_finish) {
				break;
			}
			usleep(USECONDS*10);
			continue;
		}
		int now = g_thread_info.minlog_index[log_index];
		MergeData(g_thread_info.minlog[now]);
		// g_thread_info.minlog[log_index].records.clear();
		log_index++;
	}

	g_merged_finish = true;

	return NULL;
}

/*
 * Hash worker
 */
const int g_worker_slut[] = {10, 11, 12, 13, 14, 15, 2, 3, 4, 5, 6, 7, 8, 9};
const int g_worker_slut_size = 14;

inline void AddToHashmap(int key, int tid)
{
	auto k = g_hashmap.hash_map[key];
	if (k != NULL && k->fields.size() > 0) {
		g_thread_info.writer[tid].AddRecord(k);
	}
}

void DFSAddHash(int slut, int tid)
{
	if (slut*10 > MAX_HASH) return;
	for (int i = 0; i < 10; ++i) {
		auto key = slut*10 + i;
		AddToHashmap(key, tid);
		DFSAddHash(key, tid);
	}
}

void* Worker(void* param)
{
	int tid = *(int*)param;
	CORE_BIND(tid);
	tid = tid - 3;

	TimeUsage tm(std::to_string(tid)+"#Worker");

	while (!g_merged_finish) {
		usleep(USECONDS);
	}

	g_thread_info.writer[tid].Alloc();

	if (tid == 0) AddToHashmap(1, tid);
	int slut = g_worker_slut[tid];
	AddToHashmap(slut, tid);
	DFSAddHash(slut, tid);

	return NULL;
}


/*
 * internal function
 */
void MergeData(MinLogRecord& minlog)
{
	int64_t prev_time = 0;
	/*
	for (size_t i = 0; i < minlog.records.size(); ++i) {
		if (minlog.records[i].timestamp < prev_time) {
			int64_t now_stamp = minlog.records[i].timestamp;
			int s = 0, t = i-1, mid = i/2;
			minlog.records[i].used_for_sub = true;
			minlog.records[i].secdstamp = now_stamp;
			minlog.records[i].timestamp = prev_time;
			while (s < t + 1) {
				if (minlog.records[mid].timestamp > now_stamp) {
					t = mid - 1;
				} else {
					s = mid + 1;
				}
				mid = (s+t+1)/2;
			}
			minlog.records[s].sub_field.push_back(i);
			int num = minlog.records[s].sub_field.size();
			int poi, cur_poi = num - 1;
			for (poi = num-2; poi >= 0; --poi) {
				auto sec_time_index = minlog.records[s].sub_field[poi];
				auto sec_time = minlog.records[sec_time_index].timestamp;
				if (sec_time > now_stamp) {
					minlog.records[s].sub_field[poi+1] = sec_time_index;
					cur_poi = poi;
				}
			}
			minlog.records[s].sub_field[cur_poi] = i;
		} else {
			prev_time = minlog.records[i].timestamp;
		}
	}*/

	for (auto k : minlog.records) {
		if (k.used_for_sub) continue;
		for (auto sub : k.sub_field) g_hashmap.ModifyHash(minlog.records[sub]);
		g_hashmap.ModifyHash(k);
	}
}

}
