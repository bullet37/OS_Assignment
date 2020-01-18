#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>

#define TLB_SIZE               16
#define FRAME_SIZE             256
#define LOGICAL_ADDRESS_SIZE   1000
//FILE* address_file = fopen("./addresses-locality.txt", "r+");
FILE* address_file = fopen("./addresses.txt", "r+");
FILE* backing_store = fopen("./BACKING_STORE.bin","r+");

struct logical_address    
{
	int vir_add;       // virtual address  
	int phy_add;       // physical address 
	int page_num;    
	int offset;      
	int value;          
}la[LOGICAL_ADDRESS_SIZE];

void convert(int address, int index, struct logical_address* l)  // Convert the address into the page number and offset
{
	l[index].vir_add = address;
	l[index].page_num = address / 256;
	l[index].offset = address % 256; 
}

struct page_table     
{
	int page;       
	int frame;       
}pt[256];

void init_pt()   // initialize the pagetable 
{
  	
  	for(int i = 0; i < 256; i++)
	{
    	pt[i].page = -1;
    	pt[i].frame = -1;
	}	
    
}

int check_pagetable(int page,struct page_table* pt) // Test whether the page in the pagetable
{
  for(int i = 0;i < 256;i++){
    	if(pt[i].page == page)
    		return i;             
  }
  return -1;                        // not  found
}



// For FIFO strategy

struct page_table tlb[TLB_SIZE];

void init_tlb(){          // initialize the FIFO TLB 
	for(int i = 0; i < TLB_SIZE; i++)
	{
    	tlb[i].page = -1;
    	tlb[i].frame = -1;
	}
} 

int check_tlb(int page,struct page_table* pt)     // Test whether the page in the tlb
{
  int a;
  for(a = 0;a < 16;a++){
    	if(pt[a].page == page){
    		return a;
      	}
  }
  return -1;
}



void FIFO(FILE* backing_store, int SIZE)    // The FIFO
{
	int i = 0, j = 0, k = 0;                  // i: logic address, j: pagetable, k: TLB 
	int tlb_hit = 0, page_fault = 0, index_tlb, index_pt;
	char pmemory[SIZE][256]; 
	while(i < 1000)	{
		index_tlb = check_tlb(la[i].page_num, tlb);
		if(index_tlb >= 0)                // TLB hit
		{
			tlb_hit++;
			la[i].phy_add = tlb[index_tlb].frame * 256 + la[i].offset;
			la[i].value = pmemory[tlb[index_tlb].frame][la[i].offset];
			printf("%d\n", la[i].value);
			//printf("Virtual address: %d Physical address: %d Value: %d\n", la[i].vir_add, la[i].phy_add, la[i].value);
		}
		else if(check_pagetable(la[i].page_num, pt) >= 0)  // Pagetable hit
		{
			index_pt = check_pagetable(la[i].page_num, pt);
			la[i].phy_add = pt[index_pt].frame * 256 + la[i].offset;
			la[i].value = pmemory[pt[index_pt].frame][la[i].offset];
			tlb[k].page = la[i].page_num;                  // add to TLB
			tlb[k].frame = pt[index_pt].frame;
			k = (k + 1) % 16;           
			//printf("Virtual address: %d Physical address: %d Value: %d\n", la[i].vir_add, la[i].phy_add, la[i].value);
			printf("%d\n", la[i].value);        
		}
		else                                         //  Hit fail
		{
			page_fault++;
			fseek(backing_store, la[i].page_num * 256, SEEK_SET);     // Read from backing_store file 
			fread(pmemory[j], 1, 256, backing_store);  
			pt[j].page = la[i].page_num;                            // add to pagetable
			pt[j].frame = j;
			la[i].phy_add = j * 256 + la[i].offset;
			la[i].value = pmemory[j][la[i].offset];
			tlb[k].page = la[i].page_num;                          // add to TLB
			tlb[k].frame = j;
			j = (j + 1) % SIZE;                                 
			k = (k + 1) % 16;
			//printf("Virtual address: %d Physical address: %d Value: %d\n", la[i].vir_add, la[i].phy_add, la[i].value);
			printf("%d\n", la[i].value);         
		}
		i++;
	}
	printf("Page Faults = %d\n", page_fault);
    printf("TLB Hits= %d\n",tlb_hit);
}




// For LRU strategy

struct tlb_chain          //  TLB using lru
{
    int page;       
    int frame;
    struct tlb_chain *next;
}*tlb_head, *tlb_tail, *tlb_cur;          

struct phy_memory_chain   // The chain physical memory
{
	int index;        
	char page[256];      // The data
	struct phy_memory_chain *next;   
}*pm_head, *pm_tail, *pm_cur;


void tlb_push(int a)      // Add to the TLB tail
{
    tlb_cur->frame = a;
	tlb_cur->next = NULL;
    tlb_tail->next = tlb_cur;
    tlb_tail = tlb_cur;
}


void tlb_insert(int* num, int a)        // Inset into the TLB
{	
	 tlb_push(a);
	if (*num < TLB_SIZE)          // check whether need to remove the head          
        *num += 1;   
    else{
		tlb_cur = tlb_head->next;
    	tlb_head->next = tlb_cur->next;
    	free(tlb_cur);
	} 
}

void pm_push(int a)      // Add to the physical memory tail
{	
	pm_cur->index = a;
    pm_cur->next = NULL;
    pm_tail->next = pm_cur;
    pm_tail = pm_cur;
    
}

void pm_renew(int a)   // Add to the tail and removee the head of physical memory 
{
    pm_push(a);
    pm_cur = pm_head->next;
    pm_head->next = pm_cur->next;
    free(pm_cur);
}


void LRU(FILE* backing_store, int SIZE)
{
	int i = 0, j = 0, k = 0;
	int tlb_hit = 0, page_fault = 0, index_p, tlb_num = 0, page_num;
	struct tlb_chain *tlb_p; 
	struct phy_memory_chain *pm_p;   
	
	while(i < 1000)
	{
		
	    tlb_p = tlb_head;
	    pm_p = pm_head;
		
		tlb_cur = (struct tlb_chain *)malloc(sizeof(struct tlb_chain));
		pm_cur = (struct phy_memory_chain *)malloc(sizeof(struct phy_memory_chain));
		tlb_cur->page = la[i].page_num;      // add no:i vir_add to TLB
		
		
	    while (tlb_p->next != NULL)        // go though the TLB
	    {
	        if (tlb_p->next->page == tlb_cur->page)           // TLB hit
	        {
	        	tlb_hit++;
	            tlb_push(tlb_p->next->frame);                //add to the chain
	            tlb_cur = tlb_p->next;
	            tlb_p->next = tlb_cur->next;
	            free(tlb_cur);
	            
	            la[i].phy_add = tlb_tail->frame * 256 + la[i].offset;
	            
	            while(pm_p->next->index != tlb_tail->frame)
	            	pm_p = pm_p->next;
				
				pm_push(pm_p->next->index);      // add the physical_memory to tail
				for(k=0;k<256;k++)
	            	pm_tail->page[k] = pm_p->next->page[k];
	            pm_cur = pm_p->next;
	            pm_p->next = pm_cur->next;
	            free(pm_cur);
	            
	            la[i].value = pm_tail->page[la[i].offset];
	            printf("%d\n", la[i].value);
	            break;
	        }
	        tlb_p = tlb_p->next;
	    }
	    if(!tlb_p->next)             // TLB miss
	    {
	    	if(check_pagetable(la[i].page_num, pt) >= 0)   // pagetable hit
	    	{
	    		index_p = check_pagetable(la[i].page_num, pt);
				la[i].phy_add = pt[index_p].frame * 256 + la[i].offset;
				tlb_insert(&tlb_num, pt[index_p].frame);            // insert to tlb chain
				while(pm_p->next->index != tlb_tail->frame)
	            	pm_p = pm_p->next;
	            pm_push(pm_p->next->index);        // move to the tail of pm
	            for(k=0;k<256;k++)
	            	pm_tail->page[k] = pm_p->next->page[k];
	            pm_cur = pm_p->next;
	            pm_p->next = pm_cur->next;
	            free(pm_cur);
	            la[i].value = pm_tail->page[la[i].offset];
	            printf("%d\n", la[i].value);
	            
			}
	    	else             // Pagetable miss 
	    	{
	    		page_fault++;
	    		if(j >= SIZE)        // if is already full, release the head and insert.
	    		{
	    			int m = 0; 
					while(pt[m].frame != pm_head->next->index)   // find the physical memory head
						m++;
					pt[m].page = la[i].page_num;
					pm_renew(pm_head->next->index);
					tlb_insert(&tlb_num, pm_tail->index); 
	    			
				}
				else            // pagetable not full, insert
				{
					pt[j].page = la[i].page_num;
	    			pm_push(j);
        			pt[j].frame = pm_tail->index;
        			tlb_insert(&tlb_num, pm_tail->index);
				}
				 
	    		fseek(backing_store, la[i].page_num * 256, SEEK_SET); // find from the backing store file
	    		fread(pm_tail->page, 1, 256, backing_store);          // read to the physical memory
	    		
				la[i].phy_add = pm_tail->index * 256 + la[i].offset;
				la[i].value = pm_tail->page[la[i].offset];
				printf("%d\n", la[i].value);
				j++;
			}
		}
		i++;
	}
	printf("Page Faults = %d\n", page_fault);
    printf("TLB Hits = %d\n",tlb_hit);
}

// Main 
int main(int argc, char * const argv[])
{
	int SIZE, op, t;
	void (*strategy)(FILE*, int);
	char* add = (char*)malloc(sizeof(char)*7);
	init_pt(); 
	init_tlb();
	tlb_head = (struct tlb_chain *)malloc(sizeof(struct tlb_chain));
    tlb_head->next = NULL;
    tlb_tail = tlb_head;
    pm_head = (struct phy_memory_chain *)malloc(sizeof(struct phy_memory_chain));
    pm_head->next = NULL;
    pm_tail = pm_head;  
	while((op = getopt(argc, argv, "n:t:")) != -1)   // -n: space, -t: strategy
	{
		switch(op)
		{
			case 'n':
				SIZE = atoi(optarg);
        		break;
			case 't':
				if(strcmp(optarg, "fifo") == 0)
					strategy = FIFO;
				else if(strcmp(optarg, "lru") == 0)
					strategy = LRU;
				else 
					printf("Wrong 't' parameter\n");
					exit(-1);
        		break;
			default:
				printf("Wrong input\n");
				exit(-1);
		}
  }	       
    
    for(int i=0; i<LOGICAL_ADDRESS_SIZE;i++)
    {
    	fgets(add, 7, address_file);
    	t = atoi(add);
    	convert(t, i, la);
	}
  
  (*strategy)(backing_store, SIZE);
  return 0;
}
