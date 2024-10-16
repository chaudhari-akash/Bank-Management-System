#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>

#define USER_DB "user.db"
#define ACCOUNT_DB "account.db"

#include "../helper/structure.h"

// Function prototypes
int authenticate(struct user *loginUser);   // Done
void login(struct user *loginUser);         // Done
float view_balance(struct user *loginUser); // Done
void deposit_money(struct user *loginUser); // Done
void withdraw_money(struct user *loginUser); //Done
void transfer_funds(struct user *loginUser);
void apply_loan(struct user *loginUser);
void change_password(struct user *loginUser);
void add_feedback(struct user *loginUser);
void view_transaction_history(struct user *loginUser);
void logout();                                                       // Done
unsigned long hash_password(const char *password);                   // Done
void update_user_balance(struct account *userAccount, float amount); // Done

// Main function
int main()
{
    int choice;
    struct user loginUser;
    struct account userAccount;

    while (1)
    {
        printf("Banking System\n");
        printf("1. Login\n");
        printf("2. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            login(&loginUser);
            break;
        case 2:
            printf("Exiting...\n");
            exit(0);
        default:
            printf("Invalid choice!\n");
        }
    }
    return 0;
}

// Function definitions

void login(struct user *loginUser)
{
    char password[50];
    enum Role userRole = CUSTOMER;
    int choice;

    printf("Enter username: ");
    scanf("%s", loginUser->username);
    printf("Enter password: ");
    scanf("%s", password);

    loginUser->hashed_password = hash_password(password);
    loginUser->role = userRole;

    int status = authenticate(loginUser);

    if (status == 1)
    {
        while (1)
        {
            printf("\nWelcome, %s! Choose an operation:\n", loginUser->username);
            printf("1. View Account Balance\n");
            printf("2. Deposit Money\n");
            printf("3. Withdraw Money\n");
            printf("4. Transfer Funds\n");
            printf("5. Apply for a Loan\n");
            printf("6. Change Password\n");
            printf("7. Add Feedback\n");
            printf("8. View Transaction History\n");
            printf("9. Logout\n");
            printf("Enter your choice: ");
            scanf("%d", &choice);

            switch (choice)
            {
            case 1:
                float balance = view_balance(loginUser);
                printf("Your account balance is: %.2f\n", balance);
                break;
            case 2:
                deposit_money(loginUser);
                break;
            case 3:
                withdraw_money(loginUser);
                // printf("Withdraw Money");
                break;
            case 4:
                // transfer_funds(loginUser);
                printf("Transfer Funds");
                break;
            case 5:
                // apply_loan(loginUser);
                printf("Apply for a Loan");
                break;
            case 6:
                // change_password(loginUser);
                printf("Change Password");
                break;
            case 7:
                // add_feedback(loginUser);
                printf("Add Feedback");
                break;
            case 8:
                // view_transaction_history(loginUser);
                printf("View Transaction History");
                break;
            case 9:
                logout(loginUser);
                // printf("Logout");
                return;
            default:
                printf("Invalid choice!\n");
            }
        }
    }
    else
    {
        printf("Invalid credentials!\n");
    }
}

int authenticate(struct user *loginUser)
{
    struct user fileUser;
    off_t offset;
    int fd = open(USER_DB, O_RDWR);

    if (fd < 0)
    {
        perror("Error opening user database");
        return 0;
    }

    while ((offset = lseek(fd, 0, SEEK_CUR)) != -1 && read(fd, &fileUser, sizeof(struct user)) > 0)
    {
        if (
            strcmp(loginUser->username, fileUser.username) == 0 &&
            loginUser->hashed_password == fileUser.hashed_password &&
            loginUser->role == fileUser.role)
        {
            if (fileUser.status == 1)
            {
                fileUser.status = 0;
                lseek(fd, offset, SEEK_SET);
                if (write(fd, &fileUser, sizeof(struct user)) < 0)
                {
                    perror("Error updating user status");
                    return 0; // Failed to update status
                }

                close(fd);
                return 1; // Authentication successful
            }
            else
            {
                close(fd);
                return 0; // Authentication failed
            }
        }
    }
    close(fd);
    return 0; // Authentication failed
}

void logout(struct user *loginUser)
{
    struct user fileUser;
    off_t offset;
    int fd = open(USER_DB, O_RDWR);

    while ((offset = lseek(fd, 0, SEEK_CUR)) != -1 && read(fd, &fileUser, sizeof(struct user)) > 0)
    {
        if (
            strcmp(loginUser->username, fileUser.username) == 0 &&
            loginUser->hashed_password == fileUser.hashed_password &&
            loginUser->role == fileUser.role)
        {

            fileUser.status = 1;
            lseek(fd, offset, SEEK_SET);
            int write_status = write(fd, &fileUser, sizeof(struct user));
            if (write_status < 0)
            {
                perror("Error updating user status");
            }
            else
            {
                break;
            }
        }
    }
    close(fd);
    printf("Logging out...\n");
}

unsigned long hash_password(const char *password)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *password++))
    {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

float view_balance(struct user *loginUser)
{
    struct account userAccount;
    float balance;

    int fd = open(ACCOUNT_DB, O_RDONLY);
    off_t offset;
    while ((offset = lseek(fd, 0, SEEK_CUR)) != -1 && read(fd, &userAccount, sizeof(struct account)) > 0)
    {
        if (strcmp(loginUser->username, userAccount.username) == 0)
        {
            // lock_record_read(fd, offset);
            balance = userAccount.balance;
            // unlock_record_read(fd);
        }
    }
    close(fd);

    return balance;
}

void deposit_money(struct user *loginUser)
{
    struct account userAccount;
    float deposit_amount;
    float total_balance;
    printf("Enter amount to deposit: ");
    scanf("%f", &deposit_amount);
    strcpy(userAccount.username , loginUser->username);
    userAccount.balance = view_balance(loginUser);

    if (deposit_amount > 0)
    {
        update_user_balance(&userAccount, deposit_amount);
        total_balance = view_balance(loginUser);
        printf("Deposit successful. New balance: %.2f\n", total_balance);
    }
    else
    {
        printf("Invalid amount!\n");
    }
}

void update_user_balance(struct account *userAccount, float amount)
{
    int fd = open(ACCOUNT_DB, O_RDWR);
    if (fd < 0)
    {
        perror("Error opening user database");
        return;
    }

    struct account fileUser;
    off_t offset;
    while ((offset = lseek(fd, 0, SEEK_CUR)) != -1 && read(fd, &fileUser, sizeof(struct account)) > 0)
    {
        if (strcmp(userAccount->username, fileUser.username) == 0)
        {

            update_account_structure(fd, offset, userAccount, amount);
            break;
        }
    }
    close(fd);
}

void withdraw_money(struct user *loginUser)
{
    struct account userAccount;
    float account_balance;
    float withdraw_amount;
    printf("Enter amount to withdraw: ");
    scanf("%f", &withdraw_amount);

    if (withdraw_amount > 0)
    {
        account_balance = view_balance(loginUser);
        userAccount.balance = account_balance;
        strcpy(userAccount.username, loginUser->username);
        update_user_balance(&userAccount, -withdraw_amount);
        account_balance = view_balance(loginUser);

        printf("Deposit successful. New balance: %.2f\n", account_balance);
    }
    else
    {
        printf("Invalid amount!\n");
    }
}

// void transfer_funds(struct user *loginUser)
// {
//     char recipient[50];
//     float amount;
//     printf("Enter recipient username: ");
//     scanf("%s", recipient);
//     printf("Enter amount to transfer: ");
//     scanf("%f", &amount);

//     // Logic for transferring funds between accounts (requires opening the database again)
//     // Ensure proper file locking when updating multiple records.
// }

// void apply_loan(struct user *loginUser)
// {
//     printf("Applying for a loan (details to be implemented)\n");
// }

// void change_password(struct user *loginUser)
// {
//     printf("Changing password (details to be implemented)\n");
// }

// void add_feedback(struct user *loginUser)
// {
//     printf("Adding feedback (details to be implemented)\n");
// }

// void view_transaction_history(struct user *loginUser)
// {
//     printf("Viewing transaction history (details to be implemented)\n");
// }

// void logout()
// {
//     printf("Logging out...\n");
// }

// void lock_file(int fd)
// {
//     if (flock(fd, LOCK_EX) < 0)
//     {
//         perror("File lock error");
//         exit(1);
//     }
// }

// void unlock_file(int fd)
// {
//     if (flock(fd, LOCK_UN) < 0)
//     {
//         perror("File unlock error");
//         exit(1);
//     }
// }

// void update_user_balance(struct user *user)
// {
//     int fd = open(USER_DB, O_RDWR);
//     if (fd < 0)
//     {
//         perror("Error opening user database");
//         return;
//     }

//     struct user fileUser;
//     while (read(fd, &fileUser, sizeof(struct user)) > 0)
//     {
//         if (strcmp(user->username, fileUser.username) == 0)
//         {
//             lock_file(fd);
//             fileUser.balance = user->balance;
//             lseek(fd, -sizeof(struct user), SEEK_CUR);
//             write(fd, &fileUser, sizeof(struct user));
//             unlock_file(fd);
//             break;
//         }
//     }
//     close(fd);
// }
