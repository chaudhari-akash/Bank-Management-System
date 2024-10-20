#include <stdio.h>      /* printf(), perror() */
#include <stdlib.h>     /*  */
#include <unistd.h>     /* fork(), close(), read(), _exit() */
#include <string.h>     /* strncpy() */
#include <arpa/inet.h>  /* htons() */
#include <sys/socket.h> /* socket(), bind(), listen(), accept(), connect() */
#include <netinet/in.h>

#define MAIN_MENU "Select Your Role:\n1. Admin\n2. Manager\n3. Employee\n4. Customer \n5. Exit\nEnter Your Choice: "

#define PORT 6008
#define BACKLOG 100

#include "helper/customer.h"

void client_handler(int connectionFileDescriptor);

int main()
{
    struct sockaddr_in server_address, client_address;
    int socket_descriptor, socket_bind_status, socket_listen_status, connection_desciptor;

    socket_descriptor = socket(AF_INET, SOCK_STREAM, 0); // Using IPv4 and TCP, SOCK_STREAM : Type of connection, AF_INET : Using IPv4
    if (socket_descriptor == -1)
    {
        perror("ERROR creating Server Socket!");
        _exit(0);
    }

    server_address.sin_family = AF_INET;                     // IPv4
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Bind to all available network interfaces
    server_address.sin_port = htons(PORT);                   // Server will listen to PORT

    socket_bind_status = bind(socket_descriptor, (struct sockaddr *)&server_address, sizeof(server_address)); // Assigns the address to the socket : socket_descriptor
    if (socket_bind_status == -1)
    {
        perror("ERROR binding to Server Socket!");
        _exit(0);
    }

    socket_listen_status = listen(socket_descriptor, BACKLOG); // Listen for connections on a socket : socket_descriptor
    if (socket_listen_status == -1)
    {
        perror("ERROR listening for Connections!");
        close(socket_descriptor);
        _exit(0);
    }

    while (1)
    {
        int client_address_size = sizeof(client_address);
        connection_desciptor = accept(socket_descriptor, (struct sockaddr *)&client_address, &client_address_size); // Accepting a connection on socket : socket_descriptor
        if (connection_desciptor == -1)
        {
            perror("ERROR connecting to Client!");
            close(socket_descriptor);
        }

        else
        {
            if (!fork())
            {
                client_handler(connection_desciptor);
                close(connection_desciptor);
                _exit(0);
            }
        }
        perror("Error: ");
    }

    close(socket_descriptor);

    return 0;
}

void client_handler(int connectionFileDescriptor)
{
    printf("Connected to Client!\n");
    ssize_t serverMessageBytes, clientMessageBytes;
    int userChoice;

    serverMessageBytes = send(connectionFileDescriptor, MAIN_MENU, strlen(MAIN_MENU), 0);
    if (serverMessageBytes == -1)
    {
        perror("Error writing to client socket");
    }

    clientMessageBytes = recv(connectionFileDescriptor, &userChoice, sizeof(int), 0);
    if (clientMessageBytes == -1)
    {
        perror("Error reading from client socket");
    }

    switch (userChoice)
    {
    case 1:
        struct user admin;
        login(&admin, connectionFileDescriptor, 0);
        break;
    case 2:
        struct user manager;
        login(&manager, connectionFileDescriptor, 2);
        break;
    case 3:
        struct user employee;
        login(&employee, connectionFileDescriptor, 1);
        break;
    case 4:
        struct user customer;
        login(&customer, connectionFileDescriptor, 3);
        break;
    default:
        break;
    }
}