#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#define handle_error(en, msg) \
	do{ errno = en; perror(msg); exit(EXIT_FAILURE); } while(0)
#define COUNT_THREADS 3


int SIGQUIT_received = 0;
int SIGINT_received = 0;



void sigint_handler() {
    	SIGINT_received = 1;
    	printf("Поток 2: Получен сигнал SIGINT\n");
}

void *thread_1_function() {
    	sigset_t signal_set;
    	printf("Поток 1: Начало работы\n");
    	sigfillset(&signal_set);
    	pthread_sigmask(SIG_BLOCK, &signal_set, NULL);

    	while (!(SIGQUIT_received && SIGINT_received)) {
        	sleep(1);
    	}

    	printf("Поток 1: Завершение работы\n");
    	pthread_exit(NULL);
}

void *thread_2_function() {
    	signal(SIGINT, sigint_handler);
    	printf("Поток 2: Ожидание сигнала SIGINT...\n");
    	while(!SIGINT_received) {
        	sleep(1);
    	}
    	pthread_exit(NULL);
}

void* thread_3_function() {
    	int sig;
    	sigset_t set;
    	sigemptyset(&set);
    	sigaddset(&set, SIGQUIT);
    	printf("Поток 3: Ожидание сигнала SIGQUIT...\n");
    	int result = sigwait(&set, &sig);
    	if (result == 0) {
        	printf("Поток 3: Получил сигнал SIGQUIT\n");
        	SIGQUIT_received = 1;
    	} else {
        	handle_error(result, "thread3: sigwait failed ");
    	}
    	pthread_exit(NULL);
}

int main() {
    	pthread_t* threads = (pthread_t*)malloc(3 * sizeof(pthread_t));


    	int err = pthread_create(&threads[0], NULL, thread_1_function, NULL);
    	if(err != 0) 
    		handle_error(err, "main: pthread_create failed ");
    	err = pthread_create(&threads[1], NULL, thread_2_function, NULL);
    	if(err != 0) 
    		handle_error(err, "main: pthread_create failed ");
    	err = pthread_create(&threads[2], NULL, thread_3_function, NULL);
    	if(err != 0) 
    		handle_error(err, "main: pthread_create failed ");
    	
    	
    	sleep(1);
    	err = pthread_kill(threads[1], SIGINT);
    	if(err != 0) 
    		handle_error(err, "main: pthread_kill failed");
    	sleep(1);
    	err = pthread_kill(threads[2], SIGQUIT);
    	if(err != 0) 
    		handle_error(err, "main: pthread_kill failed");
    	
    	
 	for(int i=0;i < COUNT_THREADS; i++){
 		err = pthread_join(threads[i], NULL);
 		if(err != 0){
 			handle_error(err, "main: pthread_join failed");
 		}
 	}
    	return 0;
}
