//
// Created by hhx on 2019/1/10.
//

#ifndef LOGREC_CLIENT_H
#define LOGREC_CLIENT_H

#include "parser.h"

namespace LogRec
{

class Client {
public:
    Client() {
        LoadConfig();
        // TODO: LoadConfig and connect
    }
    virtual ~Client() {
        // TODO: release memory
    }


    void Connect() {
        // TODO: connect to server
    }
    void StartWorker() {
        // TODO: set thread state and start threads
    }
    void SaveData() {
        // TODO: save data to file using g_thread_info
    }

private:
    void Worker(int tid) {
        // TODO: pick from g_thread_info.thread_record
    }

    void LoadConfig() {
    }

    std::string ip;
    int port;
    Parser parser;
};

}

#endif //LOGREC_CLIENT_H
