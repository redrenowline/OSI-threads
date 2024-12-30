#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>


#define handle_error(en, msg) \
	do{ errno = en; perror(msg); exit(EXIT_FAILURE); } while(0)

int SIGINT1_received = 0;
int SIGINT2_received = 0;



void sigint_handler1() {
    	SIGINT1_received = 1;
    	printf("Поток 2: Получен сигнал SIGINT\n");
}

void sigint_handler2() {
    	SIGINT2_received = 1;
    	printf("Поток 3: Получен сигнал SIGINT\n");
}

void *thread_1_function() {
    	void* prev = signal(SIGINT, sigint_handler1);
    	if(prev == SIG_ERR)
    		handle_error(errno, "thread2 function failed: signal() doesn't work");
    	printf("Поток 2: Ожидание сигнала SIGINT...\n");
    	while(!SIGINT1_received) {
        	sleep(1);
    	}
    	pthread_exit(NULL);
}

void* thread_2_function() {
    	void* prev = signal(SIGINT, sigint_handler2);
    	if(prev == SIG_ERR)
    		handle_error(errno, "thread2 function failed: signal() doesn't work");
    	printf("Поток 3: Ожидание сигнала SIGINT...\n");
    	while(!SIGINT2_received) {
        	sleep(1);
    	}
    	pthread_exit(NULL);
}

int main() {
    	pthread_t thread1, thread2;

    	int err = pthread_create(&thread1, NULL, thread_1_function, NULL);
    	if(err != 0) 
    		handle_error(err, "main: pthread_create failed ");
    	err = pthread_create(&thread2, NULL, thread_2_function, NULL);
    	if(err != 0) 
    		handle_error(err, "main: pthread_create failed ");
    	
    	sleep(2);
    	err = pthread_kill(thread1, SIGINT);
    	if(err != 0) 
    		handle_error(err, "main: pthread_kill failed");
    	err = pthread_kill(thread2, SIGINT);
    	if(err != 0) 
    		handle_error(err, "main: pthread_kill failed");
    	
    	
    	err = pthread_join(thread1, NULL);
    	if(err != 0) 
    		handle_error(err, "main: pthread_join failed");
    	err = pthread_join(thread2, NULL);
    	if(err != 0) 
    		handle_error(err, "main: pthread_join failed");

    	return 0;
}
