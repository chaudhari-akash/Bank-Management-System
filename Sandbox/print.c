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

void print()
{
    int fd = open(USER_DB, O_RDWR);
    struct user User;
    while (read(fd, &User, sizeof(struct user)) > 0)
    {
        printf("User ID: %d\n", User.user_id);
        printf("Username: %s\n", User.username);
         printf("Password: %ld\n", User.hashed_password);
        printf("Role: ");
        switch (User.role)
        {
        case 0:
            printf("Admin\n");
            break;
        case 1:
            printf("Employee\n");
            break;
        case 2:
            printf("Manager\n");
            break;
        case 3:
            printf("Customer\n");
            break;
        default:
            printf("Unknown Role\n");
        }
        printf("Session: %s\n", (User.session == 0) ? "In Use" : "Not in Use");
        printf("Status: %s\n", (User.status == 0) ? "Active" : "Inactive");
        printf("------------------------------------\n");
    }
}

void print_account()
{
    int fd = open(ACCOUNT_DB, O_RDWR);
    struct account User;
    while (read(fd, &User, sizeof(struct account)) > 0)
    {
        printf("User ID: %d\n", User.user_id);
        printf("Username: %s\n", User.username);
        printf("Password: %f\n", User.balance);
        printf("Balance : %f\n",User.balance);
        printf("------------------------------------\n");
    }
}


int main()
{
    print();
    // print_account();
    return 0;
}