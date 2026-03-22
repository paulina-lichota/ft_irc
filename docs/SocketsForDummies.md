# Sockets For Dummies

## What is a socket?
A socket is a **file descriptor** that can be used to
send and receive data over the network.

> Imagine you want to send a message to a friend with a phone.
> The socket is the phone.
> Both you and your friend need a socket to communicate.

In Unix/Linux, a **socket is a file descriptor**
that represents a I/O channel to a network interface.

Concretely, when you open a socket, the kernel allocates
two **buffers** (one for sending and one for receiving) and a file descriptor.
They are array of bytes in the RAM.

The **fd** is a number that identifies the pair of buffers you are using.


## Parallelism with pipes

In **pipes**, everything works on a **single machine**.
The kernel allocates only one buffer (since the communication is unidirectional)
Two local processes can communicate with each other: one by writing to and one by reading from the same pipe.
No need for a protocol, data is sent as-is.

With sockets, we work on two machines,
both with their own 2 buffers (read and write) since the communication is bidirectional.
Data now travels **across the network** instead of staying in the same machine.


## How data travels across the network?

### Sending data
Data becomes a **stream of bytes**, which travels across the network.

> Basically, you want to send a book by mail.
> You can't squeeze it into a single packet.
> You split a whole book into a lot of envelopes,
> each one containing a page, the page number and the address.

You send data by writing to a socket in your program:
` write(fd, "JOIN #42\r\n", 10) `
Now you have 10 bytes of raw data that goes to the kernel's write buffer.

> The envelopes are special: they are layered.
> Like a box into a box into a box.

The kernel **wraps the data** with some headers in this order:
1. **TCP** layer: source port, destination port, sequence number (_"the page number"_) and **flags** (control signals: SYN, ACK, FIN, RST)
2. **IP** layer: source IP, destination IP (_"the address"_)
3. **Ethernet/WiFi** layer: source MAC, destination MAC (_"the mailman"_)

> Flags tell the kernel **how to interpret** the rest of the packet

**✨ Now our packet is ready to travel across the network ✨**

### Receiving data
***On the other side***, when a packet arrives (physical signal),
the network card (**NIC**, _scheda di rete_) unwraps the first layer:
- **Ethernet/WiFi**: is this MAC address mine?
    - Yes → sends an **interrupt** to the kernel
    - No → packet discarded silently, kernel never knows

The kernel receives the **interrupt**, stops for a moment,
and unwraps remaining layers:
- **IP** → is this IP address mine?
    - Yes → continue
    - No → discard
- **TCP** → reads the ports and checks the flags.
    What happens next depends on the flags.

Let's say the flag is **ACK + data**.
The kernel strips the headers and puts the **raw bytes** into the **read buffer** of the socket.
Our program has a `poll()` running, that watches for any event on the socket.
Now we are sure that there is some data in the socket.
Our program can read the data with `read(fd, buffer, 10)`.

> `poll()` just checks for incoming events from a list of fds.

## Handshake

TCP is a **connection-oriented protocol**, which means that the client must send a message to the server to confirm its identity before it can receive any data.

