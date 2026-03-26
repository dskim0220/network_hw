Assignment 2 Writeup
=============

My name: [KIM DAESUNG]

My POVIS ID: [daesungkim]

My student ID (numeric): [20210888]

This assignment took me about [5] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPReceiver and wrap/unwrap routines:
[Upon receiving a segment with the SYN flag set, the TCP receiver identifies the header's sequence number(seqno) as the Initial Sequence Number(ISN) and updates the _set_isn flag to 1. Using this ISN, the receiver calculates the stream_idx through the unwrap function. Finally, the receiver pushes the payload, the calculated stream_idx, and the FIN flag to the StreamReassembler.] 

Implementation Challenges:
[The most challenging part of the implementation was unwrapping a 32-bit sequence number(seqno) into a 64-bit absolute sequence number(absolute_seqno). While the reverse process(wrapping) was straightforward, unwrapping required finding the value closest to the checkpoint. Initially, I attempted to manually check the values surroinding K*32, where K=checkpoint/2^32. However, this approach led to numerous edge cases that were difficult to manage.
To simplify the logic, I shifted to a method centered on the relative displacement from the checkpoint. By calculating the difference(seqno-ISN-(checkpoint(mod 2^32))) and casting it to a 32-bit signed integer(int32_t), I could automatically determine the shortest distance to the correct absolute_seqno. This signed offset was then added to the checkpoint. Finally, I implemented a safety check to ensure that if the resulting absolute_seqno was negative, it was adjusted by adding 2^32 to maintain a valid range.]
Remaining Bugs:
[Currently, there are no known bugs in the implementation. The code passed all test cases provided in the check_lab2 suite without any failures. Furthermore, as the unwrapping logic includes specific safety checks to hand edge cases-such as preventing the absolute_seqno from underflowing below zero-the implementation is expected to be robust and reliable under various conditions.]

- Optional: I had unexpected difficulty with: [
I encountered unexpected difficulty managing the various types of indices within the protocol. Simultaneously handling 32-bit sequence numbers, 64-bit absolute sequence numbers, and the zero-indexed stream index was initially confusing, leading to several indexing errors during the early stages of implementation.
]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
