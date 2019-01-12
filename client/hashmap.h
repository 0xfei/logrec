//
// Created by hhx on 2019/1/10.
//

#ifndef LOGREC_HASHMAP_H
#define LOGREC_HASHMAP_H

#include "parser.h"

#define MAX_HASH    16000000

namespace LogRec
{

class HashMap {
public:
    HashMap() {}
    virtual ~HashMap() {}

    void ModifyHash(LogRecord& record) {
	    KeyValue* key_data = NULL;
    	switch (record.code) {
		    case DEL:
		    	key_data = FindHash(record.curkey);
		    	key_data->fields.clear();
			    break;
		    case RENAME:
		    	RenameHash(record.curkey, record.newkey);
			    break;
		    case HDEL:
		    case HINCRBY:
		    case HMSET:
			    key_data = FindHash(record.curkey);
		    	OptField(record, key_data, record.code);
		    default:
			    break;
    	}
    }

	KeyValue* hash_map[MAX_HASH];

private:
	int32_t KeyHash(std::string key) {
		int32_t hash = 0;
		for (int i = 3; i < key.size(); ++i) {
			hash = (hash*10 + key[i] - '0')%MAX_HASH;
		}
		return hash;
	}

	int32_t FieldHash(std::string field) {
		int32_t hash = 0;
		for (int i = 6; i < field.size(); ++i) {
			hash = hash*10 + i - '0';
		}
		return hash;
	}

    void RenameHash(std::string cur_key, std::string new_key) {
    	int cur_hash = KeyHash(cur_key);
    	int new_hash = KeyHash(new_key);
    	KeyValue* cur_data = hash_map[cur_hash];
    	if (cur_data == NULL) return;

    	if(hash_map[new_hash] != NULL) delete hash_map[new_hash];
    	hash_map[new_hash] = cur_data;
    }

	KeyValue* FindHash(std::string key) {
    	int32_t hash = KeyHash(key);
    	if (hash == 0) return NULL;

    	if (hash_map[hash] != NULL)
		    return hash_map[hash];

    	KeyValue* new_kv = new KeyValue;
    	new_kv->key = key;
    	hash_map[hash] = new_kv;
		return new_kv;
    }

	void OptField(LogRecord& record, KeyValue* key_data,
	              OPTCODE code) {
		for (auto opt_field : record.field) {
			int record_index = -1;
			for (int i = 0; i < key_data->fields.size(); ++i) {
				if (key_data->fields[i].field == opt_field.field) {
					record_index = i;
					break;
				}
			}
			switch (code) {
				case HDEL:
					if (record_index == -1)
						return;
					key_data->fields[record_index].empty = true;
					key_data->fields[record_index].bvalue = 0;
					break;
				case HINCRBY:
					if (record_index == -1) {
						key_data->fields.push_back(opt_field);
					} else {
						key_data->fields[record_index].bvalue += opt_field.bvalue;
					}
					break;
				case HMSET:
					if (record_index == -1) {
						key_data->fields.push_back(opt_field);
					} else {
						key_data->fields[record_index].bvalue = opt_field.bvalue;
					}
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
