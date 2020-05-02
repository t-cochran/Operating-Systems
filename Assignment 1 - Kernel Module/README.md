###################################################
#
#            Programming Assignment 1
#             
#             Author: Thomas Cochran
#               January 30, 2020
#
###################################################

***********
Main Files
***********

my_driver.c
    The kernel module for /dev/simple_character_device.

tests.c
    The driver code that tests the kernel module.

module_install.sh
    Shell script that creates the character device /dev/simple_character_device.
    Sets permissions for the character device and installs the kernel module.

module_remove.sh
    Shell script that removes the kernel module.

Makefile
    Builds and installs the kernel module.
    Builds and runs the test code for the kernel module.


*****************************
Installing the kernel module
*****************************

Build the module:
    Type 'make' in the shell

Create the character device and install the module:
    Type 'make install' in the shell


*******************************
Performing tests on the module
*******************************

Setup the kernel log to be read:
    Type 'dmesg -wH' in the shell

In a new shell window, select one of the following test options. 
    
    Type 'make <option>' in the shell:

	make test1:    Test a single write then a single read.
	make test2:    Test a double write then a read.
	make grid:     Write a 40x10 grid of characters to the device.
	make seek1:    Test lseek with SEEK_CUR.
    	make seek2:    Test lseek with SEEK_CUR.
      	make seek3:    Test lseek with SEEK_END.
    	make seekall:  Run seek1, seek2, and seek3 tests in sequence.
	make fork:     Fork() and then have each process lseek and write.
    	make extreme:  Test writing and seeking beyond the device buffer.

It is helpful to 'make grid' before each seek test (seek1, seek2, seek3, fork) 
since the grid makes it easier to view how each lseek test is operating.

****************************
Uninstall the kernel module
****************************

To uninstall the kernel module, type 'make remove' in the shell.


*******
Cleanup
*******

To remove object files, type 'make clean' in the shell.

