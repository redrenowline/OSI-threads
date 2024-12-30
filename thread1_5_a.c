#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#define handle_error(en, msg) \
	{errno = en; perror(msg); pthread_exit((void*)EXIT_FAILURE); }
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
    	int err = sigfillset(&signal_set);
    	if(err != 0){
    		handle_error(errno, "thread1 function failed: sigfillset return -1");
    	}
    	err = pthread_sigmask(SIG_BLOCK, &signal_set, NULL);
	if(err != 0)
		handle_error(err, "thread1 function failed: pthread_sigmask doesn't work");

    	while (!(SIGQUIT_received && SIGINT_received)) {
        	sleep(1);
    	}

    	printf("Поток 1: Завершение работы\n");
    	pthread_exit(NULL);
}

void *thread_2_function() {
        void* prev = signal(SIGINT, sigint_handler);
    	if(prev == SIG_ERR)
    		handle_error(errno, "thread2 function failed: signal() doesn't work");
    	printf("Поток 2: Ожидание сигнала SIGINT...\n");
    	while(!SIGINT_received) {
        	sleep(1);
    	}
    	pthread_exit(NULL);
}

void* thread_3_function() {
    	int sig;
    	sigset_t set;
    	int err = sigemptyset(&set);
    	if(err != 0){
    		handle_error(errno, "thread3 function failed: sigemptyset() doesn't work");
    	}
    	err = sigaddset(&set, SIGQUIT);
    	if(err != 0){
    		handle_error(errno, "thread3 function failed: sigaddset() doesn't work");
    	}
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
	if(threads == NULL){
		handle_error(errno, "main: malloc can't get memory ");
	}


    	int err = pthread_create(&threads[0], NULL, thread_1_function, NULL);
    	if(err != 0){
    		free(threads);
    		handle_error(err, "main: pthread_create failed ");
    	}
    	err = pthread_create(&threads[1], NULL, thread_2_function, NULL);
    	if(err != 0){
    		free(threads);
    		handle_error(err, "main: pthread_create failed ");
    	}
    	err = pthread_create(&threads[2], NULL, thread_3_function, NULL);
    	if(err != 0){
    		free(threads);
    		handle_error(err, "main: pthread_create failed ");
    	}
    	
    	sleep(1);
    	err = pthread_kill(threads[1], SIGINT);
    	if(err != 0){
    		free(threads);
    		handle_error(err, "main: pthread_kill failed");
    	}
    	sleep(1);
    	err = pthread_kill(threads[2], SIGQUIT);
    	if(err != 0) {
    		free(threads);
    		handle_error(err, "main: pthread_kill failed");
    	}
    	
 	for(int i=0;i < COUNT_THREADS; i++){
 		err = pthread_join(threads[i], NULL);
 		if(err != 0){
 			free(threads);
 			handle_error(err, "main: pthread_join failed");
 		}
 	}
 	free(threads);
    	pthread_exit(NULL);
}
