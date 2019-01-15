//
// Created by hhx on 2019/1/10.
//

#ifndef LOGREC_CLIENT_H
#define LOGREC_CLIENT_H

#include <fstream>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace LogRec
{

int StartWorker();
int SaveData();

}

#endif //LOGREC_CLIENT_H
