
# Socket (Client side)

In order to connect(), you have to create a socket, then bind() it to a local address, and finally connect() it to a remote address.

The connect() function takes a pointer to a sockaddr structure, which contains the address of the remote host.
The sockaddr structure is defined as follows:

```c
struct sockaddr_in {
    short            sin_family;   // e.g., AF_INET
    unsigned short   sin_port;     // Port number (must be htons())
    struct in_addr   sin_addr;     // IPv4 address (must be inet_aton())
    char             sin_zero[8];  // Zero this if you want to
};
```

## Example usage

```c

#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

struct sockaddr_in servaddr;

// 1. Clear the structure
memset(&servaddr, 0, sizeof(servaddr));

// 2. Set address family
servaddr.sin_family = AF_INET;

// 3. Set port (e.g., 8080)
servaddr.sin_port = htons(8080);

// 4. Set IP Address (e.g., 127.0.0.1)
inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

// Use in bind()
// bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

```