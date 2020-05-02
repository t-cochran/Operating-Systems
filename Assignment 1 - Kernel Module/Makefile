#
#  Makefile for my_driver kernel module
#
TEST_DRIVER = @./tests
LOAD_MODULE = @./module_install.sh
REMOVE_MODULE = @./module_remove.sh
READ_DEVICE = @cat /dev/simple_character_device
FILES = my_driver tests modules.order Module.symvers
HIDDEN = .cache.mk .my_driver.ko.cmd .my_driver.mod.o.cmd .my_driver.o.cmd

#  Build my_driver as a kernel module
obj-m += my_driver.o

#  Compile the kernel module and test code
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -Og -o tests tests.c

#  Create the device and install the kernel module
install:
	$(LOAD_MODULE)

#  Unintsall the kernel module
remove:
	$(REMOVE_MODULE)

#  Run test code by selecting a function to test.
#  See README.md for more information.
test1:
	$(TEST_DRIVER) 1
	$(READ_DEVICE)
test2:
	$(TEST_DRIVER) 2
	$(READ_DEVICE)
grid:
	$(TEST_DRIVER) 3
	$(READ_DEVICE)
seek1:
	$(TEST_DRIVER) 4
	$(READ_DEVICE)
seek2:
	$(TEST_DRIVER) 5
	$(READ_DEVICE)
seek3:
	$(TEST_DRIVER) 6
	$(READ_DEVICE)
seekall:
	$(TEST_DRIVER) 3
	$(TEST_DRIVER) 4
	$(TEST_DRIVER) 5
	$(TEST_DRIVER) 6
	$(READ_DEVICE)
extreme:
	$(TEST_DRIVER) 7
	$(READ_DEVICE)
fork:
	$(TEST_DRIVER) 8
	$(READ_DEVICE)

#  Remove C object files, kernel object and associated files
clean:
	rm -f $(PWD)/$(FILES) *.o *.ko *.mod.c *~
	rm -f $(HIDDEN)
	rm -r .tmp_versions
