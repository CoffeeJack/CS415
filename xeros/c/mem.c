/*********************
 * CPSC 415
 * Due: Nov 6, 2012
 * Assignment 02
 * By: Vincent Tsui
 * ID: 67221069
 *********************/
/*********************
 * CPSC 415
 * Due: Oct 9, 2012
 * Assignment 01
 * By: Vincent Tsui
 * ID: 67221069
 *********************/

/* mem.c : memory manager
 */

#include <xeroskernel.h>
#include <i386.h>

/* Your code goes here */
extern  long	freemem; 	/* start of free memory (set in i386.c) */
extern char	*maxaddr;	/* max memory address (set in i386.c)	*/

static struct memHeader* pFreeMemory;
static unsigned char* memStart;

/************************************
 * function name: kmeminit
 * require: 
 * return:
 * comment: Sets up the free memory link list. It puts the HOLE into consideration.
*************************************/

extern void kmeminit( void ){
    kprintf("raw free memory at start: %d\n",freemem);
    kprintf("max addr = %d\n",maxaddr);
    
    memStart = freemem;
    pFreeMemory = memStart; //points available free memory to start of non-kernel stack memory
    pFreeMemory->size = HOLESTART - (freemem + sizeof (struct memHeader)); //set first segment to be up to the start of the hole
    pFreeMemory->next = HOLEEND; //set next free mem block to be at the end of the HOLE
    pFreeMemory->prev = NULL; //first block of free memory should have no previous 
    pFreeMemory->sanityCheck = freemem; //can be anything, set to freemem for debug
    
    //DEBUG CODE
    //kprintf("free memory list starts at %d\n",pFreeMemory);
    //kprintf("free memory list starts at %d\n",HOLESTART);
    
    pFreeMemory->next->next = NULL; //no memory after maxaddr
    pFreeMemory->next->prev = pFreeMemory; //link the two blocks
    pFreeMemory->next->size = (unsigned char*)maxaddr - (HOLEEND + sizeof (struct memHeader));
    pFreeMemory->next->sanityCheck = freemem;

    //printFreeMemList();
}

/************************************
 * function name: kmalloc
 * require: size > 0 && size < maxaddr
 * return: memory address of allocated memory, 0 if no suitable memory found
 * comment: used to allocate memory for processes
*************************************/

extern void* kmalloc( int size ){
    
    struct memHeader* memSlot;
    int original_size;
    struct memHeader* original_next;
    struct memHeader* original_prev;
    unsigned char* original_sanity_check;
    
    //DEBUG CODE
    //kprintf("---------------------\n");
    //kprintf("in kmalloc\n");
    
    //Computer the amount of memory for this request
    
    int amnt = (size)/16 + ((size%16)?1:0); //figure out how many blocks of 16 bytes needed
    
    amnt = amnt*16 + sizeof(struct memHeader); //include the header space
    
    //Scan free memory list for spot
    
    pFreeMemory = memStart; //reset list pointer to point to beginning of list
    do{
        
        //kprintf("chunk size = %d bytes\n",pFreeMemory->size);
        
        if(pFreeMemory->size>=amnt){
            //finds free memory chunk with enough space
            //kprintf("memory found\n");
            break;
        }
        else{
            pFreeMemory = pFreeMemory->next; //traverse list
        }     
    }while(pFreeMemory->next);
    

    if(pFreeMemory->next==NULL && pFreeMemory->size < amnt){
        return 0; //no suitable memory found
    }
    
    //Determine where slot starts and overlay
    
    //DEBUG CODE
    //kprintf("suitable memory at: %d\n", pFreeMemory);
    
    //Save original state for later list adjustment
    original_size = pFreeMemory->size;
    original_next = pFreeMemory->next;
    original_prev = pFreeMemory->prev;
    original_sanity_check = pFreeMemory->sanityCheck;
    
    //Fill in memSlot fields
    memSlot = pFreeMemory;
    memSlot->size = amnt;
    
    //DEBUG CODE
//    kprintf("memslot size = %d\n",memSlot->size);
//    kprintf("freememlist size = %d\n",pFreeMemory->size);
    
    //kprintf("amnt = %d\n",amnt);
    
    //Adjust free memory list

    pFreeMemory = memSlot + amnt/16;
    pFreeMemory->size = original_size - memSlot->size;
    pFreeMemory->next = original_next;
    pFreeMemory->next->prev = pFreeMemory;
    pFreeMemory->prev = original_prev;
    pFreeMemory->sanityCheck = original_sanity_check;
    
    //kprintf("sanity check = %d\n",pFreeMemory->sanityCheck);
	   

    if(pFreeMemory->prev==NULL){
        memStart = pFreeMemory; //if allocated memory is at beginning, readjust pointer to start of free memory
    }
    
    //DEBUG CODE
//    kprintf("HEADER memSlot addr = %d\n",memSlot);
//    kprintf("amnt = %d\n",amnt);
//    kprintf("pFreeMemory addr = %d\n",pFreeMemory);   
//    kprintf("pFreeMemory datastart addr = %d\n",pFreeMemory->dataStart);
    
    //return memSlot->dataStart

    return memSlot->dataStart;
}

/************************************
 * function name: kfree
 * require: ptr = valid ptr to previously allocated memory
 * return:
 * comment: calls defrag(), which combines memory if contiguous chunks found
*************************************/

extern void kfree( void* ptr ){
    
    //DEBUG CODE
    kprintf("---------------------\n");
    
    struct memHeader* memSlot = ptr - sizeof (struct memHeader); //back track to include header block
    
    pFreeMemory = memStart; //reset list pointer to point to beginning of list
    
    
    if(memSlot < memStart){
        //memory freed is before the first segment of free memory
        
        //DEBUG CODE
        //kprintf("in case 1\n");
        pFreeMemory->prev = memSlot;
        memSlot->next = pFreeMemory;
        memStart = memSlot;
           
    }else{
        //memory freed is between the start/end of current free memory
        
        //DEBUG CODE
        //kprintf("in case 2\n");
        do{
            //kprintf("looping\n");
            if(memSlot>=pFreeMemory && memSlot < pFreeMemory->next){
                break;
            }
            else{
                pFreeMemory = pFreeMemory->next; //traverse list
            } 
        }while(pFreeMemory->next); 
         
        struct memHeader *temp_next = pFreeMemory->next;
        
        pFreeMemory->next = memSlot;
        memSlot->prev = pFreeMemory;
        
        memSlot->next = temp_next;
        temp_next->prev = memSlot;
            
    }
    
    //DEBUG CODE
//    kprintf("free mem next = %d\n",pFreeMemory->next);
//    kprintf("free mem ptr + size = %d\n",pFreeMemory + pFreeMemory->size/sizeof(struct memHeader));
//
//    kprintf("free mem now = %d\n",pFreeMemory);
//    kprintf("free mem prev + size = %d\n",pFreeMemory->prev + pFreeMemory->prev->size/sizeof(struct memHeader));
    
    //merge memory if they are adjacent to one another
    defrag();
}
/************************************
 * function name: defrag
 * require: 
 * return:
 * comment: checks neighbor segments on free memory list. If contiguous, it will combine.
*************************************/

void defrag(void){
    pFreeMemory = memStart; //reset list pointer to point to beginning of list
    int memory_boundary;
    
    do{ 
        
        memory_boundary = (pFreeMemory + pFreeMemory->size/sizeof(struct memHeader));
        
        if(pFreeMemory->next == memory_boundary){
            //adjacent segment found
            //kprintf("merge next\n");
            struct memHeader* next_next = pFreeMemory->next->next;

            pFreeMemory->size = pFreeMemory->size + pFreeMemory->next->size;

            pFreeMemory->next = next_next;
            next_next->prev = pFreeMemory;
        }
        
        memory_boundary = (pFreeMemory->prev + (pFreeMemory->prev->size/16));

        if(pFreeMemory == memory_boundary){
            //adjacent segment found
            //kprintf("merge prev\n");
            
            pFreeMemory->prev->next = pFreeMemory->next;
            pFreeMemory->next->prev = pFreeMemory->prev;
            pFreeMemory->prev->size = pFreeMemory->prev->size + pFreeMemory->size;
        }
        
        //DEBUG CODE
        //kprintf("combined size = %d\n",pFreeMemory->size);
        //kprintf("sanity check = %d\n",pFreeMemory->sanityCheck);
        
        pFreeMemory = pFreeMemory->next; //traverse list
         
    }while(pFreeMemory->next); 
}

/************************************
 * function name: printFreeMemList
 * require: 
 * return:
 * comment: used for debug
*************************************/

extern void printFreeMemList(void){
    pFreeMemory = memStart; //reset list pointer to point to beginning of list
    while(pFreeMemory->next){
         kprintf("list addr: %d with size: %d\n",pFreeMemory,pFreeMemory->size);       
         pFreeMemory = pFreeMemory->next; //traverse list    
    }
    kprintf("list addr: %d with size: %d\n",pFreeMemory,pFreeMemory->size);   
}
