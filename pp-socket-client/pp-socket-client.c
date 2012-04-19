#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
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

/* buf to send */
char buf[BUF_SIZE_MAX] = {0};

int
un_connect();

int
pingpong(int sock, int pass, size_t bufsize);

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

    start_logging("pp-socket-client");

    memset(&sa, 0, sizeof(struct sigaction));
    
    sa.sa_flags = SA_NOCLDWAIT | SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    for (i = 0; i < 4; i++) {
        if ((cpid = fork()) < 0) {
            l_perr("fork");
            exit(EX_OSERR);
        }

        if (0 == cpid) {
            if (routine() < 0)
                exit(EX_SOFTWARE);
        }
    }

    if (0 != cpid)
        while (-1 != wait(&stat)) ;

    stop_logging();

    exit(EX_OK);
}

int
un_connect()
{
    int sock, csock;
    struct sockaddr_un saun;

    memset(&saun, 0, sizeof(struct sockaddr_un));
    
    if ((sock = socket(PF_LOCAL, SOCK_STREAM, 0)) < 0) {
        l_perr("socket");
        return -1;
    }
    
    saun.sun_family = AF_UNIX;
    strncpy(saun.sun_path, SOCKPATH, sizeof(saun.sun_path));
    saun.sun_path[sizeof(saun.sun_path) - 1] = '\0';

    if ((csock = connect(sock, (struct sockaddr *)&saun, sizeof(struct sockaddr_un))) < 0) {
        l_perr("connect");
        return -1;
    }
    
    return sock;
}

int
pingpong(int sock, int pass, size_t bufsize)
{
    char bufrecv[BUF_SIZE_MAX] = {0};
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
        memset(&tv, 0, 3 * sizeof(struct timeval));
        memset(&tvdiff, 0, sizeof(struct timeval));
        
        if (gettimeofday(&tv[0], NULL) < 0) {
            l_perr("gettimeofday");
            return -1;
        }
        
        if ((ssend = send(sock, buf, bufsize, 0)) < 0) {
            l_perr("send");
            return -1;
        }
        
        if (gettimeofday(&tv[1], NULL) < 0) {
            l_perr("gettimeofday");
            return -1;
        }
        
        if ((srecv = recv(sock, bufrecv, BUF_SIZE_MAX, 0)) < 0) {
            l_perr("recv");
            return -1;
        }
        
        if (gettimeofday(&tv[2], NULL) < 0) {
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
               getpid(), i + 1, pass, bufsize, tvdiff.tv_sec, tvdiff.tv_usec);
        
        memset(&tvdiff, 0, sizeof(struct timeval));
        
        timediff(&tv[1], &tv[2], &tvdiff);
        
        l_info("%d:pass %d/%d:diff befor/after recv %ld byte: %ld.%.6ld",
               getpid(), i + 1, pass, bufsize, tvdiff.tv_sec, tvdiff.tv_usec);
    }

    return 0;
}

int
routine()
{
    int sock;

    if ((sock = un_connect()) < 0) {
        l_err("can't connect to socket '%s'", SOCKPATH);
        return -1;
    }

    if (pingpong(sock, 1, 2) < 0)
        goto err;

    if (pingpong(sock, TEST_PASS, BUF_SIZE1) < 0)
        goto err;

    if (pingpong(sock, TEST_PASS, BUF_SIZE2) < 0)
        goto err;

    if (pingpong(sock, TEST_PASS, BUF_SIZE3) < 0)
        goto err;

    if (pingpong(sock, TEST_PASS, BUF_SIZE4) < 0)
        goto err;

    if (pingpong(sock, 1, 1) < 0)
        goto err;
    
    if (close(sock) < 0)
        l_perr("close");

    return 0;
 
 err:
    if (close(sock) < 0)
        l_perr("close");
    
    return -1;

}
