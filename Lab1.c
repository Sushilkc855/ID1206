#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h> 

/*These lines include necessary header files for standard input/output, process handling, inter-process communication (IPC), and message queues.*/
// Part 1: ls / | wc -l
//This defines a function part1 that represents the first part of the task. It declares an array pipe_fd to store file descriptors for a pipe.
void part1() {
    int pipe_fd[2];
    pipe(pipe_fd);
    
    /*This creates a child process using the fork system call and stores the process ID (pid). The code within the if (pid == 0) 
    block will be executed by the child process, and the code within the else if (pid > 0) block will be executed by the parent process.*/
    pid_t pid = fork();

 //In the child process, it closes the read end of the pipe, redirects the standard output to the write end of the pipe using dup2, and then executes the ls / command using execlp.
    if (pid == 0) {
        // Child process
        close(pipe_fd[0]); // Close read end of the pipe
        dup2(pipe_fd[1], STDOUT_FILENO); // Redirect stdout to the pipe write end
        close(pipe_fd[1]); // Close pipe write end

        // Execute ls /
        execlp("ls", "ls", "/", NULL);
        perror("exec");
        exit(EXIT_FAILURE);
    
    //In the parent process, it closes the write end of the pipe, redirects the standard input to the read end of the pipe using dup2, and then executes the wc -l command using execlp.
    } else if (pid > 0) {
        // Parent process
        close(pipe_fd[1]); // Close write end of the pipe
        dup2(pipe_fd[0], STDIN_FILENO); // Redirect stdin to the pipe read end
        close(pipe_fd[0]); // Close pipe read end

        // Execute wc -l
        execlp("wc", "wc", "-l", NULL);
        perror("exec");
        exit(EXIT_FAILURE);
    } else {
        perror("fork");
        exit(EXIT_FAILURE);
    }
}

// Part 2: Message queues
//This part defines a structure msg_buffer to hold messages in the message queue. It includes a message type and a text field.
struct msg_buffer {
    long msg_type;
    char msg_text[100];
};

//This defines a function part2 for the second part of the task. It generates a key for the message queue using ftok and creates a message queue using msgget.
void part2() {
    key_t key = ftok("message_queue_key", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);
    //Similar to part 1, this creates a child process using fork.
    pid_t pid = fork();

    //In the child process, it reads a message from the message queue using msgrcv, counts the number of words in the received text using strtok, and prints the word count.
    if (pid == 0) {
        // Child process (reader)
        struct msg_buffer message;
        msgrcv(msgid, &message, sizeof(message), 1, 0);

        int word_count = 0;
        char *token = strtok(message.msg_text, " ");
        while (token != NULL) {
            word_count++;
            token = strtok(NULL, " ");
        }

        printf("Word count in the file: %d\n", word_count);
        exit(EXIT_SUCCESS);
    
    //In the parent process, it opens a file (sample.txt), reads its content, sends the content through the message queue using msgsnd, and then waits for the child process to finish.
    } else if (pid > 0) {
        // Parent process (writer)
        FILE *file = fopen("sample.txt", "r");
        if (file == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        struct msg_buffer message;
        message.msg_type = 1;
        fread(message.msg_text, sizeof(char), sizeof(message.msg_text), file);

        msgsnd(msgid, &message, sizeof(message), 0);
        fclose(file);

        wait(NULL); // Wait for the child process to finish
        exit(EXIT_SUCCESS);
    } else {
        perror("fork");
        exit(EXIT_FAILURE);
    }
}


/*The main function calls both part1 and part2 functions to execute the tasks. The output includes the result of the ls / | wc -l
 command and the word count from reading a file and passing the content through a message queue.*/
int main() {
    printf("Part 1 - ls / | wc -l:\n");
    part1();

  
    printf("\nPart 2 - Message queues:\n");
    part2();

    return 0;
}
