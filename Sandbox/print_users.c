#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define USER_DB "../db/users.db"

struct user
{
    int user_id;
    char username[50];
    unsigned long hashed_password;
    int role;    // 0:Admin   1: Employee   2:Manager   3:Customer
    int session; // 0:In Use  1: Not in Use
    int status;  // 0:Active  1: Inactive
};

void print_users()
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
    close(fd);
}

int main()
{
    print_users();

    return 0;
}