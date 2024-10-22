#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>      /* printf(), perror() */
#include <stdlib.h>     /*  */
#include <unistd.h>     /* fork(), close(), read(), _exit() */
#include <string.h>     /* strncpy() */
#include <arpa/inet.h>  /* htons() */
#include <sys/socket.h> /* socket(), bind(), listen(), accept(), connect() */
#include <netinet/in.h>

#define ID_FILE "id.db"
#define USER_DB "user.db"
#define ACCOUNT_DB "account.db"
#define FEEDBACK_DB "feedback.db"

#define MANAGER_MENU "\nManager Menu:\n1.Activate/Deactivate Customer Account\n2. Assign Loan Applications\n3. Review Customer Feedback\n4. Change Password\n5. Logout\nEnter your choice: "

// Enum to represent the role of users
enum Role
{
    ADMIN,    // 0
    EMPLOYEE, // 1
    MANAGER,  // 2
    CUSTOMER  // 3
};

// Structure to represent a user
struct user
{
    int user_id;
    char username[50];
    unsigned long hashed_password;
    enum Role role;
    int session; // 0:In Use  1: Not in Use
    int status;  // 0:Active  1: Inactive
};

struct account
{
    char username[50];
    float balance;
};

struct loan
{
    int user_id;
    char username[50];
    float loan_amount;
    int status; // 0:Accepted  1:Rejected   2:Pending
    int assigned_id;
};

void employeelogin(struct user *loginUser);
int authenticate(struct user *loginUser);
int initiate_user_id();
int generate_user_id();
unsigned long hash_password(const char *password);
void logout(struct user *loginUser);
void change_password(struct user *loginUser);
void modify_customer();
int change_status(int user_id, int new_status);
void modify_username();
int change_username(int user_id, char *new_username);
void process_loan(struct user *loginUser);
void view_loan_applications(struct user *loginUser);

int main()
{
    int choice;
    struct user loginUser;

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
            adminlogin(&loginUser);
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
        if (strcmp(username, find_User.username) == 0 && loginUser->role == find_User.role && loginUser->user_id == find_User.user_id)
        {
            loginUser->hashed_password = hash_password(new_password);
            loginUser->session = find_User.session;
            loginUser->status = find_User.status;
            loginUser->user_id = find_User.user_id;
            offset = lseek(fd, -1 * sizeof(struct user), SEEK_CUR);
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

void logout(struct user *loginUser)
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

            fileUser.session = 1;
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
}

void managerlogin(struct user *loginUser)
{
    char password[50];
    enum Role userRole = EMPLOYEE;
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
            printf("%s", EMPLOYEE_MENU);
            scanf("%d", &choice);

            switch (choice)
            {
            case 1:
                modify_customer(); // Done
                break;
            case 2:
                assign_loan_applications(); // Add some structure change to add loan id and use this id to asign loan applications
                break;
            case 3:
                print_feedback(); // Done
                break;
            case 4:
                change_password(loginUser); // Done
                break;
            case 5:
                logout(loginUser); // Done
                return;
            default:
                printf("Invalid choice. Please try again.\n");
            }
        }
    }
    else
    {
        printf("Invalid credentials!\n");
    }
}

void print_feedback()
{
    struct feedback user_feedback;
    int feedback_fd = open(FEEDBACK_DB, O_RDONLY);
    if (feedback_fd < 0)
    {
        perror("Error opening feedback database");
        return;
    }

    printf("Feedback entries:\n");
    while (read(feedback_fd, &user_feedback, sizeof(struct feedback)) > 0)
    {
        printf("Username: %s\n", user_feedback.username);
        printf("Feedback: %s\n", user_feedback.feedback);
        printf("-----------------------------\n");
    }

    if (close(feedback_fd) < 0)
    {
        perror("Error closing feedback database");
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
            if (fileUser.session == 1 && fileUser.status == 0)
            {
                loginUser->user_id = fileUser.user_id;
                loginUser->session = 0;
                loginUser->status = 0;

                fileUser.session = 0;
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

int initiate_user_id()
{
    int fd = open(ID_FILE, O_RDWR | O_CREAT, 0644);
    char ID[100];
    strcpy(ID, "1000");
    write(fd, ID, sizeof(ID));
    close(fd);
}

int generate_user_id()
{

    int fd = open(ID_FILE, O_RDWR, 0644);
    int gen_id;
    char ID[100];

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();

    fcntl(fd, F_SETLK, &lock);
    read(fd, ID, sizeof(ID));
    gen_id = atoi(ID);
    gen_id += 1;
    bzero(ID, sizeof(ID));
    snprintf(ID, sizeof(ID), "%d", gen_id);
    lseek(fd, 0, SEEK_SET);
    write(fd, ID, sizeof(ID));
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);

    return gen_id;
}

unsigned long hash_password(const char *password)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *password++))
    {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash;
}

void addUser(enum Role userRole)
{
    struct user newUser;
    char username[50];
    char password[50];

    printf("Enter username: ");
    scanf("%s", username);
    strcpy(newUser.username, username);

    printf("Enter password: ");
    scanf("%s", password);
    newUser.user_id = generate_user_id();
    newUser.hashed_password = hash_password(password);
    newUser.role = userRole;
    newUser.status = 0;  // ACTIVE
    newUser.session = 1; // INACTIVE

    int user_fd = open(USER_DB, O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (user_fd < 0)
    {
        perror("Error opening user database");
        exit(1);
    }

    if (write(user_fd, &newUser, sizeof(struct user)) < 0)
    {
        perror("Error writing to user database");
        close(user_fd);
        exit(1);
    }

    if (userRole == CUSTOMER)
    {
        int account_fd = open(ACCOUNT_DB, O_WRONLY | O_APPEND | O_CREAT, 0666);
        struct account newAccount;
        strcpy(newAccount.username, username);
        newAccount.balance = 0;
        if (write(account_fd, &newAccount, sizeof(struct account)) < 0)
        {
            perror("Error writing to account database");
            close(account_fd);
            exit(1);
        }
        close(account_fd);
    }
    printf("User added successfully!\n");
    close(user_fd);
}

void modify_customer()
{
    int user_id, new_status;
    printf("Enter the User ID: ");
    scanf("%d", &user_id);
    printf("Enter the New Status(0:Activate 1:Deactivate): ");
    scanf("%d", &new_status);

    int status = change_status(user_id, new_status);
    if (status == 1)
    {
        printf("Status Changed Successfully!\n");
    }
    else if (status == 0)
    {
        printf("Status Not Updated!\n");
    }
    else if (status == 2)
    {
        printf("User with user ID %d Not Found", user_id);
    }
}

int change_status(int user_id, int new_status)
{
    int fd = open(USER_DB, O_RDWR);
    int ret = 0;
    if (fd < 0)
    {
        perror("Error opening USER_DB");
        exit(0);
    }

    struct user find_User;
    struct flock lock;
    off_t offset = -1;

    while (read(fd, &find_User, sizeof(struct user)) > 0)
    {
        if (find_User.user_id == user_id)
        {
            find_User.status = new_status;
            offset = lseek(fd, -1 * sizeof(struct user), SEEK_CUR);
            break;
        }
        else
        {
            ret = 2;
        }
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
        ssize_t write_status = write(fd, &find_User, sizeof(struct user));
        if (write_status > 0)
        {
            ret = 1;
        }
        else
        {
            ret = 0;
        }

        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
    }
    close(fd);

    return ret;
}

void modify_username()
{
    int user_id;
    char new_username[50];
    printf("Enter the User ID: ");
    scanf("%d", &user_id);
    printf("Enter the New Username ");
    scanf("%s", new_username);

    int status = change_username(user_id, new_username);
    if (status == 1)
    {
        printf("Username Changed Successfully!\n");
    }
    else if (status == 0)
    {
        printf("Username Not Updated!\n");
    }
    else if (status == 2)
    {
        printf("User with user ID %d Not Found", user_id);
    }
}

int change_username(int user_id, char *new_username)
{
    int fd = open(USER_DB, O_RDWR);
    int ret = 2;
    if (fd < 0)
    {
        perror("Error opening USER_DB");
        exit(0);
    }

    struct user find_User;
    struct flock lock;
    off_t offset = -1;

    while (read(fd, &find_User, sizeof(struct user)) > 0)
    {
        if (find_User.user_id == user_id)
        {
            strcpy(find_User.username, new_username);
            offset = lseek(fd, -1 * sizeof(struct user), SEEK_CUR);
            ret = 2;
            break;
        }
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
        ssize_t write_status = write(fd, &find_User, sizeof(struct user));
        if (write_status == sizeof(struct user))
        {
            ret = 1;
        }
        else
        {
            ret = 0;
        }

        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
    }
    close(fd);
    return ret;
}

void view_loan_applications(struct user *loginUser)
{
    struct loan loanApplications;
    char serverMessage[1000];
    int fd = open(LOAN_DB, O_RDONLY);
    if (fd < 0)
    {
        perror("Error opening feedback database");
        return;
    }

    strcpy(serverMessage, "Loan Applications:\n");
    while (read(fd, &loanApplications, sizeof(struct loan)) > 0)
    {
        if (loanApplications.assigned_id == loginUser->user_id)
        {
            char loanAmount[20];
            char loanStatus[20];
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
            strcat(serverMessage, loanApplications.username);
            snprintf(loanAmount, sizeof(loanAmount), " %.2f", loanApplications.loan_amount);
            strcat(serverMessage, loanAmount);
            strcat(serverMessage, " : ");
            strcat(serverMessage, loanStatus);
        }
    }
    printf("%s", serverMessage);
    // send(connectionFileDescriptor, serverMessage, strlen(serverMessage), 0);
    // recv(connectionFileDescriptor, dummyBuffer, sizeof(dummyBuffer), 0);
    // clearBuffers();

    close(fd);
}

void process_loan(struct user *loginUser)
{
    int user_id;
    int loan_status; // 0: Approved  1:Rejected
    printf("Enter the User ID: ");
    scanf("%d", &user_id);
    printf("1. Approve Loan\n2. Reject Loan Enter\nEnter Your Choice : ");
    scanf("%d", &loan_status);

    int fd = open(LOAN_DB, O_RDWR);
    if (fd < 0)
    {
        perror("Error opening USER_DB");
        exit(0);
    }

    struct loan loanUser;
    struct flock lock;
    off_t offset = -1;

    while (read(fd, &loanUser, sizeof(struct loan)) > 0)
    {
        if (loanUser.user_id == user_id)
        {
            loanUser.status = loan_status;
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
        struct account loanAccount, la;
        loanAccount.username = loanUser.username;

        int fd = open(ACCOUNT_DB, O_RDONLY);
        while (read(fd, &la, sizeof(struct account)) > 0)
        {
            if (strcpy(la.username, loanUser.username) == 0)
            {
                loanAccount.balance = la.balance;
                break;
            }
        }
        int status = update_user_balance(&loanAccount, loanUser.loan_amount);
        lseek(fd, offset, SEEK_SET);
        ssize_t write_status = write(fd, &loanUser, sizeof(struct loan));

        if (write_status > 0 && status == 1)
        {
            printf("Loan Processed!\n");
        }
        else
        {
            printf("Loan Not Processed!\n");
        }

        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
    }

    close(fd);
}