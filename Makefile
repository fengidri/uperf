
all: clean uperf


uperf: uperf.c udp_send.c udp_pingpong.c
	$(CC) $^ -o uperf -l pthread -g

clean:
	-rm uperf
