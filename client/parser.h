//
// Created by hhx on 2019/1/10.
//

#ifndef LOGREC_PARSER_H
#define LOGREC_PARSER_H

#include "helper.h"
#include <sstream>

namespace LogRec {

class Parser {
public:
	Parser() { currc_mins = -1; basic_mins = -1; }

    int Parse(std::string data) {
    	std::istringstream ssin(data);

	    LogRecord record;
    	int64_t value;
    	std::string opt, field;
    	ssin >> record.timestamp >> opt >> record.curkey;
    	if (IsSkip(record.curkey)) { return 0; }

    	record.mins = (record.timestamp / (1000*1000*60));
	    if (opt == "HMSET") {
		    record.code = HMSET;
		    while (ssin >> field >> value) {
		    	record.field.push_back(Field(field, value));
		    }
	    } else if (opt == "HINCRBY") {
		    record.code = HINCRBY;
		    ssin >> field >> value;
		    record.field.push_back(Field(field, value));
	    } else if (opt == "HDEL") {
		    record.code = HDEL;
		    while (ssin >> field) {
		    	record.field.push_back(Field(field, 0));
		    }
	    } else if (opt == "RENAME") {
		    record.code = RENAME;
		    ssin >> record.newkey;
	    } else if (opt == "DEL") {
		    record.code = DEL;
	    }

	    AddList(record);
	    return 0;
    }

    int AddLastIndex() {
	    g_thread_info.minlog_index.push_back(currc_mins);
	    return 0;
	}

private:
    bool IsSkip(std::string &key) {
    	if (key[0] == 'O' && key[1] == 'D' && key[2] == '_') {
    		return false;
    	}
	    return true;
    }

    int AddList(LogRecord &record) {
	    if (basic_mins == -1) {
		    basic_mins = record.mins;
	    }
	    record.mins = record.mins - basic_mins;

	    if (record.mins == currc_mins) {
	    	g_thread_info.minlog[currc_mins].records.push_back(record);
	    } else {
	    	if (currc_mins != -1) {
	    		g_thread_info.minlog_index.push_back(currc_mins);
	    	}
	    	currc_mins = record.mins;
		    g_thread_info.minlog.push_back(MinLogRecord());
		    g_thread_info.minlog[currc_mins].records.push_back(record);
	    }
	    return 0;
	}

	int32_t basic_mins;
	int32_t currc_mins;
};

}

#endif //LOGREC_PARSER_H
