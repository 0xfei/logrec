//
// Created by hhx on 2019/1/10.
//

#include "client.h"
#include "hashmap.h"
#include "worker.h"

LogRec::Client g_client;

int main()
{
	LogRec::TimeUsage tm("TotalUsage:");
	tm.Start();
	g_client.StartWorker();
	g_client.SaveData();
	tm.Output();
	return 0;
}

namespace LogRec
{

LogRecord* g_total_record[TOTAL_RECORD_SIZE];
int64_t g_basic_stamp;
int64_t g_final_stamp;

HashMap g_hashmap;
ThreadInfo g_thread_info;

int Client::StartWorker()
{
	g_thread_info.num = sysconf(_SC_NPROCESSORS_ONLN) - 1;
	std::cout << "ThreadNum: " << g_thread_info.num << std::endl;
	for (auto i = 0; i < g_thread_info.num; ++i) {
		g_thread_info.state.push_back(PARSE_DATA);
		g_thread_info.data_vec.push_back((char*)malloc(DATA_BLOCK_SIZE));
		g_thread_info.data_size.push_back(0);
		g_thread_info.writer.push_back(FileWriter());
		g_thread_info.tids.push_back(i);
	}

	pthread_create(&g_thread_info.recv_tid, NULL, Reciver, NULL);
	pthread_join(g_thread_info.recv_tid, NULL);

	for (auto i = 0; i < g_thread_info.num; ++i) {
		pthread_t thread;
		pthread_create(&thread, NULL, Worker, (void *)&g_thread_info.tids[i]);
		g_thread_info.worker.push_back(thread);
	}
	pthread_create(&g_thread_info.exec_tid, NULL, Executer, NULL);

	return 0;
}

int Client::SaveData()
{
	pthread_join(g_thread_info.exec_tid, NULL);

	for (auto t : g_thread_info.worker) {
		pthread_join(t, NULL);
	}

	for (auto k : g_thread_info.writer) {
		write(output_fd, k.addr, k.size);
	}
	close(output_fd);

	return 0;
}

}
