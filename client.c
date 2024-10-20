#include <stdio.h>      /* printf(), perror() */
#include <stdlib.h>     /*  */
#include <unistd.h>     /* fork(), close(), read(), _exit() */
#include <string.h>     /* strncpy() */
#include <arpa/inet.h>  /* htons() */
#include <sys/socket.h> /* socket(), bind(), listen(), accept(), connect() */
#include <netinet/in.h>

#define PORT 6009
#define BACKLOG 100
char readBuffer[1000], writeBuffer[1000];

void clearMemory()
{
    memset(readBuffer, 0, sizeof(readBuffer));
    memset(writeBuffer, 0, sizeof(writeBuffer));
}

void server_handler(int connection_desciptor);
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

    server_address.sin_family = AF_INET;                     // IPv4
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Bind to all available network interfaces
    server_address.sin_port = htons(PORT);                   // Server will listen to PORT

    connect_status = connect(connection_desciptor, (void *)&server_address, sizeof(server_address));
    if (connect_status == -1)
    {
        perror("ERROR connecting to Server!");
        close(connection_desciptor);
        _exit(0);
    }
    server_handler(connection_desciptor);

    close(connect_status);

    return 0;
}

void server_handler(int connection_desciptor)
{
    printf("Client has connected to the server!\n");

    ssize_t readBytes, writeBytes;
    int userChoice;

    clearMemory();
    readBytes = recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
    if (readBytes == -1)
    {
        perror("Error Occured While Reading From Server");
        exit(0);
    }

    printf("%s", readBuffer);

    scanf("%d", &userChoice);

    writeBytes = send(connection_desciptor, &userChoice, sizeof(int), 0);
    if (writeBytes == -1)
    {
        perror("Error Occured While Writing to Server");
        exit(0);
    }

    switch (userChoice)
    {
    case 1:

        break;
    case 2:
        break;
    case 3:
        break;
    case 4:
        clearMemory();
        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);

        printf("%s", readBuffer);
        scanf("%s", writeBuffer);

        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);

        // bzero(password_prompt, sizeof(password_prompt));
        // perror("Error");

        clearMemory();

        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
        printf("%s", readBuffer);
        scanf("%s", writeBuffer);
        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);

        clearMemory();


        int status = 0;
        int cust_choice;
        char dummy[30];
        bzero(dummy, sizeof(dummy));
        strcpy(dummy, "Hello\n");

        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
        int s1 = atoi(readBuffer);
        send(connection_desciptor, dummy, strlen(dummy), 0);
        bzero(dummy, sizeof(dummy));
        if (s1 == 1)
        {
            while (1)
            {
                clearMemory();
                recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                printf("%s", readBuffer);
                scanf("%s", writeBuffer);
                send(connection_desciptor, writeBuffer, sizeof(writeBuffer), 0);
                cust_choice = atoi(writeBuffer);
                clearMemory();
                switch (cust_choice)
                {
                case 1:
                    clearMemory();
                    recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                    printf("%s", readBuffer);
                    clearMemory();
                    bzero(dummy, sizeof(dummy));
                    strcpy(dummy, "Hello\n");
                    send(connection_desciptor, dummy, strlen(dummy), 0);
                    bzero(dummy, sizeof(dummy));
                    clearMemory();
                    break;
                case 2:
                    break;
                case 3:
                    break;
                case 4:
                    break;
                case 5:
                    break;
                case 6:
                    break;
                case 7:
                    break;
                case 8:
                    break;
                case 9:
                    clearMemory();
                    recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                    printf("%s", readBuffer);
                    clearMemory();
                    return;
                    break;
                }
            }
        }
        else
        {
            // recv(connection_desciptor, message, sizeof(message), 0);
            // printf("%s", message);
        }

        break;
    default:
        break;
    }
}
