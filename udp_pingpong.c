#include "uperf.h"

#define buf "uperf test test test\n"

static void udp_pingpong(struct thread *th)
{
  	int sockfd;
	int depth = 0, step, i, ret;
    char buffer[1024];

    struct sockaddr_in     servaddr, caddr;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&caddr, 0, sizeof(caddr));
	caddr.sin_family      = AF_INET;
	caddr.sin_port        = 0;
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
    sendto(sockfd, buf, sizeof(buf) - 1, 0, \
	       (const struct sockaddr *) &servaddr, sizeof(servaddr)); \
	++send;

#define recv() recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);

    struct timeval start, end;
    u64 spent, m;

    gettimeofday(&start, NULL);

	while (1) {
		sendto();
		n = recv();

        if (++m > 1000) {
            gettimeofday(&end, NULL);
            spent = (end.tv_sec - start.tv_sec) * 1000 * 1000 + \
                    end.tv_usec - start.tv_usec;

		    __sync_fetch_and_add(&config.spent, spent);
            start = end;
            m = 0;
        }

		__sync_fetch_and_add(&config.reqs, 1);
	}
    close(sockfd);
}


static void udp_pingpong_alarm(void *_)
{
    u64 reqs;
    u64 spent;

    reqs = config.reqs - config.reqs_last;
    spent = config.spent;
    config.spent = 0;

    config.reqs_last = config.reqs;

	printf("reqs: %lldw usec: %lluus\n", reqs/10000, spent / reqs);
}

struct module mod_udp_pingpong = {
	.thread = udp_pingpong,
    .stat = udp_pingpong_alarm,
};

