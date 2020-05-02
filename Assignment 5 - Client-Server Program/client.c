/*  Programming assignment #5
 *  CSPB 3753 - Operating Systems
 *  Author: Thomas Cochran
 *
 *  CLIENT PROGRAM
 *
 *  Code adapted from:
 *      Ref: Beej's Guide to Network Programming 2019
 *      Web: https://beej.us/guide/bgnet/
 *
 *  See README.md for instructions on running this program.
 */
#include "headerPA5.h"

int main(int argc, char* argv[]) {

    struct timeval tv;
    struct cmdline clientOpts;
    struct addrinfo hints, *clientInfo, *cursor;
    int ret, clientSock = 0, numbytes = sizeof(struct packet);
    char addrStr[INET6_ADDRSTRLEN], buf[MAXDATASIZE];
    socklen_t addrLen = sizeof(struct sockaddr_in);

    /* Initialize command-line arguments, hints, and 3 second timeout */
    clientOpts = parser(argc, argv, CLIENT);          // Get command-line options
    init_hints(&hints, clientOpts.socktype, CLIENT);  // Set getaddrinfo() hints
    tv.tv_sec = 3;                                    // Set a 3 second timer to timeout

    /* Pack the message sent to the server */
    struct packet data_packet;
    data_packet.version = 0x1;
    data_packet.number = htonl(clientOpts.data);
    memcpy(&buf, &data_packet, 5);

    /* Create a list of addrinfo structures for the client program */
    if (clientOpts.hostname == NULL) {      // Ipv4 address selected
        if ((ret = getaddrinfo(inet_ntoa(clientOpts.ipv4_address), clientOpts.port,
                               &hints, &clientInfo)) != 0) {
            fprintf(stderr, "[Client Program]: getaddrinfo: %s\n", gai_strerror(ret));
            return 1;
        }
    }
    else {  // Hostname selected
        if ((ret = getaddrinfo(clientOpts.hostname, clientOpts.port,
                               &hints, &clientInfo)) != 0) {
            fprintf(stderr, "[Client Program]: getaddrinfo: %s\n", gai_strerror(ret));
            return 1;
        }
    }

    /* Loop through addrinfo structs to create a socket */
    for (cursor = clientInfo; cursor != NULL; cursor = cursor -> ai_next) {

        /* Create a client socket */
        if ((clientSock = socket(cursor -> ai_family, cursor -> ai_socktype,
                                 cursor -> ai_protocol)) == -1) {
            perror("[Client Program]: socket");
            continue;
        }

        /* TCP protocol also connects to the client socket */
        if (hints.ai_socktype == SOCK_STREAM) {
            if (connect(clientSock, cursor -> ai_addr, cursor -> ai_addrlen) == -1) {
                close(clientSock);
                perror("[Client Program]: connect");
                continue;
            }
        }
        break;
    }

    /* Exit if the socket failed to create or connect */
    if (cursor == NULL && hints.ai_socktype == SOCK_STREAM) {
        fprintf(stderr, "[Client Program]: Failed to connect\n");
        freeaddrinfo(clientInfo);
        exit(1);
    }
    if (cursor == NULL && hints.ai_socktype == SOCK_DGRAM) {
        fprintf(stderr, "[Client Program]: Failed to create socket\n");
        freeaddrinfo(clientInfo);
        exit(1);
    }

    /* TCP PROTOCOL SELECTED */
    if (hints.ai_socktype == SOCK_STREAM) {

        /* Set the client socket to have a 3 second read timeout*/
        setsockopt(clientSock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv,
                   sizeof(struct timeval));

        /* Convert address from network byte order and print it */
        inet_ntop(clientInfo -> ai_family,
                  get_sock_ip((struct sockaddr*) clientInfo -> ai_addr),
                  addrStr, sizeof(addrStr));
        printf("------------------------------------------------------------------\n");
        printf("[Client Program]: Connecting to %s:%s...\n", addrStr, clientOpts.port);

        /* Send a message packet to the server */
        if ((sendall(clientSock, buf, &numbytes)) == -1) { // sendall() avoids a partial send
            fprintf(stderr, "[Client Program]: Failed to sendall\n");
            freeaddrinfo(clientInfo); close(clientSock);
            exit(1);
        }
        printf("\n[Client Program]: Sent <%d bytes> to server %s:%s via TCP.\n\n",
                numbytes, inet_ntoa(clientOpts.ipv4_address), clientOpts.port);

        /* Receieve a reply message from the server or timeout after 3 seconds*/
        printf("[Client Program]: Waiting for server reply...\n");
        if ((numbytes = recv(clientSock, buf, sizeof(uint8_t), 0)) == -1) {
            printf("[Client Program]: ERROR server reply not receieved.\n");
            perror("[Client Program]: recv");
            freeaddrinfo(clientInfo); close(clientSock);
            exit(1);
        }

        /* Server reply receieved: print a success message */
        printf("[Client Program]: <%d bytes> receieved from the server\n", numbytes);
        printf("[Client Program]: Reply message from the server: %d Success!\n\n", buf[0]);
    }

    /* UDP PROTOCOL SELECTED */
    if (hints.ai_socktype == SOCK_DGRAM) {

        /* Set the client socket to have a 3 second read timeout*/
        setsockopt(clientSock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv,
                   sizeof(struct timeval));

        /* Send a message packet to the server */
        if ((numbytes = sendto(clientSock, buf, sizeof(struct packet), 0,
                               clientInfo -> ai_addr, clientInfo -> ai_addrlen)) == -1) {
            perror("[Client Program]: sendto");
            freeaddrinfo(clientInfo); close(clientSock);
            exit(1);
        }
        printf("------------------------------------------------------------------\n");
        printf("\n[Client Program]: Sent <%d bytes> to server %s:%s via UDP.\n\n",
                numbytes, inet_ntoa(clientOpts.ipv4_address), clientOpts.port);

        /* Receieve a reply message from the server or timeout after 3 seconds */
        printf("[Client Program]: Waiting for server reply...\n\n");
        if ((numbytes = recvfrom(clientSock, buf, sizeof(struct packet), 0,
                                 clientInfo -> ai_addr, &addrLen)) == -1) {
            printf("[Client Program]: ERROR server reply not receieved.\n");
            perror("[Client Program]: recvfrom");
            freeaddrinfo(clientInfo); close(clientSock);
            exit(1);
        }

        /* Server reply receieved: print a success message */
        printf("[Client Program]: <%d bytes> receieved from the server\n", numbytes);
        printf("[Client Program]: Reply message from the server: %d Success!\n\n", buf[0]);
        close(clientSock);
    }

    exit(0);
}
