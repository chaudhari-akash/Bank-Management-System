
void print_feedback(int clientSocket)
{
    struct feedback user_feedback;
    int feedback_fd = open(FEEDBACK_DB, O_RDONLY);
    if (feedback_fd < 0)
    {
        perror("Error opening feedback database");
        return;
    }

    strcpy(serverMessage, "Feedback entries:\n");
    strcat(serverMessage, "-----------------------------\n");
    while (read(feedback_fd, &user_feedback, sizeof(struct feedback)) > 0)
    {
        strcat(serverMessage, "Username: ");
        strcat(serverMessage, user_feedback.username);
        strcat(serverMessage, "\n");
        strcat(serverMessage, "Feedback: \n");
        strcat(serverMessage, user_feedback.feedback);
        strcat(serverMessage, "\n");
        strcat(serverMessage, "-----------------------------\n");
    }

    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();

    if (close(feedback_fd) < 0)
    {
        perror("Error closing feedback database");
    }
}

void m_print_loan_applications(int clientSocket)
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
        if (loanApplications.status == 1 || loanApplications.status == 2)
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

int find_employee(int emp_id)
{
    int fd = open(USER_DB, O_RDWR);
    int ans = 0;
    struct user findUser;
    if (fd < 0)
    {
        perror("Error opening USER_DB");
        close(fd);
        exit(0);
    }

    while (read(fd, &findUser, sizeof(struct user)) > 0)
    {
        if (findUser.user_id == emp_id)
        {
            ans = 1;
            break;
        }
        else
        {
            ans = 0;
        }
    }

    close(fd);

    return ans;
}

void assign_loan_applications(int clientSocket)
{
    int employ_id, loan_id;
    int fd;
    struct loan loanUser;
    off_t offset = -1;
    struct flock lock;
    int find_emp = 0;

    strcpy(serverMessage, "Enter the Employee ID: ");
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
    employ_id = atoi(clientMessage);
    clearBuffers();
    find_emp = find_employee(employ_id);

    sprintf(serverMessage, "%d", find_emp);
    send(clientSocket, serverMessage, strlen(serverMessage), 0);
    recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();

    if (find_emp == 1)
    {
        strcpy(serverMessage, "Enter the Loan ID: ");
        send(clientSocket, serverMessage, strlen(serverMessage), 0);
        recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
        loan_id = atoi(clientMessage);
        clearBuffers();

        fd = open(LOAN_DB, O_RDWR);
        if (fd < 0)
        {
            perror("Error opening LOAN_DB");
            exit(0);
        }

        while (read(fd, &loanUser, sizeof(struct loan)) > 0)
        {
            if (loanUser.loan_id == loan_id)
            {
                loanUser.assigned_id = employ_id;
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

            lseek(fd, offset, SEEK_SET);
            ssize_t write_status = write(fd, &loanUser, sizeof(struct loan));

            if (write_status > 0)
            {
                strcpy(serverMessage, "Loan Assigned!\n");
                send(clientSocket, serverMessage, strlen(serverMessage), 0);
            }
            else
            {
                strcpy(serverMessage, "Loan Not Assigned!\n");
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
    }
    else
    {
        strcpy(serverMessage, "Employee ID Not Found!\n");
        send(clientSocket, serverMessage, strlen(serverMessage), 0);
    }
    recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
    clearBuffers();
    close(fd);
}

void manager(struct user *loginUser, int clientSocket)
{
    while (1)
    {

        char manager_p_id[20];
        sprintf(manager_p_id, "%d", loginUser->user_id);
        strcpy(serverMessage, "\nWelcome, ");
        strcat(serverMessage, loginUser->username);
        strcat(serverMessage, "!\n");
        strcat(serverMessage, "User ID: ");
        strcat(serverMessage, manager_p_id);
        strcat(serverMessage, "\n");
        strcat(serverMessage, "Choose an operation:\n");
        strcat(serverMessage, "1. Modify Customer Details\n");
        strcat(serverMessage, "2. View Loan Applications\n");
        strcat(serverMessage, "3. Assign Loan Applications\n");
        strcat(serverMessage, "4. View Customer Feedback");
        strcat(serverMessage, "5. Change Password\n");
        strcat(serverMessage, "6. Logout\n");
        strcat(serverMessage, "Enter Your Choice : ");

        send(clientSocket, serverMessage, strlen(serverMessage), 0);
        recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
        int manager_choice = atoi(clientMessage);
        clearBuffers();

        switch (manager_choice)
        {
        case 1:
            modify_details(3, clientSocket);
            break;
        case 2:
            m_print_loan_applications(clientSocket);
            break;
        case 3:
            assign_loan_applications(clientSocket);
            break;
        case 4:
            print_feedback(clientSocket);
            break;
        case 5:
            change_password(loginUser, clientSocket);
            break;
        case 6:
            logout(loginUser, clientSocket);
            return;
        default:
            send(clientSocket, "Invalid choice!\n", strlen("Invalid choice!\n"), 0);
            recv(clientSocket, dummyBuffer, sizeof(dummyBuffer), 0);
            clearBuffers();
        }
    }
}
