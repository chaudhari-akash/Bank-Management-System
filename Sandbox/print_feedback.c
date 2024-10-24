#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define FEEDBACK_DB "../db/feedback.db"

struct feedback
{
    int user_id;
    char username[50];
    char feedback[1000];
};

void view_feedback()
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

int main()
{
    view_feedback();
}
