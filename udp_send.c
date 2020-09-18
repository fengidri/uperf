#include "uperf.h"

static int udp_send_rate(int sockfd)
{
    	char buffer[22];

    	struct sockaddr_in     servaddr;

    	memset(&servaddr, 0, sizeof(servaddr));

    	servaddr.sin_family = AF_INET;
    	servaddr.sin_port = htons(config.port);
    	servaddr.sin_addr.s_addr = inet_addr(config.server);

    	int n, len;
	struct timeval s, now;
	int limit = 100, count = 0, total = 0, left;
	int rate;
	int64_t tm;
	struct timespec sleep;

	rate = config.rate / config.thread_n / 1000 / 100;// 10us

	gettimeofday(&s, NULL);

	while (1) {
    		sendto(sockfd, buffer, sizeof(buffer), 0,
		       (const struct sockaddr *) &servaddr,
               	       sizeof(servaddr));
		__sync_fetch_and_add(&config.reqs, 1);
		++count;

		if (count >= limit) {
			total += count;
			gettimeofday(&now, NULL);
			tm = (now.tv_sec - s.tv_sec) * 1000 * 1000 + now.tv_usec - s.tv_usec;

			left = total / rate * 10 - tm;

			sleep.tv_sec = 0;
			sleep.tv_nsec = left * 1000;

			if (left > 0)
				nanosleep(&sleep, NULL);

			count = 0;
		}
	}


    	close(sockfd);

}

static void udp_send()
{
  	int sockfd, ret;
    	char buffer[22];
	int v = 1;

    	struct sockaddr_in   caddr,  servaddr;

    	// Creating socket file descriptor
    	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        	perror("socket creation failed");
        	exit(EXIT_FAILURE);
    	}

	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &v, sizeof(v));
	if (ret) {
		printf("SO_REUSEPORT fail. %s\n", strerror(errno));
		return;
	}


    	memset(&caddr, 0, sizeof(caddr));
	caddr.sin_family = AF_INET;
	caddr.sin_port = config.sport;
	caddr.sin_addr.s_addr = inet_addr("0.0.0.0");

	ret = bind(sockfd, (struct sockaddr *)&caddr, sizeof(caddr));
	if (ret) {
		printf("bind fail\n");
		return;
	}


	if (config.rate) {
		 udp_send_rate(sockfd);
		 return;

	}

    	memset(&servaddr, 0, sizeof(servaddr));

    	// Filling server information
    	servaddr.sin_family = AF_INET;
    	servaddr.sin_port = htons(config.port);
    	servaddr.sin_addr.s_addr = inet_addr(config.server);

    	int n, len;

	while (1) {
    		sendto(sockfd, buffer, sizeof(buffer), 0,
		       (const struct sockaddr *) &servaddr,
               	       sizeof(servaddr));
		__sync_fetch_and_add(&config.reqs, 1);
	}


    	close(sockfd);
}

struct module mod_udp_send = {
	.thread = udp_send,
};






