#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define USER_DB "../db/users.db"
#define ACCOUNT_DB "../db/account.db"
#define ID_FILE "../db/id.db"

struct user
{
    int user_id;
    char username[50];
    unsigned long hashed_password;
    int role;    // 0:Admin   1: Employee   2:Manager   3:Customer
    int session; // 0:In Use  1: Not in Use
    int status;  // 0:Active  1: Inactive
};

struct account
{
    int user_id;
    char username[50];
    float balance;
};

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

// Function to get role from input
int getRoleFromInput()
{
    int roleInput;
    printf("Select role:\n");
    printf("1. Admin\n");
    printf("2. Employee\n");
    printf("3. Manager\n");
    printf("4. Customer\n");
    printf("Enter choice (1-4): ");
    scanf("%d", &roleInput);

    switch (roleInput)
    {
    case 1:
        return 0;
    case 2:
        return 1;
    case 3:
        return 2;
    case 4:
        return 3;
    default:
        printf("Invalid role. Defaulting to Customer.\n");
        return 3;
    }
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

// Function to add a user to the database
void addUser(int role)
{
    struct user newUser;
    char username[50];
    char password[50];

    printf("Enter username: ");
    scanf("%s", username);
    strcpy(newUser.username, username);

    printf("Enter password: ");
    scanf("%s", password);
    newUser.user_id = generate_user_id();
    newUser.hashed_password = hash_password(password);
    newUser.role = role;
    newUser.status = 0;  // ACTIVE
    newUser.session = 1; // INACTIVE

    int user_fd = open(USER_DB, O_WRONLY | O_APPEND, 0666);
    if (user_fd < 0)
    {
        perror("Error opening user database");
        exit(1);
    }

    if (write(user_fd, &newUser, sizeof(struct user)) < 0)
    {
        perror("Error writing to user database");
        close(user_fd);
        exit(1);
    }

    if (role == 3)
    {
        int account_fd = open(ACCOUNT_DB, O_WRONLY | O_APPEND, 0666);
        struct account newAccount;
        newAccount.user_id = newUser.user_id;
        strcpy(newAccount.username, username);
        newAccount.balance = 0;
        if (write(account_fd, &newAccount, sizeof(struct account)) < 0)
        {
            perror("Error writing to account database");
            close(account_fd);
            exit(1);
        }
        close(account_fd);
    }
    printf("Your User ID : %d\n", newUser.user_id);
    printf("User added successfully!\n");
    close(user_fd);
}

int main()
{
    int new_role = getRoleFromInput();
    addUser(new_role);
    return 0;
}
