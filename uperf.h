#ifndef  __UPERF_H__
#define __UPERF_H__

#define _GNU_SOURCE	       /* See feature_test_macros(7) */

#include <sys/types.h>	       /* See NOTES */
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <sched.h>

#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#include <signal.h>
typedef unsigned long long int u64;
typedef unsigned int u32;
struct config{
	char *server;
	int port;
	int (*handler)(void*);
	int thread_n;
	int conn_n;
	int channel;

    int udp_connect;
    int sendmsg;
    int sendmmsg;
    int stat;

	u64 reqs_last;
	u64 reqs;
    u64 spent;

    const char *ifname;

	int msglen;
	int depth;
	int rate;
	int sport;
    int gso;
    u64 cycle;
    u32 start;
    u32 time;

    int cpu_list[300];
    int cpu_num;

    u32 flags;
    bool flag_oob;
    bool flag_probe;
    bool flag_confirm;
    bool nonblock;
};
extern struct config config;

struct thread {
	pthread_t pthread;
    int id;
    void *data;

};


struct module{
	int (*prepare)(void*);
	void (*thread)(struct thread *);
	void (*alarm)(void*);
};


static __inline__ unsigned long long rdtsc(void)
{
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((unsigned long long)lo)|(((unsigned long long)hi)<<32);
}
#endif


