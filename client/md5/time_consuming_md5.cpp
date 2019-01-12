#include <map>
#include <string>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
//#include "md5_hashid.h"
#include "Md5.h"

std::string time_consuming_md5(const std::string& str) {
    std::string strdup;
    for (int j = 0; j < 60; ++j) {
        strdup = strdup.append(str);
    }
    MD5 str_md5(strdup);
    return str_md5.md5();
}

/*
std::string time_consuming_md5(const std::string& str) {
    std::string strdup;
    for (int j = 0; j < 60; ++j) {
        strdup = strdup.append(str);
    }
    return CMd5Hashid::hash_str(strdup.c_str(), strdup.length());
}
*/
