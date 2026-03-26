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
[The most challenging part of the implementation was to translate a sequence number(seqno) to an absolute sequence number(absolute_seqno). Translating an absolute_seqno to a seqno was easy, but the reverse was difficult. In the first, I tried the reverse sequences of the logic to calculate seqno
(absolute_seqno = K*2^32+seqno-ISN), and set K to checkpoint/2^32, and observed the front and the back of the temporal absolute_seqno. But, when I tried this way, there were many cases to consider, so it was complex. To overcame the problem, I tried to use the gap between the checkpoint and the absolute_seqno. The equation above can be changed to this equation: absolute_seqno = checkpoint-r+seqno-ISN(r is the remainder when the checkpoint is divided to 2^32.)
Then, the gap between the absolute_seqno and the checkpoint is seqno-ISN-r. if the gap is equal or larger than ]

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
