Assignment 6 Writeup
=============

My name: [KIM DAESUNG]

My POVIS ID: [daesungkim]

My student ID (numeric): [20210888]

This assignment took me about [4] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the Router:
[In the Router class, there are two main methods to implement: add_route and route_one_datagram.

1.add_route
This method receives route information (route_prefix, prefix_length, next_hop, interface_num) and adds it to the routing table. To implement this, I declared a private RouteEntry struct and a _routing_table vector. When the method is called, it constructs a new entry with the given information and pushes it to the routing table.

2.route_one_datagram
This method handles the core routing logic. When called, it searches the routing table to find the longest-prefix-match route for the incoming datagram.
To determine whether a route matches the datagram's destination, I utilized a bitwise XOR operation. By XORing the destination IP with the route_prefix, the identical bits become zero. Right-shifting (>>) this result by (32 - prefix_length) discards the irrelevant lower bits. If the final value is zero, it confirms a valid match. I also explicitly handled the edge case where prefix_length is zero.

If no matching route is found, the datagram is dropped. If a match exists, the method checks the datagram's TTL (Time to Live). If the TTL is zero or one, the datagram is dropped. Otherwise, the TTL is decremented. Finally, the datagram is forwarded to the next hop. If the matched route does not specify a next_hop, the datagram's destination address itself is used as the next hop.
]

Implementation Challenges:
[The most challenging aspect of this assignment was designing the longest-prefix matching logic. Initially, I was unsure how to efficiently determine if the prefix bits matched. However, after sketching out a few scenarios on paper, I realized that bitwise operations provided an elegant solution. Since a bitwise XOR operation yields 0 when two bits are identical, I applied an XOR between the destination IP and the route prefix, followed by a right shift to evaluate the match. Once this core logic was established, implementing the rest of the method was straightforward.]

Remaining Bugs:
[The implementation passes all test cases, and no remaining bugs have been observed.]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
