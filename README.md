# Virtual Memory Manager

A program that translates logical to phyical addresses for a virtual space of size 2^16 - 65,536 bytes. It reads from a file containing 32-bit integer logical addresses and using TLB and a page table, translates each to a physical address and outputs the value of the byte stored at the physical address.

To execute:
 - gcc memoryManager.c -pthread -o memoryManager
 - ./memoryManager addresses.txt
