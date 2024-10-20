#include <stdio.h>      /* printf(), perror() */
#include <stdlib.h>     /*  */
#include <unistd.h>     /* fork(), close(), read(), _exit() */
#include <string.h>     /* strncpy() */
#include <arpa/inet.h>  /* htons() */
#include <sys/socket.h> /* send(), recv() */
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/file.h>

#include "structure.h"

int authenticate(struct user *loginUser);
float view_balance(struct user *loginUser);
unsigned long hash_password(const char *password);
int update_user_balance(struct account *userAccount, float amount);
int update_account_structure(int fd, off_t offset, float amount);
void login(struct user *loginUser, int connectionFileDescriptor, int role_int);
void deposit_money(struct user *loginUser, int connectionFileDescriptor);
void withdraw_money(struct user *loginUser, int connectionFileDescriptor);
void transfer_funds(struct user *loginUser, int connectionFileDescriptor);
void apply_loan(struct user *loginUser, int connectionFileDescriptor);
void change_password(struct user *loginUser, int connectionFileDescriptor);
void add_feedback(struct user *loginUser, int connectionFileDescriptor);
void view_transaction_history(struct user *loginUser, int connectionFileDescriptor);
void append_transaction(struct transaction *trans);
void logout(struct user *loginUser, int connectionFileDescriptor);
void print_loan_applications(struct user *loginUser, int connectionFileDescriptor);

char serverMessage[1000], clientMessage[1000], dummyBuffer[100];

void clearBuffers()
{
    bzero(serverMessage, sizeof(serverMessage));
    bzero(clientMessage, sizeof(clientMessage));
    bzero(dummyBuffer, sizeof(dummyBuffer));
}

void login(struct user *loginUser, int connectionFileDescriptor, int role_int)
{
    enum Role userRole;
    if (role_int == 0)
    {
        userRole = ADMIN;
    }
    else if (role_int == 1)
    {
        userRole = EMPLOYEE;
    }
    else if (role_int == 2)
    {
        userRole = MANAGER;
    }
    else if (role_int == 3)
    {
        userRole = CUSTOMER;
    }

    loginUser->role = userRole;

    strcat(serverMessage, "Enter username: ");
    send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
    recv(connectionFileDescriptor, clientMessage, sizeof(clientMessage), 0);
    strcpy(loginUser->username, clientMessage);
    clearBuffers();

    strcat(serverMessage, "Enter password: ");
    send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
    recv(connectionFileDescriptor, clientMessage, sizeof(clientMessage), 0);
    loginUser->hashed_password = hash_password(clientMessage);
    clearBuffers();

    int choice = authenticate(loginUser);

    snprintf(serverMessage, sizeof(serverMessage), "%d", choice);
    send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
    recv(connectionFileDescriptor, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();

    if (choice == 1)
    {
        while (1)
        {
            clearBuffers();
            strcpy(serverMessage, "\nWelcome, ");
            strcat(serverMessage, loginUser->username);
            strcat(serverMessage, "! Choose an operation:\n");
            strcat(serverMessage, "1. View Account Balance\n");
            strcat(serverMessage, "2. Deposit Money\n");
            strcat(serverMessage, "3. Withdraw Money\n");
            strcat(serverMessage, "4. Transfer Funds\n");
            strcat(serverMessage, "5. Apply for a Loan\n");
            strcat(serverMessage, "6. View Loan Applications\n");
            strcat(serverMessage, "7. Change Password\n");
            strcat(serverMessage, "8. Add Feedback\n");
            strcat(serverMessage, "9. View Transaction History\n");
            strcat(serverMessage, "10. Logout\n");
            strcat(serverMessage, "Enter Your Choice : ");

            send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
            recv(connectionFileDescriptor, clientMessage, sizeof(clientMessage), 0);
            int choice_2 = atoi(clientMessage);
            clearBuffers();

            switch (choice_2)
            {
            case 1:

                float balance = view_balance(loginUser);
                sprintf(serverMessage, "Your account balance is: %.2f\n", balance);
                send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
                recv(connectionFileDescriptor, dummyBuffer, sizeof(dummyBuffer), 0);
                clearBuffers();

                break;
            case 2:

                deposit_money(loginUser, connectionFileDescriptor);

                break;
            case 3:

                withdraw_money(loginUser, connectionFileDescriptor);

                break;
            case 4:

                transfer_funds(loginUser, connectionFileDescriptor);

                break;
            case 5:

                apply_loan(loginUser, connectionFileDescriptor);

                break;
            case 6:

                print_loan_applications(loginUser, connectionFileDescriptor);

                break;
            case 7:

                change_password(loginUser, connectionFileDescriptor);

                break;
            case 8:

                add_feedback(loginUser, connectionFileDescriptor);

                break;
            case 9:

                view_transaction_history(loginUser, connectionFileDescriptor);

                break;
            case 10:

                logout(loginUser, connectionFileDescriptor);
                recv(connectionFileDescriptor, dummyBuffer, sizeof(dummyBuffer), 0);
                clearBuffers();
                return;

            default:

                send(connectionFileDescriptor, "Invalid choice!\n", strlen("Invalid choice!\n"), 0);
                recv(connectionFileDescriptor, dummyBuffer, sizeof(dummyBuffer), 0);
                clearBuffers();
            }
        }
    }
    else
    {
        send(connectionFileDescriptor, "Invalid credentials!\n", strlen("Invalid credentials!\n"), 0);
        recv(connectionFileDescriptor, dummyBuffer, sizeof(dummyBuffer), 0);
        clearBuffers();
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

void deposit_money(struct user *loginUser, int connectionFileDescriptor)
{
    struct account userAccount;
    struct transaction transactionRecord;
    float deposit_amount, total_balance;

    clearBuffers();
    strcpy(serverMessage, "Enter amount to deposit: ");
    send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
    recv(connectionFileDescriptor, clientMessage, sizeof(clientMessage), 0);
    deposit_amount = atof(clientMessage);
    clearBuffers();

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

        snprintf(serverMessage, sizeof(serverMessage), "Deposit successful. New balance: %.2f\n", total_balance);
        send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
    }
    else
    {
        strcpy(serverMessage, "Invalid amount!\n");
        send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
    }
    recv(connectionFileDescriptor, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();
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

void withdraw_money(struct user *loginUser, int connectionFileDescriptor)
{
    struct account userAccount;
    struct transaction transactionRecord;
    float total_balance, withdraw_amount;
    int update_status;

    clearBuffers();
    strcpy(serverMessage, "Enter amount to withdraw: ");
    send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
    recv(connectionFileDescriptor, clientMessage, sizeof(clientMessage), 0);
    withdraw_amount = atof(clientMessage);
    clearBuffers();

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

            snprintf(serverMessage, sizeof(serverMessage), "Withdraw successful!\nNew balance: %.2f\n", total_balance);
            send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
        }
        else
        {
            total_balance = view_balance(loginUser);
            snprintf(serverMessage, sizeof(serverMessage), "Insufficient Balance!\nWithdraw failed.\nBalance: %.2f\n", total_balance);
            send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
        }
    }
    else
    {
        snprintf(serverMessage, sizeof(serverMessage), "Invalid amount!\n");
        send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
    }
    recv(connectionFileDescriptor, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();
}

void transfer_funds(struct user *loginUser, int connectionFileDescriptor)
{
    char recipient[50];
    struct transaction senderTransactionRecord, recieverTransactionRecord;
    struct account senderAccount, recieverAccount;
    off_t sender_offset, reciever_offset;
    struct flock sender_record_lock, reciever_record_lock;
    int sender_lock_status, reciever_lock_status;
    int sender_fd, reciever_fd;
    float amount;

    strcpy(serverMessage, "Enter recipient username: ");
    send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
    recv(connectionFileDescriptor, clientMessage, sizeof(clientMessage), 0);
    strcpy(recipient, clientMessage);
    clearBuffers();

    strcpy(serverMessage, "Enter amount to transfer: ");
    send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
    recv(connectionFileDescriptor, clientMessage, sizeof(clientMessage), 0);
    amount = atof(clientMessage);
    clearBuffers();

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
        snprintf(serverMessage, sizeof(serverMessage), "Insufficent Balance\nTransfer Unsuccessfull!\n");
        send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
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

        snprintf(serverMessage, sizeof(serverMessage), "Transfer Successfull!\nNew Balance : %.2f\n", view_balance(loginUser));
        send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
    }
    recv(connectionFileDescriptor, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();
}

void apply_loan(struct user *loginUser, int connectionFileDescriptor)
{
    float loan_amount;
    struct loan loanAccount;
    struct flock lock;
    int lock_status, write_bytes;

    // printf("Enter the Loan Amount: ");
    // scanf("%f", &loan_amount);

    strcpy(serverMessage, "Enter the Loan Amount: ");
    send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
    recv(connectionFileDescriptor, clientMessage, sizeof(clientMessage), 0);
    loan_amount = atof(clientMessage);
    clearBuffers();

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

    strcpy(serverMessage, "Loan Successfully Applied\n");
    send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
    recv(connectionFileDescriptor, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();
}

void change_password(struct user *loginUser, int connectionFileDescriptor)
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

    strcpy(serverMessage, "Enter the new password : ");
    send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
    recv(connectionFileDescriptor, clientMessage, sizeof(clientMessage), 0);
    strcpy(new_password, clientMessage);
    clearBuffers();

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
            strcpy(serverMessage, "Password Change Successful\n");
            send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
        }
        else
        {
            perror("Error While Changing Password");
            strcpy(serverMessage, "Password Change Unsuccessful\n");
            send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
        }
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
    }
    recv(connectionFileDescriptor, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();
    close(fd);
}

void add_feedback(struct user *loginUser, int connectionFileDescriptor)
{
    char feedback[1000];
    int bytes_read, feedback_fd, write_status;
    struct feedback user_feedback;
    struct flock lock;

    strcpy(serverMessage, "Enter Feedback: ");
    send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
    recv(connectionFileDescriptor, clientMessage, sizeof(clientMessage), 0);
    strcpy(feedback, clientMessage);
    clearBuffers();

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

    strcpy(serverMessage, "Feedback Submitted!\n");
    send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
    recv(connectionFileDescriptor, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();

    lock.l_type = F_UNLCK;
    fcntl(feedback_fd, F_SETLK, &lock);
    close(feedback_fd);
}

void view_transaction_history(struct user *loginUser, int connectionFileDescriptor)
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

    strcpy(serverMessage, "\t\tYour Transactions\n");

    while (read(fd, &trans, sizeof(struct transaction)) > 0)
    {

        if (strcmp(trans.username, loginUser->username) == 0)
        {
            if (trans.type == 0)
            {
                // snprintf(serverMessage, sizeof(serverMessage), "%s : ", debit);
                strcat(serverMessage, debit);
            }
            else
            {
                // snprintf(serverMessage, sizeof(serverMessage), "%s : ", credit);
                strcat(serverMessage, credit);
            }

            // printf("%f : ", trans.amount);
            // printf("Balance : ");
            // printf("%f : ", trans.total_amount);
            // printf("\n");
            char str[20];
            sprintf(str, "%.2f", trans.amount);
            strcat(serverMessage, str);
            strcat(serverMessage, ": Balance : ");
            char str1[20];
            sprintf(str1, "%.2f\n", trans.total_amount);
            strcat(serverMessage, str1);
        }
    }

    send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
    recv(connectionFileDescriptor, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();

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

void print_loan_applications(struct user *loginUser, int connectionFileDescriptor)
{
    struct loan loanApplications;
    int fd = open(LOAN_DB, O_RDONLY);
    if (fd < 0)
    {
        perror("Error opening feedback database");
        return;
    }

    strcpy(serverMessage, "Loan Applications:\n");
    while (read(fd, &loanApplications, sizeof(struct loan)) > 0)
    {
        if (strcmp(loanApplications.username, loginUser->username) == 0)
        {
            char loanAmount[20];
            char loanStatus[20];
            if (loanApplications.status == 0)
            {
                strcpy(loanStatus, "Accepted\n");
            }
            else if (loanApplications.status == 1)
            {
                strcpy(loanStatus, "Rejected\n");
            }
            else if (loanApplications.status == 2)
            {
                strcpy(loanStatus, "Pending\n");
            }
            snprintf(loanAmount, sizeof(loanAmount), "%.2f", loanApplications.loan_amount);
            strcat(serverMessage, loanAmount);
            strcat(serverMessage, " : ");
            strcat(serverMessage, loanStatus);
        }
    }
    send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
    recv(connectionFileDescriptor, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();

    close(fd);
}