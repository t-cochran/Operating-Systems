#include <linux/init.h>      /* module_init, module_exit */
#include <linux/module.h>    /* module documentation macros */
#include <linux/kernel.h>    /* printk */
#include <linux/types.h>     /* ssize_t, loff_t types */
#include <linux/unistd.h>    /* read, write, llseek */
#include <linux/fcntl.h>     /* open */
#include <linux/errno.h>     /* errno */
#include <linux/uaccess.h>   /* copy_to_user, copy_from_user */
#include <linux/fs.h>        /* filp, file_ops, register_chrdev */

/*
 *  Constants
 */
#define DEVICE_NAME    "simple_character_device"
#define BUF_SIZE       1024
#define MAJOR_NUMBER   200
#define MINOR_NUMBER   0

/*
 *  Documentation
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Thomas Cochran");
MODULE_DESCRIPTION("driver for /dev/simple_character_device");

/*
 *  Function prototypes
 */
int driver_init(void);
void driver_exit(void);
int my_open(struct inode* inode, struct file* filp);
int my_release(struct inode* inode, struct file* filp);
ssize_t my_read(struct file* filp, char * usr_buf, size_t count, loff_t* offset);
ssize_t my_write(struct file* filp, const char* usr_buf, size_t count, loff_t* offset);
loff_t my_llseek(struct file* filp, loff_t offset, int whence);

/*
 *  Initialize Global Variables
 */
static char device_buf[BUF_SIZE];
static int open_count = 0;

/*
 *  Initialize file operations members
 */
struct file_operations file_ops = {
    .owner = THIS_MODULE,
    .llseek = my_llseek,
    .read = my_read,
    .write = my_write,
    .open = my_open,
    .release = my_release
};

/*
 *  Register the device when this module is loaded
 */
int driver_init(void) {
    int ret;
    if ((ret = register_chrdev(MAJOR_NUMBER, DEVICE_NAME, &file_ops)) == 0) {
        printk(KERN_NOTICE "[Success] %s [Maj:%d] Registration Successful!\n",
               DEVICE_NAME, MAJOR_NUMBER);
        return 0;
    }
    printk(KERN_ALERT "[Error] Cannot register driver.\n");
    return ret;
}

/*
 *  Invoked when the device file is opened.
 */
int my_open(struct inode* inode, struct file* filp) {
    open_count += 1;
    printk("----------------------------------------------------------\n");
    printk(KERN_NOTICE "[OPENED]: %s %d time(s). \n", DEVICE_NAME, open_count);
    return 0;
}

/*
 *  Invoked when the device file is closed.
 */
int my_release(struct inode* inode, struct file* filp) {
    open_count -= 1;
    printk(KERN_NOTICE "[CLOSED]: %s.\n", DEVICE_NAME);
    printk("----------------------------------------------------------\n");
    return 0;
}

/*
 *  Read data from the device (in kernel space) to user space.
 *      Returns: (Success)  The number of bytes successfully read from the device
 *                 (Error)  -EINVAL  (error code: invalid argument)
 */
ssize_t my_read(struct file* filp, char* usr_buf, size_t count, loff_t* offset) {

    size_t a_count, bytes_copied;
    ssize_t bytes_remain;

    /* Track the remaining buffer if the device remains open() */
    size_t buf_remain = BUF_SIZE - *offset;

    /* Handle requests to read when end of file has been reached */
    if (buf_remain == 0) {
        printk(KERN_ALERT "Reached EOF. Returning 0 bytes read. \n");
        return 0;
    }

    /* Ensure the requested read size is within the device buffer maximum */
    if (count <= buf_remain) {
        a_count = count;
    }
    else {
        a_count = buf_remain;
    }
    printk(KERN_ALERT "%ld bytes requested. Reading %ld bytes. \n",
           count, a_count);

    /* copy_to_user: Returns the number of bytes that could not be copied */
    if ((bytes_remain = copy_to_user(usr_buf, device_buf + *offset, a_count)) < 0) {
        printk(KERN_ALERT "copy_to_user error \n");
        return -EINVAL;
    }

    /* Compute the actual number of bytes copied */
    bytes_copied = a_count - bytes_remain;

    /* Update the file position for read */
    *offset += bytes_copied;

    /* Return the amount of bytes read */
    printk("Read %ld bytes from the device.\n", bytes_copied);
    return bytes_copied;
}

/*
 *  Write data from user space to the device buffer in kernel space.
 *      Returns: (Success)  The number of bytes successfully written to the device
 *                 (Error)  -ENOBUFS (error code: no buffer space available)
 *                 (Error)  -EINVAL  (error code: invalid argument)
 */
ssize_t my_write(struct file* filp, const char* usr_buf, size_t count, loff_t* offset) {

    size_t a_count, bytes_copied;
    ssize_t bytes_remain;

    /* If the device remains open(), track the remaining bytes in the device buffer */
    size_t buf_remain = BUF_SIZE - *offset;

    /* Handle requests to write when the device buffer is full */
    if (buf_remain == 0) {
        printk(KERN_ALERT "Cannot write. Device buffer is full.\n");
        return -ENOBUFS;
    }

    /* Ensure the requested read size is within the device buffer maximum */
    if (count <= buf_remain) {
        a_count = count;
    }
    else {
        a_count = buf_remain;
    }
    printk(KERN_ALERT "%ld bytes requested. Writing %ld bytes. \n", count, a_count);

    /* copy_from_user: Returns the number of bytes that could not be copied */
    if ((bytes_remain = copy_from_user(device_buf + *offset, usr_buf, a_count)) < 0) {
        printk(KERN_ALERT "copy_from_user error \n");
        return -EINVAL;
    }

    /* Compute the actual number of bytes copied */
    bytes_copied = a_count - bytes_remain;

    /* Update the file position for write */
    *offset += bytes_copied;

    /* Return the amount of bytes written */
    printk("Wrote %ld bytes to the device.\n", bytes_copied);
    return bytes_copied;
}

/*
 *  Change the current the read/write file position.
 *      Returns: (Success)  The new file position
 *                 (Error)  -EINVAL (error code: invalid argument)
 */
loff_t my_llseek(struct file* filp, loff_t offset, int whence) {

    loff_t seek_offset;

    /* Handle each starting position */
    switch(whence) {
        /* SEEK_SET: Seek from the beginning of file */
        case SEEK_SET:
            seek_offset = offset;
            break;
        /* SEEK_SET: Seek from the current file position */
        case SEEK_CUR:
            seek_offset = filp -> f_pos + offset;
            break;
        /* SEEK_END: Seek from the end of file */
        case SEEK_END:
            seek_offset = BUF_SIZE + offset;
            break;
        /* Error */
        default:
            printk(KERN_ALERT "Invalid argument.\n");
            return -EINVAL;
    }
    printk("seek_offset is: %lld\n", seek_offset);

    /* Handle cases when the new position out of buffer bounds */
    if (seek_offset > BUF_SIZE) {
        seek_offset = BUF_SIZE;
    }
    if (seek_offset < 0) {
        seek_offset = 0;
    }

    /* Set the new file offset position in the file structure */
    filp -> f_pos = seek_offset;

    /* Return the adjusted file offset in bytes */
    printk("llseek to %lld bytes.\n", seek_offset);
    return seek_offset;
}

/*
 *  Unregisters the device when module this module is removed
 */
void driver_exit(void) {
    /* Remove device registration only if the device file is closed */
    if (open_count == 0) {
        unregister_chrdev(MAJOR_NUMBER, DEVICE_NAME);
        printk(KERN_NOTICE "[Success] %s [Maj:%d] Registration Removed.\n",
               DEVICE_NAME, MAJOR_NUMBER);
    }
    else {
        printk(KERN_ALERT "[Error] Cannot unregister the device while /dev/%s is open.",
               DEVICE_NAME);
    }
}

/*
 *  Macros called at module insertion and removal time to setup
 *  the entry point for this module and call init or exit routines.
 */
module_init(driver_init);
module_exit(driver_exit);
