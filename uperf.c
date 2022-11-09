
#include "uperf.h"

struct config config = {
	.thread_n = 1,
	.msglen = 16,
	.depth = 1,
	.sport = 0,
    .port = 8080,
    .time = 10,
    .server = "0.0.0.0",
};

struct thread *threads;

static struct module *mod;
static struct module def;

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

static void udp_echo(struct thread *th)
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
    u64 us;
    u64 latency;
    static struct timeval now, last;

    if (mod->stat) {
        mod->stat(NULL);
	    alarm(1);
        return;
    }

    reqs = config.reqs - config.reqs_last;

    gettimeofday(&now, NULL);

    us = (now.tv_usec - last.tv_usec) + (now.tv_sec - last.tv_sec) * 1000 * 1000;

    last = now;
    config.reqs_last = config.reqs;


    latency = reqs ? us * 1000 / reqs : 0;
    reqs = reqs * 1000 * 1000 / us;

    if (reqs)
        printf("reqs: %lldw %lld latency: %lluns us: %lld\n",
               reqs/10000, reqs,
               latency,
               us);

    if (now.tv_sec - config.start > config.time) {

        exit(0);
    }

	alarm(1);

}

static int parse_cpu(char *cpu_mask, int *cpu_list, int num)
{
    enum {
        STAGE_NUM,
        STAGE_END,
        STAGE_STEP,
    }state = STAGE_NUM;

    int cpu = 0, end = 0, step;
    int idx = 0;
    char *p;
    int len;

    len = strlen(cpu_mask);

    for (p = cpu_mask; p - cpu_mask <= len; ++p) {
        switch(state) {
        case STAGE_STEP:
            if (*p <= '9' && *p >= '0') {
                step = step * 10 + *p - '0';
                continue;
            }
            goto more;

        case STAGE_END:
            if (*p <= '9' && *p >= '0') {
                end = end * 10 + *p - '0';
                continue;
            }

            step = 1;

            if (*p == ':') {
                state = STAGE_STEP;
                step = 0;
                continue;
            }

more:

            for (cpu += step; cpu <= end; cpu += step) {
                if (idx >= num)
                    return idx;

                cpu_list[idx++] = cpu;
            }
            state = STAGE_NUM;
            cpu = 0;
            continue;

        case STAGE_NUM:
            if (*p <= '9' && *p >= '0') {
                cpu = cpu * 10 + *p - '0';
                continue;
            }

            if (idx >= num)
                return idx;

            cpu_list[idx++] = cpu;

            if (*p == ',') {
                cpu = 0;
                continue;
            }

            if (*p == '-') {
                end = 0;
                state = STAGE_END;
                continue;
            }

            return idx;
        }
    }

    return idx;
}

static int thread_create_with_cpu(pthread_t *th, int cpu, void *(*fun)(void *), void *arg)
{
    pthread_attr_t attr;
    cpu_set_t cpuset;
    int ret;

    pthread_attr_init(&attr);
    CPU_ZERO(&cpuset);

    CPU_SET(cpu, &cpuset);

    ret = pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset);
    if (ret != 0) {
        printf("pthread_attr_setaffinity_np\n");
        return ret;
    }

    ret = pthread_create(th, &attr, fun, arg);

    return ret;
}


static int thread_create(struct thread *th, int idx, void *(*fun)(void *))
{
    th->id = idx;

    if (!config.cpu_num) {
		return pthread_create(&th->pthread, NULL, fun, th);
    }


    thread_create_with_cpu(&th->pthread, config.cpu_list[idx], fun, th);
}

extern struct module mod_udp_send;
extern struct module mod_udp_recv;
extern struct module mod_udp_pingpong;

int parse_args(int argc, char *argv[])
{
    char *p, *v;
    int i;

	for (i = 1; i < argc; ++i) {
		p = argv[i];

		if (strcmp(p, "--") == 0) {
            if (!mod->args)
                return -1;

            if (mod->args(argc - i - 1, argv + i + 1))
                return -1;
            break;
		}

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
			mod = &mod_udp_recv;
			continue;
		}

		if (strcmp(p, "--udp-connect") == 0) {
			config.udp_connect = 1;
			continue;
		}

		if (strcmp(p, "--flag-oob") == 0) {
			config.flag_oob = 1;
			continue;
		}

		if (strcmp(p, "--flag-probe") == 0) {
			config.flag_probe = 1;
			continue;
		}

		if (strcmp(p, "--flag-confirm") == 0) {
			config.flag_confirm = 1;
			continue;
		}

		if (strcmp(p, "--stat") == 0) {
			config.stat = 1;
			continue;
		}

		if (strcmp(p, "--nonblock") == 0) {
			config.nonblock = 1;
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

		if (strcmp(p, "--ifname") == 0) {
			config.ifname = v;
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
		if (strcmp(p, "--cpu") == 0) {
            config.cpu_num = parse_cpu(v, config.cpu_list, sizeof(config.cpu_list)/sizeof(config.cpu_list[0]));
            if (config.cpu_num < 0) {
                printf("cpu parse error\n");
                return -1;
            }
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

		if (strcmp(p, "--flags") == 0) {
			config.flags |= strtol(v, NULL, 16);
			continue;
		}

		if (strcmp(p, "--time") == 0) {
			config.time = strtol(v, NULL, 10);
			continue;
		}

	}

    if (config.thread_n == 1 && config.cpu_num)
        config.thread_n = config.cpu_num;

    return 0;
}


int main(int argc, char *argv[])
{
    int port = 8080, i, ret;
	char *p = NULL, *v = NULL;

	mod = &def;

    if (parse_args(argc, argv))
        return -1;

    if (config.stat) {
        signal(SIGALRM, alarm_handler);
        alarm(1);
    }

	signal(SIGPIPE, SIG_IGN);

    {
        struct timeval now;
        gettimeofday(&now, NULL);
        config.start = now.tv_sec;
    }

	if (!mod->thread) {
		return -1;
	}

    if (mod->prepare) {
        if (mod->prepare(NULL))
            return -1;
    }

    if (config.thread_n == 1) {
        mod->thread(NULL);
        return 0;
    }

    if (config.cpu_num && config.cpu_num < config.thread_n) {
        printf("cpu num too less then thread num.\n");
        return -1;
    }


	struct thread *th;

    th = malloc(sizeof(*th) * config.thread_n);

    threads = th;

    memset(th, 0, sizeof(*th) * config.thread_n);

	for (i = 0; i < config.thread_n; ++i) {
        thread_create(th + i, i, (void *(*)(void*))mod->thread);
    }

	for (i = 0; i < config.thread_n; ++i)
		pthread_join(th[i].pthread, NULL);

}
