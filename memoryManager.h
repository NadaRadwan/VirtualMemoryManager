#define NUMFRAMES 256 //page size
#define PAGESIZE 256 //num bytes per page
#define NUMPAGES 256 //num entries in page table
#define NUMTRANS 16 //num entries in translation lookaside buffer
#define NUMLOGADDRESSES 1000 //num logical addresses in addresses.txt

#include "stdlib.h"
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

struct address{
	int logicalAddr; //from text file
	int physicalAddr;
	int pageNum;
	int offset;
	int value;
};

//global variables
struct address a = {0x00000000,0,0,0,0};

int logicalAddresses[NUMLOGADDRESSES]; //stores logical addresses from addresses.txt
int pageTable[NUMPAGES]; //array representing pageTable storing frameNumbers indexed by pageNum
int tlb[NUMTRANS][2]; //translation lookaside buffer - 2D array storing page number and corresponding frame number
int mainMemory[NUMFRAMES][PAGESIZE]; //2D array storing the value stored at each frame on each page
int transCount=0; //keep track of num elements in translation lookaside buffer (max 16)
int frameNum=0; //keep track of available frame number to be assigned a page
int pageFault=0; //keep track of number of times a page is not found in the page table when searched
int tlbHit=0; //keep track of number of times a page is found in tlb

//functions defined in memoryManager.c
void *backingStoreLookup();
void *pageTableLookup();
void *tlbLookup();
void *pgAndOffsetExtractor();
void *initPageTable();
void *initTlbTable();
void *getValue();
