#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

static void usage(char *prog)
{
	fprintf(stderr, "usage: %s [options]\n"
			"options:\n"
			"   -p port     Port to listen on (default 22124)\n"
			"   -s port     Port for localhost server (default 22123)\n"
			"   -?          Help (this message)\n"
			, prog);
	exit(1);
}

#define NBUFS 5

struct bufs {
	int len;
	char buf[1000];
} bufs[NBUFS];

int curbuf = 0;

int main(int argc, char **argv)
{
	extern char *optarg;
	extern int optind;
	int c;
	char *tmp;
	int n, proto;
	int sock;
	struct sockaddr_in listensa, serversa, clientsa;
	socklen_t salen;
	unsigned int num;
	unsigned short serverport = 22123;
	unsigned short listenport = 22124;
	//char buf[2048];
	char errbuf[128];

	while ((c = getopt(argc, argv, "s:p:?")) != EOF)
		switch (c) {
			case 'p':       /* port to listen on */
				num = strtoul(optarg, &tmp, 10);
				if (num < 1 || num > 65535 || *tmp) {
					fprintf(stderr, "%s: invalid value for -%c\n", argv[0], c);
					usage(argv[0]);
				}
				listenport = num;
				break;
			case 's':       /* port for local server */
				num = strtoul(optarg, &tmp, 10);
				if (num < 1 || num > 65535 || *tmp) {
					fprintf(stderr, "%s: invalid value for -%c\n", argv[0], c);
					usage(argv[0]);
				}
				serverport = num;
				break;
			case '?':       /* help */
			default:
				usage(argv[0]);
		}

	if (optind < argc)
		usage(argv[0]);

	if (listenport == 0) {
		fprintf(stderr, "Need -p option for listen port\n");
		usage(argv[0]);
	}

	if (serverport == 0) {
		fprintf(stderr, "Need -s option for server port\n");
		usage(argv[0]);
	}

	srandom(time(NULL));

	/* create listening socket */
	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		fprintf(stderr, "can't create socket: %s\n", strerror_r(errno, errbuf, sizeof(errbuf)));
		exit(1);
	}

	memset(&listensa, 0, sizeof(listensa));
	listensa.sin_family = AF_INET;
	listensa.sin_addr.s_addr = 0;
	listensa.sin_port = htons(listenport);
	salen = sizeof(listensa);

	if (bind(sock, (struct sockaddr *)&listensa, sizeof(listensa)) < 0) {
		fprintf(stderr, "can't bind to listening port: %s\n", strerror_r(errno, errbuf, sizeof(errbuf)));
		exit(1);
	}

	/* set up destination address for server */
	memset(&serversa, 0, sizeof(serversa));
	serversa.sin_family = AF_INET;
	serversa.sin_addr.s_addr = htonl(0x7F000001);
	serversa.sin_port = htons(serverport);
	salen = sizeof(serversa);

	printf("Listening for packets on port %u\n", listenport);

	salen = sizeof(listensa);
	while ((n = recvfrom(sock, bufs[curbuf].buf, sizeof(bufs[curbuf].buf), 0, (struct sockaddr *)&listensa, &salen)) > 0) {

		if (listensa.sin_addr.s_addr == serversa.sin_addr.s_addr && listensa.sin_port == serversa.sin_port) {
			/* packet from the server */

			if (sendto(sock, bufs[curbuf].buf, n, 0, (struct sockaddr *)&clientsa, sizeof(clientsa)) < 0) {
				fprintf(stderr, "Failed to send packet to client: %s\n", strerror_r(errno, errbuf, sizeof(errbuf)));
			}
		} else {
			/* packet from remote client - make a note of where */
			clientsa = listensa;

			/* only potentially disrupt order of audio packets */
			proto = bufs[curbuf].buf[0] == 0 && bufs[curbuf].buf[1] == 0;

			if (!proto && curbuf < NBUFS-1 && random() < RAND_MAX/5) {
				/* queue up the packet 20% of the time, provided we haven't queued too many */
				bufs[curbuf].len = n;
				curbuf++;
			} else {
				if (sendto(sock, bufs[curbuf].buf, n, 0, (struct sockaddr *)&serversa, sizeof(serversa)) < 0) {
					fprintf(stderr, "Failed to send packet to server: %s\n", strerror_r(errno, errbuf, sizeof(errbuf)));
				}

				/* now send any queued packets in reverse order */
				while (curbuf) {
					--curbuf;
					n = bufs[curbuf].len;

					if (sendto(sock, bufs[curbuf].buf, n, 0, (struct sockaddr *)&serversa, sizeof(serversa)) < 0) {
						fprintf(stderr, "Failed to send packet to server: %s\n", strerror_r(errno, errbuf, sizeof(errbuf)));
					}
				}
			}
		}
	}

	printf("Loop returned with n = %d\n", n);

	close(sock);

	return 0;
}
