#include <sys/time.h>
#include <stdlib.h>

void
timediff(const struct timeval *tv1, const struct timeval *tv2, struct timeval *res)
{
    if (NULL == tv1 ||
        NULL == tv2 ||
        NULL == res)
        return;

    if (tv2->tv_sec == tv1->tv_sec) {
        res->tv_sec = 0;
        res->tv_usec = tv2->tv_usec - tv1->tv_usec;
    } else {
        if (tv2->tv_sec > tv1->tv_sec) {
            res->tv_sec = tv2->tv_sec - tv1->tv_sec - 1;
            res->tv_usec = (tv2->tv_usec + 1000000) - tv1->tv_usec;
        }
    }
}
