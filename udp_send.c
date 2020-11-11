#include "uperf.h"
#include "linux/udp.h"
#define SOL_UDP 17


static __thread int count;
static __thread int total;
static __thread struct timeval start;

static int delay(struct timeval *s)
{
	int limit = 100, left;
	int rate;
	int64_t tm;
    struct timeval now;
	struct timespec sleep;

	rate = config.rate / config.thread_n / 1000 / 100;// 10us

	++count;

	if (count >= limit) {
		total += count;
		gettimeofday(&now, NULL);
		tm = (now.tv_sec - s->tv_sec) * 1000 * 1000 + now.tv_usec - s->tv_usec;

		left = total / rate * 10 - tm;

		sleep.tv_sec = 0;
		sleep.tv_nsec = left * 1000;

		if (left > 0)
			nanosleep(&sleep, NULL);

		count = 0;
	}

    return 0;
}

static void udp_send()
{
  	int sockfd, ret;
    char *buffer;
	int v = 1;
    struct sockaddr_in   caddr,  servaddr;

    buffer = malloc(config.msglen);

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    /* reuseport */
	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &v, sizeof(v));
	if (ret) {
		printf("SO_REUSEPORT fail. %s\n", strerror(errno));
		return;
	}

    /* gso */
    if (config.gso) {
        ret = setsockopt(sockfd, SOL_UDP, UDP_SEGMENT, &config.gso,
                         sizeof(config.gso));
	    if (ret) {
		    printf("UDP_SEGMENT fail. %s\n", strerror(errno));
		    return;
	    }
    }

    /* bind */
    memset(&caddr, 0, sizeof(caddr));
	caddr.sin_family = AF_INET;
	caddr.sin_port = 0;
	caddr.sin_addr.s_addr = inet_addr("0.0.0.0");

	ret = bind(sockfd, (struct sockaddr *)&caddr, sizeof(caddr));
	if (ret) {
		printf("bind fail\n");
		return;
	}

    /* the peer addr */
    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(config.port);
    servaddr.sin_addr.s_addr = inet_addr(config.server);


	gettimeofday(&start, NULL);

    int n, len;

	while (1) {
    	sendto(sockfd, buffer, config.msglen, 0,
		       (const struct sockaddr *) &servaddr, sizeof(servaddr));
		__sync_fetch_and_add(&config.reqs, 1);

	    if (config.rate)
            delay(&start);
	}


    close(sockfd);
}

struct module mod_udp_send = {
	.thread = udp_send,
};






