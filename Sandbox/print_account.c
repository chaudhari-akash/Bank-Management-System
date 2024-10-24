#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define ACCOUNT_DB "../db/account.db"

struct account
{
    int user_id;
    char username[50];
    float balance;
};

void print_account()
{
    int fd = open(ACCOUNT_DB, O_RDWR);
    struct account User;
    while (read(fd, &User, sizeof(struct account)) > 0)
    {
        printf("User ID: %d\n", User.user_id);
        printf("Username: %s\n", User.username);
        printf("Password: %f\n", User.balance);
        printf("Balance : %f\n", User.balance);
        printf("------------------------------------\n");
    }
    close(fd);
}

int main()
{
    print_account();
    return 0;
}