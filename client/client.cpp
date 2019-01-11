//
// Created by hhx on 2019/1/10.
//

#include "client.h"
#include "hashmap.h"

extern LogRec::HashMap g_hashmap;
extern LogRec::ThreadInfo g_thread_info;

LogRec::Client g_client;

int main()
{
    g_client.Connect();
    g_client.StartWorker();
    g_client.SaveData();

	return 0;
}
