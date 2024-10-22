#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define TRANSACTION_DB "transaction.db"

void print_transaction(struct transaction *trans)
{
    printf("Transaction Details:\n");
    printf("User ID: %d\n", trans->user_id);
    printf("Username: %s\n", tran->username);
    printf("Transaction Type: %s\n", trans->type == 0 ? "Debit" : "Credit");
    printf("Amount: %.2f\n", trans->amount);
    printf("Total Amount: %.2f\n", trans->total_amount);
    printf("Timestamp: %s\n", trans->timestamp);
    printf("-------------------------\n");
}


int main(){
    int fd = open(TRANSACTION_DB,O_RDWR);

    while(read(fd,&trans,sizeof(struct trans)) > 0){
        print_transaction(trans)
    }
    retrun 0;
}