#include <stdio.h>      /* printf(), perror() */
#include <stdlib.h>     /*  */
#include <unistd.h>     /* fork(), close(), read(), _exit() */
#include <string.h>     /* strncpy() */
#include <arpa/inet.h>  /* htons() */
#include <sys/socket.h> /* socket(), bind(), listen(), accept(), connect() */
#include <netinet/in.h>

#define PORT 6008
#define BACKLOG 100
char readBuffer[1000], writeBuffer[1000], dummyBuffer[100];

void clearMemory()
{
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));
    bzero(dummyBuffer, sizeof(dummyBuffer));
}

void fillDummy()
{
    bzero(dummyBuffer, sizeof(dummyBuffer));
    strcpy(dummyBuffer, "This is a Dummy Message\n");
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
    printf("Connected to the Server!\n");

    ssize_t readBytes, writeBytes, dummyBytes;

    while (1)
    {
        int userChoice = 0;
        clearMemory();
        readBytes = recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
        if (readBytes == -1)
        {
            perror("Error Occured While Reading From Server");
            exit(0);
        }

        printf("%s", readBuffer);
        scanf("%s", writeBuffer);

        writeBytes = send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
        if (writeBytes == -1)
        {
            perror("Error Occured While Writing to Server");
            exit(0);
        }
        userChoice = atoi(writeBuffer);
        clearMemory();

        switch (userChoice)
        {
        case 1:
            recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
            printf("%s", readBuffer);
            scanf("%s", writeBuffer);
            send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
            clearMemory();

            recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
            printf("%s", readBuffer);
            scanf("%s", writeBuffer);
            send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
            clearMemory();

            fillDummy();
            recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
            int adminstatus = atoi(readBuffer);
            send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
            clearMemory();

            if (adminstatus == 1)
            {
                while (1)
                {
                    int admin_cust_choice;

                    recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                    printf("%s", readBuffer);
                    scanf("%s", writeBuffer);
                    send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                    admin_cust_choice = atoi(writeBuffer);
                    clearMemory();

                    switch (admin_cust_choice)
                    {
                    case 1:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 2:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 3:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 4:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 5:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 6:
                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 7:
                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();
                        break;

                    default:
                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();
                        break;
                    }
                }
            }
            else
            {
                recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                printf("%s", readBuffer);
                fillDummy();
                send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                clearMemory();
            }
            break;
        case 2:
            recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
            printf("%s", readBuffer);
            scanf("%s", writeBuffer);
            send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
            clearMemory();

            recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
            printf("%s", readBuffer);
            scanf("%s", writeBuffer);
            send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
            clearMemory();

            fillDummy();
            recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
            int managerstatus = atoi(readBuffer);
            send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
            clearMemory();

            if (managerstatus == 1)
            {
                while (1)
                {
                    int manager_choice;

                    recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                    printf("%s", readBuffer);
                    scanf("%s", writeBuffer);
                    send(connection_desciptor, writeBuffer, sizeof(writeBuffer), 0);
                    manager_choice = atoi(writeBuffer);
                    clearMemory();

                    switch (manager_choice)
                    {
                    case 1:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 2:
                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 3:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        int f_e = atoi(readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        if (f_e == 1)
                        {
                            recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                            printf("%s", readBuffer);
                            scanf("%s", writeBuffer);
                            send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                            clearMemory();
                        }

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 4:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 5:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 6:
                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;

                    default:
                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();
                        break;
                    }
                }
            }
            else
            {
                recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                printf("%s", readBuffer);
                fillDummy();
                send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                clearMemory();
            }

            break;
        case 3:

            recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
            printf("%s", readBuffer);
            scanf("%s", writeBuffer);
            send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
            clearMemory();

            recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
            printf("%s", readBuffer);
            scanf("%s", writeBuffer);
            send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
            clearMemory();

            fillDummy();
            recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
            int employeestatus = atoi(readBuffer);
            send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
            clearMemory();

            if (employeestatus == 1)
            {
                while (1)
                {
                    int employee_choice;

                    recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                    printf("%s", readBuffer);
                    scanf("%s", writeBuffer);
                    send(connection_desciptor, writeBuffer, sizeof(writeBuffer), 0);
                    employee_choice = atoi(writeBuffer);
                    clearMemory();

                    switch (employee_choice)
                    {
                    case 1:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 2:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 3:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 4:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 5:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 6:
                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;

                    default:
                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();
                        break;
                    }
                }
            }
            else
            {
                recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                printf("%s", readBuffer);
                fillDummy();
                send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                clearMemory();
            }

            break;
        case 4:
            recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
            printf("%s", readBuffer);
            scanf("%s", writeBuffer);
            send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
            clearMemory();

            recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
            printf("%s", readBuffer);
            scanf("%s", writeBuffer);
            send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
            clearMemory();

            fillDummy();
            recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
            int status = atoi(readBuffer);
            send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
            clearMemory();

            if (status == 1)
            {
                while (1)
                {
                    int cust_choice;

                    recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                    printf("%s", readBuffer);
                    scanf("%s", writeBuffer);
                    send(connection_desciptor, writeBuffer, sizeof(writeBuffer), 0);
                    cust_choice = atoi(writeBuffer);
                    clearMemory();

                    switch (cust_choice)
                    {
                    case 1:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 2:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 3:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 4:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 5:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 6:
                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();
                        break;
                    case 7:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        scanf("%s", writeBuffer);
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 8:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        char feedback[1000];
                        printf("%s", readBuffer);
                        fflush(stdout);
                        int bytes_read = read(STDIN_FILENO, writeBuffer, sizeof(writeBuffer));
                        writeBuffer[bytes_read - 1] = '\0';
                        send(connection_desciptor, writeBuffer, strlen(writeBuffer), 0);
                        clearMemory();

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 9:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;
                    case 10:

                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();

                        break;

                    default:
                        recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                        printf("%s", readBuffer);
                        fillDummy();
                        send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                        clearMemory();
                        break;
                    }
                }
            }
            else
            {
                recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
                printf("%s", readBuffer);
                fillDummy();
                send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
                clearMemory();
            }

            break;
        case 5:
            recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
            printf("%s", readBuffer);
            fillDummy();
            send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
            clearMemory();
            return;
            break;
        default:
            recv(connection_desciptor, readBuffer, sizeof(readBuffer), 0);
            printf("%s", readBuffer);
            fillDummy();
            send(connection_desciptor, dummyBuffer, strlen(dummyBuffer), 0);
            clearMemory();
            break;
        }
    }
}
