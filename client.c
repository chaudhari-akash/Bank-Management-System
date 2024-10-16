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
    struct sockaddr_in server_address;
    int connection_desciptor, connect_status;
    connection_desciptor = socket(AF_INET, SOCK_STREAM, 0); // Using IPv4 and TCP, SOCK_STREAM : Type of connection, AF_INET : Using IPv4
    if (connection_desciptor == -1)
    {
        perror("ERROR creating Client Socket!");
        _exit(0);
    }

    server_address.sin_family = AF_INET;         // IPv4
    server_address.sin_addr.s_addr = INADDR_ANY; // Bind to all available network interfaces
    server_address.sin_port = htons(PORT);       // Server will listen to PORT

    connect_status = connect(connection_desciptor, (void *)&server_address, sizeof(server_address));
    if (connect_status == -1)
    {
        perror("ERROR connecting to Server!");
        close(connection_desciptor);
        _exit(0);
    }

    server_handler(connection_desciptor);

    close(connection_desciptor);

    return 0;
}
