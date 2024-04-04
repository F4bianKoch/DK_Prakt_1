#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>

int pipe_desc_initiator[2];
int pipe_desc_responder[2];

void abp_initiator() {
    char response[2];
    int res_length = read(pipe_desc_initiator[0], &response, 2);

    if (res_length == -1) {
        fprintf(stderr, "Error: while reading the response!\n");
    }

    write(STDOUT_FILENO, &response, res_length);
    write(STDOUT_FILENO, "\n", 1);
}

void abp_responder() {
    char message[2];
    int msg_length = read(pipe_desc_responder[0], &message, 2);

    if (msg_length == -1) {
        fprintf(stderr, "Error: while reading the response!\n");
    }

    write(STDOUT_FILENO, &message, msg_length);
    write(STDOUT_FILENO, "\n", 1);
}

int main() {
    char message_whole[] = "TheQuickBrownRabbit";
    char response_whole[] = "JumpsOverTheLazyFox";

    // generate pipes
    if (pipe(pipe_desc_initiator) == -1) {
        fprintf(stderr, "Error: while generating the initiator pipe!\n");
        return 0;
    }

    if (pipe(pipe_desc_responder) == -1) {
        fprintf(stderr, "Error: while generating the responder pipe!\n");
        return 0;
    }

    // fork process
    int c_pid = fork();
    if (c_pid == -1) {
        fprintf(stderr, "Error: while forking process!\n");
        return 0;
    }

    // parent process
    if (c_pid > 0) {
        // close unneeded pipe gates
        close(pipe_desc_initiator[1]);
        close(pipe_desc_responder[0]);

        // prep parent system
        struct sigaction actioninitiator = {0};
        actioninitiator.sa_handler = abp_initiator;
        if (sigaction (SIGUSR1, &actioninitiator, 0) != 0) {
            fprintf(stderr, "Error: while preping parent system for signal\n");
        }

        sleep(1);

        for (int i = 0; i < strlen(message_whole); i++) {
            char message[2] = {message_whole[i], '1'};

            if (write(pipe_desc_responder[1], &message, 2) == -1) {
                fprintf(stderr, "Error: while sending the message from parent!\n");
            }

            if (kill(c_pid, SIGUSR2) == -1) {
                fprintf(stderr, "Error: while killing child!\n");
            }

            pause();
        }

        sleep(1);

        printf("Parent done!\n");

        exit(0);
    }

    // child process
    if (c_pid == 0) {
        // close unneeded pipe gates
        close(pipe_desc_initiator[0]);
        close(pipe_desc_responder[1]);

        // prep child system
        struct sigaction actionresponder = {0};
        actionresponder.sa_handler = abp_responder;
        if (sigaction (SIGUSR2, &actionresponder, 0) != 0) {
            fprintf(stderr, "Error: while preping parent system for signal\n");
        }

        for (int i = 0; i < strlen(response_whole); i++) {
            pause();

            char response[2] = {response_whole[i], '1'};
            if (write(pipe_desc_initiator[1], &response, 2) == -1) {
                fprintf(stderr, "Error: while sending the message from child!\n");
            }

            if (kill(getppid(), SIGUSR1) == -1) {
                fprintf(stderr, "Error: while killing parent!\n");
            }

        }

        sleep(1);

        printf("Child done!\n");

        exit(0);
    }

    return 0;
}