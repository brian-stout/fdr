#define _GNU_SOURCE

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "roman_numeral.h"

void asm_fib_calc(int number);
char * fibonacci_parser(char * buf);
char * roman_numeral_parser(char * buf);
char * hex_convert_parser(char * buf);

int main(int argc, char *argv[])
{
    if(argc != 2) {
        fprintf(stderr, "%s <IP>\n", argv[0]);
        return 1;
    }

    // Port numbers are in the range 1-65535, plus null byte
    char port_num[8];
    snprintf(port_num, sizeof(port_num), "%hu", getuid());
    printf("%hu\n", getuid());

    struct addrinfo *results;
    struct addrinfo hints = {0};
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    int err = getaddrinfo(argv[1], port_num, &hints, &results);
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

    int number = strtol(fib_number+1, NULL, 10);

    unsigned long part1 = 0, part2 = 0, part3 = 0, part4 = 0;
	asm_fib_calc(number);
	//Cuillo helped here
	//TODO: Figure out stack arguments, and call snprintf in function and pass
	//			address
    __asm__(
        "mov %0, r12\n"
        "mov %1, r13\n"
        "mov %2, r14\n"
        "mov %3, r15"
        : "=r"(part1), "=r"(part2), "=r"(part3), "=r"(part4)
    );
    char *buf = calloc(1, 128);
	//Use string concats instead to avoid extra 0's
    if(part1 > 0) {
        snprintf(buf, 64, "0x%lx%lx%lx%lx", part1, part2, part3, part4);
    } else if(part2 > 0) {
        snprintf(buf, 64, "0x%lx%lx%lx", part2, part3, part4);
    } else if(part3 > 0) {
        snprintf(buf, 64, "0x%lx%lx", part3, part4);
    } else if(part4 > 0) {
        snprintf(buf, 64, "0x%lx", part4);
    } else {
        snprintf(buf, 64, "0x0");
    }


    return buf;
}

char * roman_numeral_parser(char * rom_number)
{
    int number = roman_numeral_converter(rom_number+1);
    printf("%d\n", number);

    char *buf = calloc(1, 128);
    snprintf(buf, 64, "%d", number);
    printf("%s\n", buf);

    return buf; 
}
char * hex_convert_parser(char * buf)
{
    printf("test: %s\n", buf+1);
    buf[1] = '5';

    return NULL;
}
