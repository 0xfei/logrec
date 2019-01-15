//
// Created by hhx on 2019/1/16.
//

#include "hashmap.h"
#include "md5/time_consuming_md5.h"

namespace LogRec
{

KeyValue* g_hash_map[MAX_HASH];

const int g_sorted_field[100] = {
		1,10,100,11,12,13,14,15,16,17,18,19,
		2,20,21,22,23,24,25,26,27,28,29,
		3,30,31,32,33,34,35,36,37,38,39,
		4,40,41,42,43,44,45,46,47,48,49,
		5,50,51,52,53,54,55,56,57,58,59,
		6,60,61,62,63,64,65,66,67,68,69,
		7,70,71,72,73,74,75,76,77,78,79,
		8,80,81,82,83,84,85,86,87,88,89,
		9,90,91,92,93,94,95,96,97,98,99};


void ModifyHash(LogRecord* record) {
	KeyValue* key_data = g_hash_map[record->curkey];
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

void FinishKey(int key) {
	KeyValue* key_data = g_hash_map[key];
	if (key_data->field_num == 0) {
		return;
	}
	sprintf(key_data->buffer, "OD_%d", key);
	for (int i = 0; i < 100; ++i) {
		auto field = g_sorted_field[i];
		auto value = key_data->fields[field];
		if (value > 0) {
			sprintf(key_data->buffer, "%s field_%d %s",
			        key_data->buffer, field,
			        std::to_string(value).c_str());
		}
	}
	key_data->buffer[strlen(key_data->buffer)] = '\n';
}

void RenameHash(int cur_hash, int new_hash) {
	if (cur_hash == new_hash) return;
	KeyValue* cur_data = g_hash_map[cur_hash];
	if (cur_data->field_num == 0) return;
	g_hash_map[cur_hash] = g_hash_map[new_hash];
	g_hash_map[new_hash] = cur_data;

	for (int i = 0; i <= 100; ++i) {
		g_hash_map[cur_hash]->fields[i] = 0;
	}
	g_hash_map[cur_hash]->field_num = 0;
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

}