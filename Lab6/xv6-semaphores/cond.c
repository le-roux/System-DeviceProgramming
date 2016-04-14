#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"
#include "spinlock.h"

#define COUNT 12
#define COUNT_LIMIT 5

char buf[8192];
char name[3];
int stdout = 1;
int counter = 0;

void condition_test(void) {
  int pid;
  int cond;

  cond = cond_alloc();
  
  printf(stdout, "A\n");
  pid = fork();
  if (pid) {
	sleep(20);
	for (counter = 0; counter < COUNT; counter++) {
		cond_lock(cond);
		cond_set(cond, cond_get(cond) + 1);
		if (cond_get(cond) == COUNT_LIMIT) {
			cond_signal(cond);
			printf(stdout, "signal\n");
		}
		cond_unlock(cond);
		sleep(10);
	}
   	wait();
	printf(stdout, "destroy\n");
   	cond_destroy(cond);
	exit();
  }
  else {
    sleep(5);
    printf (stdout,  "B\n");
	while (1) {
		cond_lock(cond);
		while (cond_get(cond) < COUNT_LIMIT) {
			printf(stdout, "waiting\n");
			cond_wait(cond);
		}
		cond_unlock(cond);
		printf (stdout, "signal received\n");
		cond_set(cond, 0);
	}
	exit();
  }
}

int main(int argc, char *argv[])
{
  condition_test();
  exit();
}
