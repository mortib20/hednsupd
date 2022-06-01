/*
    This is a (unsecure because http) dyndns client for he.net DNS Servers only IPv4
    Yes, it could be done easier but...

    You need to get the Base64 of your domain:dnskey via htpasswd or from the internet
    Usage: Usage: ./hednsupd [hostname] [base64]
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

/*

1. Check if the arguments are valid
2. Get the IP address of dyn.dns.he.net
3. Create a socket to that IP
4. Send a HTTP header and the base64 encoding over the socket on port 80
5. Receive response from server
6. Check if the server updated the DNS entry

*/

typedef struct{
    char *hostname;
    char *dnskey;
    struct addrinfo *serveraddr;
} ARGS;

int checkArguments(char *argv[], int argc, ARGS *args);
int getAddress(ARGS *args);
int updateDNS(const ARGS args);
void cleanUp(ARGS *args);

void base64_encode(char *text);

int main(int argc, char *argv[])
{
    
    ARGS args;
    
    if(checkArguments(argv, argc, &args) != 0)
        goto END;

    if(getAddress(&args) != 0)
        goto END;

    if(updateDNS(args) != 0)
        goto END;

    END:
    cleanUp(&args);

    return 0;
}

int checkArguments(char *argv[],int argc, ARGS *args)
{
    if(argc < 3)
    {
        printf("Usage: %s [HOSTNAME] [DDNSKEY]\n", argv[0]);
        return -1;
    }

    args->hostname = argv[1];
    args->dnskey = argv[2];

    return 0;
}

int getAddress(ARGS *args)
{
    size_t error;
    struct addrinfo hints = 
    {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_CANONNAME,
        .ai_canonname = NULL,
        .ai_next = NULL,
        .ai_addr = NULL,
        .ai_addrlen = 0,
        .ai_protocol = 0
    };

    error = getaddrinfo("dyn.dns.he.net", "80", &hints, &args->serveraddr);
    if(error != 0)
    {
        printf("Error getaddrinfo(): %s\n", gai_strerror(error));
        return -1;
    }
}

int updateDNS(const ARGS args)
{
    int sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sd < 0)
    {
        printf("Error updateDNS(socket): %s\n", strerror(errno));
        return -1;
    }

    if(connect(sd, args.serveraddr->ai_addr, args.serveraddr->ai_addrlen) == -1)
    {
        printf("Error updateDNS(connect): %s\n", strerror(errno));
        return -1;
    }

    char request[500];
    sprintf(request,
        "GET /nic/update?hostname=%s HTTP/1.0\r\n"
        "Host: dyn.dns.he.net\r\n"
        "Authorization: Basic %s\r\n"
        "User-Agent: hednsupd/2.0\r\n"
        "Accept: */*\r\n"
        "\r\n", args.hostname, args.dnskey);
    
    if(send(sd, request, strlen(request), 0) == -1)
    {
        printf("Error updateDNS(send): %s", strerror(errno));
        return -1;
    }

    char *response;
    int resplength = recv(sd, response, 3000, MSG_PEEK | MSG_WAITALL | MSG_CONFIRM);
    response = (char*) malloc(sizeof(char) * resplength);
    recv(sd, response, resplength, 0);

    int headlength = strcspn(response, "\r\n\r\n");
    char *header = (char*) malloc(sizeof(char) * headlength);
    strncpy(header, response, headlength);

    printf("%s\n", header);

    free(response);
    free(header);

    return 0;
}

void cleanUp(ARGS *args)
{
    freeaddrinfo(args->serveraddr);
    args->hostname = NULL;
    args->dnskey = NULL;
    args->serveraddr = NULL;
}

void base64_encode(char *text)
{
    char base64_alphabet[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWabcdefghijklmnopqrstuvwxyz0123456789+/";
}