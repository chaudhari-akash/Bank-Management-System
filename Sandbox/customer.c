#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>

#define USER_DB "user.db"
#define ACCOUNT_DB "account.db"
#define LOAN_DB "loan.db"
#define FEEDBACK_DB "feedback.db"
#define TRANSACTION_DB "transaction.db"

#include "../helper/structure.h"

// Function prototypes
int authenticate(struct user *loginUser);     // Done
void login(struct user *loginUser);           // Done
float view_balance(struct user *loginUser);   // Done
void deposit_money(struct user *loginUser);   // Done
void withdraw_money(struct user *loginUser);  // Done
void transfer_funds(struct user *loginUser);  // Done
void apply_loan(struct user *loginUser);      // Done
void change_password(struct user *loginUser); // Done
void add_feedback(struct user *loginUser);    // Done
void view_transaction_history(struct user *loginUser);
void logout();                                                       // Done
unsigned long hash_password(const char *password);                   // Done
void update_user_balance(struct account *userAccount, float amount); // Done
void append_transaction(struct transaction *trans);

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
                transfer_funds(loginUser);
                // printf("Transfer Funds");
                break;
            case 5:
                apply_loan(loginUser);
                // printf("Apply for a Loan");
                break;
            case 6:
                change_password(loginUser);
                // printf("Change Password");
                break;
            case 7:
                add_feedback(loginUser);
                // printf("Add Feedback");
                break;
            case 8:
                view_transaction_history(loginUser);
                // printf("View Transaction History");
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
            lock_record_read(fd, offset);
            balance = userAccount.balance;
            unlock_record_read(fd, offset);
        }
    }
    close(fd);

    return balance;
}

void deposit_money(struct user *loginUser)
{
    struct account userAccount;
    struct transaction trans;
    float deposit_amount;
    float total_balance;
    printf("Enter amount to deposit: ");
    scanf("%f", &deposit_amount);
    strcpy(userAccount.username, loginUser->username);
    userAccount.balance = view_balance(loginUser);

    if (deposit_amount > 0)
    {
        update_user_balance(&userAccount, deposit_amount);
        total_balance = view_balance(loginUser);

        strcpy(trans.username, loginUser->username);
        trans.amount = deposit_amount;
        trans.type = 1;
        trans.total_amount = total_balance;
        append_transaction(&trans);

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
    struct transaction trans;
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

        strcpy(trans.username, loginUser->username);
        trans.amount = withdraw_amount;
        trans.type = 0;
        trans.total_amount = account_balance;
        append_transaction(&trans);

        printf("Withdraw successful. New balance: %.2f\n", account_balance);
    }
    else
    {
        printf("Invalid amount!\n");
    }
}

void transfer_funds(struct user *loginUser)
{
    char recipient[50];
    struct transaction trans_1, trans_2;
    float amount;
    printf("Enter recipient username: ");
    scanf("%s", recipient);
    printf("Enter amount to transfer: ");
    scanf("%f", &amount);

    int sender_fd = open(ACCOUNT_DB, O_RDWR);
    int reciever_fd = open(ACCOUNT_DB, O_RDWR);

    off_t sender_offset, reciever_offset;
    struct account sender_account, reciever_account;

    printf("Finding...\n");

    while ((sender_offset = lseek(sender_fd, 0, SEEK_CUR)) != -1 && read(sender_fd, &sender_account, sizeof(struct account)) > 0)
    {
        printf("Sender UserName : %s\n", sender_account.username);
        printf("Sender Balance : %2f\n", sender_account.balance);

        if (strcmp(loginUser->username, sender_account.username) == 0)
        {
            break;
        }
    }

    printf("Sender UserName : %s\n", sender_account.username);
    printf("Sender Balance : %2f\n", sender_account.balance);

    while ((reciever_offset = lseek(reciever_fd, 0, SEEK_CUR)) != -1 && read(reciever_fd, &reciever_account, sizeof(struct account)) > 0)
    {
        if (strcmp(recipient, reciever_account.username) == 0)
        {
            break;
        }
    }

    printf("Reciever UserName : %s\n", reciever_account.username);
    printf("Reciever Balance : %2f\n", reciever_account.balance);

    struct flock sender_record_lock, reciever_record_lock;
    int sender_lock_status, reciever_lock_status;

    sender_record_lock.l_type = F_WRLCK;               // Write lock
    sender_record_lock.l_whence = SEEK_SET;            // Relative to the start of the file
    sender_record_lock.l_start = sender_offset;        // Start of the lock
    sender_record_lock.l_len = sizeof(struct account); // Length of the data (bytes)
    sender_record_lock.l_pid = getpid();

    sender_lock_status = fcntl(sender_fd, F_SETLK, &sender_record_lock);

    reciever_record_lock.l_type = F_WRLCK;               // Write lock
    reciever_record_lock.l_whence = SEEK_SET;            // Relative to the start of the file
    reciever_record_lock.l_start = reciever_offset;      // Start of the lock
    reciever_record_lock.l_len = sizeof(struct account); // Length of the data (bytes)
    reciever_record_lock.l_pid = getpid();

    reciever_lock_status = fcntl(reciever_fd, F_SETLK, &reciever_record_lock);

    if (amount > sender_account.balance)
    {
        printf("Insufficent Balance");
        close(sender_fd);
        close(reciever_fd);
    }
    else
    {
        lseek(sender_fd, sender_offset, SEEK_SET);
        float balance = sender_account.balance;
        float new_amount = balance - amount;
        sender_account.balance = new_amount;

        strcpy(trans_1.username, loginUser->username);
        trans_1.amount = amount;
        trans_1.type = 0;
        trans_1.total_amount = new_amount;
        append_transaction(&trans_1);

        write(sender_fd, &sender_account, sizeof(struct account));
        sender_record_lock.l_type = F_UNLCK;
        fcntl(sender_fd, F_SETLK, &sender_record_lock);
        close(sender_fd);

        // read(reciever_fd, &reciever_account, sizeof(struct account));
        lseek(reciever_fd, reciever_offset, SEEK_SET);
        reciever_account.balance += amount;

        strcpy(trans_2.username, loginUser->username);
        trans_2.amount = amount;
        trans_2.type = 1;
        trans_2.total_amount = reciever_account.balance;
        append_transaction(&trans_2);

        write(reciever_fd, &reciever_account, sizeof(struct account));
        reciever_record_lock.l_type = F_UNLCK;
        fcntl(reciever_fd, F_SETLK, &reciever_record_lock);
        close(reciever_fd);
    }
}

void apply_loan(struct user *loginUser)
{
    float loan_amount;
    printf("Enter the Loan Amount: ");
    scanf("%f", &loan_amount);

    int loan_fd = open(LOAN_DB, O_WRONLY | O_APPEND);
    if (loan_fd < 0)
    {
        perror("Error opening LOAN_DB");
        return;
    }

    struct loan loan_account;

    strcpy(loan_account.username, loginUser->username);
    loan_account.loan_amount = loan_amount;
    loan_account.status = 2;

    int status = write(loan_fd, &loan_account, sizeof(struct loan));
    if (status != sizeof(struct loan))
    {
        perror("Error writing to LOAN_DB");
        close(loan_fd);
        return;
    }
    close(loan_fd);

    printf("Loan Successfully Applied\n");
}

void change_password(struct user *loginUser)
{
    int fd = open(USER_DB, O_RDWR);
    if (fd < 0)
    {
        perror("Error opening USER_DB");
        return;
    }

    char username[50];
    char new_password[50];
    strcpy(username, loginUser->username);
    off_t offset;
    struct user find_User;

    printf("Enter the new password : ");
    scanf("%s", new_password);

    ssize_t read_status;
    while ((read_status = read(fd, &find_User, sizeof(struct user))) == sizeof(struct user))
    {
        if (strcmp(username, find_User.username) == 0)
        {

            loginUser->hashed_password = hash_password(new_password);
            offset = lseek(fd, -1 * sizeof(struct user), SEEK_CUR); // Move back to the correct position
            break;
        }
    }

    if (read_status == -1)
    {
        perror("Error reading USER_DB");
        close(fd);
        return;
    }

    if (offset != -1)
    {
        lseek(fd, offset, SEEK_SET);
        int write_status = write(fd, loginUser, sizeof(struct user));
        if (write_status == sizeof(struct user))
        {
            printf("Password Change Successful\n");
        }
        else
        {
            perror("Error While Changing Password");
        }
    }
    else
    {
        printf("User not found.\n");
    }

    close(fd);
}

void add_feedback(struct user *loginUser)
{
    char feedback[1000];
    int bytes_read, feedback_fd, write_status;
    struct feedback user_feedback;

    printf("Enter Feedback: ");
    fflush(stdout);
    bytes_read = read(STDIN_FILENO, feedback, sizeof(feedback));
    feedback[bytes_read - 1] = '\0';

    strcpy(user_feedback.username, loginUser->username);
    strcpy(user_feedback.feedback, feedback);

    feedback_fd = open(FEEDBACK_DB, O_WRONLY | O_APPEND);

    if (feedback_fd == -1)
    {
        perror("Error opening feedback database");
        return;
    }

    write_status = write(feedback_fd, &user_feedback, sizeof(struct feedback));

    if (write_status == -1)
    {
        perror("Error writing feedback");
    }
    close(feedback_fd);
}

void view_transaction_history(struct user *loginUser)
{
    int fd = open(TRANSACTION_DB, O_RDONLY);
    char debit[20] = "DEBIT";
    char credit[20] = "CREDIT";
    if (fd == -1)
    {
        perror("Error opening transaction database");
        return;
    }

    struct flock lock;
    lock.l_type = F_RDLCK;    // Read lock
    lock.l_whence = SEEK_SET; // Relative to the start of the file
    lock.l_start = 0;         // Lock the entire file
    lock.l_len = 0;           // 0 means until the end of the file
    lock.l_pid = getpid();    // Process ID

    while (fcntl(fd, F_SETLKW, &lock) == -1)
        ;

    struct transaction trans;

    while (read(fd, &trans, sizeof(struct transaction)) > 0)
    {
        if (strcmp(trans.username, loginUser->username) == 0)
        {
            if (trans.type == 0)
            {
                printf("%s : ", debit);
            }
            else
            {
                printf("%s : ", credit);
            }

            printf("%f : ", trans.amount);
            printf("Balance : ");
            printf("%f : ", trans.total_amount);
            printf("\n");
        }
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &lock);
    close(fd);
}

void append_transaction(struct transaction *trans)
{
    int fd = open(TRANSACTION_DB, O_WRONLY | O_APPEND);
    if (fd == -1)
    {
        perror("Error opening transaction database");
        return;
    }

    struct flock lock;
    lock.l_type = F_WRLCK;    // Write lock
    lock.l_whence = SEEK_SET; // Relative to the start of the file
    lock.l_start = 0;         // Lock the entire file
    lock.l_len = 0;           // 0 means until the end of the file
    lock.l_pid = getpid();    // Process ID

    while (fcntl(fd, F_SETLKW, &lock) == -1)
        ;

    if (write(fd, trans, sizeof(struct transaction)) == -1)
    {
        perror("Error writing transaction");
    }

    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &lock) == -1)
    {
        perror("Error unlocking file");
    }

    close(fd);
}