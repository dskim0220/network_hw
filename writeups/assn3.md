Assignment 3 Writeup
=============

My name: [KIM DAESUNG]

My POVIS ID: [daesungkim]

My student ID (numeric): [20210888]

This assignment took me about [7] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPSender:
[   
I have implemented four major methods in TCPSender Class: fill_window, ack_received, tick and send_empty_segment.

    1. fill_window

The fill_window method is responsible for reading data from the ByteStream and sending segments as long as there is available space in the receiver's window. First, the effective window size is determined based on the last received window_size. If the window size is 0, it is treated as 1 to perform zero-window probing. The method then iterates while there is available space in the window. If the SYN flag has not been sent, the method sets the SYN flag and marks _syn_sent as true. It calculates the payload size by taking the minimum of the remaining window space and MAX_PAYLOAD_SIZE. If the ByteStream reaches EOF and there is still space in the window for a FIN flag, _fin_sent and the segment's FIN flag are set to true. Each generated segment is assigned a sequence number and pushed into both the _segments_out queue for transmission and the _outstanding queue for retransmission tracking. Finally, _next_seqno and _bytes_in_flight are updated. If the retransmission timer is not running, it is started and the elapsed time is reset to 0.

    2. ack_received

This method receives ackno and window_size from the receiver and removes acknowledged segments from _outstanding in the range from ackno to ackno+window_size. After converting ackno to abs_ackno, it compares with the _next_seqno value that the receiver has, and if it is greater than that value, it is out of range so the method terminates immediately. Then, _last_window_size is updated with the newly received window_size. If abs_ackno is greater than the last acknowledged sequence number (_last_acked_seqno), that value is changed to abs_ackno, the rto (_current_rto) is set to the initial value (_initial_retransmission_timeout), and the retransmission count (_consecutive_retransmissions) and elapsed time (_elapsed_time) are initialized to 0.
Afterward, it iterates through the _outstanding queue and removes entries if their abs_seqno is within the range determined above. If the queue becomes completely empty, the timer is turned off, and fill_window is called since the window needs to be filled again.

    3. tick

This method first receives the time since the method was last called until it is called again (ms_since_last_tick) as input to calculate the total elapsed time (_elapsed_time). If _elapsed_time is greater than RTO, the segment at the front of _outstanding is put back into _segments_out for retransmission. According to the rules, if the window size is not 0, _consecutive_retransmissions is incremented by 1 and the RTO value is doubled. Then, the timer is reset to 0.

    4. send_empty_segment

This method creates a segment with length 0. First, seqno is set by wrapping _next_seqno, and then it is placed in the _segments_out queue.]

Implementation Challenges:
[   1. Variable Identification and State Management

The most difficult part of the implementation was determining which additional
variables were required beyond the starter code. I had to introduce members
such as _outstanding for tracking segments, _current_rto for the timeout,
and _is_timer_running to manage the timer state. Deciding on the types and
initialization of these variables early in the development was a hurdle.
I resolved this by meticulously re-reading the assignment specs, focusing
on method definitions and rules for outstanding segments. I mapped out every
value that needed to be tracked and iteratively refined the variable set
during implementation, removing unnecessary components as the logic matured.

    2.FIN Flag Logic and Boundary Conditions
I encountered a specific challenge while handling the FIN flag logic.
Originally, I only checked if a FIN had been sent (_fin_sent == 0) before
setting the flag, but I neglected to verify if the stream had actually
reached EOF. This caused the flag to be set incorrectly. To fix this, I
re-examined the state transition rules for the FIN flag and revised the
code to ensure the flag is only triggered when both conditions—no prior
FIN sent and reaching the end of the stream—are satisfied.]

Remaining Bugs:
[One potential edge case concerns the "Zero Window" persistence. When the receiver's window size is zero, the current implementation intentionally avoids doubling the RTO or increasing the consecutive retransmission count, adhering to the standard zero-window probing behavior. While it is correct for probing, the code does not implement a definitive timeout or a maximum retry limit specifically for a persistent zero-window state.
Aside from the limitation, the implementation has been verified to pass all provided test cases and handles standard packet loss and reordering.]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
