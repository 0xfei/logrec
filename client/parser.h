//
// Created by hhx on 2019/1/10.
//

#ifndef LOGREC_PARSER_H
#define LOGREC_PARSER_H

#include "helper.h"

class Parser {
public:
	Parser() {}
	virtual ~Parser() {}

	void Parse(std::string data);

private:
	void ParseHMSET(LogRecord& log);
	void ParseHINCRBY(LogRecord& log);
	void ParseHDEL(LogRecord& log);
	void ParseRENAME(LogRecord& log);
	void ParseDEL(LogRecord& log);

	void AddList(LogRecord& record);
};


#endif //LOGREC_PARSER_H
