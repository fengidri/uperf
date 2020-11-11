#ifndef  __UPERF_H__
#define __UPERF_H__
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

#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#include <signal.h>
struct config{
	char *server;
	int port;
	int (*handler)(void*);
	int thread_n;
	int conn_n;
	int channel;
	int reqs;
	int msglen;
	int depth;
	int rate;
	int sport;
    int gso;
};
extern struct config config;


struct module{
	void (*start)(void*);
	void (*thread)(void*);
	void (*alarm)(void*);
};
#endif


