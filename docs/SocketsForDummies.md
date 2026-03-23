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

TCP is a **connection-oriented protocol**, which means that before any data can be exchanged, both sides must agree that they are ready to communicate. This agreement is called the **three-way handshake**, exactly three messages are exchanged.

> Imagine you want to call a friend.
> Before you can talk, you both need to confirm the line is working.
> You say "hello?", they say "hello! can you hear me?", you say "yes, I can hear you".
> Only now you start the real conversation.
> That's the handshake.

### Why does it exist?
TCP guarantees that data arrives complete and in order. To do this, both sides need to know that the other exists, is reachable, and is ready. They also need to agree on the **sequence numbers** the counters that will be used to track every byte sent.

### The three steps (SYN, SYN-ACK, ACK)

**1. SYN** — the client sends a packet with the SYN flag to the server. Inside there is a randomly chosen **sequence number** `X`. This number will be used to track the order of the bytes sent.

> *"Hello? I want to connect. I'll start counting from 1000."*

**2. SYN-ACK** — the server receives the SYN and replies with a packet carrying two flags: SYN and ACK.
- The **ACK** says "I received your SYN" and contains `X+1` — "I expect your next byte to start from X+1"
- The **SYN** contains the server's own sequence number `Y`

> *"Hello! I heard you, I'm ready. I'll start counting from 5000, and I expect you to continue from 1000+1."*

**3. ACK** — the client sends a final ACK containing `Y+1` — "I received your SYN, I expect your next byte to start from Y+1". The connection is now established.

> *"Perfect, I heard you too. Let's talk."*

### What are sequence numbers?

Every byte transmitted in TCP has a sequence number. They serve two purposes:
- **Reorder** packets that arrive out of order
- **Detect** lost packets that need to be retransmitted

The first packet sent by the client contains a randomly chosen **ISN** (Initial Sequence Number).

The initial sequence numbers are **random** for security reasons — it prevents attackers from injecting fake packets by guessing the numbers.

> Remember the book sent by mail analogy?
> The sequence number is the page number written on each envelope.
> If envelope 42 arrives before envelope 41, you wait.
> If envelope 41 never arrives, you ask for it again.

#### 👾 ISN spoofing for ꧁𓊈𒆜𝓟𝓻𝓸𒆜𓊉꧂
The number is random to prevent attackers from guessing it.
In a **spoofing** attack, the attacker forges sends a packet pretending to be
one of the two parties.
If the sequence number were predictable, the forged packet would
be accepted as legitimate.
A random ISN forces the attacker to guess from billions of possible values.

> Back to the phone call analogy:
> You and your friend agree on a secret code word before talking.
> If someone else picks up the phone and pretends to be your friend,
> they won't know the code word. You hang up.
> The random ISN is that code word.

### Where does the kernel fit in?

The entire handshake is handled **autonomously by the kernel**, **your program does nothing** during this phase. When the handshake completes, the kernel puts the established connection in an internal queue called the **accept queue** and waits for your program to call `accept()`.

> Think of it like a receptionist at the front desk.
> The receptionist (kernel) greets every visitor, checks their identity, and has them sit in the waiting room (accept queue).
> Only when you call `accept()` does the receptionist bring the visitor to your office.
> You never deal with the greeting yourself — it already happened.

This is why `listen()` takes a **backlog** parameter — it sets the maximum size of that waiting room. If the waiting room is full when a new client tries to connect, the kernel silently drops the SYN and the client will time out and retry.

### After the handshake

Once `accept()` picks up the connection, the TCP channel is open. In the context of your IRC server, the **IRC handshake** begins — the client must now send:

```bash
PASS mypassword
NICK mynickname
USER myusername 0 * :myrealname
```

> The TCP handshake just opened the phone line.
> The IRC handshake is the actual conversation that follows —
> the client introduces itself and proves it knows the password.

Only after these three commands does the server send the welcome messages `001 002 003 004` and the client is fully registered.
