//
// Created by hhx on 2019/1/10.
//

#ifndef LOGREC_CLIENT_H
#define LOGREC_CLIENT_H

#include "parser.h"
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace LogRec
{

class Client {
public:
    Client() { LoadConfig(); }
    virtual ~Client() { }

    int StartWorker();
    int SaveData();

private:
    void LoadConfig() {
    	output_fd = open("output.data", O_WRONLY|O_CREAT|O_TRUNC, 0644);
    	if (output_fd == -1) {
    		exit(-1);
    	}
    }

    int output_fd;
};

}

#endif //LOGREC_CLIENT_H
