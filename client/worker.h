//
// Created by hhx on 2019/1/12.
//

#ifndef LOGREC_WORKER_H
#define LOGREC_WORKER_H

#ifdef _APPLE_
inline void CORE_BIND(int id) {
	return;
}
#else
inline void CORE_BIND(int id) {
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(id, &mask);
	pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);
}
#endif

namespace LogRec
{

void *Worker(void *param);
void *Reciver(void *param);
void *Modifier(void *param);
void *Merger(void *param);

}

#endif //LOGREC_WORKER_H
