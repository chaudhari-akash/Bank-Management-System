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
    char username[50];
    unsigned long hashed_password;
    enum Role role;
    int status; // 0:Active  1:Inactive
};

// Structure to represent a account
struct account
{
    char username[50];
    float balance;
};

struct loan
{
    char username[50];
    float loan_amount;
    int status; // 0:Accepted  1:Rejected   2:Pending
};

struct feedback
{
    char username[50];
    char feedback[1000];
};

struct transaction
{
    char username[50];
    int type; // 0: Debit  1 : Credit
    float amount;
    float total_amount;
};

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>

void update_account_structure(int fd, off_t offset, struct account *userAccount, float amount)
{
    struct flock lock;
    int lock_status;
    int lseek_status;
    int write_status;
    int unlock_status;

    lock.l_type = F_WRLCK;               // Write lock
    lock.l_whence = SEEK_SET;            // Relative to the start of the file
    lock.l_start = offset;               // Start of the lock
    lock.l_len = sizeof(struct account); // Length of the data (bytes)
    lock.l_pid = getpid();               // Process ID of the process holding the lock

    lock_status = fcntl(fd, F_SETLK, &lock);
    if (lock_status == -1)
    {
        perror("Error locking file");
        close(fd);
        _exit(0);
    }

    lseek_status = lseek(fd, offset, SEEK_SET);
    if (lseek_status == -1)
    {
        perror("Error seeking to position");
        close(fd);
        _exit(0);
    }

    struct account fileUser;

    read(fd, &fileUser, sizeof(struct account));
    float total_amount = userAccount->balance + amount;
    strcpy(fileUser.username, userAccount->username);

    fileUser.balance = userAccount->balance;
    if (total_amount >= 0)
    {
        fileUser.balance += amount;
    }
    else
    {
        printf("Not Enough Balance in your Account");
    }

    lseek(fd, -1 * sizeof(struct account), SEEK_CUR);

    write_status = write(fd, &fileUser, sizeof(struct account));
    if (write_status < 0)
    {
        perror("Error writing to file");
        close(fd);
        _exit(0);
    }

    lock.l_type = F_UNLCK; // Release the lock
    unlock_status = fcntl(fd, F_SETLK, &lock);
    if (unlock_status == -1)
    {
        perror("Error unlocking file");
        close(fd);
        _exit(0);
    }

    close(fd);
}

void lock_record_read(int fd, off_t offset)
{
    struct flock lock;
    int lock_status;
    int lseek_status;

    lock.l_type = F_RDLCK;               // Read lock
    lock.l_whence = SEEK_SET;            // Relative to the start of the file
    lock.l_start = offset;               // Start of the lock
    lock.l_len = sizeof(struct account); // Length of the data (bytes)
    lock.l_pid = getpid();               // Process ID of the process holding the lock

    lock_status = fcntl(fd, F_SETLK, &lock);
    if (lock_status == -1)
    {
        perror("Error locking file");
        close(fd);
        _exit(0);
    }
}

void unlock_record_read(int fd, off_t offset)
{
    struct flock lock;
    int lock_status;
    int lseek_status;

    lock.l_type = F_UNLCK;               // Unlock
    lock.l_whence = SEEK_SET;            // Relative to the start of the file
    lock.l_start = offset;               // Start of the lock
    lock.l_len = sizeof(struct account); // Length of the data (bytes)
    lock.l_pid = getpid();               // Process ID of the process holding the lock

    lock_status = fcntl(fd, F_SETLK, &lock);
    if (lock_status == -1)
    {
        perror("Error Unlocking file");
        close(fd);
        _exit(0);
    }
}
