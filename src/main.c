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

int main(int argc, char **argv)
{
    
}