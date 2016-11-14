#include <inc/lib.h>

// Simple Network Time Protocol (SNTP) Version 4
// https://tools.ietf.org/html/rfc4330

#define SNTP_HOST		"time-nw.nist.gov"
#define SNTP_PORT		123

#define SNTP_VERSION		4
#define SNTP_VN_SHIFT		3
#define SNTP_VN(x)		(((x) >> SNTP_VN_SHIFT) & 0x7)

#define SNTP_MODE_CLIENT	3
#define SNTP_MODE_SERVER	4
#define SNTP_MODE(x)		((x) & 0x7)

struct sntp_msg {
	uint8_t li_vn_mode;
	uint8_t stratum;
	uint8_t poll;
	uint8_t precision;
	uint32_t root_delay;
	uint32_t root_dispersion;
	uint32_t reference_identifier;
	uint32_t reference_timestamp[2];
	uint32_t priginate_timestamp[2];
	uint32_t receive_timestamp[2];
	uint32_t transmit_timestamp[2];
} __packed;

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
	struct sockaddr_in dst = {
		.sin_family = AF_INET,
		.sin_port = htons(SNTP_PORT),
	};
	struct sockaddr_in src = {
		.sin_family = AF_INET,
		.sin_port = htons(0),
		.sin_addr.s_addr = htonl(INADDR_ANY),
	};
	struct sntp_msg req = {
		.li_vn_mode = (SNTP_VERSION << SNTP_VN_SHIFT) | SNTP_MODE_CLIENT,
	};
	struct sntp_msg rep = {};
	seconds_t epoch;
	struct tm tm;

	binaryname = "sntp";
	static_assert(sizeof(struct sntp_msg) == 48);

	if (nsipc_gethostbyname(SNTP_HOST, (struct ip_addr *)&dst.sin_addr.s_addr))
		die("gethostbyname");

	if ((sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		die("socket");

	if (bind(sockfd, (struct sockaddr *)&src, sizeof(src)))
		die("bind"); 

	if (connect(sockfd, (struct sockaddr *)&dst, sizeof(dst)))
		die("connect");

	if (write(sockfd, &req, sizeof(req)) != sizeof(req))
		die("write");

	if (readn(sockfd, &rep, sizeof(rep)) != sizeof(rep))
		die("read");

	close(sockfd);

	if (SNTP_VN(rep.li_vn_mode) != SNTP_VERSION)
		die("invalid version");
	if (SNTP_MODE(rep.li_vn_mode) != SNTP_MODE_SERVER)
		die("invalid mode");
	if (rep.stratum == 0)
		die("KoD");

	// convert NTP time (1900/1/1) to epoch time (1970/1/1)
	epoch = ntohl(rep.receive_timestamp[0]) - 2208988800;
	epoch_to_tm(epoch, &tm);
	printf("%llu (%04u-%02u-%02u %02u:%02u:%02u)\n", epoch,
	       tm.tm_year + 1900, tm.tm_mon, tm.tm_mday,
	       tm.tm_hour, tm.tm_min, tm.tm_sec);
}
