#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/time.h>
#include <limits.h>

/* 
 *  Struct for collecting command-line input
 */
struct cmdline {
    uint32_t data;
    char socktype[3];
    struct in_addr ipv4_address;
    char* hostname;
    char* port;
};

/* 
 *  Struct for packing data messages sent by the client program.
 * 
 *  Why #pragma pack(1):
 * 
 *    Ensures the compiler does not add additional padding bytes to 
 *    achieve 4 byte alignment. I am using pack(1) so the struct is 
 *    packed 1 byte at a time. Here are the sizes with/without pragma:
 *        
 *        *No pragma pack(1):    sizeof(struct packet) = 8 bytes
 *        
 *           *pragma pack(1):    sizeof(struct packet) = 5 bytes
 * 
 *    With pragma pack(1) we achieve the intended 5-byte packet.
 */
#pragma pack(1)
struct packet {
    uint8_t version;
    uint32_t number;
};

/* 
 *  Attribute to notify compiler of unused parameters
 */
#define UNUSED_PARAM __attribute__((unused))

/* 
 *  Definitions
 */
#define BACKLOG         10      // Size of the pending connection queue
#define MAXDATASIZE     100     // Maximum number of bytes sent by client
#define SERVER          0
#define CLIENT          1

/*
 *  Function prototypes
 */
void sigchld_handler(int s);
int checkInteger(char* opt);
void* get_sock_ip(struct sockaddr* socket_address_info);
struct cmdline parser(int argc, char* argv[], int client);
void usageErrorMsg();
void init_sigchld_handler(struct sigaction* sa);
void init_hints(struct addrinfo* hints, char* socktype, int client);
int sendall(int s, char* buf, int* len);
struct packet init_packet(uint8_t ver, uint32_t num, struct packet*);
