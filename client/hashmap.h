//
// Created by hhx on 2019/1/10.
//

#ifndef LOGREC_HASHMAP_H
#define LOGREC_HASHMAP_H

#include "helper.h"


namespace LogRec
{

extern KeyValue* g_hash_map[MAX_HASH];

void ModifyHash(LogRecord* record);
void FinishKey(int key);
void RenameHash(int cur_hash, int new_hash);
void OptField(LogRecord* record, KeyValue* key_data, OPTCODE code);

}

#endif //LOGREC_HASHMAP_H
