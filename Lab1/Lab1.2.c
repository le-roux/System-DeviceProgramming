#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#define END_TIME 60

void timer(int sig_nb);
void altern(int sig_nb);
void end_child(int sig_nb);
void end_parent(int sig_nb);

int print = 1;

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Wrong number of arguments\n Correct syntax is %s filename", argv[0]);
		return 1;
	}
	char* filename = argv[1];
	int ret_val = fork();
	if (ret_val) {
		//Parent process
		signal(SIGUSR1, altern);
		signal(SIGUSR2, end_parent);
		FILE* f = fopen(filename, "r");
		if (f == NULL) {
			return 1;
		}
		char line[100] = {};
		size_t len = 0;
		int line_number = 0;
		while(1) {
			//Print the file line by line on the standard output
			while(fgets(line, 100, f) != NULL) {
				line_number++;
				if (print) {
					printf("%i %s", line_number, line);	
				}
			}
			line_number = 0;
			rewind(f);
		}
	} else {
		//Child process
		int total_time = 0;
		srand(time(NULL));
		int time = rand() % 10 + 1;
		while(total_time + time < END_TIME) {
			signal(SIGALRM, timer);
			total_time += time;
			alarm(time);
			pause();
			time = rand() % 10 + 1;			
		}
		//Final signal, that should terminate the processes
		signal(SIGALRM, end_child);
		alarm(END_TIME - total_time);
		pause();
	}
	return 0;
}

//SIGALRM handler
void timer(int sig_nb) {
	kill(getppid(), SIGUSR1);
	signal(SIGALRM, timer);
}

//SIGUSR1 handler, activate or deactivate the printing of the file content
void altern(int sig_nb) {
	print = 1 - print;
	signal(SIGUSR1, altern);
}

//final SIGALRM handler
void end_child(int sig_nb) {
	kill(getppid(), SIGUSR2);
	kill(getpid(), SIGTERM);
}

//SIGUSR2 handler
void end_parent(int sig_nb) {
	kill(getpid(), SIGTERM);
}
