####################################################
#              Programming Assignment 5            #
#                  Thomas Cochran                  #
#                    CSPB 3753                     #
####################################################

********
 Files:
********

client.c
    Code for the client program

server.c
    Code for the server program

helpers.c
    Code for helper functions used by server.c and client.c

headerPA5.h
    Header file used by client.c, server.c, helpers.c 
    Contains include directives, definitions and function prototypes.

TCP_VM_results.png
    A screenshot of the client and server programs each running on a separate
    virtual machine using using TCP to pass data. The left VM is the client, 
    the right VM is the server.

UDP_VM_results.png
    A screenshot of the client and server programs each running on a separate
    virtual machine using using UDP to pass data. The left VM is the client, 
    the right VM is the server.

Makefile
    Builds client and server programs.


*****************************
 Build client.c and server.c
*****************************
To build the programs, type "make" in a bash terminal in file directory containing client.c and server.c


*********************************************
 Where to run the client and server programs
*********************************************
Each program must be run in separate terminals or virtual machines:

    (1) 2x Terminals:
        - Find your local ip by entering: "hostname -i" in bash
        - Use telnet with your local IP and a random port number by entering: "telnet <hostname> <port>"
        - Use this local IP for your client program
        - Run both programs in separate terminals.
   
    (2) 2x Virtual Machines:
        - Outlined by the assignment but briefly: in VMware Workstation open two Ubuntu VMs
        - For both VMs: select virtual network editor -> host-only to begin creating a curstom network
        - Configure a subnet IP and mask if not already configured, then 'use local DHCP service'
        - Run the VMs and then download the client and server source code on both VMs 
        - For both VMs: Set network adapter to use the custom network
        - For both VMs: Open a terminal in this directory, enter'ip addr' to list assigned IP addresses
        - Select an IP from this range for the client program to use
        - Run the server program in one of the VMs, then run the client program in the other

See TCP_VM_results.png or UDP_VM_results.png to see the results on 2x virtual machines.

The server program will wait to receieve data from the client program. So, the server program
should be run before the client program.

************************
 Run the server program
************************
After building server.c, the server program must be run with command-line options and arguments:

Usage: [-OPTION] [VALUE]
Server program receives messages from a client.

        -t <tcp> or <udp>        protocol for incoming connections
        -p <number>              port number (1025 to 65535)
        
Example:

        ./server -t tcp -p 3224 

In this example, the server will run a TCP protocol and listen to port 3224.
These options and their associated arguments can be listed in any order.


************************
 Run the client program
************************
Each program must be run in separate terminals or virtual machines, and the server program
must be run before the client.

After building client.c, the client program must be run with command-line options and arguments:

Usage: [-OPTION] [VALUE]
Client program creates messages and sends them to a server.

        -x <data>                32-bit unsigned integer message
        -t <tcp> or <udp>        connection protocol
        -s <ip>                  IPv4 address or hostname
        -p <number>              port number (1025 to 65535)

Example:

        ./client -x 10101010 -t tcp -s 127.0.1.1 -p 3224

In this example, the client will run a TCP protocol, use 127.0.1.1:3224 IPv4 address and port, and
send the message '10101010' to the server. These options and their associated arguments can be listed 
in any order.


******************
 Makefile options
******************
Typing the following in a bash terminal in file directory containing client.c and server.c
will run each program given pre-made options and arguments

    (1) "make client" 
        Runs the client program UDP protocol, address 127.0.1.1:3105, data sent: '100'

    (2) "make server" 
        Runs the server program with UDP protocol listening to port 3015

To cleanup object files and .txt files before rebuilding, type "make clean" in a bash terminal.
