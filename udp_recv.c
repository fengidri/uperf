
#include "uperf.h"
#include "linux/udp.h"
#include "linux/ip.h"
#include <sys/socket.h>
#include <net/if.h>

#include <linux/filter.h>

static int fds[1000];
static int g_usleep;
static int cpu_offset;

static void stat(void *_)
{
    struct thread *th;
    static u64 reqs_last[1024];
    u64 total = 0, speed;
    int i;

    for (i = 0; i < config.thread_n; ++i) {
        th = threads + i;

        speed = th->reqs - reqs_last[i];
        reqs_last[i] = th->reqs;
        total += speed;

        printf("thread %d: recv %llu\n", i, speed);
    }

    total = total/ 1000 / 100;

    printf("total: %llu.%lluM\n", total/10, total %10);
}


static void attach_cbpf(int fd,uint16_t mod)
{
    struct sock_filter code[] = {
        /* A = raw_smp_processor_id() */
        { BPF_LD  | BPF_W | BPF_ABS, 0, 0, SKF_AD_OFF + SKF_AD_CPU},
        /* return A */
        { BPF_RET | BPF_A, 0, 0, 0 },
    };
    struct sock_fprog p = {
        .len = 2,
        .filter = code,
    };

    mod = mod;
    if (setsockopt(fd, SOL_SOCKET, SO_ATTACH_REUSEPORT_CBPF, &p, sizeof(p))) {
        perror("socket SO_ATTACH_REUSEPORT_CBPF failed");
        exit(-1);
    }
}

static int udp_listen_fd()
{
  	int sockfd, rc, ret;
    int v = 1;
    int n;

    struct sockaddr_in     servaddr;

    int type;

    type = SOCK_DGRAM;
    if (config.nonblock)
        type |= SOCK_NONBLOCK;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, type, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &v, sizeof(v));
	if (ret) {
		printf("SO_REUSEPORT fail. %s\n", strerror(errno));
		return -1;
	}


    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(config.port);
    servaddr.sin_addr.s_addr = inet_addr(config.server);

	rc = bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
	if (rc < 0) {
        perror("socket bind failed");
		return -1;
	}

    attach_cbpf(sockfd, 0);

    return sockfd;
}

static int udp_prepare(void *_)
{
    int i, fd;

    for (i = 0; i < config.thread_n; ++i) {
        fd = udp_listen_fd();
        if (fd < 0) {
            printf("create udp liten fd fail\n");
            return -1;
        }

        fds[i] = fd;
    }

    return 0;
}

static void udp_recv(struct thread *th)
{
    int n, rc;
    char buffer[2000];
    int sockfd = fds[th->id];

    n = 0;
	while (1) {
		rc = recv(sockfd, buffer, sizeof(buffer), 0);
        if (rc < 0) {
            usleep(g_usleep);
            continue;
        }

        ++n;
        if (n == 10) {
            th->reqs += n;
            n = 0;
        }
	}


    close(sockfd);
}

static int args(int argc, char *argv[])
{
    char *p, *v;
    int i;

	for (i = 0; i < argc; ++i) {
		p = argv[i];

		if (++i >= argc) {
			printf("except args for %s\n", p);
			return -1;
		}

		v = argv[i];

		if (strcmp(p, "--usleep") == 0) {
            g_usleep = atoi(v);
			continue;
		}
		if (strcmp(p, "--cpu-offset") == 0) {
            cpu_offset = atoi(v);
			continue;
		}
    }
    return 0;
}

struct module mod_udp_recv = {
    .args = args,
    .prepare = udp_prepare,
	.thread = udp_recv,
    .stat = stat,
};
