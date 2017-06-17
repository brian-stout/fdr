#define _GNU_SOURCE

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <openssl/bn.h>

#include "roman_numeral.h"
#include "fibonacci.h"

char * fibonacci_parser(char * buf);
char * roman_numeral_parser(char * buf);
char * hex_convert_parser(char * buf);

int main(void)
{
    // Port numbers are in the range 1-65535, plus null byte
    char port_num[8];
    snprintf(port_num, sizeof(port_num), "%hu", getuid());
    printf("%hu\n", getuid());

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
    BIGNUM * big_number;
    big_number = BN_fibonacci(number);
    char * hex_string;
    hex_string = BN_bn2hex(big_number);
    size_t len = strlen(hex_string)+2;
    char *buf = malloc(len);
    buf[0] = '0';
    buf[1] = 'x';
    buf[2] = '\0';

    strncat(buf, hex_string, len);
    free(hex_string);

    return buf;
}

char * roman_numeral_parser(char * rom_number)
{
    char *buf = calloc(1, 128);

    int number = roman_numeral_converter(rom_number+1);

    snprintf(buf, 64, "0x%x", number);
    printf("%s\n", buf);

    return buf; 
}
char * hex_convert_parser(char * dec_number)
{
    BIGNUM *big_number;
    big_number = BN_new();

    BN_dec2bn(&big_number, dec_number+1);

    char * hex_string;
    hex_string = BN_bn2hex(big_number);
    BN_free(big_number);

    size_t len = strlen(hex_string)+2;

    char *buf = malloc(len);
    buf[0] = '0';
    buf[1] = 'x';
    buf[2] = '\0';

    strncat(buf, hex_string+1, len);
    free(hex_string);

    return buf;
}
