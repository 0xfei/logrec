//
// Created by hhx on 2019/1/10.
//

#include "client.h"
#include "hashmap.h"
#include "worker.h"

#define DATA_BLOCK_SIZE (1024*1024*1024)

int main()
{
	LogRec::TimeUsage tm("TotalUsage:");
	tm.Start();
	LogRec::StartWorker();
	LogRec::SaveData();
	tm.Output();
	return 0;
}

namespace LogRec
{

ThreadInfo g_thread_info;

int StartWorker()
{
	AllocHashMap();

	g_thread_info.num = sysconf(_SC_NPROCESSORS_ONLN) - 1;
	for (auto i = 0; i < g_thread_info.num; ++i) {
		g_thread_info.state.push_back(PARSE_DATA);
		g_thread_info.data_vec.push_back((char*)malloc(DATA_BLOCK_SIZE));
		g_thread_info.data_size.push_back(0);
		g_thread_info.writer.push_back(FileWriter());
		g_thread_info.tids.push_back(i);
	}

	pthread_create(&g_thread_info.recv_tid, NULL, Reciver, NULL);

	for (auto i = 0; i < g_thread_info.num; ++i) {
		pthread_t thread;
		pthread_create(&thread, NULL, Worker, (void *)&g_thread_info.tids[i]);
		g_thread_info.worker.push_back(thread);
	}

	return 0;
}

int SaveData()
{
	int output_fd = open("output.data", O_WRONLY|O_CREAT|O_TRUNC, 0644);
	if (output_fd == -1) {
		exit(-1);
	}

	pthread_join(g_thread_info.recv_tid, NULL);

	for (int i = g_thread_info.num-1; i >= 0; --i) {
		pthread_join(g_thread_info.worker[i], NULL);
	}

	for (auto k : g_thread_info.writer) {
		write(output_fd, k.addr, k.size);
	}
	close(output_fd);

	return 0;
}

}
