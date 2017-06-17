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
void null_terminator(char * buf);

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

void null_terminator(char * buf)
{
        int len = strlen(buf);

        for(int i = 0; i < len; i++)
        {
            if (buf[i] == '\n') {
                buf[i] = '\0';
            }
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

        null_terminator(buf);

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

char * fibonacci_parser(char * fib_number)
{
    //Prevent strtol from returning a negative number
    if(fib_number[1] == '-') {
        return NULL;
    }
    //Sets up the endptr for strtol for error checking
    char * endptr = NULL;

    //Converts string to a base 10 number
    int number = strtol(fib_number+1, &endptr, 10);

    //Checking endptr for non-digit stuff
    if(endptr) {
        //If the first thing is a 0 it's valid input. STRTOL returns this sometimes
        //  for unknown reasons, probably the stdin new line
        if(endptr[0] == 0) {
            //Resetting to null just to be sure
            endptr = NULL; 
            //If it gets here, the data is invalid
        } else {
            return NULL;
        }
    }

    //The range for the value is 0-300 (inclusive)
    if(number > 300) {
        return NULL;
    }

    //Create a big number type
    BIGNUM * big_number;

    //Call BN_fib function to return a big_number of the hex value
    big_number = BN_fibonacci(number);

    //Setting up a pointer to call bn2hex to
    char * hex_string;

    //Converts a big_number into a string representation of a hex value
    hex_string = BN_bn2hex(big_number);

    //Don't need the big_number anymore, so free it
    BN_free(big_number);

    //Find the length of the hex plus room for the addition 0 and x characters
    //  as well as a terminating null byte
    size_t len = strlen(hex_string) + 3;

    //Create another room for the 0x null byte combo
    char *buf = malloc(len);

    //Set the preface to the return string '0x'
    buf[0] = '0';
    buf[1] = 'x';
    //Terminate with a nullbyte because strncat might need a nullbyte to know where to stop?
    //  the man page wasn't clear on this.
    buf[2] = '\0';

    strncat(buf, hex_string, len);

    //Free the OPENSSL created string
    OPENSSL_free(hex_string);

    return buf;
}

char * roman_numeral_parser(char * rom_number)
{
    int number = roman_numeral_converter(rom_number+1);

    //The range is 0 to MMMM (4000)  Anything outside this range should be terminated
    if(number < 0 || number > 4000) {
        return NULL;
    }

    //Create data for string.  Should only need 6 characters, round up to a power of 2
    //  highest hex value supported (4000) is 0xfa0
    char *buf = calloc(1, 8);

    //Using snprintf to format the string
    snprintf(buf, 8, "0x%x", number);

    return buf; 
}
char * hex_convert_parser(char * dec_number)
{
    //Creating a big number because 10^20 is well over the limit of ULLONG_MAX
    BIGNUM *big_number;

    //Initializing BN
    big_number = BN_new();

    int length = 0;

    //Converts the string (up to 100000000000000000000) to a big_number
    length = BN_dec2bn(&big_number, dec_number+1);

    //If length is zero dec2bn errored no alpha chars or received no input
    if(length == 0) {
        BN_free(big_number);
        return NULL;
    }

    //If the length is 1 but the first digit is 0 than the user inputed D0
    //  this entire portion is just to get 0x0 as an output.
    //By default it outputs 0x if you let the rest of the function run
    if(length == 1 && dec_number[1] == '0') {
    char *buf = malloc(4);
        buf[0] = '0';
        buf[1] = 'x';
        buf[2] = '0';
        buf[3] = '\0';    
        BN_free(big_number);
        return buf;    
    }
    //The max number 10^20 is 21 digits long, if it exceeds this it exceeds the range
    if(length > 21) {
        return NULL;
    //If the number is 21 digits long we have to make sure the 21st digit is one
    //  and every other digit is 0
    } else if (length == 21) {
        if(dec_number[1] - '0' > 1) {
            BN_free(big_number);
            return NULL;
        }
        for(int i = 2; i <= 21; i++)
        {
            if((dec_number[i] - '0') > 0) {
                BN_free(big_number);
                return NULL;
            }  
        }        
    }

    //If all that's good 
    char * hex_string;
    hex_string = BN_bn2hex(big_number);
    BN_free(big_number);

    size_t len = strlen(hex_string) + 3;

    char *buf = malloc(len);
    buf[0] = '0';
    buf[1] = 'x';
    buf[2] = '\0';

    strncat(buf, hex_string+1, len);
    OPENSSL_free(hex_string);

    

    return buf;
}
