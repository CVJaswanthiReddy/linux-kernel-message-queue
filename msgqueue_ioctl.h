#ifndef MSGQUEUE_IOCTL_H
#define MSGQUEUE_IOCTL_H

#include <linux/ioctl.h>

#define MSGQUEUE_MAGIC 'M'

#define MSGQUEUE_GET_COUNT \
    _IOR(MSGQUEUE_MAGIC, 1, size_t)

#define MSGQUEUE_CLEAR _IO(MSGQUEUE_MAGIC, 2)

#endif
