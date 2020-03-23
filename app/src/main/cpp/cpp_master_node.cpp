#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>//inet_addr
#include <unistd.h>    //write
#include <pthread.h>//for threading , link with lpthread

#include <map>

#define PORT 8080

#include "util.h"

//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>


void *connection_handler(void *);

typedef struct client_info {
    char ipAddr[25];
    int port;
    int sock_desc;
} client_info;

typedef struct params {
    int sd;
    int key;
} params;

std::map<int, client_info> CLIENT_TABLE;

#define BUFFER_LEN 1024

void send_file(const char *path, client_info& client) {
    char buffer[BUFFER_LEN];

    FILE *fp = fopen(path, "r");

    fseek(fp, 0, SEEK_END);
    int filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    for (int i = 0; i < filesize / BUFFER_LEN; i++) {
        fread(buffer, BUFFER_LEN, 1, fp);
        write(client.sock_desc, buffer, BUFFER_LEN);

    }
    if (filesize % BUFFER_LEN != 0) {
        int leftbytes = filesize - ftell(fp);
        fread(buffer, leftbytes, 1, fp);
        write(client.sock_desc, buffer, leftbytes);
    }

}

void print_client_details(std::map<int, client_info> m) {
    log_output("Client Table :\n");
    for (auto x: m) {
        log_output("Client %d : %s:%d\n", x.first, x.second.ipAddr, x.second.port);
    }
}

int mainfunc() {
    int socket_desc, new_socket, c, *new_sock, i;
    struct sockaddr_in server, client;
    char *message;

    //Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        log_output("Sorry. The Socket could not be created!\n");
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    //Bind
    if (bind(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0) {
        log_output("Socket Binding Failed!");
        return 1;
    }
    log_output("Server Socket has been bound to the port : %d\n", PORT);

    //Listen
    listen(socket_desc, 3);

    //Accept and incoming connection
    log_output("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    i = 0;
    while ((new_socket = accept(socket_desc, (struct sockaddr *) &client, (socklen_t *) &c))) {
        log_output("Connection Accepted from : %s:%d (Client %d)\n", inet_ntoa(client.sin_addr),
                   ntohs(client.sin_port), i);

        //Reply to the client
        pthread_t sniffer_thread;
        // new_sock = (int*)malloc(1);
        // *new_sock = new_socket;
        params *p = (params *) malloc(sizeof(params));
        p->sd = new_socket;
        p->key = i;
        client_info *c = (client_info *) malloc(sizeof(client));
        strcpy(c->ipAddr, inet_ntoa(client.sin_addr));
        c->port = ntohs(client.sin_port);
        c->sock_desc = new_socket;
        CLIENT_TABLE.insert(std::pair<int, client_info>(i, *c));
        print_client_details(CLIENT_TABLE);
        //every thread created is linked to p_thread_t sniffer thread? No clashes with previous iterations? Look into this, how thread creation happens
        //maintain a table of thread with file descriptor and IP, each connection handler should be able to delete the record from the table as well.
        //Also send data via specific threads, see how to access specific threads, by IP address?
        if (pthread_create(&sniffer_thread, NULL, connection_handler, (void *) p) < 0) {
            log_output("could not create thread\n");
            return 1;
        }
        i++;
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
        //puts("Handler assigned");
    }

    if (new_socket < 0) {
        log_output("accept failed\n");
        return 1;
    }

    return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc) {
    //Get the socket descriptor
    params *p = (params *) socket_desc;
    int sock = p->sd;
    int key = p->key;
    int read_size;
    char *message, client_message[2000];

    //Send some messages to the client
    // message = "Greetings! I am your connection handler\n";
    // write(sock , message , strlen(message));
    //
    // message = "Now type something and i shall repeat what you type \n";
    // write(sock , message , strlen(message));

    //Receive a message from client
    while ((read_size = recv(sock, client_message, 2000, 0)) > 0) {
        //Send the message back to client
        write(sock, client_message, strlen(client_message));
    }

    if (read_size == 0) //when is read size zero and when is it 1?
    {
        log_output("Client Disconnected\n");
        CLIENT_TABLE.erase(key);
        print_client_details(CLIENT_TABLE);
        fflush(stdout);
    } else if (read_size == -1) {
        log_output("Client Disconnected\n");
        print_client_details(CLIENT_TABLE);
        CLIENT_TABLE.erase(key);
    }

    //Free the socket pointer
    free(socket_desc);

    return 0;
}
