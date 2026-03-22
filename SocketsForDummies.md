# Sockets For Dummies

A socket is a **file descriptor** that can be used to send and receive data over the network.

> Imagine you want to send a message to a friend with a phone.
> The socket is the phone.
> Both you and your friend need a socket to communicate.

In Unix/Linux, a socket is a file descriptor that represents a I/O channel to a network interface.
Concretely, when you open a socket, the kernel allocates two buffers (one for sending and one for receiving) and a file descriptor. They are array of bytes in the RAM.
The fd is a number that identifies the pair of buffers you are using.

### Parallelism with pipes

In pipes, everything works on a single machine.
The kernel allocates only one buffer (since the communication is unidirectional)
Two local processes can communicate with each other: one by writing to and one by reading from the same pipe.
No need for a protocol, data is sent as-is.

With sockets, we work on two machines, both with their own buffers (read and write) since the communication is bidirectional.
Data now travels across the network instead of staying in the same machine.

### How data travels across the network?

Data becomes a stream of bytes, which travels across the network.

> Basically, you want to send a book by mail.
> You can't squeeze it into a single packet.
> You split a whole book into a lot of envelopes,
> each one containing a page, the page number and the address.

You send data by writing to a socket:
` write(fd, "JOIN #42\r\n", 10) `
Now you have 10 bytes of raw data that goes to the kernel's write buffer.

> The envelopes are special: they are layered.
> Like a box into a box into a box.

The kernel wraps the packet with some headers in this order:
- **TCP** layer: source port, destination port, sequence number (_"the page number"_)
- **IP** layer: source IP, destination IP (_"the address"_)
- **Ethernet/WiFi** layer: source MAC, destination MAC (_"the mailman"_)

**✨ Now our packet is ready to travel across the network ✨**


***On the other side***, the kernel (always listening - no need for a program) receives the packet and unwraps it in reverse.
When a packet arrives, the network card (**NIC**, _scheda di rete_) tells to the kernel "Hey, I got a packet!". This is called an **interrupt**, a hardware event that the kernel can react to.

The kernel stops for a moment, unwraps the packet and looks for the destination port. Now we have 2 scenarios:
1. If a socket is listening on that port, the kernel takes the raw data and puts it in the socket's buffer.
2. If no socket is listening on that port, IT THROWS THE PACKET. the kernel sends an ICMP **"port unreachable"** message to the sender.
