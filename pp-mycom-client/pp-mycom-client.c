#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sysexits.h>
#include <unistd.h> 
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>

#include "logging.h"
#include "def.h"
#include "timeutils.h"
#include "mycom.h"

int
pingpong(int client, int to, int pass, size_t bufsize);

int
routine();

int
main(int argc, char **argv)
{
    struct sigaction sa;
    int i;
    pid_t cpid;
    key_t key;
    int stat;
    
    start_logging("pp-mycom-client");

    memset(&sa, 0, sizeof(struct sigaction));
    
    sa.sa_flags = SA_NOCLDWAIT;
    sigaction(SIGCHLD, &sa, NULL);

    for (i = 0; i < 4; i++) {
        if ((cpid = fork()) < 0) {
            l_perr("fork");
            exit(EX_OSERR);
        }

        if (0 == cpid) {
            if (routine() < 0)
                exit(EX_SOFTWARE);
            
            exit(EX_OK);
        }
    }
    
    while (-1 != wait(&stat)) ;

    stop_logging();
    
    exit(EX_OK);
}

int
pingpong(int client, int to, int pass, size_t bufsize)
{
    char bufrecv[BUF_SIZE_MAX] = {0}, buf[BUF_SIZE_MAX] = {0};
    int fd;
    ssize_t ssend, srecv;
    struct timeval tv[3], tvdiff;
    int i;

    if ((fd = open("/dev/urandom", O_RDONLY)) < 0) {
        l_perr("open");
        return -1;
    }

    if (read(fd, buf, bufsize) < 0) {
        l_perr("read");
        if (close(fd) < 0)
            l_perr("close");
        return -1;
    }
    
    if (close(fd) < 0)
        l_perr("close");

    for (i = 0; i < pass; i++) {
        memset(&tv, 0, 4 * sizeof(struct timeval));
        memset(&tvdiff, 0, sizeof(struct timeval));
        
        if (gettimeofday(&tv[0], NULL) < 0) {
            l_perr("gettimeofday");
            return -1;
        }
        
        if ((ssend = mycom_send(client, to, buf, bufsize)) < 0) {
            if (errno) 
                l_perr("mycom_send");
            else
                l_err("mycom_send: -1");
            return -1;
        }
        
        if (gettimeofday(&tv[1], NULL) < 0) {
            l_perr("gettimeofday");
            return -1;
        }

        if ((srecv = mycom_recv(client, to, bufrecv, bufsize)) < 0) {
            if (errno) 
                l_perr("mycom_recv");
            else
                l_err("mycom_recv: -1");
            return -1;
        }
        
        if (gettimeofday(&tv[3], NULL) < 0) {
            l_perr("gettimeofday");
            return -1;
        }
        
        l_debug("sending %ld byte", ssend);
        
        if (ssend != bufsize)
            l_warn("send size != buffer size!");
        
        l_debug("recv %ld byte", srecv);
        
        if (srecv != bufsize)
            l_warn("recv size != buffer size!");
        
        if (memcmp(buf, bufrecv, bufsize) != 0)
            l_warn("buf != recv buf");
        
        timediff(&tv[0], &tv[1], &tvdiff);
        
        l_info("%d:pass %d/%d:diff befor/after send %ld byte: %ld.%.6ld",
               client, i + 1, pass, bufsize, tvdiff.tv_sec, tvdiff.tv_usec);
        
        memset(&tvdiff, 0, sizeof(struct timeval));
        
        timediff(&tv[1], &tv[2], &tvdiff);
        
        l_info("%d:pass %d/%d:diff befor/after recv %ld byte: %ld.%.6ld",
               client, i + 1, pass, bufsize, tvdiff.tv_sec, tvdiff.tv_usec);
    }

    return 0;
}

int
routine()
{
    int sock;
    int client;
    int to;

    if ((client = mycom_get(BUF_SIZE_MAX)) < 0) {
        if (errno) 
            l_perr("mycom_connect");
        else
            l_err("mycom_connect: -1");
    }

    to = client - 4;

    if (to < 0) {
        l_err("to < 0");
        return -1;
    }

    if (pingpong(client, to, TEST_PASS, BUF_SIZE1) < 0)
        return -1;

    if (pingpong(client, to, TEST_PASS, BUF_SIZE2) < 0)
        return -1;

    if (pingpong(client, to, TEST_PASS, BUF_SIZE3) < 0)
        return -1;

    if (pingpong(client, to, TEST_PASS, BUF_SIZE4) < 0)
        return -1;

    mycom_destroy(client);

    return 0;
}
