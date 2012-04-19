#ifndef __MYCOM_H__
# define __MYCOM_H__

# define __NR_mycom_get 337
# define __NR_mycom_destroy 338
# define __NR_mycom_send 339
# define __NR_mycom_recv 340

# define mycom_get(size)                        \
    syscall(__NR_mycom_get, size)

# define mycom_destroy(client)                  \
    syscall(__NR_mycom_destroy, client)

# define mycom_send(client, to, buf, size)          \
    syscall(__NR_mycom_send, client, to, buf, size)

# define mycom_recv(client, from, buf, size)            \
    syscall(__NR_mycom_recv, client, from, buf, size)

# define mycom_wait(client, from, msec)             \
    syscall(__NR_mycom_recv, client, from, msec)

#endif /* __MYCOM_H__ */
