#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/linkage.h>
#include <linux/syscalls.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("0.0.1");

#define MAX_MYCOM_CLIENTS 8

struct mycom_clients_s {
    int used;
    int from;

    void *buf;
    size_t capacity_, size_;

    wait_queue_head_t wq;
};

static struct mycom_clients_s mycom_clients[MAX_MYCOM_CLIENTS];

/* @todo: rename: connect -> get */
SYSCALL_DEFINE1(mycom_get, size_t, size)
{
    int i, con_idx;
    printk("mycom_get\n");

    con_idx = -1;

    for (i = 0; i < MAX_MYCOM_CLIENTS; i++) {
        if (!mycom_clients[i].used) { 
            con_idx = i;
            break;
        }
    }

    printk("mycom_get: get con_idx == %d\n", con_idx);

    if (-1 == con_idx) {
        printk("mycom_get: max clients!!\n");
        return -1;
    }

    mycom_clients[con_idx].buf = kzalloc(size, GFP_KERNEL);
    if (NULL == mycom_clients[con_idx].buf) {
        printk("ERR: can't alloc mememory");
        return -1;
    }

    init_waitqueue_head(&mycom_clients[con_idx].wq);

    mycom_clients[con_idx].capacity_ = size;
    mycom_clients[con_idx].size_ = 0;
	mycom_clients[con_idx].used = 1;
	mycom_clients[con_idx].from = -1;

    return con_idx;
}

SYSCALL_DEFINE1(mycom_destroy, int, client)
{
    printk("mycom_destroy\n");

    if (mycom_clients[client].used) {
        kfree(mycom_clients[client].buf);
        mycom_clients[client].buf = NULL; 
        mycom_clients[client].capacity_ = 0;
        mycom_clients[client].size_ = 0;
        mycom_clients[client].used = 0;
        mycom_clients[client].from = -1;
    }

    return 0;
}

SYSCALL_DEFINE4(mycom_send, int, client, int, to, const char __user *, buf, size_t, size)
{
    unsigned long ret;

    printk("mycom send\n");

    if (client < 0 || MAX_MYCOM_CLIENTS <= client) {
        printk("mycom_send: client == %d\n", client);
        return -1;
    }

    if (to < 0 || MAX_MYCOM_CLIENTS <= client) {
        printk("mycom_send: to == %d\n", to);
        return -1;
    }

    if (!mycom_clients[client].used) {
        printk("mycom_send: [client].used == %d\n",
               mycom_clients[client].used);
        return -1;
    }

    if (!mycom_clients[to].used) {
        printk("mycom_send:[to].used == %d\n",
               mycom_clients[to].used);
        return -1;
    }

    if (mycom_clients[to].size_) {
        printk("mycom_send:[to] busy\n");
        return 0;
    }

    if (mycom_clients[to].capacity_ < size) {
        printk("mycom_send:[to].capacity %ld; size %ld\n", 
               mycom_clients[to].capacity_, size);
        return -1;
    }

    ret = copy_from_user(mycom_clients[to].buf, buf, size);
    if (0 != ret) {
        printk("mycom_send: copy_from_user failed\n");
        return -1;
    }

    mycom_clients[to].size_ = size;
    mycom_clients[to].from = client;

    wake_up(&mycom_clients[to].wq);

    printk("mycom_send: send %ld byte\n", size);

    return (long)size;
}

SYSCALL_DEFINE4(mycom_recv, int, client, int, from, char __user *, buf, size_t, size)
{
    unsigned long ret;

    printk("mycom recv\n");

    if (from < 0 || MAX_MYCOM_CLIENTS <= from) {
        printk("mycom_recv: from == %d\n", from);
        return -1;
    }

    if (client < 0 || MAX_MYCOM_CLIENTS <= client) {
        printk("mycom_recv: client == %d\n", client);
        return -1;
    }

    if (!mycom_clients[client].used) {
        printk("mycom_recv: [client].used == %d\n",
               mycom_clients[client].used);
        return -1;
    }

    if (!mycom_clients[from].used) {
        printk("mycom_recv: [client].used == %d\n",
               mycom_clients[client].used);
    }

    printk("mycom_recv: wait data\n");

    if (wait_event_interruptible((mycom_clients[client].wq), 
                                 (mycom_clients[client].from != -1)) < 0) {
        printk("mycom_recv: wait interrupted\n");
        return -1;
    }

    if (mycom_clients[client].from != from) {
        /* discard data from unexpected sender */
        printk("mycom_recv: unexpected sender\n");
        mycom_clients[client].from = -1;
        mycom_clients[client].size_ = 0;
        return -1;
    }

    if (size < mycom_clients[client].size_) {
        printk("mycom_recv: user buffer too small\n");
        return -1;
    }

    size = mycom_clients[client].size_;
    ret = copy_to_user(buf, mycom_clients[client].buf, size);
    mycom_clients[client].from = -1;
    mycom_clients[client].size_ = 0;

    return (long)((0 == ret) ? size : -1);
}

/*
SYSCALL_DEFINE3(mycom_wait, int, client, int, from, long, msec)
{
    printk("mycom wait\n");

    if (0 > client || client > MAX_MYCOM_CLIENTS) {
	printk("mycom_wait: client == %d\n", client);
        return -1;
    }

    if (0 > from || from > MAX_MYCOM_CLIENTS) {
        printk("mycom_wait: from == %d\n", from);
        return -1;
    }
    
    if (0 == mycom_clients[client].used) {
        printk("mycom_wait: client.used == %d\n", mycom_clients[client].used);
        return -1;
    }

    if (wait_event_interruptible_timeout((mycom_clients[client].wq), 
                           (mycom_clients[client].recv != -1),
                           msecs_to_jiffies(msec)) < 0) {
	printk("mycom_wait: timeout\n");
        return 0;
    }

    if (mycom_clients[client].recv != from) {
        mycom_clients[client].recv = 0;
	printk("mycom_wait: recv != from\n");
        return 0;
    }

    return mycom_clients[client].recv;
}
*/
