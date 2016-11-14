#include <inc/lib.h>

// RFC 867: Daytime Protocol
#define SERVER_HOST	"time-nw.nist.gov"
#define SERVER_PORT	13

static void
die(char *m, ...)
{
	cprintf("Failed to invoke %s\n", m);
	exit();
}

void
umain(int argc, char **argv)
{
	int sockfd;
	char buf[64];
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(SERVER_PORT),
	};

	binaryname = "daytime";

	if (nsipc_gethostbyname(SERVER_HOST, (struct ip_addr *)&addr.sin_addr.s_addr))
		die("resolve");

	if ((sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		die("socket");

	if (connect(sockfd, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in)))
		die("connect");
	
	for (;;) {
		int r = read(sockfd, buf, sizeof(buf) - 1);

		if (r == 0)
			break;
		if (r < 0)
			die("read");
		buf[r] = 0;
		printf("%s", buf);
	}

	close(sockfd);
}
