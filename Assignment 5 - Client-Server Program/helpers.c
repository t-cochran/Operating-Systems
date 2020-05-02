/*  Programming assignment #5
 *  CSPB 3753 - Operating Systems
 *  Author: Thomas Cochran
 * 
 *  Helper functions used by client.c and server.c
 * 
 *  Some of the functions reference code from:
 *      Ref: Beej's Guide to Network Programming 2019
 *      Web: https://beej.us/guide/bgnet/
 */
#include "headerPA5.h"

/*
 *  parser
 * 
 *  Description:
 *    Command-line options are collected in any order by getopt()
 *    and are stored in the struct 'cmdline' where input can be
 *    accessed by the client or server program.
 * 
 *  Use:
 *    Called by the client or server program to collect input data.
 * 
 *  See README.md for the full command-line options available.
 */
struct cmdline parser(int argc, char* argv[], int client) {

    struct cmdline options;
    int opt = 0, numopts = 0;

    /* Fill the command-line options struct with getopt() */
    while ((opt = getopt(argc, argv, ": x: t: s: p:")) != -1) {

        switch(opt) {
            /* Data sent */
            case 'x' : 
                if (!client) { 
                    printf("\nERROR: %s: Unrecognized server option: '-%c'\n", 
                            argv[0], opt);
                    usageErrorMsg();
                } 
                if (checkInteger(optarg)) {
                    options.data = atoi(optarg);
                }
                break;

            /* Socket type */
            case 't' : 
                strncpy(options.socktype, optarg, 3);
                break;

            /* IPv4 address or hostname */
            case 's' : 
                if (!client) { 
                    printf("\nERROR: %s: Unrecognized server option: '-%c'\n", 
                            argv[0], opt);
                    usageErrorMsg();
                }
                // Check if a valid ipv4 network address is given
                if (inet_pton(AF_INET, optarg, &options.ipv4_address)) {
                    options.hostname = NULL;
                }
                // Otherwise assume the argument is a hostname
                else {
                    options.ipv4_address.s_addr = 0;
                    options.hostname = optarg;
                }
                break;

            /* Port number */
            case 'p' : 
                if (checkInteger(optarg)) {
                    options.port = optarg;
                }
                break;

            /* Error: An option has no argument */
            case ':' : 
                printf("\nERROR: %s: Missing argument after option: '-%c'\n", 
                        argv[0], optopt);
                usageErrorMsg();
                break;

            /* Error: An option is not recognized */
            case '?' : 
                printf("\nERROR: %s: Unrecognized option: '-%c'\n", 
                        argv[0], optopt);
                usageErrorMsg(); 
                break;

            /* Default: print usage options */
            default:
                usageErrorMsg();
        }
        numopts++;
    }

    /* Too many command-line arguments are selected */
    if ((numopts > 4 && client) || (numopts > 2 && !client)) {
        printf("\nERROR: %s: Too many options\n", argv[0]);
        usageErrorMsg();
    }

    /* Command-line arguments are missing */
    if (numopts < 4 && client) {
        printf("\nERROR: %s: Not enough options selected\n", argv[0]);
        usageErrorMsg();
    }

    /* A command-line argument is not given a option */
    if (optind < argc) {
        printf("\nERROR: %s: argument \"%s\" is not associated with an option\n", 
                argv[0], argv[optind]);
        usageErrorMsg();
    }

    /* Port number is not within range: 1024 to 65535 */
    if (!(atoi(options.port) > 1024 && atoi(options.port) <= 65535)) {
        printf("\nERROR: %s: port number \"%s\" is not allowed.\n", argv[0], options.port);
        printf("\tSelect a port number between: 1025 to 65535\n");
        usageErrorMsg();
    }
    /* The socket type is not 'udp' or 'tcp' */
    if (!(!strcmp(options.socktype, "udp") ^ !strcmp(options.socktype, "tcp"))) {
        printf("\nERROR: %s: socket type \"%s\" not allowed\n", argv[0], options.socktype);
        usageErrorMsg();
    }
    
    return options;
}

/*
 *  checkInteger
 * 
 *   Description:
 *     Checks whether an integer string pointed to by 'opt' will
 *     exceed the bounds of a unsigned 32-bit ingeter.
 * 
 *   Use:
 *     Called by 'parser' when checking command-line arguments
 *     Returns 1 if all is well, otherwise terminates and prints
 *     an error message.
 */
int checkInteger(char* opt) {

    long int check = strtol(opt, 0, 10);

    if (check == LONG_MIN && errno == ERANGE) {       // Check for underflow
        printf("\nERROR: data and port arguments must be a 32-bit unsigned integers\n");
        usageErrorMsg();
    }
    if (check == LONG_MAX && errno == ERANGE) {  // Check for overflow
        printf("\nERROR: data and port arguments must be a 32-bit unsigned integers\n");
        usageErrorMsg();
    }
    if (check > UINT_MAX || check < 0) {  // Check for u_int_32 max and negative
        printf("\nERROR: data and port arguments must be a 32-bit unsigned integers\n");
        usageErrorMsg();
    }
    // Good to go! The integer should be uint32_t (I hope..)
    return 1;
}

/*
 *  usageErrorMsg
 * 
 *   Description:
 *     Print the format for command-line options.
 *     Displays command-line options available to the client 
 *     or server program.
 * 
 *   Use:
 *     Called by the command-line parser and checkInteger functions 
 *     when an input error occurs.
 */
void usageErrorMsg() {
    printf("\nUsage: [-OPTION] [VALUE]\n");
    printf("Client program creates messages and sends them to a server.\n\n");
    printf("\t-x <data> \t\t 32-bit unsigned integer message\n");
    printf("\t-t <tcp> or <udp> \t server connection protocol\n");
    printf("\t-s <ip> \t\t IPv4 address of the server\n");
    printf("\t-p <number> \t\t port number used by the server\n\n");
    printf("Server program receives messages from a client.\n\n");
    printf("\t-t <tcp> or <udp> \t protocol for incoming connections\n");
    printf("\t-p <number> \t\t port number to listen for messages\n\n");
    exit(0);
}

/*  
 *  sendall
 * 
 *  Description:
 *    If send() does not return the full amount of bytes intended
 *    to be sent, then some bytes remain in a buffer within the 
 *    kernel. Remedy this by repeatedly calling send() until the 
 *    amount returned by send() is equivalent to the total amount 
 *    expected to be sent.
 *  
 *  Use:
 *    Called by the client and server programs when sending
 *    packets to each other via TCP
 * 
 *  SOURCE: Beej's Guide to Network Programming 2019 (page 52)
 */
int sendall(int s, char* buf, int* len) {

    int total = 0;          // Running total of bytes sent
    int bytesleft = *len;   // Bytes remaining to send
    int n = 0;

    while (total < *len) {
        if ((n = send(s, buf+total, bytesleft, 0)) == -1) {
            perror("send");
        }
        if (n == -1) break;
        total += n;
        bytesleft -= n;
    }

    *len = total; // len indicates total bytes actually sent
    return n == -1 ? -1 : 0;  // return -1 on failure, 0 on success
}

/*  
 *  init_sigchld_handler
 * 
 *  Description:
 *    Changes the default sigchld handler to reap zombie 
 *    processes
 *  
 *  Use:
 *    Called by the server program during initialization
 *    to ensure child processes serving client requests
 *    are reaped by the sigchld_handler.
 * 
 *  SOURCE: Beej's Guide to Network Programming 2019 
 *          (pages 31 to 32)
 */
void init_sigchld_handler(struct sigaction* sa) {

    sa -> sa_handler = sigchld_handler;
    sigemptyset(&sa -> sa_mask);
    sa -> sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, sa, NULL) == -1) {
        perror("[Server Program]: sigaction");
        exit(1);
    }
}

/*  
 *  init_hints
 * 
 *  Description:
 *    Specifies the criterea for selecting socket address 
 *    structs returned by getaddrinfo()
 *  
 *  Use:
 *    Called by the client and server programs during 
 *    initialization
 * 
 *  SOURCE: Beej's Guide to Network Programming 2019 
 *          (pages 19 to 20)
 */
void init_hints(struct addrinfo* hints, char* socktype, int client) {

    memset(hints, 0, sizeof(*hints));        // Empty the addrinfo struct
    hints -> ai_family = AF_INET;            // IPv4 socket addresses

    if (!client) {
        hints -> ai_flags = AI_PASSIVE;      // Server socket address flag
    }
    if (strncmp(socktype, "tcp", 3) == 0) {
        hints -> ai_socktype = SOCK_STREAM;  // TCP socket selected
    } 
    if (strncmp(socktype, "udp", 3) == 0) {
        hints -> ai_socktype = SOCK_DGRAM;   // UDP socket selected
    }
}

/*
 *  sigchld_handler
 * 
 *   Description:
 *     Reap zombie child processes created by the server program
 * 
 *   Use:
 *     The server program directs the default sigchld handler to
 *     this function so the zombie processes it creates are reaped.
 * 
 *   ADAPTED FROM: Beej's Guide to Network Programming 2019 
 *                 (pages 31)
 */
void sigchld_handler(UNUSED_PARAM int s) {

    int saved_errno, status;

    /* Save and restore errno */
    saved_errno = errno;

    /* Wait for a child to exit and check exit status */
    while(waitpid(-1, &status, WNOHANG | WUNTRACED) > 0) {
        
        /* Child terminated normally */
        if (WIFEXITED(status)) {
            printf("[Success] Server request completed.\n");
        }
        /* Child did not terminate normally */
        else {
            perror("[Error] Server request exit");
        }
    }
    errno = saved_errno;
}

/* 
 *  get_sock_ip
 * 
 *  Description:
 *     Get a pointer to a socket's IPv4 address by casting
 *     the argument struct sockaddr to the parallel struct
 *     sockaddr_in, and then accessing the sin_addr, the 
 *     socket's internet address.
 * 
 *  Use: 
 *     Called by the server and client programs to be passed
 *     to inet_ntop() so socket address's can be converted 
 *     from network byte order and then printed. 
 * 
 *  SOURCE: Beej's Guide to Network Programming 2019 
 *          (pages 33 to 35)
 */ 
void* get_sock_ip(struct sockaddr* socket_address_info) {
    return &(((struct sockaddr_in*)socket_address_info)->sin_addr);
}
