
void show_employees(int clientSocket)
{
    int fd = open(USER_DB, O_RDONLY);
    struct user employeeUser;
    strcpy(serverMessage, "\tEmployees List\n");
    strcat(serverMessage, "-------------------------------------------\n");
    strcat(serverMessage, "Employee ID\t\tEmployee Username\n");
    strcat(serverMessage, "-------------------------------------------\n");
    while (read(fd, &employeeUser, sizeof(struct user)) > 0)
    {
        if (employeeUser.role == 1 && employeeUser.status == 0)
        {
            char formatted_entry[100];
            snprintf(formatted_entry, sizeof(formatted_entry), "%-16d\t%-20s\n",
                     employeeUser.user_id, employeeUser.username);
            strcat(serverMessage, formatted_entry);
        }
    }
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();
    close(fd);
}

int change_status(int user_id, int new_status, int role)
{
    int fd = open(USER_DB, O_RDWR);
    int return_val = 0;
    if (fd < 0)
    {
        perror("Error opening USER_DB");
        exit(0);
    }

    struct user findUser;
    struct flock lock;
    off_t offset = -1;
    ssize_t write_bytes;

    while (read(fd, &findUser, sizeof(struct user)) > 0)
    {
        if (findUser.user_id == user_id && findUser.role == role)
        {
            if (findUser.session == 0)
            {
                return_val = 3;
                break;
            }
            else
            {
                findUser.status = new_status;
                offset = lseek(fd, -1 * sizeof(struct user), SEEK_CUR);
                break;
            }
        }
        else
        {
            return_val = 2;
        }
    }

    if (offset != -1)
    {
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = offset;
        lock.l_len = sizeof(struct user);
        lock.l_pid = getpid();

        while (fcntl(fd, F_SETLK, &lock) == -1)
            ;

        lseek(fd, offset, SEEK_SET);

        write_bytes = write(fd, &findUser, sizeof(struct user));
        if (write_bytes > 0)
        {
            return_val = 1;
        }
        else
        {
            return_val = 0;
        }

        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
    }
    close(fd);

    return return_val;
}

int change_username(int user_id, char *new_username, int role)
{
    int fd = open(USER_DB, O_RDWR);
    int return_val = 0;
    if (fd < 0)
    {
        perror("Error opening USER_DB");
        exit(0);
    }

    struct user findUser;
    struct flock lock;
    off_t offset = -1;
    ssize_t write_bytes;

    while (read(fd, &findUser, sizeof(struct user)) > 0)
    {
        if (findUser.user_id == user_id && findUser.role == role)
        {
            if (findUser.session == 0)
            {
                return_val = 3;
                break;
            }
            else
            {
                strcpy(findUser.username, new_username);
                offset = lseek(fd, -1 * sizeof(struct user), SEEK_CUR);
                break;
            }
        }
        else
        {
            return_val = 2;
        }
    }

    if (offset != -1)
    {
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = offset;
        lock.l_len = sizeof(struct user);
        lock.l_pid = getpid();

        while (fcntl(fd, F_SETLK, &lock) == -1)
            ;

        lseek(fd, offset, SEEK_SET);

        write_bytes = write(fd, &findUser, sizeof(struct user));
        if (write_bytes == sizeof(struct user))
        {
            return_val = 1;
        }
        else
        {
            return_val = 0;
        }

        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
    }
    close(fd);
    return return_val;
}

void modify_details(int role, int clientSocket)
{
    int user_id, new_status;
    char new_username[50];
    int status;
    int record;

    strcpy(serverMessage, "Enter the User ID: ");
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
    user_id = atoi(clientMessage);
    clearBuffers();

    strcpy(serverMessage, "Actions Possible\n1. Change Username\n2. Activate/Deactivate Account\nEnter Youe Choice : ");
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
    record = atoi(clientMessage);
    clearBuffers();

    if (record == 2)
    {
        strcpy(serverMessage, "Enter the New Status(0:Activate 1:Deactivate): ");
        send(clientSocket, serverMessage, strlen(serverMessage), 0);
        recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
        new_status = atoi(clientMessage);
        clearBuffers();
        status = change_status(user_id, new_status, role);
        if (status == 1)
        {
            strcpy(serverMessage, "Status Changed Successfully!\n");
            send(clientSocket, serverMessage, strlen(serverMessage), 0);
        }
        else if (status == 0)
        {
            strcpy(serverMessage, "Status Not Updated!\n");
            send(clientSocket, serverMessage, strlen(serverMessage), 0);
        }
        else if (status == 2)
        {
            sprintf(serverMessage, "User with user ID %d Not Found\n", user_id);
            send(clientSocket, serverMessage, strlen(serverMessage), 0);
        }
        else if (status == 3)
        {
            sprintf(serverMessage, "Status Not Updated!\nUser with user ID %d is Currently Active\n", user_id);
            send(clientSocket, serverMessage, strlen(serverMessage), 0);
        }
        recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
        clearBuffers();
    }
    else if (record == 1)
    {
        strcpy(serverMessage, "Enter the New Username: ");
        send(clientSocket, serverMessage, strlen(serverMessage), 0);
        recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
        strcpy(new_username, clientMessage);
        clearBuffers();

        status = change_username(user_id, new_username, role);
        if (status == 1)
        {
            strcpy(serverMessage, "Username Changed Successfully!\n");
            send(clientSocket, serverMessage, strlen(serverMessage), 0);
        }
        else if (status == 0)
        {
            strcpy(serverMessage, "Username Not Updated!\n");
            send(clientSocket, serverMessage, strlen(serverMessage), 0);
        }
        else if (status == 2)
        {
            sprintf(serverMessage, "User with user ID %d Not Found", user_id);
            send(clientSocket, serverMessage, strlen(serverMessage), 0);
        }
        else if (status == 3)
        {
            sprintf(serverMessage, "Status Not Updated!\nUser with user ID %d is Currently Active\n", user_id);
            send(clientSocket, serverMessage, strlen(serverMessage), 0);
        }
        recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
        clearBuffers();
    }
    else
    {
        send(clientSocket, "Invalid Choice", strlen("Invalid Choice"), 0);
        recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
        clearBuffers();
    }
}

void manage_roles(int clientSocket)
{
    int user_id;
    int fd = open(USER_DB, O_RDWR);
    int r = 0;

    strcpy(serverMessage, "Enter the User ID: ");
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
    user_id = atoi(clientMessage);
    clearBuffers();

    struct user findUser;
    struct flock lock;
    off_t offset = -1;
    ssize_t write_bytes;

    while (read(fd, &findUser, sizeof(struct user)) > 0)
    {
        if (findUser.user_id == user_id && findUser.role == 1)
        {
            if (findUser.session == 1 && findUser.status == 0)
            {
                r = 1;
                findUser.role = 2;
            }
            else
            {
                r = 2;
            }

            offset = lseek(fd, -1 * sizeof(struct user), SEEK_CUR);
            break;
        }
    }

    if (offset != -1)
    {
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = offset;
        lock.l_len = sizeof(struct user);
        lock.l_pid = getpid();
        while (fcntl(fd, F_SETLK, &lock) == -1)
            ;

        lseek(fd, offset, SEEK_SET);
        write_bytes = write(fd, &findUser, sizeof(struct user));
        if (write_bytes == sizeof(struct user) && r == 1)
        {
            snprintf(serverMessage, sizeof(serverMessage), "\nEmployee %s Role Changed to Manager!\n", findUser.username);
            send(clientSocket, serverMessage, strlen(serverMessage), 0);
        }
        else if (r == 0)
        {
            snprintf(serverMessage, sizeof(serverMessage), "\nEmployee with ID %d Not Found!\n", user_id);
            send(clientSocket, serverMessage, strlen(serverMessage), 0);
        }
        else if (r == 2)
        {
            snprintf(serverMessage, sizeof(serverMessage), "\nEmployee with ID %d Currently Active!\n", user_id);
            send(clientSocket, serverMessage, strlen(serverMessage), 0);
        }
        else
        {
            strcpy(serverMessage, "\nEmployee Role Change Unsuccessfull\n");
            send(clientSocket, serverMessage, strlen(serverMessage), 0);
        }

        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
    }
    else
    {
        snprintf(serverMessage, sizeof(serverMessage), "Employee with User ID : %d Not Found\n", user_id);
        send(clientSocket, serverMessage, strlen(serverMessage), 0);
    }
    recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();
    close(fd);
}

void admin(struct user *loginUser, int clientSocket)
{
    int admin_run = 1;

    while (admin_run == 1)
    {
        char admin_p_id[20];
        sprintf(admin_p_id, "%d", loginUser->user_id);
        strcpy(serverMessage, "\nWelcome, ");
        strcat(serverMessage, loginUser->username);
        strcat(serverMessage, "!\n");
        strcat(serverMessage, "User ID: ");
        strcat(serverMessage, admin_p_id);
        strcat(serverMessage, "\n");
        strcat(serverMessage, "Choose an operation:\n");
        strcat(serverMessage, "1. Add New Bank Employee\n");
        strcat(serverMessage, "2. Show Employees\n");
        strcat(serverMessage, "3. Modify Customer Details\n");
        strcat(serverMessage, "4. Modify Employee Details\n");
        strcat(serverMessage, "5. Manage Roles\n");
        strcat(serverMessage, "6. Change Password\n");
        strcat(serverMessage, "7. Logout\n");
        strcat(serverMessage, "Enter Your Choice : ");

        send(clientSocket, serverMessage, strlen(serverMessage), 0);
        recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
        int admin_choice = atoi(clientMessage);
        clearBuffers();

        switch (admin_choice)
        {
        case 1:
            addUser(1, clientSocket);
            break;
        case 2:
            show_employees(clientSocket);
            break;
        case 3:
            modify_details(3, clientSocket);
            break;
        case 4:
            modify_details(1, clientSocket);
            break;
        case 5:
            manage_roles(clientSocket);
            break;
        case 6:
            change_password(loginUser, clientSocket);
            break;
        case 7:
            logout(loginUser, clientSocket);
            admin_run = 0;
            break;
        default:
            send(clientSocket, "Invalid choice!\n", strlen("Invalid choice!\n"), 0);
            recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
            clearBuffers();
            break;
        }
    }
}
