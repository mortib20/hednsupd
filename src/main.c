#include <arpa/inet.h> // Conversion host <> network order
#include <errno.h> // Error definitions
#include <netdb.h> // Definitions for network database operations
#include <netinet/in.h> // IP implementation
#include <netinet/tcp.h> // TCP implementation
#include <stdio.h> // Standard input/output
#include <stdlib.h> // Standard library definitions
#include <string.h> // String operations
#include <sys/socket.h> // Socket interface
#include <unistd.h> // Standard symbolic constants and types

/*

1. Check if the arguments are valid
2. Get the IP address of dyn.dns.he.net
3. Create a socket to that IP
4. Send a HTTP header and the base64 encoding over the socket on port 80
5. Receive response from server
6. Check if the server updated the DNS entry

*/

int main(int argc, char* argv[])
{
    /* Step 1 */
    if (argc < 3) {
        printf("Usage: %s HOSTNAME HOSTNAME:DNSKEY\n", argv[0]);
        goto CLEANUP;
    }

    /* Variable Declaration */
    size_t error;
    int clientsocket, request_length, response_length, header_length;
    char request[500], response[1500], header[50];
    struct addrinfo hints, *result, *current_result;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_addrlen = 0;
    hints.ai_flags = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;

    /* Step 2 */
    if ((error = getaddrinfo("dyn.dns.he.net", "80", &hints, &result)) != 0) {
        printf("getaddrinfo(): %s\n", gai_strerror(error));
        goto CLEANUP;
    }

    /* Step 3 */
    for (current_result = result; current_result != NULL; current_result = current_result->ai_next) {
        clientsocket = socket(current_result->ai_family, current_result->ai_socktype, current_result->ai_protocol);

        if (clientsocket == -1)
            continue;

        if (connect(clientsocket, current_result->ai_addr, current_result->ai_addrlen) != -1)
            break;

        close(clientsocket);
    }

    freeaddrinfo(result);

    if (current_result == NULL) {
        printf("connect(): %s\n", strerror(errno));
        goto CLEANUP;
    }

    /* Step 4 */
    request_length = sprintf(request,
        "GET /nic/update?hostname=%s HTTP/1.0\r\n"
        "Host: dyn.dns.he.net\r\n"
        "Authorization: Basic %s\r\n"
        "User-Agent: hednsupd/2.0\r\n"
        "Accept: */*\r\n"
        "\r\n",
        argv[1], argv[2]);

    request_length = send(clientsocket, request, request_length, 0);

    /* Step 5 */
    response_length = recv(clientsocket, response, sizeof(response), MSG_WAITALL);
    response[response_length] = '\0';
    close(clientsocket);
    header_length = strcspn(response, "\r\n");
    strncpy(header, response, header_length);
    header[header_length] = '\0';

    /* Step 6 */
    if(strcmp(header, "HTTP/1.0 200 OK") == 0)
        printf("DNS has been updated!\n");
    else
        printf("DNS failed to update!\n");

CLEANUP:

    return 0;
}