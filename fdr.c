#define _GNU_SOURCE

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "roman_numeral.h"
#include "fibonacci.h"
#include "parsers.h"

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

    //Only do if you're a child process
    if(pid == 0) {
        //Creates a sever on the fork using the port_num
        create_server_fork(port_num);
        is_child = true; //Mark the child's data with a boolean so we always know
    } else if (pid < 0) {
        perror("Fork Error!\n");
    }

    if(is_child == false) {
        pid = fork();
        if(pid == 0) {
            is_child = true;
            //Redoing the snprintf to add 100 to the uid
            snprintf(port_num, sizeof(port_num), "%hu", uid + 1000);
            //Creating another server with a different port
            create_server_fork(port_num);
        } else if (pid < 0) {
            perror("Fork Error!\n");
        }
    }

    //Do it a third time for the third part
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

    //Pauses the parent process to keep it from exiting and zombifying the children
    //  unpauses when all the children are terminated or exit
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
    const char * host = "tooldev";

    printf("Intializing server on port %s at host %s \n", port_num, host);

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

    //A lot of this code is repurposed from the classes' example of a UDP server
    struct addrinfo *results;
    struct addrinfo hints = {0};
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    //Localhost is the default
    int err = getaddrinfo(host, port_num, &hints, &results);
    if(err != 0) {
        fprintf(stderr, "Could not parse address: %s\n", gai_strerror(err));
        return 2;
    }

    //Creates the socket
    int sd = socket(results->ai_family, results->ai_socktype, 0);
    if(sd < 0) {
        perror("Could not create socket");
        freeaddrinfo(results);
        return 3;
    }

    //Calls the OS to bind the socket
    err = bind(sd, results->ai_addr, results->ai_addrlen);
    if(err < 0) {
        perror("Could not create bind");
        close(sd);
        freeaddrinfo(results);
        return 4;
    }

    freeaddrinfo(results);

    //Infinite receive and send loop.  Main server functions
	for(;;) {
        //buffer to store received data from
        char buf[256];
        struct sockaddr_storage client;
        socklen_t client_sz = sizeof(client);

        ssize_t received = recvfrom(sd, buf, sizeof(buf), 0,
                (struct sockaddr *)&client, &client_sz);

        //If nothing got received there's been an error
        if(received < 0) {
            perror("Problem receiving");
        }
        //Null terminates the UDP DGRAM in the last possible array index if more than the max
        //  data has been received.
        if(received == 256) {
            buf[received-1] = '\0';
        //Otherwise null terminate it where data stops
        } else {
            buf[received] = '\0';
        }

        //Going to assign a string to a point as a return since the fib parser
        //  won't rewrite buff properly and can't return an array type
        char * func_ret = NULL;

        //Removes newline since running netcat by itself produces different datagrams
        newline_terminator(buf);

        switch(buf[0])
        {
            //Falls through for upper and lower case
            case 'F':
            case 'f':
                //Sends string to a function that sets up the data for
                //  specific modules on parsing the data
                func_ret = fibonacci_parser(buf);
                break;
            case 'R':
            case 'r':
                func_ret = roman_numeral_parser(buf);
                break;
            case 'D':
            case 'd':
                //Passes string to a function that handles everything since it's
                //  simple enough not to need it's own module
                func_ret = hex_convert_parser(buf);
                break;
            default:
                printf("Error!\n");
        }

        //All the fail conditions on the functions return NULL, if func_ret is NULL
        //  It means the functions didn't complete properly

        //TODO: Create error pointer for all the functions to write specific errors to
        if(func_ret == NULL) {
            continue;
        }

        //Copies func_ret into buff, probably can just pass sendto func_ret but this works
        //  for now and fiddling with it will probably require adjustments to the parser

        //TODO: Adjust the parser to just pass func_ret to sendto()
        size_t len = strlen(func_ret);
        strncpy(buf, func_ret, len);

        //All three functions specifically allocate memory for the func_ret string
        //  and need to be freed here
        free(func_ret);

        //Makes sure client receives a string terminated in a newline and a nullbyte
        buf[len] = '\n';
        buf[len+1] = '\0';

        //Sends the response back to the sender
        ssize_t sent = sendto(sd, buf, strlen(buf), 0, (struct sockaddr *)&client, client_sz);
        if(sent < 0)
        {
            perror("Invalid Send");
        }
	}
}

