#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include "../msgqueue_ioctl.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jaswanthi");
MODULE_DESCRIPTION("Linux character device message queue");
struct msgqueue_dev {
    struct cdev cdev;
    struct mutex lock;
    wait_queue_head_t readq;
    wait_queue_head_t writeq;
    
    char *buffer;
    dev_t devno;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
};


static int msgqueue_open(struct inode *inode, struct file *file);
static int msgqueue_release(struct inode *inode, struct file *file);
static ssize_t msgqueue_write(struct file *file, 
                              const char __user *user_buf,
                              size_t len, 
                              loff_t *offset                                
);
static ssize_t msgqueue_read(struct file *file,
                            char __user *user_buf,
                            size_t len,
                            loff_t *offset
);

static __poll_t msgqueue_poll(struct file *file, poll_table *wait);

static long msgqueue_ioctl(struct file *file, unsigned int cmd, unsigned long arg);


static const struct file_operations msgqueue_fops={
    .owner=THIS_MODULE,
    .open= msgqueue_open,
    .release= msgqueue_release,
    .write= msgqueue_write,
    .read= msgqueue_read,
    .poll= msgqueue_poll,
    .unlocked_ioctl= msgqueue_ioctl,
};
static struct msgqueue_dev msgdev;

static int msgqueue_open(struct inode *inode, struct file *file){

    (void)inode;
    (void)file;
    pr_info("msgqueue: file has been opened\n");
    return 0;
}

static int msgqueue_release(struct inode *inode, struct file *file){
    pr_info("msgqueue: file has been closed\n");
    return 0;

}

static ssize_t msgqueue_write(struct file *file,
                              const char __user *user_buf,
                              size_t len,
                              loff_t *offset                              
){
    
    size_t available;
    size_t bytes_to_write;
    size_t first_part;
    size_t second_part;
    size_t copied_first;
    size_t copied_second;
    size_t total_copied;
    unsigned long not_copied_first;
    unsigned long not_copied_second;
    int ret;

    if(len ==0){
        return 0;
    }

    for(;;){
        mutex_lock(&msgdev.lock);
        if(msgdev.count < msgdev.capacity){
            break;
        }
        if(file -> f_flags & O_NONBLOCK){
            mutex_unlock(&msgdev.lock);
            return -EAGAIN;
        }

        mutex_unlock(&msgdev.lock);
        ret= wait_event_interruptible(msgdev.writeq, 
                                      READ_ONCE(msgdev.count )< msgdev.capacity
                                        );
        if(ret < 0){
            return ret;
        }
    }
    available= msgdev.capacity - msgdev.count;

    bytes_to_write= min(len, available);

    first_part= min(bytes_to_write, msgdev.capacity - msgdev.head);
    second_part= bytes_to_write- first_part;
    
    not_copied_first=copy_from_user(msgdev.buffer+msgdev.head,
                    user_buf,
                    first_part);

    copied_first= first_part - not_copied_first;

    

    copied_second = 0;
    total_copied = copied_first;
    if(not_copied_first == 0 && second_part > 0){
        not_copied_second=copy_from_user(msgdev.buffer,
                      user_buf + first_part, 
                      second_part);
        copied_second = second_part - not_copied_second;
        total_copied+=copied_second;
    }
    
    if(total_copied == 0){
        mutex_unlock(&msgdev.lock);
        return -EFAULT;
    }
    msgdev.head = (msgdev.head + total_copied) % msgdev.capacity;
    msgdev.count = msgdev.count + total_copied;
    

    mutex_unlock(&msgdev.lock);
    wake_up_interruptible(&msgdev.readq);

    return total_copied;


}

static ssize_t msgqueue_read(struct file *file,
                            char __user *user_buf,
                            size_t len,
                            loff_t *offset
){
    size_t bytes_to_read;
    size_t first_part;
    size_t second_part;
    size_t copied_first;
    size_t copied_second;
    size_t total_copied;
    unsigned long not_copied_first;
    unsigned long not_copied_second;
    int ret;

    if(len ==0){
        return 0;
    }

   
    for(;;){
        mutex_lock(&msgdev.lock);
        if(msgdev.count > 0){
            break;
        }
        if(file-> f_flags & O_NONBLOCK){
            mutex_unlock(&msgdev.lock);
            return -EAGAIN;
        }
        mutex_unlock(&msgdev.lock);
        ret= wait_event_interruptible(msgdev.readq, READ_ONCE(msgdev.count) >0);
        if(ret < 0){
            return ret;
        }

        
    }
    
    bytes_to_read= min(msgdev.count , len);
    first_part= min(bytes_to_read, msgdev.capacity - msgdev.tail);
    second_part= bytes_to_read - first_part;

    not_copied_first = copy_to_user(user_buf,
                    msgdev.buffer+msgdev.tail,
                    first_part);
    copied_first = first_part - not_copied_first;

    copied_second =0;
    total_copied= copied_first;

    if(not_copied_first == 0 && second_part > 0){
        not_copied_second = copy_to_user(user_buf+ first_part,
                    msgdev.buffer,
                    second_part
        );
        copied_second = second_part - not_copied_second;
        total_copied+= copied_second;
    }
    
    if(total_copied == 0){
        mutex_unlock(&msgdev.lock);
        return -EFAULT;
    }

    msgdev.tail= (msgdev.tail+ total_copied) % msgdev.capacity;
    msgdev.count= msgdev.count - total_copied;


    mutex_unlock(&msgdev.lock);
    wake_up_interruptible(&msgdev.writeq);

    return total_copied;
}

static __poll_t msgqueue_poll(struct file *file, poll_table *wait){
    __poll_t mask=0;
    
    poll_wait(file, &msgdev.readq, wait);
    poll_wait(file, &msgdev.writeq, wait);

    mutex_lock(&msgdev.lock);

    if(msgdev.count > 0){
        mask |= POLLIN | POLLRDNORM;

    }
    if(msgdev.count < msgdev.capacity){
        mask |= POLLOUT | POLLWRNORM;
    }

    mutex_unlock(&msgdev.lock);

    return mask;
}

static long msgqueue_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
    switch(cmd){
        case MSGQUEUE_GET_COUNT:{
            size_t current_count;
            mutex_lock(&msgdev.lock);
            current_count = msgdev.count;
            mutex_unlock(&msgdev.lock);

            if(copy_to_user((void __user *)arg, &current_count, sizeof(current_count))){
                return -EFAULT;
            }

            break;
        }
        case MSGQUEUE_CLEAR:{
            mutex_lock(&msgdev.lock);
            msgdev.head=0;
            msgdev.tail=0;
            msgdev.count=0;
            mutex_unlock(&msgdev.lock);

            wake_up_interruptible(&msgdev.writeq);

            break;
        }

        default:
            return -ENOTTY; 
    }
    return 0;
}

static int __init msgqueue_init(void){
    
    int ret;
    ret= alloc_chrdev_region(&msgdev.devno, 0,  1, "msgqueue");
    
    if(ret < 0){
        pr_err("msgqueue: unable to allocate device number\n");
        return ret;
    }
    pr_info("device registered: major=%d minor=%d\n",
        MAJOR(msgdev.devno),
        MINOR(msgdev.devno)
    );
    mutex_init(&msgdev.lock);
    init_waitqueue_head(&msgdev.readq);
    init_waitqueue_head(&msgdev.writeq);

    msgdev.capacity= 4096;
    msgdev.buffer= kmalloc(msgdev.capacity, GFP_KERNEL);
    if(!msgdev.buffer){
        pr_err("msgqueue: failed to allocate memory buffer\n");
        unregister_chrdev_region(msgdev.devno,1);
        return -ENOMEM;
    }
    msgdev.head=0;
    msgdev.tail=0;
    msgdev.count=0;
    cdev_init(&msgdev.cdev, &msgqueue_fops);


    ret =cdev_add(&msgdev.cdev, msgdev.devno, 1);
    if(ret != 0){
        kfree(msgdev.buffer);
        unregister_chrdev_region(msgdev.devno, 1);
        pr_err("msgqueue: failed to add cdev\n");
        return ret;
    }
    return 0;

}
static void __exit msgqueue_exit(void){
    cdev_del(&msgdev.cdev);
    kfree(msgdev.buffer);
    unregister_chrdev_region(msgdev.devno,1);
    pr_info("msgqueue: device unregistered\n");
}
module_init(msgqueue_init);
module_exit(msgqueue_exit);
