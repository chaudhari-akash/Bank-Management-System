void login(struct user *loginUser, int user_role, int clientSocket)
{
    loginUser->role = user_role;
    int user_auth = 0;

    strcpy(serverMessage, "Enter Username: ");
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
    strcpy(loginUser->username, clientMessage);
    clearBuffers();

    strcpy(serverMessage, "Enter password: ");
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
    loginUser->hashed_password = hash_password(clientMessage);
    clearBuffers();

    user_auth = authenticate(loginUser);
    printf("user_auth : %d\n", user_auth);
    snprintf(serverMessage, sizeof(serverMessage), "%d", user_auth);
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();

    if (user_auth == 1)
    {
        switch (user_role)
        {
        case 0:
            admin(loginUser, clientSocket);
            break;
        case 1:
            employee(loginUser, clientSocket);
            break;
        case 2:
            manager(loginUser, clientSocket);
            break;
        case 3:
            customer(loginUser, clientSocket);
            break;
        default:
            printf("Problem in getting Role\n");
            break;
        }
    }
    else
    {
        send(clientSocket, "Invalid credentials!\n", strlen("Invalid credentials!\n"), 0);
        recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
        clearBuffers();
    }
}
