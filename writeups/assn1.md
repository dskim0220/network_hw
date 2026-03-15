Assignment 1 Writeup
=============

My name: [KIM DAESUNG]

My POVIS ID: [daesungkim]

My student ID (numeric): [20210888]

This assignment took me about [6] hours to do (including the time on studying, designing, and writing the code).

Program Structure and Design of the StreamReassembler:
[In this assignment, I utilized the std::map<size_t, std::string> structure to store the unassembled substrings. This choice was driven by the map's ability to automatically sort elements by their keys(indices) in assending order, which is highly efficient to manage sequential data, and merge the segments.

The push_substring method operates in three primary stages:
1. Boundary Calculation: It first calculates the first_unread and first_unacceptable indices to determine the valid capacity window.
2. Data Slicing: Based on these boundaries, the method identifies the precise start and end indices of the incoming data to slice ot into a valid substring.
3. Merging and Reassembly: It them merges the sliced string with any existing overlapping segments in the map to maintain a minimal set of contiguous blocks. I IMPLEMENTED A MERGING LOGIC to ensure NO OVERLAPPING SUBSTRINGS are stored, strictly adhering to the memory constraints defined by the capacity.
Finally, the method iterates through the map to check if the leading segments are ready to be written into the ByteStream in the correct sequence.
]

Implementation Challenges:
[There are two challenging points: 
1. Handling boundary conditions: Accurately calculating boundary conditions was a critical task, as these boundaries dictate the core logic of the reassembler-including whether a segment can be accepted into the map, the specific range of strings to be extracted, and the sequential data ready for the ByteStream. To ensure the precision of these calculations and avoid off-by-one errors, I utilized manual visualization, mapping out indices and boundaries on paper. This approach helped me clarify the relationships between _next_idx, _first_unacceptable, and the incoming segment's range, allowing for more robust implementation of the sliding window.  

2. Implementing the Merging Logic: Implementing the merging logic was the most challenging aspect of the project. The primary difficulty lay in accounting for all possible edge cases where new data might overlap with multiple existing segments in the map.
To strictly adhere to the memory limit(capacity) constraints, I had to ensure that the map never stored redundant data. This required a meticulous process of:
1. Identifying overlapping ranges.
2. Slicing and merging segments into a single contiguous block.
3. Updating the std::map while simultaneously keeping the _unassembled_bytes count accurate. 

Maintaining these assumptions and invariants-specifically, that no two segments in the map overlap-was exceptionally time-consuming. I overcame this by systematically tracing index boundaries and ensuring that every insertion or deletion was balanced by a corresponding update to the byte counter. This rigorous approach allowed me to maintain the preconditions of the ByteStream while keeping the memory footprint within the allowed limits.

I debugged the implementation primarily using the make check_lab1 command. For every failed test case, I tried to analyze the error messages and logs to identify logic flaws and syntax errors. Through iterative process of testing and fixing, the reassembler successfully passed all test cases. 

]

Remaining Bugs:
[Currently, there are no known bugs as the implementation passes all provided test cases. 
However, from a performance perspective, the current merging logic involves iterating through the map to find and resolve overlaps, resulting in a time complexity of O(N) per call.]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
