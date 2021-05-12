
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <assert.h>
#include <fcntl.h>

#include "bperf.h"

void tinywork() {
	volatile uint64_t j = 4;
	volatile uint64_t r = 4;
	while (j++ < (1L << 20)) {
		r |= j;
		r *= j;
		r -= (r >> 5);
	}
}
void leastwork() {
	volatile uint64_t j = 4;
	volatile uint64_t r = 4;
	while (j++ < (1L << 23)) {
		r |= j;
		r *= j;
		r -= (r >> 5);
	}
}
void middlework() {
	volatile uint64_t j = 4;
	volatile uint64_t r = 4;
	while (j++ < (1L << 24)) {
		r |= j;
		r *= j;
		r -= (r >> 5);
	}
}
void mostwork() {
	volatile uint64_t j = 4;
	volatile uint64_t r = 4;
	while (j++ < (1L << 25)) {
		r |= j;
		r *= j;
		r -= (r >> 5);
	}
}
void in_kernel() {
	#define BUFF_SIZE (4096 * 10)
	char buff[BUFF_SIZE];
	int const devrand = open("/dev/urandom", O_RDONLY);
	assert(devrand > 0);
	for (int i = 0; i < 100; i++) {
		read(devrand, buff, BUFF_SIZE);
	}
}

int main() {


	bperf_start();
	for (int i = 0; i < 10; i++) {
		in_kernel();
		tinywork();
		leastwork();
		middlework();
		mostwork();
		printf("finished round!\n");
	}
	bperf_stop();
	bperf_display();
}
