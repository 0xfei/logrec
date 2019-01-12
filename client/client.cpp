//
// Created by hhx on 2019/1/10.
//

#include "client.h"
#include "hashmap.h"
#include "worker.h"


LogRec::Client g_client;

int main()
{
	g_client.StartWorker();
	g_client.SaveData();
	return 0;
}


namespace LogRec
{

HashMap g_hashmap;
ThreadInfo g_thread_info;

int Client::StartWorker()
{
	g_thread_info.num = 1;

	pthread_create(&g_thread_info.recv_tid, NULL, Reciver, NULL);
	pthread_create(&g_thread_info.parse_tid, NULL, Modifier, NULL);
	pthread_create(&g_thread_info.merge_tid, NULL, Merger, NULL);

	for (int i = 0; i < g_thread_info.num; ++i) {
		int tid = i + 3;
		pthread_t thread;
		pthread_create(&thread, NULL, Worker, (void*)&tid);
		g_thread_info.worker.push_back(thread);
		g_thread_info.writer.push_back(FileWriter());
	}

	return 0;
}

int Client::SaveData()
{
	pthread_join(g_thread_info.recv_tid, NULL);
	pthread_join(g_thread_info.parse_tid, NULL);
	pthread_join(g_thread_info.merge_tid, NULL);

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
