#include "ns.h"

void
timer(envid_t ns_envid, nanoseconds_t interval) {
	nanoseconds_t start, elapsed = 0;

	binaryname = "ns_timer";

	while (1) {
		if (elapsed < interval)
			nanosleep(interval - elapsed);

		ipc_send(ns_envid, NSREQ_TIMER, 0, 0);

		start = uptime();
		while (1) {
			envid_t whom;

			ipc_recv(&whom, 0, 0);
			if (whom != ns_envid) {
				cprintf("NS TIMER: timer thread got IPC message from env %x not NS\n", whom);
				continue;
			}
			break;
		}
		elapsed = uptime() - start;
	}
}
