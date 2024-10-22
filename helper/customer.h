
float view_balance(struct user *loginUser)
{
    struct account userAccount;
    float balance = -1;
    struct flock lock;
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
        if (loginUser->user_id == userAccount.user_id)
        {
            offset = lseek(fd, -1 * sizeof(struct account), SEEK_CUR);

            lock.l_type = F_RDLCK;
            lock.l_whence = SEEK_SET;
            lock.l_start = offset;
            lock.l_len = sizeof(struct account);
            lock.l_pid = getpid();
            fcntl(fd, F_SETLK, &lock);

            balance = userAccount.balance;

            lock.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lock);

            break;
        }
    }

    close(fd);

    return balance;
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
        if (strcmp(userAccount->username, fileUser.username) == 0 && userAccount->user_id == fileUser.user_id)
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

void deposit_money(struct user *loginUser, int clientSocket)
{
    struct account userAccount;
    struct transaction transactionRecord;
    float deposit_amount, total_balance;
    char datetime[20];
    int update_status;

    clearBuffers();
    strcpy(serverMessage, "Enter amount to deposit: ");
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
    deposit_amount = atof(clientMessage);
    clearBuffers();

    strcpy(userAccount.username, loginUser->username);
    userAccount.user_id = loginUser->user_id;
    userAccount.balance = view_balance(loginUser);

    if (deposit_amount > 0)
    {
        update_status = update_user_balance(&userAccount, deposit_amount);

        total_balance = view_balance(loginUser);

        time_t now = time(NULL);
        struct tm *local = localtime(&now);
        strftime(datetime, sizeof(datetime), "%d-%m-%Y %H:%M", local);

        transactionRecord.user_id = loginUser->user_id;
        strcpy(transactionRecord.username, loginUser->username);
        transactionRecord.type = 1;
        transactionRecord.amount = deposit_amount;
        transactionRecord.total_amount = total_balance;
        strcpy(transactionRecord.timestamp, datetime);
        append_transaction(&transactionRecord);

        snprintf(serverMessage, sizeof(serverMessage), "Deposit successful. New balance: %.2f\n", total_balance);
        send(clientSocket, serverMessage, strlen(serverMessage), 0);
    }
    else
    {
        strcpy(serverMessage, "Invalid amount!\n");
        send(clientSocket, serverMessage, strlen(serverMessage), 0);
    }
    recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();
}

void withdraw_money(struct user *loginUser, int clientSocket)
{
    struct account userAccount;
    struct transaction transactionRecord;
    float total_balance, withdraw_amount;
    int update_status;
    char datetime[20];

    clearBuffers();
    strcpy(serverMessage, "Enter amount to withdraw: ");
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
    withdraw_amount = atof(clientMessage);
    clearBuffers();

    if (withdraw_amount > 0)
    {
        strcpy(userAccount.username, loginUser->username);
        userAccount.user_id = loginUser->user_id;
        userAccount.balance = view_balance(loginUser);

        update_status = update_user_balance(&userAccount, -withdraw_amount);
        if (update_status == 1)
        {
            total_balance = view_balance(loginUser);
            time_t now = time(NULL);
            struct tm *local = localtime(&now);
            strftime(datetime, sizeof(datetime), "%d-%m-%Y %H:%M", local);

            transactionRecord.user_id = loginUser->user_id;
            strcpy(transactionRecord.username, loginUser->username);
            transactionRecord.type = 0;
            transactionRecord.amount = withdraw_amount;
            transactionRecord.total_amount = total_balance;
            strcpy(transactionRecord.timestamp, datetime);

            append_transaction(&transactionRecord);

            snprintf(serverMessage, sizeof(serverMessage), "Withdraw successful!\nNew balance: %.2f\n", total_balance);
            send(clientSocket, serverMessage, strlen(serverMessage), 0);
        }
        else
        {
            total_balance = view_balance(loginUser);
            snprintf(serverMessage, sizeof(serverMessage), "Insufficient Balance!\nWithdraw failed.\nBalance: %.2f\n", total_balance);
            send(clientSocket, serverMessage, strlen(serverMessage), 0);
        }
    }
    else
    {
        snprintf(serverMessage, sizeof(serverMessage), "Invalid amount!\n");
        send(clientSocket, serverMessage, strlen(serverMessage), 0);
    }
    recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();
}

void transfer_funds(struct user *loginUser, int clientSocket)
{
    int reciever_user_id;
    struct transaction senderTransactionRecord, recieverTransactionRecord;
    struct account senderAccount, recieverAccount;
    off_t sender_offset, reciever_offset;
    struct flock sender_record_lock, reciever_record_lock;
    int sender_lock_status, reciever_lock_status;
    int sender_fd, reciever_fd;
    float amount;

    strcpy(serverMessage, "Enter recipient User ID: ");
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
    reciever_user_id = atoi(clientMessage);
    clearBuffers();

    strcpy(serverMessage, "Enter amount to transfer: ");
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
    amount = atof(clientMessage);
    clearBuffers();

    sender_fd = open(ACCOUNT_DB, O_RDWR);
    reciever_fd = open(ACCOUNT_DB, O_RDWR);

    while (read(sender_fd, &senderAccount, sizeof(struct account)) > 0)
    {
        if (loginUser->user_id == senderAccount.user_id)
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
        send(clientSocket, serverMessage, strlen(serverMessage), 0);
        close(sender_fd);
        close(reciever_fd);
    }
    else
    {
        lseek(sender_fd, sender_offset, SEEK_SET);
        float new_amount = senderAccount.balance - amount;
        senderAccount.balance = new_amount;

        char datetime_send[20];
        time_t now = time(NULL);
        struct tm *local = localtime(&now);
        strftime(datetime_send, sizeof(datetime_send), "%d-%m-%Y %H:%M", local);

        senderTransactionRecord.user_id = loginUser->user_id;
        strcpy(senderTransactionRecord.username, loginUser->username);
        senderTransactionRecord.amount = amount;
        senderTransactionRecord.type = 0;
        senderTransactionRecord.total_amount = senderAccount.balance;
        strcpy(senderTransactionRecord.timestamp, datetime_send);

        append_transaction(&senderTransactionRecord);

        lseek(sender_fd, sender_offset, SEEK_SET);
        write(sender_fd, &senderAccount, sizeof(struct account));
        sender_record_lock.l_type = F_UNLCK;
        fcntl(sender_fd, F_SETLK, &sender_record_lock);
        close(sender_fd);

        while (read(reciever_fd, &recieverAccount, sizeof(struct account)) > 0)
        {
            if (reciever_user_id == recieverAccount.user_id)
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

        char datetime_recv[20];
        time_t now_2 = time(NULL);
        struct tm *local_2 = localtime(&now_2);
        strftime(datetime_recv, sizeof(datetime_recv), "%d-%m-%Y %H:%M", local_2);

        recieverTransactionRecord.user_id = recieverAccount.user_id;
        strcpy(recieverTransactionRecord.username, recieverAccount.username);
        recieverTransactionRecord.amount = amount;
        recieverTransactionRecord.type = 1;
        recieverTransactionRecord.total_amount = recieverAccount.balance;
        strcpy(recieverTransactionRecord.timestamp, datetime_recv);
        append_transaction(&recieverTransactionRecord);

        lseek(reciever_fd, reciever_offset, SEEK_SET);
        write(reciever_fd, &recieverAccount, sizeof(struct account));
        reciever_record_lock.l_type = F_UNLCK;
        fcntl(reciever_fd, F_SETLK, &reciever_record_lock);
        close(reciever_fd);

        snprintf(serverMessage, sizeof(serverMessage), "Transfer Successfull!\nNew Balance : %.2f\n", view_balance(loginUser));
        send(clientSocket, serverMessage, strlen(serverMessage), 0);
    }
    recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();
}

void apply_loan(struct user *loginUser, int clientSocket)
{
    float loan_amount;
    struct loan loanAccount;
    struct flock lock;
    int lock_status, write_bytes;

    strcpy(serverMessage, "Enter the Loan Amount: ");
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
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
    loanAccount.user_id = loginUser->user_id;
    loanAccount.assigned_id = -1;
    loanAccount.loan_id = generate_user_id();

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
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();
}

void print_loan_applications(struct user *loginUser, int clientSocket)
{
    struct loan loanApplications;
    int fd = open(LOAN_DB, O_RDONLY);
    if (fd < 0)
    {
        perror("Error opening feedback database");
        return;
    }

    strcpy(serverMessage, "        Loan Applications:      \n");
    strcat(serverMessage, "--------------------------------\n");
    strcat(serverMessage, "|   Loan Amount   |   Status   |\n");
    strcat(serverMessage, "--------------------------------\n");
    while (read(fd, &loanApplications, sizeof(struct loan)) > 0)
    {
        if (loanApplications.user_id == loginUser->user_id)
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
            char loanEntry[100];
            snprintf(loanEntry, sizeof(loanEntry), "   %-12s      %-8s   \n", loanAmount, loanStatus);
            strcat(serverMessage, loanEntry);
        }
    }

    strcat(serverMessage, "--------------------------------\n");
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();

    close(fd);
}

void add_feedback(struct user *loginUser, int clientSocket)
{
    char feedback[1000];
    int bytes_read, feedback_fd, write_status;
    struct feedback user_feedback;
    struct flock lock;

    strcpy(serverMessage, "Enter Feedback: ");
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
    strcpy(feedback, clientMessage);
    clearBuffers();

    user_feedback.user_id = loginUser->user_id;
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
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();

    lock.l_type = F_UNLCK;
    fcntl(feedback_fd, F_SETLK, &lock);
    close(feedback_fd);
}

void view_transaction_history(struct user *loginUser, int clientSocket)
{
    int fd = open(TRANSACTION_DB, O_RDONLY);
    char debit[20] = "DEBIT";
    char credit[20] = "CREDIT";
    if (fd == -1)
    {
        perror("Error opening transaction database");
        return;
    }
    struct transaction trans;

    strcpy(serverMessage, "\t\tYour Transactions\n");
    strcat(serverMessage, "--------------------------------------------------------------------\n");
    strcat(serverMessage, "|   Type   |   Amount   |   Total Balance   |   Date/Time          |\n");
    strcat(serverMessage, "--------------------------------------------------------------------\n");

    while (read(fd, &trans, sizeof(struct transaction)) > 0)
    {
        if (trans.user_id == loginUser->user_id)
        {
            char amountStr[20];
            char totalBalanceStr[20];
            char transactionType[10];
            if (trans.type == 0)
            {
                strcat(transactionType, debit);
            }
            else
            {
                strcat(transactionType, credit);
            }

            sprintf(amountStr, "%.2f", trans.amount);
            sprintf(totalBalanceStr, "%.2f", trans.total_amount);
            char transactionEntry[1000];
            snprintf(transactionEntry, sizeof(transactionEntry), "   %-7s    %-8s    %-15s    %s \n",
                     transactionType, amountStr, totalBalanceStr, trans.timestamp);
            strcat(serverMessage, transactionEntry);
            bzero(transactionType, sizeof(transactionType));
        }
    }
    strcat(serverMessage, "--------------------------------------------------------------------\n");

    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();
    close(fd);
}

void customer(struct user *loginUser, int clientSocket)
{
    while (1)
    {
        char user_p_id[20];
        sprintf(user_p_id, "%d", loginUser->user_id);
        strcpy(serverMessage, "\nWelcome, ");
        strcat(serverMessage, loginUser->username);
        strcat(serverMessage, "!\n");
        strcat(serverMessage, "User ID: ");
        strcat(serverMessage, user_p_id);
        strcat(serverMessage, "\n");
        strcat(serverMessage, "Choose an operation:\n");
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

        send(clientSocket, serverMessage, strlen(serverMessage), 0);
        recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
        int customer_choice = atoi(clientMessage);
        clearBuffers();

        switch (customer_choice)
        {
        case 1:
            float balance = view_balance(loginUser);
            if (balance != -1)
            {
                sprintf(serverMessage, "Your account balance is: %.2f\n", balance);
            }
            else
            {
                strcpy(serverMessage, "Unable to Retrive Account Balance\n");
            }
            send(clientSocket, serverMessage, strlen(serverMessage), 0);
            recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
            clearBuffers();
            break;
        case 2:
            deposit_money(loginUser, clientSocket);
            break;
        case 3:
            withdraw_money(loginUser, clientSocket);
            break;
        case 4:
            transfer_funds(loginUser, clientSocket);
            break;
        case 5:
            apply_loan(loginUser, clientSocket);
            break;
        case 6:
            print_loan_applications(loginUser, clientSocket);
            break;
        case 7:
            change_password(loginUser, clientSocket);
            break;
        case 8:
            add_feedback(loginUser, clientSocket);
            break;
        case 9:
            view_transaction_history(loginUser, clientSocket);
            break;
        case 10:
            logout(loginUser, clientSocket);
            return;
        default:
            send(clientSocket, "Invalid choice!\n", strlen("Invalid choice!\n"), 0);
            recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
            clearBuffers();
            break;
        }
    }
}

