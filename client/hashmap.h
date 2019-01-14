//
// Created by hhx on 2019/1/10.
//

#ifndef LOGREC_HASHMAP_H
#define LOGREC_HASHMAP_H

#include "helper.h"


namespace LogRec
{

class HashMap {
public:
    HashMap() {}
    virtual ~HashMap() {}

    void ModifyHash(LogRecord* record) {
	    KeyValue* key_data = FindHash(record->curkey);
    	switch (record->code) {
		    case DEL:
		    	for (int i = 0; i <= 100; ++i) {
		    		key_data->fields[i] = 0;
		    	}
		    	key_data->field_num = 0;
			    break;
		    case RENAME:
		    	RenameHash(record->curkey, record->newkey);
			    break;
		    case HDEL:
		    case HINCRBY:
		    case HMSET:
		    	OptField(record, key_data, record->code);
		    default:
			    break;
    	}
    }

    void CreateHash(int key) {
	    KeyValue* new_kv = new KeyValue;
	    hash_map[key] = new_kv;
	    new_kv->key = key;
    }

	KeyValue* hash_map[MAX_HASH];

private:
	KeyValue* FindHash(int hash) {
		return hash_map[hash];
	}

    void RenameHash(int cur_hash, int new_hash) {
    	KeyValue* cur_data = hash_map[cur_hash];
    	if (cur_data == NULL) return;
    	hash_map[cur_hash] = hash_map[new_hash];
    	hash_map[new_hash] = cur_data;

	    for (int i = 0; i <= 100; ++i) {
		    hash_map[cur_hash]->fields[i] = 0;
	    }
	    hash_map[cur_hash]->field_num = 0;
    }

	void OptField(LogRecord* record, KeyValue* key_data,
	              OPTCODE code) {
		for (int i = 0; i < record->field_num; ++i) {
			int opt_field = record->field[i];
			int64_t opt_value = record->value[i];

			switch (code) {
				case HDEL:
					if (key_data->fields[opt_field] != 0) {
						key_data->field_num = key_data->field_num - 1;
					}
					key_data->fields[opt_field] = 0;
					break;
				case HINCRBY:
					if (key_data->fields[opt_field] == 0) {
						key_data->field_num = key_data->field_num + 1;
					}
					key_data->fields[opt_field] += opt_value;
					break;
				case HMSET:
					if (key_data->fields[opt_field] == 0) {
						key_data->field_num = key_data->field_num + 1;
					}
					key_data->fields[opt_field] = opt_value;
					break;
				default:
					break;
			}
		}
	}
};

extern HashMap g_hashmap;

}

#endif //LOGREC_HASHMAP_H
