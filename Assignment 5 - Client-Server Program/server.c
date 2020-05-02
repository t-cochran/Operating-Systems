/*  Programming assignment #5
 *  CSPB 3753 - Operating Systems
 *  Author: Thomas Cochran
 * 
 *  SERVER PROGRAM
 * 
 *  Code adapted from: 
 *      Ref: Beej's Guide to Network Programming 2019
 *      Web: https://beej.us/guide/bgnet/
 * 
 *  See README.md for instructions on running this program.
 */
#include "headerPA5.h"

int main(int argc, char* argv[]) {

    uint8_t reply = 1;
    struct sigaction sa;   
    struct sockaddr_in from;  
    struct cmdline serverOpt;
    struct addrinfo hints, *serverInfo, *cursor;  
    socklen_t fromlen = sizeof(struct sockaddr_in);                                                       
    socklen_t addrLen = sizeof(struct sockaddr_in);
    char addrStr[INET_ADDRSTRLEN], buf[MAXDATASIZE];
    int ret, serverSock, connection, set = 1, numbytes = 0;                      

    /* Initialize command-line arguments, hints, and sigchld handler */
    serverOpt = parser(argc, argv, SERVER);           // Get command-line options
    init_hints(&hints, serverOpt.socktype, SERVER);   // Set getaddrinfo() hints
    init_sigchld_handler(&sa);                        // Set a sigchld handler

    /* Create a list of addrinfo structures for the server program */
    if ((ret = getaddrinfo(NULL, serverOpt.port, &hints, &serverInfo)) != 0) {
        fprintf(stderr, "[Server Program]: getaddrinfo: %s\n", gai_strerror(ret));
        return 1;
    }

    /* Loop through addrinfo structs until a socket can be created and bound */
    for (cursor = serverInfo; cursor != NULL; cursor = cursor -> ai_next) {

        /* Create a server socket */
        if ((serverSock = socket(cursor -> ai_family, cursor -> ai_socktype, 
                                 cursor -> ai_protocol)) == -1) {
            perror("[Server Program]: socket");
            continue;
        }

        /* Bind the server socket */
        if (bind(serverSock, cursor -> ai_addr, cursor -> ai_addrlen) == -1) {
            close(serverSock);
            perror("[Server Program]: bind");
            continue;
        }
        break;
    }

    /* Exit if the socket failed to bind */
    if (cursor == NULL) {
        fprintf(stderr, "[Server Program]: failed to bind socket\n");
        freeaddrinfo(serverInfo);
        exit(1);
    }

    /* TCP PROTOCOL SELECTED */
    if (hints.ai_socktype == SOCK_STREAM) { 

        /* Set the socket option to reuse the socket address */                  
        if (setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &set, 
                       sizeof(int)) == -1) {
            perror("[Server Program]: setsockopt");
            close(serverSock); freeaddrinfo(serverInfo);
            exit(1);
        }

        /* Server listening loop */
        while(1) {

            /* Listen for incoming connections */
            printf("------------------------------------------------------------------\n");
            printf("[Server Program]: Waiting for connections...\n");
            if (listen(serverSock, BACKLOG) == -1) {
                perror("[Server Program]: listen");
                close(serverSock); freeaddrinfo(serverInfo);
                exit(1);
            }

            /* Accept a connection */
            if ((connection = accept(serverSock, (struct sockaddr*)&from, &addrLen)) == -1) {
                perror("[Server Program]: accept");
                close(serverSock); close(connection); freeaddrinfo(serverInfo);
                exit(1);
            }

            /* Convert connected address from network byte order and print it */
            inet_ntop(from.sin_family,
                      get_sock_ip((struct sockaddr*)&from), addrStr, 
                      sizeof(addrStr));
            printf("[Server Program]: Connected to client: %s:%s via TCP.\n\n", 
                    addrStr, serverOpt.port);

            /* Receieve a packet from the client */
            if ((numbytes = recv(connection, buf, sizeof(struct packet), 0)) == -1) {
                perror("[Server Program]: recv");
                close(serverSock); close(connection); freeaddrinfo(serverInfo);
                exit(1);
            }
            printf("[Server Program]: <%d bytes> receieved from the client.\n", numbytes);

            /* Convert the packet to host byte order and unpack the message */
            uint32_t data_receieved = ntohl(((struct packet*)buf) -> number);
            printf("[Server Program]: The sent number is: %d\n\n", data_receieved);

            /* Fork() a child process to send a reply message */
            printf("[Server Program]: Sending reply message to the client.\n");
            if (!fork()) {
                // sendall() avoids a partial send
                if ((sendall(connection, (char*)&reply, &numbytes)) == -1) {
                    fprintf(stderr, "[Server Program]: Failed to sendall\n");
                }
                close(serverSock); close(connection); freeaddrinfo(serverInfo);
                exit(0);
            }

            /* Server terminates if 0 is receieved */
            if (data_receieved == 0) {
                printf("\n[Server Program]: Termination signal receieved. Et tu Brute...?\n");
                printf("[Server Program]: Server shutting down.\n");
                close(serverSock); close(connection); freeaddrinfo(serverInfo);
                exit(0);
            }

            /* Server closes the connection and continues listening */
            close(connection);
        }
    }

    /* UDP PROTOCOL SELECTED */
    if (hints.ai_socktype == SOCK_DGRAM) {

        /* Server listening loop */
        while(1) {

            /* Server blocks until it receieves a datagram */
            printf("------------------------------------------------------------------\n");
            printf("[Server Program]: Waiting to receieve a datagram...\n\n");
            if ((numbytes = recvfrom(serverSock, buf, sizeof(struct packet), 0,
                                    (struct sockaddr*)&from, &fromlen)) == -1) {
                perror("[Server Program]: recvfrom");
                close(serverSock); freeaddrinfo(serverInfo);
                exit(1);
            }

            /* Datagram receieved: print the sender address and port */
            printf("[Server Program]: <%d bytes> Receieved from %s:%s via UDP.\n", 
                   numbytes, inet_ntoa(from.sin_addr), serverOpt.port);

            /* Convert the packet to host byte order and unpack the message */
            uint32_t data_receieved = ntohl(((struct packet*)buf) -> number);
            printf("[Server Program]: The sent number is: %d\n\n", data_receieved);

            /* Send a reply message to the client */
            printf("[Server Program]: Sending reply message to the client.\n");
            if ((sendto(serverSock, &reply, sizeof(reply), 0, (struct sockaddr*)&from, 
                        fromlen)) == -1) {
                perror("[Server Program]: sendto");
                close(serverSock); freeaddrinfo(serverInfo);
                exit(1);
            }
            
            /* If the data receieved is 0 (i.e. the kill signal), terminate the server */
            if (data_receieved == 0) {
                printf("\n[Server Program]: Termination signal receieved. Et tu Brute...?\n");
                printf("[Server Program]: Server shutting down.\n");
                close(serverSock); freeaddrinfo(serverInfo);
                exit(0);
            }
        }
    }

    exit(0);
}
