#define _GNU_SOURCE

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>

#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <openssl/bn.h>

#include "roman_numeral.h"
#include "fibonacci.h"

char * fibonacci_parser(char * buf);
char * roman_numeral_parser(char * buf);
char * hex_convert_parser(char * buf);
void signal_handler(int signal);

int create_server_fork(char * port_num);

int main(void)
{
    // Port numbers are in the range 1-65535, plus null byte
    char port_num[8];

    //Grab userid to use different ports
    uid_t uid = getuid();

    //converts the uid to a string to be used for the socket
    snprintf(port_num, sizeof(port_num), "%hu", uid);

    //Variable used to prevent child processes from forking their own children
    bool is_child = false;

    //TODO: Grab all the PIDs into a PID array to run a infinite loop on waiting for
    //      exits instead of just one child process for more stability.
    pid_t pid = fork();
    
    if(pid == 0) {
        create_server_fork(port_num);
        is_child = true;
    } else if (pid < 0) {
        perror("Fork Error!\n");
    }

    if(is_child == false) {
        pid = fork();
        if(pid == 0) {
            is_child = true;
            snprintf(port_num, sizeof(port_num), "%hu", uid + 1000);
            create_server_fork(port_num);
        } else if (pid < 0) {
            perror("Fork Error!\n");
        }
    }

    if(is_child == false) {
        pid = fork();

        if(pid == 0) {
            is_child = true;
            snprintf(port_num, sizeof(port_num), "%hu", uid + 2000);
            create_server_fork(port_num);
        } else if (pid < 0) {
            perror("Fork Error!\n");
        }
    }

    int status;
    pid = wait(&status);
    printf("Child with PID %ld exited with status 0x%x.\n", (long)pid, status);


}

void signal_handler(int signal)
{
    switch(signal) {
        case SIGINT:
            //Ctrl+C will exit normally
            write(0, "Received SIGINT, exiting fork\n", 30);
            exit(0);
        case SIGTERM:
            write(0, "Received SIGTERM, exiting fork\n", 31);
            exit(0);
        default:
            //Should never happen
            write(0, "Bad Signal", 10);
            return;
    }
}

int create_server_fork(char * port_num)
{
    //Creating signal handler to catch interrupts
    //TODO: Throw in function and pass PID so it makes sense.  Possibly find a way
    //          to do a clean exit to make sure we can valgrind everything
    struct sigaction sa;

    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    //Checking for errors via the signals

    if(sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Error on SIGHTERM\n");
    }
    if(sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error on SIGINT\n");
    }  

    struct addrinfo *results;
    struct addrinfo hints = {0};
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    int err = getaddrinfo("localhost", port_num, &hints, &results);
    if(err != 0) {
        fprintf(stderr, "Could not parse address: %s\n", gai_strerror(err));
        return 2;
    }

    int sd = socket(results->ai_family, results->ai_socktype, 0);
    if(sd < 0) {
        perror("Could not create socket");
        freeaddrinfo(results);
        return 3;
    }

    err = bind(sd, results->ai_addr, results->ai_addrlen);
    if(err < 0) {
        perror("Could not create bind");
        close(sd);
        freeaddrinfo(results);
        return 4;
    }

    freeaddrinfo(results);

	for(;;) {
        char buf[256];
        struct sockaddr_storage client;
        socklen_t client_sz = sizeof(client);

        ssize_t received = recvfrom(sd, buf, sizeof(buf), 0,
                (struct sockaddr *)&client, &client_sz);

        if(received < 0) {
            perror("Problem receiving");
        }
        if(received == 256) {
            buf[received-1] = '\0';
        } else {
            buf[received] = '\0';
        }

        //Going to assign a string to a point as a return since the fib parser
        //  won't rewrite buff properly and can't return an array type
        char * func_ret = NULL;

        switch(buf[0])
        {
            case 'F':
            case 'f':
                func_ret = fibonacci_parser(buf);
                break;
            case 'R':
            case 'r':
                func_ret = roman_numeral_parser(buf);
                break;
            case 'D':
            case 'd':
                func_ret = hex_convert_parser(buf);
                break;
            default:
                printf("Error!\n");
        }
        if(func_ret == NULL) {
            continue;
        }

        size_t len = strlen(func_ret);
        strncpy(buf, func_ret, len);
        free(func_ret);


        buf[len] = '\n';
        buf[len+1] = '\0';

        ssize_t sent = sendto(sd, buf, strlen(buf), 0, (struct sockaddr *)&client, client_sz);
        if(sent < 0)
        {
            perror("Invalid Send");
        }
	}
}

char * fibonacci_parser(char * fib_number)
{
    if(fib_number[1] == '-') {
        printf("Exiting!\n");
        return NULL;
    }
    char * endptr = NULL;

    int number = strtol(fib_number+1, &endptr, 10);

    if(endptr) {
        if(endptr[0] == 0) {
            endptr = NULL;
        } else {
            printf("%d\n", endptr[0]);
            return NULL;
        }
    }

    if(number < 0) {
        return NULL;
    }
    if(number > 300) {
        return NULL;
    }

    BIGNUM * big_number;
    big_number = BN_fibonacci(number);
    char * hex_string;
    hex_string = BN_bn2hex(big_number);
    BN_free(big_number);

    size_t len = strlen(hex_string) + 3;

    char *buf = malloc(len);
    buf[0] = '0';
    buf[1] = 'x';
    buf[2] = '\0';

    strncat(buf, hex_string, len);
    OPENSSL_free(hex_string);

    return buf;
}

char * roman_numeral_parser(char * rom_number)
{
    char *buf = calloc(1, 128);

    int number = roman_numeral_converter(rom_number+1);
    if(number < 0 || number > 4000) {
        return NULL;
    }

    snprintf(buf, 64, "0x%x", number);
    printf("%s\n", buf);

    return buf; 
}
char * hex_convert_parser(char * dec_number)
{
    BIGNUM *big_number;

    big_number = BN_new();

    int length = 0;
    length = BN_dec2bn(&big_number, dec_number+1);
    if(length == 0) {
        return NULL;
    }
    if(length >= 21) {
        if ((dec_number[21] - '0') != 0) {
            return NULL;
        }
    }

    char * hex_string;
    hex_string = BN_bn2hex(big_number);
    BN_free(big_number);

    size_t len = strlen(hex_string)+2;

    char *buf = malloc(len);
    buf[0] = '0';
    buf[1] = 'x';
    buf[2] = '\0';

    strncat(buf, hex_string+1, len);
    OPENSSL_free(hex_string);

    return buf;
}
