
all: uperf

uperf: uperf.c udp_send.c
	$(CC) $^ -o uperf -l pthread -g

