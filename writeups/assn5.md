Assignment 5 Writeup
=============

My name: [KIM DAESUNG]

My POVIS ID: [daesungkim]

My student ID (numeric): [20210888]

This assignment took me about [9] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the NetworkInterface:
[The NetworkInterface class has three methods: 1. send_datagram 2. recv_frame 3. tick.

1. send_datagram
The send_datagram method resolves the MAC address for the next hop's IP address and transmits the Internet datagram. 
It searches the ARP cache(_arp_table) for the MAC address corresponding to the target IP. If a match is found, it immediately encapsulates the datagra into an Ethernet frame(as the payload) and sends it.
If the MAC address is unknown, it queues the datagram in _waiting_dgrams and broadcasts an ARP request. To prevent network flooding, it checks _arp_timer; a new ARP request is sent only if no request has been sent to that IP within the last 5 seconds.(5,000ms).

2. recv_frame
The recv_frame method processes incoming Ethernet frames and learns network mappings. It ignores any frames where the destination Ethernet address is neither its own MAC address nor the broadcast address.
If a valid IPv4 frame is received, it parses the payload and returns the Internet datagram upon success.
For ARP frames, it parses the ARPMessage and caches the sender's IP to MAC mapping for 30 seconds(30,000ms). If it receives an ARP request targeting its own IP, it sends back an ARP reply. 
It checks if there are any pending datagrams in the waiting queue for the newly learned IP. If so, it encapsulates them into Ethernet frames, transmits them, and safely clears the queue and the timer for that IP.

3. tick
The tick method manages the state of the network interface as time elapses.
The ARP cache stores entries with a Time to Live(ttl). It decreases the ttl by ms_since_last_tick. To prevent integer underflow, if the remaining TTL is less than or equal to the elapsed time, the entry is considered expired and safely erased from the table. 
It also updates the _arp_timer by incrementing the elapsed time for pending ARP requests, allowing future requests to be sent once the 5 second threshold is met.]

Implementation Challenges:
[The most challenging part of this assignment was designing the logic to handle incoming ARP frames within the recv_frame method. While parsing the payload and caching the sender's IP-to-MAC mapping was straightforward, orchestrating the subsequent steps required deeper consideration of the protocol's bidirectional nature.
I realized that just as my interface broadcasts requests when it lacks a MAC address, other nodes will do the same. To address this, I implemented logic to check if an incoming ARP frame was a request targeting my interface's IP, ensuring that it promptly generates and returns an ARP reply with its own MAC address.
The crucial breakthrough came when I understood the exact purpose of the waiting queue. I realized that once an ARP reply is received and the target MAC address is learned, the interface must immediately retrieve and transmit all datagrams that were queued up because they could not be sent earlier. Implementing this queue-flushing mechanism was the final key to completing the method's logic and maintaining the invariant that no datagram is permanently lost just because of a missing ARP entry.]

Remaining Bugs:
[Based on the provided test suite, no remaining bugs or unhandled edge cases were found in the current implementation.]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
