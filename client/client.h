//
// Created by hhx on 2019/1/10.
//

#ifndef LOGREC_CLIENT_H
#define LOGREC_CLIENT_H

#include "parser.h"

class Client {
public:
	Client() {}
	virtual ~Client() {}

	void Connect();

private:
	int32_t cur_mins;
	Parser parser;
};


#endif //LOGREC_CLIENT_H
