#define USER_DB "Sandbox/user.db"
#define ACCOUNT_DB "Sandbox/account.db"
#define LOAN_DB "Sandbox/loan.db"
#define FEEDBACK_DB "Sandbox/feedback.db"
#define TRANSACTION_DB "Sandbox/transaction.db"
#define ID_FILE "Sandbox/id.db"

#define MAIN_MENU "Select Your Role:\n1. Admin\n2. Manager\n3. Employee\n4. Customer \n5. Exit\nEnter Your Choice: "

#define PORT 6008
#define BACKLOG 100

struct user
{
    int user_id;
    char username[50];
    unsigned long hashed_password;
    int role;    // 0:Admin   1: Employee   2:Manager   3:Customer
    int session; // 0:In Use  1: Not in Use
    int status;  // 0:Active  1: Inactive
};

struct account
{
    int user_id;
    char username[50];
    float balance;
};

struct loan
{
    int loan_id;
    int user_id;
    char username[50];
    float loan_amount;
    int status; // 0:Accepted  1:Rejected   2:Pending
    int assigned_id;
};

struct feedback
{
    int user_id;
    char username[50];
    char feedback[1000];
};

struct transaction
{
    int user_id;
    char username[50];
    int type; // 0: Debit  1 : Credit
    float amount;
    float total_amount;
    char timestamp[100];
};
