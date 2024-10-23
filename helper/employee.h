
void view_loan_applications(struct user *loginUser, int clientSocket)
{
    struct loan loanApplications;
    int fd = open(LOAN_DB, O_RDONLY);
    if (fd < 0)
    {
        perror("Error opening feedback database");
        return;
    }

    strcpy(serverMessage, "Loan Applications:\n");
    strcat(serverMessage, "---------------------------------------------\n");
    strcat(serverMessage, "Loan ID   | Username      | Amount   | Status\n");
    strcat(serverMessage, "---------------------------------------------\n");
    while (read(fd, &loanApplications, sizeof(struct loan)) > 0)
    {
        if (loanApplications.assigned_id == loginUser->user_id && (loanApplications.status == 1 || loanApplications.status == 2))
        {
            char loanAmount[20];
            char loanStatus[20];
            char loanDetails[200];
            if (loanApplications.status == 0)
            {
                strcpy(loanStatus, "Accepted \n");
            }
            else if (loanApplications.status == 1)
            {
                strcpy(loanStatus, "Rejected \n");
            }
            else if (loanApplications.status == 2)
            {
                strcpy(loanStatus, "Pending \n");
            }

            snprintf(loanAmount, sizeof(loanAmount), " %.2f", loanApplications.loan_amount);
            snprintf(loanDetails, sizeof(loanDetails), "%-10d| %-13s| %-8s| %-8s\n",
                     loanApplications.loan_id,
                     loanApplications.username,
                     loanAmount,
                     loanStatus);

            strcat(serverMessage, loanDetails);
        }
    }
    printf("%s", serverMessage);
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();

    close(fd);
}

void process_loan(struct user *loginUser, int clientSocket)
{
    int loan_id;
    int loan_status; // 0: Approved  1:Rejected
    char username[50];
    int user_id;
    int loan_amount;
    struct loan loanUser;
    struct transaction transactionRecord;
    struct flock lock;
    off_t offset = -1;
    int status = 1;
    char datetime[20];

    strcpy(serverMessage, "Enter the Loan ID: ");
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
    loan_id = atoi(clientMessage);
    clearBuffers();

    strcpy(serverMessage, "1. Approve Loan\n2. Reject Loan Enter\nEnter Your Choice : ");
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
    loan_status = atoi(clientMessage) - 1;
    clearBuffers();

    int fd = open(LOAN_DB, O_RDWR);
    if (fd < 0)
    {
        perror("Error opening LOAN_DB");
        close(fd);
        exit(0);
    }

    while (read(fd, &loanUser, sizeof(struct loan)) > 0)
    {
        if (loanUser.loan_id == loan_id)
        {
            loanUser.status = loan_status;

            strcpy(username, loanUser.username);
            user_id = loanUser.user_id;
            loan_amount = loanUser.loan_amount;

            offset = lseek(fd, -1 * sizeof(struct loan), SEEK_CUR);
            break;
        }
    }

    if (offset != -1)
    {

        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = offset;
        lock.l_len = sizeof(struct loan);
        lock.l_pid = getpid();
        fcntl(fd, F_SETLK, &lock);

        if (loanUser.status == 0)
        {
            struct account updateAccount;
            struct user view;

            view.user_id = user_id;

            updateAccount.user_id = user_id;
            updateAccount.balance = view_balance(&view);
            strcpy(updateAccount.username, username);

            status = update_user_balance(&updateAccount, loan_amount);
            time_t now = time(NULL);
            struct tm *local = localtime(&now);
            strftime(datetime, sizeof(datetime), "%d-%m-%Y %H:%M", local);

            transactionRecord.user_id = loginUser->user_id;
            strcpy(transactionRecord.username, username);
            transactionRecord.type = 1;
            transactionRecord.amount = loan_amount;
            transactionRecord.total_amount = updateAccount.balance;
            strcpy(transactionRecord.timestamp, datetime);
            append_transaction(&transactionRecord);
        }

        lseek(fd, offset, SEEK_SET);
        ssize_t write_status = write(fd, &loanUser, sizeof(struct loan));

        if (write_status > 0 && status == 1)
        {
            strcpy(serverMessage, "Loan Processed!\n");
            send(clientSocket, serverMessage, strlen(serverMessage), 0);
        }
        else
        {
            strcpy(serverMessage, "Loan Not Processed!\n");
            send(clientSocket, serverMessage, strlen(serverMessage), 0);
        }

        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
    }
    else
    {
        strcpy(serverMessage, "Loan ID Not Found!\n");
        send(clientSocket, serverMessage, strlen(serverMessage), 0);
    }
    recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();
    close(fd);
}

void employee(struct user *loginUser, int clientSocket)
{
    int emp_run = 1;
    while (emp_run == 1)
    {
        char employee_p_id[20];
        sprintf(employee_p_id, "%d", loginUser->user_id);
        strcpy(serverMessage, "\nWelcome, ");
        strcat(serverMessage, loginUser->username);
        strcat(serverMessage, "!\n");
        strcat(serverMessage, "User ID: ");
        strcat(serverMessage, employee_p_id);
        strcat(serverMessage, "\n");
        strcat(serverMessage, "Choose an operation:\n");
        strcat(serverMessage, "1. Add New Customer\n");
        strcat(serverMessage, "2. Modify Customer Details\n");
        strcat(serverMessage, "3. View Loan Applications\n");
        strcat(serverMessage, "4. Approve/Reject Loans Applications\n");
        strcat(serverMessage, "5. Change Password\n");
        strcat(serverMessage, "6. Logout\n");
        strcat(serverMessage, "Enter Your Choice : ");

        send(clientSocket, serverMessage, strlen(serverMessage), 0);
        recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
        int employee_choice = atoi(clientMessage);
        clearBuffers();

        switch (employee_choice)
        {
        case 1:
            addUser(3, clientSocket);
            break;
        case 2:
            modify_details(3, clientSocket);
            break;
        case 3:
            view_loan_applications(loginUser, clientSocket);
            break;
        case 4:
            process_loan(loginUser, clientSocket);
            break;
        case 5:
            change_password(loginUser, clientSocket);
            break;
        case 6:
            logout(loginUser, clientSocket);
            emp_run = 0;
            break;
        default:
            send(clientSocket, "Invalid choice!\n", strlen("Invalid choice!\n"), 0);
            recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
            clearBuffers();
        }
    }
}
