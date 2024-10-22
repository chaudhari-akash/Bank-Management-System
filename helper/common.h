
char serverMessage[1000], clientMessage[1000], dummyBuffer[100];

void clearBuffers()
{
    bzero(serverMessage, sizeof(serverMessage));
    bzero(clientMessage, sizeof(clientMessage));
    bzero(dummyBuffer, sizeof(dummyBuffer));
}

int authenticate(struct user *loginUser)
{
    struct user tempUser;
    int fd, write_bytes;
    off_t offset = -1;
    struct flock lock;
    int ret = 2;

    fd = open(USER_DB, O_RDWR);
    if (fd == -1)
    {
        perror("Error opening user database");
        return 0;
    }

    while (read(fd, &tempUser, sizeof(struct user)) > 0)
    {
        if (
            strcmp(loginUser->username, tempUser.username) == 0 &&
            loginUser->hashed_password == tempUser.hashed_password &&
            loginUser->role == tempUser.role)
        {
            loginUser->user_id = tempUser.user_id;
            loginUser->status = tempUser.status;

            if (tempUser.session == 1)
            {
                tempUser.session = 0;
                loginUser->session = tempUser.session;
                offset = lseek(fd, -1 * sizeof(struct user), SEEK_CUR);

                lock.l_type = F_WRLCK;
                lock.l_whence = SEEK_SET;
                lock.l_start = offset;
                lock.l_len = sizeof(struct user);
                lock.l_pid = getpid();
                fcntl(fd, F_SETLK, &lock);
                lseek(fd, offset, SEEK_SET);
                write_bytes = write(fd, &tempUser, sizeof(struct user));

                if (write_bytes == -1)
                {
                    perror("Error updating user session");
                    lock.l_type = F_UNLCK;
                    fcntl(fd, F_SETLK, &lock);

                    ret = 0;
                }

                lock.l_type = F_UNLCK;
                fcntl(fd, F_SETLK, &lock);
                ret = 1;
            }
            else
            {
                ret = 0;
            }
            break;
        }
        else
        {
            ret = 2;
        }
    }
    close(fd);
    return ret;
}

unsigned long hash_password(char *password)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *password++))
    {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

void change_password(struct user *loginUser, int clientSocket)
{
    char new_password[100];
    off_t offset = -1;
    struct user tempUser;
    struct flock lock;
    int fd;

    fd = open(USER_DB, O_RDWR);
    if (fd < 0)
    {
        perror("Error opening USER_DB");
        exit(0);
    }

    bzero(new_password, sizeof(new_password));

    strcpy(serverMessage, "Enter the new password : ");
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
    strcpy(new_password, clientMessage);
    clearBuffers();

    while (read(fd, &tempUser, sizeof(struct user)) > 0)
    {
        if (loginUser->user_id == tempUser.user_id)
        {
            loginUser->hashed_password = hash_password(new_password);
            offset = lseek(fd, -1 * sizeof(struct user), SEEK_CUR); // Move back to the correct position
            break;
        }
    }

    lseek(fd, offset, SEEK_SET);

    if (offset != -1)
    {
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = offset;
        lock.l_len = sizeof(struct user);
        lock.l_pid = getpid();

        fcntl(fd, F_SETLK, &lock);
        off_t of = (fd, offset, SEEK_SET);
        int write_status = write(fd, loginUser, sizeof(struct user));
        if (write_status == sizeof(struct user))
        {
            strcpy(serverMessage, "Password Change Successful\n");
            send(clientSocket, serverMessage, strlen(serverMessage), 0);
        }
        else
        {
            perror("Error While Changing Password");
            strcpy(serverMessage, "Password Change Unsuccessful\n");
            send(clientSocket, serverMessage, strlen(serverMessage), 0);
        }
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
    }
    else
    {
        strcpy(serverMessage, "No Such User Found\n");
        send(clientSocket, serverMessage, strlen(serverMessage), 0);
    }
    recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();
    close(fd);
}

void logout(struct user *loginUser, int clientSocket)
{
    struct user tempUser;
    int fd, write_bytes;
    off_t offset;
    struct flock lock;

    fd = open(USER_DB, O_RDWR);
    if (fd == -1)
    {
        perror("Error opening user database");
        return;
    }

    lseek(fd, 0, SEEK_SET);

    while (read(fd, &tempUser, sizeof(struct user)) > 0)
    {
        if (
            strcmp(loginUser->username, tempUser.username) == 0 &&
            loginUser->hashed_password == tempUser.hashed_password &&
            loginUser->role == tempUser.role)
        {
            tempUser.session = 1;
            offset = lseek(fd, -1 * sizeof(struct user), SEEK_CUR);

            lock.l_type = F_WRLCK;
            lock.l_whence = SEEK_SET;
            lock.l_start = offset;
            lock.l_len = sizeof(struct user);
            lock.l_pid = getpid();
            fcntl(fd, F_SETLK, &lock);

            write_bytes = write(fd, &tempUser, sizeof(struct user));
            if (write_bytes < 0)
            {
                perror("Error updating user status");
                lock.l_type = F_UNLCK;
                fcntl(fd, F_SETLK, &lock);
            }
            else
            {
                lock.l_type = F_UNLCK;
                fcntl(fd, F_SETLK, &lock);
                break;
            }
        }
    }
    close(fd);
    send(clientSocket, "Logging out...\n", strlen("Logging out...\n"), 0);
    recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();
}

int initiate_user_id()
{
    int fd = open(ID_FILE, O_RDWR | O_CREAT, 0644);
    char ID[100];
    strcpy(ID, "1000");
    write(fd, ID, sizeof(ID));
    close(fd);
}

int generate_user_id()
{

    int fd = open(ID_FILE, O_RDWR, 0644);
    int gen_id;
    char ID[100];

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();

    fcntl(fd, F_SETLK, &lock);
    read(fd, ID, sizeof(ID));
    gen_id = atoi(ID);
    gen_id += 1;
    bzero(ID, sizeof(ID));
    snprintf(ID, sizeof(ID), "%d", gen_id);
    lseek(fd, 0, SEEK_SET);
    write(fd, ID, sizeof(ID));
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);

    return gen_id;
}

void addUser(int role, int clientSocket)
{
    struct user newUser;
    struct flock lock, accountLock;

    strcpy(serverMessage, "Enter Username: ");
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
    strcpy(newUser.username, clientMessage);
    clearBuffers();

    strcpy(serverMessage, "Enter password: ");
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
    newUser.hashed_password = hash_password(clientMessage);
    clearBuffers();

    newUser.user_id = generate_user_id();
    newUser.role = role;
    newUser.status = 0;  // ACTIVE
    newUser.session = 1; // INACTIVE

    int user_fd = open(USER_DB, O_WRONLY | O_APPEND, 0666);
    if (user_fd < 0)
    {
        perror("Error opening user database");
        exit(1);
    }

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_END;
    lock.l_start = 0;
    lock.l_len = sizeof(struct user);
    lock.l_pid = getpid();

    while (fcntl(user_fd, F_SETLKW, &lock) == -1)
        ;

    if (write(user_fd, &newUser, sizeof(struct user)) < 0)
    {
        perror("Error writing to user database");
        close(user_fd);
        exit(1);
    }

    lock.l_type = F_UNLCK;
    if (fcntl(user_fd, F_SETLK, &lock) == -1)
    {
        perror("Error unlocking file");
    }

    if (role == 3)
    {
        int account_fd = open(ACCOUNT_DB, O_WRONLY | O_APPEND, 0666);
        struct account newAccount;
        newAccount.user_id = newUser.user_id;
        strcpy(newAccount.username, newUser.username);
        newAccount.balance = 0;

        accountLock.l_type = F_WRLCK;
        accountLock.l_whence = SEEK_END;
        accountLock.l_start = 0;
        accountLock.l_len = sizeof(struct account);
        accountLock.l_pid = getpid();

        while (fcntl(account_fd, F_SETLKW, &accountLock) == -1)
            ;

        if (write(account_fd, &newAccount, sizeof(struct account)) < 0)
        {
            perror("Error writing to account database");
            close(account_fd);
            exit(1);
        }
        accountLock.l_type = F_UNLCK;
        if (fcntl(account_fd, F_SETLK, &accountLock) == -1)
        {
            perror("Error unlocking file");
        }
        close(account_fd);
    }
    if (role == 1)
    {
        strcpy(serverMessage, "Employee added successfully!\n");
        send(clientSocket, serverMessage, strlen(serverMessage), 0);
    }
    else if (role == 3)
    {
        strcpy(serverMessage, "Employee added successfully!\n");
        send(clientSocket, serverMessage, strlen(serverMessage), 0);
    }
    recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();
    close(user_fd);
}

