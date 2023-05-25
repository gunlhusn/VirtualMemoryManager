# Virtual Memory Manager

<h3> This repository implements a simulated virtual memory manager 
    by accomplishing the following:</h3>

- Implement TLB and Page Table structures
- Using the above, map virtual addresses to physical ones
- Update the TLB using FIFO or LRU logic
- Handle the case where physical memory is less than the virtual memory by implementing clock (circular second chance) algorithm
- Calculate and output the translated address count, page fault rate, and TLB hit rate
- Read values from disk simulator (bin)
- Read (virtual addresses) and write (corresponding physical addresses, values and statistics) to file (txt)
