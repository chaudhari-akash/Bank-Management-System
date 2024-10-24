#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define TRANSACTION_DB "../db/transaction.db"

struct transaction
{
    int user_id;
    char username[50];
    int type; // 0: Debit  1 : Credit
    float amount;
    float total_amount;
    char timestamp[100];
};

void print_transaction(struct transaction *trans)
{
    printf("Transaction Details:\n");
    printf("User ID: %d\n", trans->user_id);
    printf("Username: %s\n", trans->username);
    printf("Transaction Type: %s\n", trans->type == 0 ? "Debit" : "Credit");
    printf("Amount: %.2f\n", trans->amount);
    printf("Total Amount: %.2f\n", trans->total_amount);
    printf("Timestamp: %s\n", trans->timestamp);
    printf("-------------------------\n");
}

int main()
{
    int fd = open(TRANSACTION_DB, O_RDWR);
    struct transaction trans;

    while (read(fd, &trans, sizeof(struct transaction)) > 0)
    {
        print_transaction(&trans);
    }
    close(fd);
    return 0;
}