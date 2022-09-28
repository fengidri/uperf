#define _GNU_SOURCE
#include "uperf.h"
#include "linux/udp.h"
#include "linux/ip.h"
#include <sys/socket.h>
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


#define MSG_PROBE	0x10	/* Do not send. Only probe path f.e. for MTU */

static void udp_send_simple(int fd, int flags, struct sockaddr_in *servaddr)
{
    char *buffer = malloc(config.msglen);

    while (1) {
    	    sendto(fd, buffer, config.msglen, flags,
		           (const struct sockaddr *) servaddr,
                   sizeof(*servaddr));
            __sync_fetch_and_add(&config.reqs, 1);
    }
}

static void udp_send_connect(int fd, int flags)
{
    char *buffer = malloc(config.msglen);

    while (1) {
        send(fd, buffer, config.msglen, flags);
        __sync_fetch_and_add(&config.reqs, 1);
    }
}

static void udp_send()
{
  	int sockfd, ret;
    char *buffer;
	int v = 1;
    int flags = config.flags;
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

    v = IP_PMTUDISC_DO;

    ret = setsockopt(sockfd, IPPROTO_IP, IP_MTU_DISCOVER, &v, sizeof(v));
	if (ret) {
		printf("IP_DONTFRAG fail. %s\n", strerror(errno));
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
	caddr.sin_port = htons(config.sport);
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

    if (config.udp_connect) {
        ret = connect(sockfd, (const struct sockaddr *) &servaddr, sizeof(servaddr));
	    if (ret) {
		    printf("connect fail\n");
		    return;
	    }
    }

    if (config.flag_oob)
        flags |= MSG_OOB;

    if (config.flag_probe)
        flags |= MSG_PROBE;

    if (config.flag_confirm)
        flags |= MSG_CONFIRM;


	gettimeofday(&start, NULL);

    if (!config.udp_connect) {
        udp_send_simple(sockfd, flags, &servaddr);
        return ;
    } else {
        udp_send_connect(sockfd, flags);
    }

    int n, len, i;
    u64 cycle;

	while (1) {
        cycle = rdtsc();
        n = 1;
        if (config.udp_connect) {
            if (config.sendmsg) {
                struct msghdr msg = {};
                struct iovec iovec[1000];
                msg.msg_iovlen = config.sendmsg;
                msg.msg_iov = iovec;
                for (i =0; i < config.sendmsg; ++i) {
                    iovec[i].iov_base = buffer;
                    iovec[i].iov_len = config.msglen;
                }

                n = sendmsg(sockfd, &msg, 0);
                n = n / config.msglen;

            } else if (config.sendmmsg) {
                struct mmsghdr vec[1000], *v;
                struct iovec io;

                io.iov_base = buffer;
                io.iov_len = config.msglen;

                for (i =0; i < config.sendmmsg; ++i) {
                    v = vec + i;
                    v->msg_hdr.msg_iov = &io;
                    v->msg_hdr.msg_iovlen = 1;
                }

                n = sendmmsg(sockfd, vec, config.sendmmsg, 0);
            }
            else{
                send(sockfd, buffer, config.msglen, 0);
            }
        }
        else{
    	    sendto(sockfd, buffer, config.msglen, 0,
		           (const struct sockaddr *) &servaddr, sizeof(servaddr));
        }
        config.cycle += rdtsc() - cycle;

		__sync_fetch_and_add(&config.reqs, n);

	    if (config.rate)
            delay(&start);
	}


    close(sockfd);
}

struct module mod_udp_send = {
	.thread = udp_send,
};






