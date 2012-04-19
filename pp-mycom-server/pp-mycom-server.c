#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sysexits.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>

#include "logging.h"
#include "def.h"
#include "mycom.h"

/* func decl */

int
pinpong(int sock, int csock);

int
bind();

int
routine();


/* func release */

int
main(int argc, char **argv)
{
    struct sigaction sa;
    int stat;

    start_logging("pp-mycom-server");

    // signals... SIGCHLD
    memset(&sa, 0, sizeof(struct sigaction));
    
    sa.sa_flags = SA_NOCLDWAIT | SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    if (routine() < 0)
        exit(EX_SOFTWARE);

    stop_logging();

    exit(EX_OK);
}

int
pingpong()
{
    char buf[BUF_SIZE_MAX] = {0};
    ssize_t srecv, ssend;
    int client, from;

    if ((client = mycom_get(BUF_SIZE_MAX)) < 0) {
        if (errno) 
            l_perr("mycom_connect");
        else
            l_err("mycom_connect: -1");
        return -1;
    }

    from = client + 4;
    
    while (1) {
        if ((srecv = mycom_recv(client, from, buf, BUF_SIZE_MAX)) < 0) {
            if (errno) 
                l_perr("mycom_recv");
            else
                l_err("mycom_recv: -1");
            return -1;
        }

        if (0 == srecv)
            continue;

        if ((ssend = mycom_send(client, from, buf, srecv)) < 0) {
            if (errno) 
                l_perr("mycom_send");
            else
                l_err("mycom_send: -1");
            return -1;
        }
        
        l_debug("recv %ld bytes", srecv);
        l_debug("send %ld bytes", ssend);
        
        if (srecv != ssend) {
            l_warn("recv bytes != send bytes!");
        }
        
        if (srecv == 1)
            break;
    }

    mycom_destroy(client);

    return 0;
}

int
routine()
{
    int i;
    int ret;
    pid_t cpid;
    int stat;

    for (i = 0; i < 4; i++) {
        if ((cpid = fork()) < 0) {
            l_perr("fork");
            return -1;
        } else {
            if (0 == cpid) {
                if (pingpong() < 0) {
                    l_err("can't ping or pong ):");
                    return -1;
                }
                return 0;
            }
            usleep(1000);
        }
    }

    while (-1 != wait(&stat)) ;

    return 0;
}

