#include <stdio.h>      /* printf(), perror() */
#include <stdlib.h>     /*  */
#include <unistd.h>     /* fork(), close(), read(), _exit() */
#include <string.h>     /* strncpy() */
#include <arpa/inet.h>  /* htons() */
#include <sys/socket.h> /* socket(), bind(), listen(), accept(), connect() */
#include <netinet/in.h>

#define PORT 6006
#define BACKLOG 100

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

    server_address.sin_family = AF_INET;         // IPv4
    server_address.sin_addr.s_addr = INADDR_ANY; // Bind to all available network interfaces
    server_address.sin_port = htons(PORT);       // Server will listen to PORT

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
    }

    close(socket_descriptor);

    return 0;
}