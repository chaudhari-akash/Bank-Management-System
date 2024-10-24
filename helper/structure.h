#define USER_DB "db/users.db"
#define ACCOUNT_DB "db/account.db"
#define LOAN_DB "db/loan.db"
#define FEEDBACK_DB "db/feedback.db"
#define TRANSACTION_DB "db/transaction.db"
#define ID_FILE "db/id.db"

#define MAIN_MENU "Select Your Role:\n1. Admin\n2. Manager\n3. Employee\n4. Customer \n5. Exit\nEnter Your Choice: "

char serverMessage[1000], clientMessage[1000], dummyBuffer[100];

void clearBuffers()
{
    bzero(serverMessage, sizeof(serverMessage));
    bzero(clientMessage, sizeof(clientMessage));
    bzero(dummyBuffer, sizeof(dummyBuffer));
}

void fillDummy()
{
    bzero(dummyBuffer, sizeof(dummyBuffer));
    strcpy(dummyBuffer, "This is a Dummy Message\n");
}

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
