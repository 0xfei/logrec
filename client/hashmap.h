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
	    KeyValue* key_data = hash_map[record->curkey];
    	switch (record->code) {
		    case DEL:
                RemoveHash(key_data);
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

	KeyValue* hash_map[MAX_HASH];

private:
    void RenameHash(int cur_hash, int new_hash) {
		if (cur_hash == new_hash) return;
    	KeyValue* cur_data = hash_map[cur_hash];
    	if (cur_data->field_num == 0) return;
    	hash_map[cur_hash] = hash_map[new_hash];
    	hash_map[new_hash] = cur_data;

    	RemoveHash(hash_map[cur_hash]);
    }

    void RemoveHash(KeyValue* key_data) {
        if (key_data->field_num == 0) return;
        for (int i = 0; i < key_data->index_num; ++i) {
            key_data->field_info[i].value = 0;
        }
        key_data->field_num = 0;
    }

	void OptField(LogRecord* record, KeyValue* key_data,
	              OPTCODE code) {
		for (int i = 0; i < record->field_num; ++i) {
			int opt_field = record->field[i];
			int64_t opt_value = record->value[i];
			int index = key_data->field_index[opt_field];
			int index_num = key_data->index_num;

			switch (code) {
				case HDEL:
					if (index < index_num && key_data->field_info[index].field == opt_field) {
						if (key_data->field_info[index].value != 0) {
                            key_data->field_info[index].value = 0;
                            key_data->field_num = key_data->field_num - 1;
						}
					}
					break;
				case HINCRBY:
					if (index < index_num && key_data->field_info[index].field == opt_field) {
					    if (key_data->field_info[index].value == 0) {
                            key_data->field_num = key_data->field_num + 1;
					    }
						key_data->field_info[index].value += opt_value;
					} else {
                        NewFieldInfo(key_data, index_num, opt_field, opt_value);
					}
					break;
				case HMSET:
					if (index < index_num && key_data->field_info[index].field == opt_field) {
                        if (key_data->field_info[index].value == 0) {
                            key_data->field_num = key_data->field_num + 1;
                        }
						key_data->field_info[index].value = opt_value;
					} else {
					    NewFieldInfo(key_data, index_num, opt_field, opt_value);
					}
					break;
				default:
					break;
			}
		}
	}

	void NewFieldInfo(KeyValue* key_data, int index_num, int opt_field, int64_t opt_value) {
        key_data->field_index[opt_field] = index_num;
        key_data->field_info[index_num].value = opt_value;
        key_data->field_info[index_num].field = opt_field;
        key_data->field_info[index_num].weight = X2Weight(opt_field);
        key_data->index_num += 1;
        key_data->field_num += 1;
    }

	int X2Weight(int x) {
		return (x/100+1)*100 + ((x%100)/10+1)*10 + (x%10+1);
	}
};

extern HashMap g_hashmap;

}

#endif //LOGREC_HASHMAP_H
