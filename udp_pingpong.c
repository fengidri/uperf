#include "uperf.h"

#define buf "uperf test test test\n"

static void udp_pingpong(void *_)
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
    sendto(sockfd, buf, sizeof(buf) - 1, 0, \
	       (const struct sockaddr *) &servaddr, sizeof(servaddr)); \
	++send;

#define recv() recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);

	while (1) {
		sendto();
		n = recv();
		__sync_fetch_and_add(&config.reqs, 1);
	}
    close(sockfd);
}

struct module mod_udp_pingpong = {
	.thread = udp_pingpong,
};

