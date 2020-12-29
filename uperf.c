
#include "uperf.h"

struct config config = {
	.thread_n = 1,
	.msglen = 16,
	.depth = 1,
	.sport = 0,
};

static struct module *mod;
static struct module def;

static void udp_recv()
{
  	int sockfd, rc;
    char buffer[22];

    struct sockaddr_in     servaddr;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(config.port);
    servaddr.sin_addr.s_addr = inet_addr(config.server);

	rc = bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
	if (rc < 0) {
		return;
	}


	while (1) {
		recv(sockfd, buffer, sizeof(buffer), 0);
		__sync_fetch_and_add(&config.reqs, 1);
	}


    close(sockfd);
}


static void tcp_echo_one_conn()
{
  	int sockfd;
    char buffer[1024];
	int epfd;
	int i, n;

    struct sockaddr_in     servaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(config.port);
    servaddr.sin_addr.s_addr = inet_addr(config.server);

	connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	printf("create channel...\n");


	for (i = 0; i < config.channel/config.thread_n; ++i) {
		n = send(sockfd, buffer, config.msglen, 0);
		if (n < config.msglen) {
        	perror("channel send fail");
			return;
		}
	}

	printf("start recv....\n");


	while(1) {
		__sync_fetch_and_add(&config.reqs, 1);


		n = recv(sockfd, buffer, config.msglen, 0);
		if (n > 0)
			send(sockfd, buffer, config.msglen, 0);
		else{
			printf("recv %d\n", n);
			return;
		}
	}
}

#if 0
static int tcp_echo()
{
#define MAX_EVENTS 10240
  	int sockfd;
    char buffer[1024];
	struct timespec tv;
	int epfd;
	int nr;

	struct epoll_event ev, *e, events[MAX_EVENTS];

    struct sockaddr_in     servaddr;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(config.port);
    servaddr.sin_addr.s_addr = inet_addr(config.server);

    int n, len, num;

	printf("start echo tcp....\n");



    epfd = epoll_create(MAX_EVENTS);
    if (epollfd < 0)
    {
        error("Error creating epoll..\n");
		return -1;
    }

    ev.events = EPOLLIN;
    ev.data.fd = sockfd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev) == -1)
    {
        error("Error adding new listeding socket to epoll..\n");
    }

	for (i = 0; i < config.channel; ++i) {
		send(fd, buffer, config.msglen);
	}

	atomic_add()

	    while (1) {

		    nr = epoll_wait(epfd, events, MAX_EVENTS, -1);
		    if (-1 == nr) {
			    continue;
		    }
		    for (i = 0; i < nr; ++i) {
			    e = events + i;
			    fd = e->data.fd;

			    n = recv(fd, buffer, config.msglen);
			    if (n >= msglen)
				    send(fd, buffer, config.msglen);
		    }
	    }



}
#endif
static void udp_echo_server()
{
  	int sockfd;
    char buffer[22];
	struct timespec tv;
	int addrlen;

	tv.tv_sec = 0;
	tv.tv_nsec = 1000 * 1000 * 10;

    struct sockaddr_in     servaddr, fromaddr;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(config.port);
    servaddr.sin_addr.s_addr = inet_addr(config.server);

    int n, len, num, rc;;

	printf("start echo udp....\n");
	rc = bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
	if (rc < 0) {
		return;
	}

	num = 0;

	while (1) {
		recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&fromaddr, &addrlen);
    	sendto(sockfd, buffer, sizeof(buffer), 0,
		       (const struct sockaddr *) &fromaddr, addrlen);
		__sync_fetch_and_add(&config.reqs, 1);
		++num;
	}


    close(sockfd);
}

static void udp_echo(void *_)
{
  	int sockfd;
	int depth = 0, step, i, ret;
    char buffer[22];

    struct sockaddr_in     servaddr, caddr;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&caddr, 0, sizeof(caddr));
	caddr.sin_family = AF_INET;
	caddr.sin_port = 0;
	caddr.sin_addr.s_addr = inet_addr("0.0.0.0");


	ret = bind(sockfd, (struct sockaddr *)&caddr, sizeof(caddr));
	if (ret) {
		printf("bind fail\n");
		return;
	}

    // Filling server information
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(config.port);
    servaddr.sin_addr.s_addr = inet_addr(config.server);

    int n, len, num;
	uint64_t recv, send;

	printf("start echo udp....\n");

	num = 0;
	recv = send = 0;

#define sendto() \
    sendto(sockfd, buffer, sizeof(buffer), 0, \
	       (const struct sockaddr *) &servaddr, sizeof(servaddr)); \
	++send;

#define recv() recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);

	struct timespec sleep;
	sleep.tv_sec = 0;
	sleep.tv_nsec = 10 * 1000;

	while (1) {
		sendto();
		n = recv();

		if (n > 0) {
			++recv;
			__sync_fetch_and_add(&config.reqs, 1);

			if (send < recv + recv / 10) {
				sendto();
			}
		}
		else{
			nanosleep(&sleep, NULL);
			sendto();
			sendto();
		}
		if (recv > 10000) {
			recv = 0;
			send = 0;
		}
	}

    //retry:
    //	for (step = 1; ; step = step + 20  ) {
    //		for (i = 0; i < step; ++i)
    //			sendto();
    //
    //		depth += step;
    //
    //		for (i = 0; i < depth; ++i) {
    //			recv();
    //			sendto();
    //		}
    //
    //		if (depth >= config.depth)
    //			break;
    //	}
    //
    //	printf("depth: %d\n", config.depth);
    //
    //	int last_num;
    //	struct timeval s, e;
    //
    //	gettimeofday(&s, NULL);
    //	last_num = 0;
    //
    //	while (1) {
    //		sendto();
    //		recv();
    //
    //		__sync_fetch_and_add(&config.reqs, 1);
    //		++num;
    //		if (num % 100000) {
    //			gettimeofday(&e, NULL);
    //			if (e.tv_sec - s.tv_sec > 1) {
    //				if (num - last_num == 0)
    //					goto retry;
    //
    //				last_num = num;
    //				s = e;
    //			}
    //
    //
    //		}
    //	}

    close(sockfd);
}


static void alarm_handler(int sig)
{
    u64 cycle;
    u64 reqs;

    reqs = config.reqs - config.reqs_last;

    config.reqs_last = config.reqs;

    if (config.reqs)
        cycle = config.cycle / config.reqs;
    else
        cycle = 0;

	printf("reqs: %lldw %lld cycle/reqs: %llu\n",
           reqs/10000,
           reqs,
           cycle);
	alarm(1);
}


extern struct module mod_udp_send;
extern struct module mod_udp_pingpong;

int main(int argc, char *argv[])
{
    int port = 8080, i, ret;
	char *p = NULL, *v = NULL;

	mod = &def;

	for (i = 1; i < argc; ++i) {
		p = argv[i];

		if (strcmp(p, "udp_send") == 0) {
			mod = &mod_udp_send;
			continue;
		}

		if (strcmp(p, "udp_echo") == 0) {
			mod->thread = udp_echo;
			continue;
		}

		if (strcmp(p, "udp_pingpong") == 0) {
			mod = &mod_udp_pingpong;
			continue;
		}

		if (strcmp(p, "udp_echo_server") == 0) {
			mod->thread = udp_echo_server;
			continue;
		}
		if (strcmp(p, "tcp_echo") == 0) {
			mod->thread = tcp_echo_one_conn;
			continue;
		}
		if (strcmp(p, "udp_recv") == 0) {
			mod->thread = udp_recv;
			continue;
		}

		if (strcmp(p, "--udp-connect") == 0) {
			config.udp_connect = 1;
			continue;
		}

		if (++i >= argc) {
			printf("except args for %s\n", p);
			return -1;
		}

		v = argv[i];

		if (strcmp(p, "-h") == 0) {
			config.server = v;
			continue;
		}

		if (strcmp(p, "-p") == 0) {
			config.port = atoi(v);
			continue;
		}
		if (strcmp(p, "--sport") == 0) {
			config.sport = atoi(v);
			continue;
		}
		if (strcmp(p, "-t") == 0) {
			config.thread_n = atoi(v);
			continue;
		}
		if (strcmp(p, "--depth") == 0) {
			config.depth = atoi(v);
			continue;
		}
		if (strcmp(p, "--channel") == 0) {
			config.channel = atoi(v);
			continue;
		}
		if (strcmp(p, "--rate") == 0) {
			config.rate = atoi(v);
			continue;
		}

		if (strcmp(p, "--gso") == 0) {
			config.gso = atoi(v);
			continue;
		}
		if (strcmp(p, "--msglen") == 0) {
			config.msglen = atoi(v);
			continue;
		}

		if (strcmp(p, "--sendmsg") == 0) {
			config.sendmsg = atoi(v);
			continue;
		}
		if (strcmp(p, "--sendmmsg") == 0) {
			config.sendmmsg = atoi(v);
			continue;
		}
	}

	signal(SIGALRM, alarm_handler);
	signal(SIGPIPE, SIG_IGN);
	alarm(1);


	pthread_t th[100] = {0};

	if (!mod->thread) {
		return -1;
	}

	for (i = 0; i < config.thread_n; ++i)
		pthread_create(th + i, NULL, (void *(*)(void*))mod->thread, NULL);

	for (i = 0; i < config.thread_n; ++i)
		pthread_join(th[i], NULL);

}
