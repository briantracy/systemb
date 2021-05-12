
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <signal.h>
#include <sys/time.h>
#include <inttypes.h>
#include <string.h>

#include "bperf.h"

#define MAX_TRACKED_FUNCTIONS 100
#define MICROSECONDS_BETWEEN_TICKS 20

#define MAX(A, B) ((A) < (B) ? (B) : (A))

/// One entry per function (up to a fixed number defined above)
static struct {
	const char *name;
	uint64_t timescalled;

} symbol_map[MAX_TRACKED_FUNCTIONS];
static size_t num_functions_tracked = 0;

/// Record one call to a given function. Create a new entry in the symbol
/// map if necessary.
void track_call(const char *const symbol) {
    for (size_t i = 0; i < MAX_TRACKED_FUNCTIONS; i++) {
        if (symbol_map[i].name == NULL) {
            symbol_map[i].name = symbol;
            num_functions_tracked++;
        }
        if (strcmp(symbol_map[i].name, symbol) == 0) {
            symbol_map[i].timescalled++;
            return;
        }
    }
    fprintf(stderr, "could not find symbol `%s`, or no space left\n", symbol);
}

void bperf_tick(int signum, siginfo_t *siginfo, void *ctx) {
    (void)signum; (void)siginfo;
    Dl_info info;
    greg_t const rip = ((ucontext_t *)ctx)->uc_mcontext.gregs[REG_RIP];
    int const s = dladdr((void *)rip, &info);
    if (s == 0) {
        fprintf(stderr, "dlerror: `%s`\n", dlerror());
        return;
    }
    track_call(info.dli_sname);
}

void bperf_start() {
	struct sigaction timesliceact;
	timesliceact.sa_sigaction = bperf_tick;
	timesliceact.sa_flags = SA_SIGINFO;
	struct timeval interval = {0, MICROSECONDS_BETWEEN_TICKS};
	struct itimerval timerval;
	timerval.it_value = interval;
	timerval.it_interval = interval;
	sigaction(SIGVTALRM, &timesliceact, 0);
	setitimer(ITIMER_VIRTUAL, &timerval, 0);
    // time slicing is started!
}

void bperf_stop() {
    struct sigaction ign;
    ign.sa_handler = SIG_IGN;
    ign.sa_flags = 0;
    sigaction(SIGVTALRM, &ign, NULL);
}


void bperf_display() {
    // just to make sure the user did not forget to stop the timer
    bperf_stop();

    uint64_t total_calls = 0;
    int max_len = 0;
    for (size_t i = 0; i < num_functions_tracked; i++) {
        total_calls += symbol_map[i].timescalled;
        max_len = MAX(max_len, (int)strlen(symbol_map[i].name));
    }
    printf("bperf tracked %lu functions with %lu total samples\n", num_functions_tracked, total_calls);
    printf("%*s  | #calls | %%calls | graphic\n", max_len, "name");
    char percentage_bar[13];
    memset(percentage_bar, '\0', 13);
    percentage_bar[0] = '[';
    percentage_bar[11] = ']';
    for (size_t i = 0; i < num_functions_tracked; i++) {
        double const percentage =
            ((double)symbol_map[i].timescalled / (double)total_calls) * 100.0;
        int const tenth_percentage = ((int)(percentage)) / 10;
        for (int i = 0; i < 10; i++) {
            if (i < tenth_percentage) {
                percentage_bar[i + 1] = '#';
            } else {
                percentage_bar[i + 1] = ' ';
            }
        }

        printf("%*s   %-8lu %-5.2f   %s\n", max_len + 1, symbol_map[i].name, symbol_map[i].timescalled, percentage, percentage_bar);

    }

}




