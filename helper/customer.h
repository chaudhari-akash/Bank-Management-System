#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include <stdio.h>      /* printf(), perror() */
#include <stdlib.h>     /*  */
#include <unistd.h>     /* fork(), close(), read(), _exit() */
#include <string.h>     /* strncpy() */
#include <arpa/inet.h>  /* htons() */
#include <sys/socket.h> /* socket(), bind(), listen(), accept(), connect() */
#include <netinet/in.h>

#include "structure.h"

int authenticate(struct user *loginUser);
float view_balance(struct user *loginUser);
unsigned long hash_password(const char *password);
void login(struct user *loginUser, int connectionFileDescriptor);
void deposit_money(struct user *loginUser);
void withdraw_money(struct user *loginUser);
void transfer_funds(struct user *loginUser);
void apply_loan(struct user *loginUser);
void change_password(struct user *loginUser);
void add_feedback(struct user *loginUser);
void view_transaction_history(struct user *loginUser);
int update_user_balance(struct account *userAccount, float amount);
void append_transaction(struct transaction *trans);
void logout(struct user *loginUser, int connectionFileDescriptor);
int update_account_structure(int fd, off_t offset, float amount);

void login(struct user *loginUser, int connectionFileDescriptor)
{
    enum Role userRole = CUSTOMER;
    loginUser->role = userRole;
    char serverMessage[1000], clientMessage[1000], dummyBuffer[100];
    bzero(serverMessage, sizeof(serverMessage));
    bzero(clientMessage, sizeof(clientMessage));
    bzero(dummyBuffer, sizeof(dummyBuffer));
    strcpy(dummyBuffer, "This is Dummy Message!\n");

    strcat(serverMessage, "Enter username: ");
    send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
    recv(connectionFileDescriptor, clientMessage, sizeof(clientMessage), 0);
    strcpy(loginUser->username, clientMessage);
    bzero(serverMessage, sizeof(serverMessage));
    bzero(clientMessage, sizeof(clientMessage));

    strcat(serverMessage, "Enter password: ");
    send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
    recv(connectionFileDescriptor, clientMessage, sizeof(clientMessage), 0);

    loginUser->hashed_password = hash_password(clientMessage);
    bzero(serverMessage, sizeof(serverMessage));
    bzero(clientMessage, sizeof(clientMessage));
    bzero(dummyBuffer, sizeof(dummyBuffer));

    int status = 0;
    status = authenticate(loginUser);

    snprintf(serverMessage, sizeof(serverMessage), "%d", status);

    send(connectionFileDescriptor, &serverMessage, strlen(serverMessage), 0);
    recv(connectionFileDescriptor, dummyBuffer, sizeof(dummyBuffer), 0);

    int s2 = atoi(serverMessage);
    bzero(dummyBuffer, sizeof(dummyBuffer));
    bzero(serverMessage, sizeof(serverMessage));
    bzero(clientMessage, sizeof(clientMessage));

    if (s2 == 1)
    {
        while (1)
        {
            bzero(serverMessage, sizeof(serverMessage));
            bzero(clientMessage, sizeof(clientMessage));

            strcpy(serverMessage, "\nWelcome, ");
            strcat(serverMessage, loginUser->username);
            strcat(serverMessage, "! Choose an operation:\n");
            strcat(serverMessage, "1. View Account Balance\n");
            strcat(serverMessage, "2. Deposit Money\n");
            strcat(serverMessage, "3. Withdraw Money\n");
            strcat(serverMessage, "4. Transfer Funds\n");
            strcat(serverMessage, "5. Apply for a Loan\n");
            strcat(serverMessage, "6. Change Password\n");
            strcat(serverMessage, "7. Add Feedback\n");
            strcat(serverMessage, "8. View Transaction History\n");
            strcat(serverMessage, "9. Logout\n");
            strcat(serverMessage, "Enter Your Choice : \n");

            send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
            recv(connectionFileDescriptor, clientMessage, sizeof(clientMessage), 0);
            printf("String Message : %s\n", clientMessage);
            int c2 = atoi(clientMessage);
           
            bzero(serverMessage, sizeof(serverMessage));
            bzero(clientMessage, sizeof(clientMessage));
            printf("Integer Message : %d\n", c2);
           
            bzero(serverMessage, sizeof(serverMessage));
            bzero(clientMessage, sizeof(clientMessage));
            switch (c2)
            {
            case 1:

                float balance = view_balance(loginUser);
               
                bzero(serverMessage, sizeof(serverMessage));
                bzero(clientMessage, sizeof(clientMessage));
                sprintf(serverMessage, "Your account balance is: %.2f\n", balance);
                send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
                int re = recv(connectionFileDescriptor, clientMessage, sizeof(clientMessage), 0);
                // printf("%s\n",clientMessage);
                printf("%d",re);
                bzero(serverMessage, sizeof(serverMessage));
                bzero(clientMessage, sizeof(clientMessage));
                break;
            case 2:
                deposit_money(loginUser);
                break;
            case 3:
                withdraw_money(loginUser);
                break;
            case 4:
                transfer_funds(loginUser);
                break;
            case 5:
                apply_loan(loginUser);
                break;
            case 6:
                change_password(loginUser);
                break;
            case 7:
                add_feedback(loginUser);
                break;
            case 8:
                view_transaction_history(loginUser);
                break;
            case 9:
                
                bzero(serverMessage, sizeof(serverMessage));
                bzero(clientMessage, sizeof(clientMessage));
                logout(loginUser, connectionFileDescriptor);
               
                bzero(serverMessage, sizeof(serverMessage));
                bzero(clientMessage, sizeof(clientMessage));
                return;
            default:
                printf("Invalid choice!\n");
            }
        }
    }
    else
    {
        write(connectionFileDescriptor, "Invalid credentials!\n", strlen("Invalid credentials!\n"));
        // printf("Invalid credentials!\n");
    }
}

int authenticate(struct user *loginUser)
{
    struct user fileUser;
    int fd, write_bytes;

    fd = open(USER_DB, O_RDWR);
    if (fd == -1)
    {
        perror("Error opening user database");
        return 0;
    }

    lseek(fd, 0, SEEK_SET);

    while (read(fd, &fileUser, sizeof(struct user)) > 0)
    {
        if (
            strcmp(loginUser->username, fileUser.username) == 0 &&
            loginUser->hashed_password == fileUser.hashed_password &&
            loginUser->role == fileUser.role)
        {
            if (fileUser.status == 1)
            {
                fileUser.status = 0;
                lseek(fd, -1 * sizeof(struct user), SEEK_CUR);
                write_bytes = write(fd, &fileUser, sizeof(struct user));
                if (write_bytes == -1)
                {
                    perror("Error updating user status");
                    close(fd);
                    return 0;
                }

                close(fd);
                return 1;
            }
            else
            {
                close(fd);
                return 0;
            }
        }
    }
    close(fd);
    return 0;
}

void logout(struct user *loginUser, int connectionFileDescriptor)
{
    struct user fileUser;
    int fd, write_bytes;
    fd = open(USER_DB, O_RDWR);
    if (fd == -1)
    {
        perror("Error opening user database");
        return;
    }

    lseek(fd, 0, SEEK_SET);

    while (read(fd, &fileUser, sizeof(struct user)) > 0)
    {
        if (
            strcmp(loginUser->username, fileUser.username) == 0 &&
            loginUser->hashed_password == fileUser.hashed_password &&
            loginUser->role == fileUser.role)
        {

            fileUser.status = 1;
            lseek(fd, -1 * sizeof(struct user), SEEK_CUR);
            write_bytes = write(fd, &fileUser, sizeof(struct user));
            if (write_bytes < 0)
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
    send(connectionFileDescriptor, "Logging out...\n", strlen("Logging out...\n"), 0);
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
    float balance = -1;
    off_t offset;
    int fd;

    fd = open(ACCOUNT_DB, O_RDONLY);

    if (fd == -1)
    {
        perror("Error opening the account database");
        return balance;
    }

    while (read(fd, &userAccount, sizeof(struct account)) > 0)
    {
        if (strcmp(loginUser->username, userAccount.username) == 0)
        {
            offset = lseek(fd, -1 * sizeof(struct account), SEEK_CUR);
            lock_record_read(fd, offset);
            balance = userAccount.balance;
            unlock_record_read(fd, offset);
            break;
        }
    }

    close(fd);

    return balance;
}

void deposit_money(struct user *loginUser)
{
    struct account userAccount;
    struct transaction transactionRecord;
    float deposit_amount, total_balance;

    printf("Enter amount to deposit: ");
    scanf("%f", &deposit_amount);

    strcpy(userAccount.username, loginUser->username);
    userAccount.balance = view_balance(loginUser);

    if (deposit_amount > 0)
    {
        update_user_balance(&userAccount, deposit_amount);
        total_balance = view_balance(loginUser);

        strcpy(transactionRecord.username, loginUser->username);
        transactionRecord.amount = deposit_amount;
        transactionRecord.type = 1;
        transactionRecord.total_amount = total_balance;
        append_transaction(&transactionRecord);
        printf("Deposit successful. New balance: %.2f\n", total_balance);
    }
    else
    {
        printf("Invalid amount!\n");
    }
}

int update_user_balance(struct account *userAccount, float amount)
{
    struct account fileUser;
    off_t offset;
    int fd, update_structure_status;

    fd = open(ACCOUNT_DB, O_RDWR);
    if (fd < 0)
    {
        perror("Error opening user database");
        return 0;
    }

    while (read(fd, &fileUser, sizeof(struct account)) > 0)
    {
        if (strcmp(userAccount->username, fileUser.username) == 0)
        {
            offset = lseek(fd, -sizeof(struct account), SEEK_CUR);
            if (offset == -1)
            {
                perror("Error seeking the file to update the account");
                close(fd);
                return 0;
            }
            update_structure_status = update_account_structure(fd, offset, amount);
            break;
        }
    }
    close(fd);
    if (update_structure_status == 1)
    {
        return 1;
    }
    return 0;
}

int update_account_structure(int fd, off_t offset, float amount)
{
    struct flock lock;
    struct account fileUser;
    int lock_status, read_bytes, write_bytes, lseek_status, unlock_status;
    float total_amount;
    int success = 0;

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = offset;
    lock.l_len = sizeof(struct account);
    lock.l_pid = getpid();

    lock_status = fcntl(fd, F_SETLK, &lock);
    if (lock_status == -1)
    {
        perror("Error locking file");
        return 0;
    }

    lseek_status = lseek(fd, offset, SEEK_SET);
    if (lseek_status == -1)
    {
        perror("Error seeking to position");
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        return 0;
    }

    read_bytes = read(fd, &fileUser, sizeof(struct account));
    if (read_bytes < 0)
    {
        perror("Error reading account data");
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        return 0;
    }

    total_amount = fileUser.balance + amount;

    if (total_amount >= 0)
    {
        fileUser.balance += amount;
        success = 1;
    }
    else
    {
        printf("Not Enough Balance in your Account!\n");
    }

    lseek(fd, -1 * sizeof(struct account), SEEK_CUR);

    write_bytes = write(fd, &fileUser, sizeof(struct account));
    if (write_bytes < 0)
    {
        perror("Error writing to file");
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        return 0;
    }

    lock.l_type = F_UNLCK;
    unlock_status = fcntl(fd, F_SETLK, &lock);

    if (unlock_status == -1)
    {
        perror("Error unlocking file");
    }
    return success;
}

void withdraw_money(struct user *loginUser)
{
    struct account userAccount;
    struct transaction transactionRecord;
    float total_balance, withdraw_amount;
    int update_status;

    printf("Enter amount to withdraw: ");
    scanf("%f", &withdraw_amount);

    if (withdraw_amount > 0)
    {
        strcpy(userAccount.username, loginUser->username);
        userAccount.balance = view_balance(loginUser);

        update_status = update_user_balance(&userAccount, -withdraw_amount);
        if (update_status == 1)
        {
            total_balance = view_balance(loginUser);
            strcpy(transactionRecord.username, loginUser->username);
            transactionRecord.amount = withdraw_amount;
            transactionRecord.type = 0;
            transactionRecord.total_amount = total_balance;

            append_transaction(&transactionRecord);

            printf("Withdraw successful. New balance: %.2f\n", total_balance);
        }
        else
        {
            total_balance = view_balance(loginUser);
            printf("Withdraw failed. Balance: %.2f\n", total_balance);
        }
    }
    else
    {
        printf("Invalid amount!\n");
    }
}

void transfer_funds(struct user *loginUser)
{
    char recipient[50];
    struct transaction senderTransactionRecord, recieverTransactionRecord;
    struct account senderAccount, recieverAccount;
    off_t sender_offset, reciever_offset;
    struct flock sender_record_lock, reciever_record_lock;
    int sender_lock_status, reciever_lock_status;
    int sender_fd, reciever_fd;
    float amount;

    printf("Enter recipient username: ");
    scanf("%s", recipient);
    printf("Enter amount to transfer: ");
    scanf("%f", &amount);

    sender_fd = open(ACCOUNT_DB, O_RDWR);
    reciever_fd = open(ACCOUNT_DB, O_RDWR);

    while (read(sender_fd, &senderAccount, sizeof(struct account)) > 0)
    {
        if (strcmp(loginUser->username, senderAccount.username) == 0)
        {
            sender_offset = lseek(sender_fd, -1 * sizeof(struct account), SEEK_CUR);
            break;
        }
    }

    sender_record_lock.l_type = F_WRLCK;
    sender_record_lock.l_whence = SEEK_SET;
    sender_record_lock.l_start = sender_offset;
    sender_record_lock.l_len = sizeof(struct account);
    sender_record_lock.l_pid = getpid();

    sender_lock_status = fcntl(sender_fd, F_SETLK, &sender_record_lock);

    lseek(sender_fd, sender_offset, SEEK_SET);
    read(sender_fd, &senderAccount, sizeof(struct account));

    if (amount > senderAccount.balance)
    {
        printf("Insufficent Balance");
        close(sender_fd);
        close(reciever_fd);
    }
    else
    {
        lseek(sender_fd, sender_offset, SEEK_SET);
        float new_amount = senderAccount.balance - amount;
        senderAccount.balance = new_amount;

        strcpy(senderTransactionRecord.username, loginUser->username);
        senderTransactionRecord.amount = amount;
        senderTransactionRecord.type = 0;
        senderTransactionRecord.total_amount = senderAccount.balance;
        append_transaction(&senderTransactionRecord);

        lseek(sender_fd, sender_offset, SEEK_SET);
        write(sender_fd, &senderAccount, sizeof(struct account));
        sender_record_lock.l_type = F_UNLCK;
        fcntl(sender_fd, F_SETLK, &sender_record_lock);
        close(sender_fd);

        while (read(reciever_fd, &recieverAccount, sizeof(struct account)) > 0)
        {
            if (strcmp(recipient, recieverAccount.username) == 0)
            {
                reciever_offset = lseek(reciever_fd, -1 * sizeof(struct account), SEEK_CUR);
                break;
            }
        }

        reciever_record_lock.l_type = F_WRLCK;
        reciever_record_lock.l_whence = SEEK_SET;
        reciever_record_lock.l_start = reciever_offset;
        reciever_record_lock.l_len = sizeof(struct account);
        reciever_record_lock.l_pid = getpid();

        reciever_lock_status = fcntl(reciever_fd, F_SETLK, &reciever_record_lock);

        lseek(reciever_fd, reciever_offset, SEEK_SET);
        read(reciever_fd, &recieverAccount, sizeof(struct account));
        recieverAccount.balance += amount;

        strcpy(recieverTransactionRecord.username, recipient);
        recieverTransactionRecord.amount = amount;
        recieverTransactionRecord.type = 1;
        recieverTransactionRecord.total_amount = recieverAccount.balance;
        append_transaction(&recieverTransactionRecord);

        lseek(reciever_fd, reciever_offset, SEEK_SET);
        write(reciever_fd, &recieverAccount, sizeof(struct account));
        reciever_record_lock.l_type = F_UNLCK;
        fcntl(reciever_fd, F_SETLK, &reciever_record_lock);
        close(reciever_fd);
    }
}

void apply_loan(struct user *loginUser)
{
    float loan_amount;
    struct loan loanAccount;
    struct flock lock;
    int lock_status, write_bytes;
    printf("Enter the Loan Amount: ");
    scanf("%f", &loan_amount);

    int loan_fd = open(LOAN_DB, O_WRONLY | O_APPEND);
    if (loan_fd < 0)
    {
        perror("Error opening LOAN_DB");
        return;
    }

    strcpy(loanAccount.username, loginUser->username);
    loanAccount.loan_amount = loan_amount;
    loanAccount.status = 2;

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_END;
    lock.l_start = 0;
    lock.l_len = sizeof(struct account);
    lock.l_pid = getpid();

    lock_status = fcntl(loan_fd, F_SETLK, &lock);
    if (lock_status == -1)
    {
        perror("Error locking the loan database");
        close(loan_fd);
        return;
    }
    lseek(loan_fd, 0, SEEK_END);

    write_bytes = write(loan_fd, &loanAccount, sizeof(struct loan));
    if (write_bytes < 0)
    {
        perror("Error writing to loan database");
        lock.l_type = F_UNLCK;
        fcntl(loan_fd, F_SETLK, &lock);
        close(loan_fd);
        return;
    }

    lock.l_type = F_UNLCK;
    fcntl(loan_fd, F_SETLK, &lock);

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
    struct flock lock;

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
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = offset;
        lock.l_len = sizeof(struct user);
        lock.l_pid = getpid();

        fcntl(fd, F_SETLK, &lock);

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
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
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
    struct flock lock;

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

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_END;
    lock.l_start = 0;
    lock.l_len = sizeof(struct feedback);
    lock.l_pid = getpid();

    fcntl(feedback_fd, F_SETLK, &lock);

    write_status = write(feedback_fd, &user_feedback, sizeof(struct feedback));

    if (write_status == -1)
    {
        perror("Error writing feedback");
    }

    lock.l_type = F_UNLCK;
    fcntl(feedback_fd, F_SETLK, &lock);
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
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_END;
    lock.l_start = 0;
    lock.l_len = sizeof(struct transaction);
    lock.l_pid = getpid();

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