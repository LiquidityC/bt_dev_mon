#include <errno.h>
#include <stdatomic.h>
#include <stdio.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#include "defines.h"
#include "bt_handler.h"

#define INTERVAL_MIN 15
#define INTERVAL_SEC (INTERVAL_MIN * 60)

#define handle_error_en(en, msg) \
    do { errno = en; perror(msg); exit(EXIT_FAILURE); } while(0)

#define handle_error(en, msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while(0)

static atomic_int quit = 0;

static void *thread_exec(void *args)
{
    (void) args;

    printf("Launching thread\n");
    while (!quit) {

        bt_ping("B8:90:47:25:49:00");

        for (size_t i = 0; i < INTERVAL_SEC; ++i) {
            sleep(1);
            if (quit)
                break;
        }
        printf("\n");
    }

    return NULL;
}

static void sig_quit(int sig)
{
    psignal(sig, "Caught signal");
    quit = true;
}

int main(int argc, char *argv[])
{
    struct sigaction sa;
    pthread_attr_t attr;
    int s;

    /* Setup the signal handler */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sig_quit;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    /* Setup and spawn the execution thread */
    s = pthread_attr_init(&attr);
    if (s != 0) {
        handle_error_en(s, "pthread_attr_init");
    }

    pthread_t tid;
    s = pthread_create(&tid, &attr, &thread_exec, NULL);
    if (s != 0) {
        handle_error_en(s, "pthread_create");
    }

    /* Clean up and wait for thread to exit */
    s = pthread_attr_destroy(&attr);
    if (s != 0) {
        handle_error_en(s, "pthread_attr_destroy");
    }

    s = pthread_join(tid, NULL);
    if (s != 0) {
        handle_error_en(s, "pthread_join");
    }

    return 0;
}
