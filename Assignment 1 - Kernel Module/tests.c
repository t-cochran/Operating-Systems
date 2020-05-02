#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

/*
 *  Constants
 */
#define DEVICE_FILE     "/dev/simple_character_device"
#define MAJOR_NUM       200
#define BUF_SIZE        1024

/*
 * Macros
 */
static inline void fail(void) {
    printf("\033[1m\033[31m" "[ ERROR ]\t" "\033[0m");
}
static inline void pass(void) {
    printf("\033[1m\033[32m" "[ OK ]\t\t" "\033[0m");
}

/*
 *  Global variables
 */
char usr_buf[BUF_SIZE];

/*
 *  Test code
 */
int main(int argc, char* argv[]) {


    int fd;
    char *msg, *msg2;
    loff_t seek_offset;
    ssize_t bytes_read, bytes_write, bytes_write2, size, size2;



    /** Access the device from user space *********************************************************/
    if (*argv[1]) {
    /* Check if user space code can access the device */
    if (access(DEVICE_FILE, F_OK) == -1) {
        printf("\n");fail(); printf("Test code cannot access %s.\n\n", DEVICE_FILE);
        _exit(1);
    }
    printf("\n"); pass(); printf("Test code accessed device \"%s\"\n\n", DEVICE_FILE); }



    /** TEST 1: Write/Read the device *************************************************************/
    if(*argv[1] == '1') {

    /* Write to the device */
    fd = open(DEVICE_FILE, O_WRONLY);
    msg = "1 2 3 4 5 6 7 8 9 10\n";
    size = strlen(msg);
    if ((bytes_write = write(fd, msg, size)) == size) {
        pass(); printf("Wrote %ld bytes to \"%s\"\n", size, DEVICE_FILE);
        printf("\t\tW: %s\n", msg);
    }
    else {
        fail(); printf("Failed to write %ld bytes to \"%s\"\n", size, DEVICE_FILE);
    }
    close(fd);

    /* Read from the device */
    fd = open(DEVICE_FILE, O_RDONLY);
    if ((bytes_read = read(fd, usr_buf, size)) >= 0) {
        pass(); printf("Read %ld bytes to \"%s\"\n", size, DEVICE_FILE);
        printf("\t\tR: %s\n", usr_buf);
    }
    else {
        fail(); printf("Failed to read %ld bytes to \"%s\".\n", size, DEVICE_FILE);
    }
    close(fd);

    /* Compare the string written to the device with the string read from the device */
    if (strcmp(msg, usr_buf) == 0) {
        pass(); printf("The read is equivalent to the write.\n\n");
    }
    else {
        fail(); printf("Write is not equivalent to read\n Write(%s) Read(%s)\n\n", msg, usr_buf);
    } }



    /** TEST 2: Write/Write/Read the device *******************************************************/

    if(*argv[1] == '2') {

    /* Open the device */
    fd = open(DEVICE_FILE, O_WRONLY);

    /* Write to the device */
    msg = "stepping all over your TEST 1 write!";
    size = strlen(msg);
    if ((bytes_write = write(fd, msg, size)) == size) {
        pass(); printf("Wrote %ld bytes to \"%s\"\n", size, DEVICE_FILE);
        printf("\t\tW: %s\n\n", msg);
    }
    else {
        fail(); printf("Failed to write %ld bytes to \"%s\"\n", size, DEVICE_FILE);
    }

    /* Write to the device again */
    msg2 = " <-- that's mean!\n";
    size2 = strlen(msg2);
    if ((bytes_write2 = write(fd, msg2, size2)) == size2) {
        pass(); printf("Wrote %ld bytes to \"%s\"\n", size2, DEVICE_FILE);
        printf("\t\tW: %s\n\n", msg2);
    }
    else {
        fail(); printf("Failed to write %ld bytes to \"%s\"\n", size, DEVICE_FILE);
    }
    close(fd);

    /* Read from the device */
    fd = open(DEVICE_FILE, O_RDONLY);
    size = strlen(msg) + strlen(msg2);
    if ((bytes_read = read(fd, usr_buf, size)) >= 0) {
        pass(); printf("Read %ld bytes to \"%s\"\n", size, DEVICE_FILE);
    }
    else {
        fail(); printf("Failed to read %ld bytes to \"%s\".\n", size, DEVICE_FILE);
    }
    close(fd);

    /* Compare the length of the two writes */
    if (strlen(usr_buf) == 54) {
        pass(); printf("Two writes with no close extended the file offset. \n");
        printf("\t\tR: %s \n", usr_buf);
    }
    else {
        fail(); printf("The two writes did not create a 54 byte message\n");
    } }



    /** TEST 3: Write a grid for lseek tests *******************************************************/

    if(*argv[1] == '3') {

    /* Open the device */
    fd = open(DEVICE_FILE, O_WRONLY);

    /* Write 400 bytes worth of zeros in a 10x40 grid */
    msg = "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n";
    size = strlen(msg);
    bytes_read = 0;
    for (int i=0; i < 10; i++) {
        bytes_read += write(fd, msg, size);
    }
    if (bytes_read == 400) {
        pass(); printf("Wrote 400 bytes to the device. \n\n");
    }
    else {
        fail(); printf("failed to write 400 bytes\n");
    }
    close(fd); }



    /** TEST 4: Seek to the end of the grid with lseek() and SEEK_SET ******************************/

    if(*argv[1] == '4') {

    /*  Open the device */
    fd = open(DEVICE_FILE, O_RDWR);

    /* Invoke lseek() with SEEK_SET to reach the end of the grid. */
    msg = " THE END\n";
    if ((seek_offset = lseek(fd, 391, SEEK_SET)) == -1) {
        fail(); printf("could not lseek\n");
    }
    pass(); printf("Called lseek() to move %ld bytes with SEEK_SET selected.\n\n", seek_offset);

    /* Write to the device */
    size = strlen(msg);
    if ((bytes_write = write(fd, msg, size)) == size) {
        pass(); printf("Wrote %ld bytes to \"%s\"\n", size, DEVICE_FILE);
        printf("\t\tW: %s\n", msg);
    }
    else {
        fail(); printf("Failed to write %ld bytes to \"%s\"\n", size, DEVICE_FILE);
    }
    close(fd); }



    /** TEST 5: Seek to the middle of the grid with lseek() and SEEK_CUR ***************************/

    if(*argv[1] == '5') {

    /* Open device */
    fd = open(DEVICE_FILE, O_RDWR);

    /* Write to the device 4 rows of zeros */
    msg = "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n";
    size = strlen(msg);
    bytes_write = 0;
    for (int i=0; i < 5; i++) {
        bytes_write += write(fd, msg, size);
    }
    if (bytes_write == 200) {
        pass(); printf("Wrote 200 bytes to the device. \n\n");
    }
    else {
        fail(); printf("failed to write 200 bytes\n");
    }

    /* Seek 18 bytes from the current write position with SEEK_CUR */
    if ((seek_offset = lseek(fd, 13, SEEK_CUR)) == -1) {
        fail(); printf("could not lseek\n");
    }
    pass(); printf("Called lseek() to move %ld bytes with SEEK_CUR selected.\n\n", seek_offset);

    /* Write to the device from the new lseek location */
    bytes_write = 0;
    msg = " THE MIDDLE ";
    size = strlen(msg);
    if ((bytes_write = write(fd, msg, size)) == size) {
        pass(); printf("Wrote %ld bytes to \"%s\"\n", size, DEVICE_FILE);
        printf("\t\tW: %s\n\n", msg);
    }
    else {
        fail(); printf("Failed to write %ld bytes to \"%s\"\n\n", size, DEVICE_FILE);
    }
    close(fd); }



    /** TEST 6: Seek to the beginning of the grid with lseek() and SEEK_END *************************/
    if(*argv[1] == '6') {

    /* Open device */
    fd = open(DEVICE_FILE, O_RDWR);

    /* Seek backwards using a negative offset and SEEK_END */
    if ((seek_offset = lseek(fd, -1024, SEEK_END)) == -1) {
        fail(); printf("could not lseek\n");
    }
    printf("\t        NOTE: lseek() implementation sets negative offsets to 0.\n");
    pass(); printf("Called lseek() to move %ld bytes with SEEK_END selected.\n\n", seek_offset);

    /* Write to the device from the new lseek location */
    bytes_write = 0;
    msg = "THE BEGINNING ";
    size = strlen(msg);
    if ((bytes_write = write(fd, msg, size)) == size) {
        pass(); printf("Wrote %ld bytes to \"%s\"\n", size, DEVICE_FILE);
        printf("\t\tW: %s\n\n", msg);
    }
    else {
        fail(); printf("Failed to write %ld bytes to \"%s\"\n\n", size, DEVICE_FILE);
    }
    close(fd); }



    /** TEST 7: Check some fringe cases ***********************************************************/
    if(*argv[1] == '7') {

    /* Write more characters than the 1024 byte device buffer maximum */
    fd = open(DEVICE_FILE, O_RDWR);
    msg = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n";
    size = strlen(msg);
    bytes_write = 0;
    pass(); printf("Attempting to overflow the buffer...\n\n");
    for (int i=0; i < 30; i++) {
        bytes_write += write(fd, msg, size);  // 30 x 40 bytes = 1200 bytes
    }
    close(fd);

    /* Seek in the negative direction with SEEK_SET and write */
    fd = open(DEVICE_FILE, O_RDWR);
    if ((seek_offset = lseek(fd, -1024, SEEK_SET)) == -1) {
        fail(); printf("could not lseek\n");
    }
    pass(); printf("Called lseek() to move %ld bytes with SEEK_SET selected.\n\n", seek_offset);
    close(fd);

    /* Seek in the positive direction with SEEK_END and write */
    fd = open(DEVICE_FILE, O_RDWR);
    if ((seek_offset = lseek(fd, 1024, SEEK_END)) == -1) {
        fail(); printf("could not lseek\n");
    }
    pass(); printf("Called lseek() to move %ld bytes with SEEK_END selected.\n\n", seek_offset);
    close(fd); }



    /** TEST 8: Share open file between a parent and child process *****************************/
    if(*argv[1] == '8') {
    pid_t pid;
    loff_t offset_child, offset_parent;

    /* Open the device */
    fd = open(DEVICE_FILE, O_RDWR);

    /* Fork a child process */
    if ((pid = fork()) == 0) {  /* Child process: should inherit parent open file descriptor */

        /* Seek with SEEK_SET */
        if ((offset_child = lseek(fd, 120, SEEK_SET)) == -1) {
            fail(); printf("Child process: could not lseek\n");
        }
        pass(); printf("Child process: lseek() %ld bytes with SEEK_SET selected.\n", offset_child);

        /* Write a message */
        msg = "I'm the child process.\nThe parent is waiting for me to exit.\n";
        size = strlen(msg);
        bytes_write = 0;
        if ((bytes_write = write(fd, msg, size)) == size) {
            pass(); printf("Child process: Wrote %ld bytes to \"%s\"\n", size, DEVICE_FILE);
        }
        else {
            fail(); printf("Child process: Failed to write %ld bytes to \"%s\"\n", size, DEVICE_FILE);
        }

        /* Print current file offset */
        pass(); printf("Child process: Current file offset is: %lld\n\n",
                       (long long)lseek(fd, 0, SEEK_CUR));

        /* Child process exits */
        exit(0);

    }
    else {  /* Parent process */

        /* Wait for the child process to exit */
        if ((wait(NULL)) == -1){
            fail(); printf("Parent process: Could not wait() for child to exit. \n");
        }
        pass(); printf("Parent process: The child process has completed. My turn!\n");

        /* Seek with SEEK_CUR */
        if ((offset_parent = lseek(fd, 99, SEEK_CUR)) == -1) {
            fail(); printf("Parent process: could not lseek\n");
        }
        pass(); printf("Parent process: lseek() %ld bytes with SEEK_CUR selected.\n", offset_parent);

        /* Write a message */
        msg = "This is the parent process speaking.\nWe are sharing an open file descriptor!\n";
        size = strlen(msg);
        bytes_write = 0;
        if ((bytes_write = write(fd, msg, size)) == size) {
            pass(); printf("Parent process: Wrote %ld bytes to \"%s\"\n\n", size, DEVICE_FILE);
        }
        else {
            fail(); printf("Parent process: Failed to write %ld bytes to \"%s\"\n\n", size, DEVICE_FILE);
        }

        /* Parent process exits */
        exit(0);
    }
    /* Close the file */
    close(fd); }

    return 0;
}
