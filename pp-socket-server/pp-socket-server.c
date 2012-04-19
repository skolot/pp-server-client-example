#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sysexits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <signal.h>

#include "logging.h"
#include "def.h"

/* func decl */

int
bind();

int
routine();

/* func release */

int
main(int argc, char **argv)
{
    struct sigaction sa;

    start_logging("pp-socket-server");

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
un_bind()
{
    struct sockaddr_un saun;
    int sock;

    memset(&saun, 0, sizeof(struct sockaddr_un));

    saun.sun_family = AF_UNIX;
    strncpy(saun.sun_path, SOCKPATH, sizeof(saun.sun_path));
    saun.sun_path[sizeof(saun.sun_path) - 0] = '\0';

    if ((sock = socket(PF_LOCAL, SOCK_STREAM, 0)) < 0) {
        l_perr("socket");
        return -1;
    }

    if (bind(sock, (struct sockaddr *)&saun, sizeof(struct sockaddr_un)) < 0) {
        l_perr("bind");

        if (close(sock) < 0)
            l_perr("close");
        return -1;
    }

    if (listen(sock, 5) < 0) {
        l_perr("listen");
        
        if (close(sock) < 0)
            l_perr("close");

        return -1;
    }

    return sock;
}

int
pingpong(int csock)
{
    char buf[BUF_SIZE_MAX] = {0};
    ssize_t srecv, ssend;

    while (1) {
        if ((srecv = recv(csock, buf, BUF_SIZE_MAX, 0)) < 0) {
            l_perr("recv");
            return -1;
        }

        if ((ssend = send(csock, buf, srecv, 0)) < 0) {
            l_perr("send");
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

    return 0;
}

int
routine()
{
    fd_set rfds;
    int sock;
    int csock;
    int ret;
    struct timeval tv;
    pid_t cpid;
    int status;

    if ((sock = un_bind()) < 0) {
        l_err("can't bind socket '%s'", SOCKPATH);
        return -1;
    } 

    memset(&tv, 0, sizeof(struct timeval));
    
    tv.tv_sec = 0;
    tv.tv_usec = 100;

    do {
        FD_ZERO(&rfds);
        FD_SET(sock, &rfds);
        
        if ((ret = select(sock + 1, &rfds, NULL, NULL, &tv)) < 0) {
            l_perr("select");
            break; // XXX: goto err
        }
        
        if (ret) {
            if (FD_ISSET(sock, &rfds)) {
                if ((csock = accept(sock, NULL, NULL)) < 0) {
                    l_perr("accept");
                    
                    if (close(sock) < 0)
                        l_perr("close");
                    
                    return -1;
                }

                l_debug("accpet");

                if ((cpid = fork()) < 0) {
                    l_perr("fork");
                    if (close(sock) < 0)
                        l_perr("close");
                    if (close(sock) < 0)
                        l_perr("close");
                    return -1;
                } else {
                    if (0 == cpid) {
                        if (pingpong(csock) < 0) {
                            l_err("can't ping or pong ):");
                            
                            if (close(sock) < 0)
                                l_perr("close");
                            
                            if (close(csock) < 0)
                                l_perr("close");
                            
                            return -1;
                        }
                    }
                }
                
                if (close(csock) < 0)
                    l_perr("close");
            }
        }
    } while (1);
    
    if (close(sock) < 0)
        l_perr("close");

    unlink(SOCKPATH);

    return 0;
}


