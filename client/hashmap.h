//
// Created by hhx on 2019/1/10.
//

#ifndef LOGREC_HASHMAP_H
#define LOGREC_HASHMAP_H

#include "parser.h"

#define MAX_HASH    20000000

namespace LogRec
{

class HashMap {
public:
    HashMap() {}

    virtual ~HashMap() {}

    void HMSET(LogRecord &log);

    void HINCRBY(LogRecord &log);

    void HDEL(LogRecord &log);

    void RENAME(LogRecord &log);

    void DEL(LogRecord &log);

private:
    std::vector<KeyValue> hash_map[MAX_HASH];
};

}

#endif //LOGREC_HASHMAP_H
