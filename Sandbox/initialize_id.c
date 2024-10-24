#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

enum Role
{
    ADMIN,    // 0
    EMPLOYEE, // 1
    MANAGER,  // 2
    CUSTOMER  // 3
};

struct user
{
    char username[50];
    unsigned long hashed_password; // Store hashed password as an unsigned long
    enum Role role;
};

// Simple hash function (DJB2) for hashing passwords
unsigned long hash_password(const char *password)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *password++))
    {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash;
}

int authenticate(struct user *loginUser)
{
    int fd = open("user_db.txt", O_RDONLY);
    if (fd < 0)
    {
        perror("Error opening user database");
        return -1;
    }

    struct user fileUser;
    unsigned long hashed = loginUser->hashed_password;

    while (read(fd, &fileUser, sizeof(struct user)) > 0)
    {
        if (strcmp(loginUser->username, fileUser.username) == 0 &&
            hashed == fileUser.hashed_password)
        {
            loginUser->role = fileUser.role;
            close(fd);
            return 1;
        }
    }
    close(fd);
    return 0;
}

void displayRole(enum Role role)
{
    switch (role)
    {
    case ADMIN:
        printf("Logged in as Admin\n");
        break;
    case EMPLOYEE:
        printf("Logged in as Employee\n");
        break;
    case MANAGER:
        printf("Logged in as Manager\n");
        break;
    case CUSTOMER:
        printf("Logged in as Customer\n");
        break;
    default:
        printf("Unknown Role\n");
    }
}

void addUser(const char *username, const char *password, enum Role role)
{
    struct user newUser;
    strncpy(newUser.username, username, sizeof(newUser.username) - 1);
    newUser.hashed_password = hash_password(password);
    newUser.role = role;

    int fd = open("user_db.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd < 0)
    {
        perror("Error opening user database");
        return;
    }

    write(fd, &newUser, sizeof(struct user));
    close(fd);
}

int main()
{
    struct user loginUser;
    printf("Enter username: ");
    scanf("%s", loginUser.username);
    printf("Enter password: ");
    char password[50];
    scanf("%s", password);
    loginUser.hashed_password = hash_password(password);

    int status = authenticate(&loginUser);

    if (status == 1)
    {
        printf("Login successful!\n");
        displayRole(loginUser.role);
    }
    else
    {
        printf("Invalid credentials!\n");
    }

    return 0;
}
