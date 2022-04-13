# hednsupd
A (unsecure) DynDNS for he.net DNS Server

## Compile
```
mkdir build && cd build && cmake .. && make
```
## Usage
> You need to get the Base64 of your domain:dnskey via htpasswd or https://www.base64encode.org/

Usage: hednsupd hostname base64 [debug=1]

hostname = The hostname to update (example.org)

base64 = domain:dnskey in base64

# Example
hednsupd example.org ZG9tYWluOmRuc2tleQ==

with debug info

hednsupd example.org ZG9tYWluOmRuc2tleQ== 1
