

#include"memoryManager.h"

int main (int argc, char* argv[])
{
	FILE *file;
	int i=0;
	pthread_t id; //id of the pthread
	int numTranslations=0; //counter to keep track of num logical address translations 
	
	//reading logical addresses from file & storing them in an array
	file=fopen(argv[1],"r");

	if( file == NULL){
    		printf("\n fopen failed\n");
      	 	pthread_exit(NULL);
   	 }	

	fscanf(file, "%d", &i);
	int x=0; //counter to populate logicalAddresses array
	while(!feof (file)){
		//printf("%d", i);
		logicalAddresses[x++]=i; //add the logical address i to the array
		fscanf(file, "%d", &i);
	}
	fclose(file);


	//Initialize the tlb
	pthread_create(&id, NULL, initTlbTable,NULL);
	pthread_join(id,NULL);
	
	//Initialize the pagetable
	pthread_create(&id, NULL, initPageTable, (void *)NULL);
	pthread_join(id,NULL);

	for(i=0;i<NUMLOGADDRESSES;i++)
	{
		a.logicalAddr=logicalAddresses[i];
		
		pthread_create(&id,NULL, pgAndOffsetExtractor, NULL);
		pthread_join(id,NULL);
		pthread_create(&id, NULL, tlbLookup,NULL);
		pthread_join(id,NULL);		
		printf("Virtual Address: %i,  Physical Address: %i,  Value : %i\n",a.logicalAddr,a.physicalAddr,a.value);
		numTranslations++;
	}
	printf("Number of translated Addresses = %d\n",numTranslations);
	printf("Page Faults = %d\n",pageFault);
	float pageFaultRate=(float)pageFault/(float)numTranslations;
	printf("Page Fault Rate = %.3f\n",pageFaultRate);
	printf("TLB Hits = %d\n",tlbHit);
	float tlbHitRate=(float)tlbHit/(float)numTranslations;
	printf("TLB Hit Rate = %.3f\n",tlbHitRate);

	return 0;
}

void *initTlbTable(){
	int i;
	for(i=0; i<NUMTRANS; i++)
		tlb[i][0] = -1;

	pthread_exit(0);
}

void *initPageTable(){
	int i;
	for(i=0; i<NUMPAGES; i++)
		pageTable[i]=-1;

	pthread_exit(0);
}

//This function extracts the pageNum and the offset from the logical address
void *pgAndOffsetExtractor()
{
	int maskedInteger=a.logicalAddr &0xFFFF; //mask rightmost 16bits
	a.pageNum=maskedInteger >> 8;
	a.offset=maskedInteger & 0xFF;
	//printf("PageNum: %d\n",a.pageNum);
	pthread_exit(NULL);
}

void *tlbLookup(){
	pthread_t id;
	int flag = 0; //indicate where to look
	int i;

	for ( i=0 ; i < NUMTRANS ; i++ ) //searching tlb
	{
		if ( tlb[i][0] == a.pageNum )
		{
			a.physicalAddr=tlb[i][1]*PAGESIZE+a.offset;	  
			tlbHit++;
			flag = 1;	
			break;
		}
	}
	
	//If pageNum is not inside tlb, look inside pageTable	
	if (flag == 0)
	{
		//printf("pageNum is not inside tlb");
		pthread_create(&id, NULL, pageTableLookup,NULL);
		pthread_join(id,NULL);

		//Update tlb
		tlb[transCount][0] = a.pageNum;
		tlb[transCount][1] = frameNum;
		transCount++;

		//if num elements in tlb=16, start overriding elements at 0 again
		if (transCount == NUMTRANS){
			transCount = 0;
		}
	}

	return 0;
}


/*looks up frameNum from pageNum
*if frameNum is not found, pageFault is incremented
*and we look up the frameNum in backingStore
*/

void *pageTableLookup(){
	pthread_t id;

	if( pageTable[a.pageNum] == -1){	
		pageTable[a.pageNum]=frameNum; //assign the the logical address to an available frame
		pthread_create(&id, NULL, backingStoreLookup, NULL);
		pthread_join(id,NULL);
		frameNum++;
		pageFault++;
	}

	pthread_create(&id, NULL, getValue, NULL);
	pthread_join(id,NULL);

	//physicalAddr=frameNum*PageSize + offset
	a.physicalAddr=pageTable[a.pageNum]*PAGESIZE+a.offset;
	
	pthread_exit(NULL);
}


//fetch the the page from backingStore to mainMemory
void *backingStoreLookup(){
	//If page x caused a fault bring in that page to memory at the frameNum location
	char temp[NUMFRAMES]; //stores values at each offset in the page
	int i;

	FILE *file;
	file=fopen("BACKING_STORE.bin","r");

   	 if(file == NULL){
    		printf("\n fopen failed\n");
      	 	pthread_exit(0);
   	 }
	
	//sets file position of file to the pageNum*PAGESIZE
	if(fseek(file,a.pageNum*PAGESIZE,SEEK_SET)!=0)
	{
		printf("\nfseek failed\n");
		pthread_exit(0);	
	
	}

	//reads PAGESIZE elements each of size 1 from file into temp
	if(fread(temp,1,PAGESIZE,file)!=PAGESIZE ){
		printf("\nfread failed\n");
		pthread_exit(0);
	}

	fclose(file);

	for(i=0; i<PAGESIZE; i++)
		mainMemory[frameNum][i]=temp[i]; //populate the values at each offset value inside the frame

	pthread_exit(0);
}

// get the value stored at the logical address from mainMemory
void *getValue(){
	a.value=mainMemory[pageTable[a.pageNum]][ a.offset ];//Read the value from physical memory
	pthread_exit(NULL);
}


