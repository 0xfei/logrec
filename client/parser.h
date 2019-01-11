//
// Created by hhx on 2019/1/10.
//

#ifndef LOGREC_PARSER_H
#define LOGREC_PARSER_H

#include "helper.h"

namespace LogRec {

class Parser {
public:
    Parser() {}
    virtual ~Parser() {}

    int Parse(std::string data);

private:
    int32_t Msec2Mins(int64_t msec) {
        return (msec / 1000) % 60;
    }

    bool IsSkip(LogHeader &header) {
        return true;
    }

    int32_t KeyHash(std::string key) {
        return 0;
    }

    int32_t FieldHash(std::string field) {
        return 0;
    }

    int ParseHMSET(LogHeader &header, SARR &ds);

    int ParseHINCRBY(LogHeader &header, SARR &ds);

    int ParseHDEL(LogHeader &header, SARR &ds);

    int ParseRENAME(LogHeader &header, SARR &ds);

    int ParseDEL(LogHeader &header, SARR &ds);

    int AddList(LogRecord &record);

    int32_t cur_mins;
};

}

#endif //LOGREC_PARSER_H
