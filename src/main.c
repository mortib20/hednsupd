/*
    This is a (unsecure because http) dyndns client for he.net DNS Servers only IPv4
    Yes, it could be done easier but...

    You need to get the Base64 of your domain:dnskey via htpasswd or from the internet
    Usage: Usage: hednsupd hostname base64 [debug=1]
    hostname = The hostname to update (example.org)
    base64 = domain:dnskey in base64
*/

#include <stdlib.h>         // Standard library definitions
#include <stdio.h>          // Standard input/output
#include <string.h>         // String operations
#include <unistd.h>         // Standard symbolic constants and types
#include <sys/socket.h>     // Socket interface
#include <netinet/in.h>     // IP implementation
#include <netinet/tcp.h>    // TCP implementation
#include <arpa/inet.h>      // Conversion host <> network order
#include <netdb.h>          // Definitions for network database operations
#include <errno.h>          // Error definitions

int DEBUG = 0;

typedef struct {
    char* domain;
    char* base64;
} arguments;

arguments getArguments(int argv, char **argc);
int updateDNS(arguments args);

int main(int argv, char **argc)
{
    if(argv == 1)
    {
        printf("Usage: %s hostname base64 [debug=1]\n\n", argc[0]);
        goto END;
    }

    if(argv == 4)
        DEBUG = 1;

    arguments args;

    args = getArguments(argv, argc);

    if(updateDNS(args) != 0)
    {
        printf("Error updating DNS record: %s\n", strerror(errno));
        goto END;
    } else {
        printf("Successfully updated!\n");
    }

    END:

    return 0;
}

/* get the first arguments */
arguments getArguments(int argv, char **argc)
{
    arguments targs;

    targs.domain = argc[1];
    targs.base64 = argc[2];

    return targs;
}

/* send an update request to he.net dns server via http */
int updateDNS(arguments args)
{
    /* Variables */
    size_t error;
    struct addrinfo hints, *result;
    int connection;
    char response[500], request[500], header[20];

    /* hints for getaddrinfo() */
    hints.ai_family = AF_INET; // Add IPv6 support
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    hints.ai_protocol = 0;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;

    /* getaddrinfo and check for error */
    error = getaddrinfo("dyn.dns.he.net", "80", &hints, &result);
    if(error != 0)
        return -1;

    /* create socket */
    connection = socket(AF_INET, SOCK_STREAM, 0);

    /* connecto to dyn.dns.he.net on port 80 and check for error */
    error = connect(connection, result->ai_addr, result->ai_addrlen);
    if(error != 0)
        return -1;

    sprintf(request,
        "GET /nic/update?hostname=%s HTTP/1.0\r\n"
        "Host: dyn.dns.he.net\r\n"
        "Authorization: Basic %s\r\n"
        "User-Agent: hednsupd/1.0\r\n"
        "Accept: */*\r\n"
        "\r\n",
    args.domain, args.base64);

    /* send request*/
    write(connection, request, strlen(request));
    if(DEBUG == 1)
        printf("%s\n", request);

    /* receive response */
    read(connection, response, sizeof(response));
    if(DEBUG == 1)
        printf("%s\n", response);

    /* close connection and free addrinfo */
    close(connection);
    freeaddrinfo(result);

    /* copy 0-15 chars from response to header*/
    strncpy(header, response, 15);
    if(DEBUG == 1)
        printf("%s\n", header);

    /* check if ip updated successfully */
    if(strcmp(header, "HTTP/1.0 200 OK") == 0)
        return 0;
    else
        return -1;
}