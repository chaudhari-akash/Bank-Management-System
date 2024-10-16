#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define USER_DB "user.db"
#define ACCOUNT_DB "account.db"

// Enum to represent the role of users
enum Role
{
    ADMIN,    // 0
    EMPLOYEE, // 1
    MANAGER,  // 2
    CUSTOMER  // 3
};

// Structure to represent a user
struct user
{
    char username[50];
    unsigned long hashed_password;
    enum Role role;
    int status; // 0:Active  1:Inactive
};

struct account
{
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
enum Role getRoleFromInput()
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
        return ADMIN;
    case 2:
        return EMPLOYEE;
    case 3:
        return MANAGER;
    case 4:
        return CUSTOMER;
    default:
        printf("Invalid role. Defaulting to Customer.\n");
        return CUSTOMER;
    }
}

// Function to add a user to the database
void addUser()
{
    struct user newUser;
    struct account newAccount;
    char username1[50];

    // Get username and password
    printf("Enter username: ");
    scanf("%s", username1);
    strcpy(newUser.username, username1);
    strcpy(newAccount.username, username1);

    newAccount.balance = 0;
    printf("Enter password: ");
    char pass[50];
    scanf("%s", pass);
    newUser.hashed_password = hash_password(pass);
    // Get role
    newUser.role = getRoleFromInput();
    newUser.status = 1;

    // Open the user database file
    int fd = open(USER_DB, O_WRONLY | O_APPEND | O_CREAT, 0666);
    int fd2 = open(ACCOUNT_DB, O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (fd < 0)
    {
        perror("Error opening user database");
        exit(1);
    }

    // Write user data to the file
    if (write(fd, &newUser, sizeof(struct user)) < 0)
    {
        perror("Error writing to user database");
        close(fd);
        exit(1);
    }

    printf("User added successfully!\n");

    if (write(fd2, &newAccount, sizeof(struct account)) < 0)
    {
        perror("Error writing to account database");
        close(fd2);
        exit(1);
    }

    printf("Account added successfully!\n");

    // Close the file
    close(fd);
    close(fd2);
}

int main()
{
    addUser();
    return 0;
}
