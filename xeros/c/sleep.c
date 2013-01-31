/*********************
 * CPSC 415
 * Due: Nov 6, 2012
 * Assignment 02
 * By: Vincent Tsui
 * ID: 67221069
 *********************/

/* sleep.c : sleep device (assignment 2)
 */

#include <xeroskernel.h>

/* Your code goes here */

static int global_ticker;

/************************************
 * function name: sleep
 * require: 
 * return:
 * comment: add a process to sleep queue
*************************************/

void sleep(struct pcb* process, unsigned int milliseconds){
    
    int ticks = msToTicks(milliseconds);
    
    //kprintf("ticks = %d\n",ticks);
    
    addToSleepQueue(process,ticks);
}

/************************************
 * function name: tick
 * require: 
 * return:
 * comment: decrement the duration of processes in sleep queue. Process is add back to ready queue if duration <= 0
*************************************/

void tick(){
    
    //kprintf("ticker = %d\n",global_ticker);
    global_ticker++;
    struct pcb* process_to_wake;
    struct pcb* iterator = queue[SLEEP_BLOCKED];

    if(queue[SLEEP_BLOCKED]){
        
        //decrement all the sleep duration
        while(iterator->next){
                
            iterator->sleep_duration = iterator->sleep_duration - 1;
            iterator = iterator->next;
        }
        iterator->sleep_duration = iterator->sleep_duration - 1;

        if(queue[SLEEP_BLOCKED]->sleep_duration <= 0){
            //kprintf("ticker = %d\n",global_ticker);            
            
            //reset global ticker
            global_ticker = 0;
            
            ready(queue[SLEEP_BLOCKED]); //wake up process
            
            //kprintf("printing sleep queue\n");
            //printQueue(queue[SLEEP_BLOCKED]);
        }
    } 
}

/************************************
 * function name: msToTicks
 * require: 
 * return:
 * comment: converts milliseconds to ticks (size of tick indicated in kernel.h)
*************************************/

int msToTicks(unsigned int milliseconds){        
    
    if(milliseconds <= QUANTUM_SIZE) return 1;
    
    return milliseconds/QUANTUM_SIZE + (milliseconds%QUANTUM_SIZE ? 1 : 0);
}

/************************************
 * function name: addToSleepQueue
 * require: 
 * return:
 * comment: add process to sleep queue ordered by ticks
*************************************/

void addToSleepQueue(struct pcb* process, int tick){
    
    struct pcb* iterator = queue[SLEEP_BLOCKED];
    struct pcb* old_next;
    int total = 0;
    int i = 0;
    
    for(i=0;i<15000;i++); //delay to prevent screw up

    //kprintf("in adding to sleep queue\n");
    process->sleep_duration = tick;
    
    if(!iterator){
        //sleep queue is empty
        //kprintf("empty sleep queue!\n");
        queue[SLEEP_BLOCKED] = process;
        //process->sleep_duration = tick;
    }
    else{
        if(tick < iterator->sleep_duration ){
            //add to front          
            process->next = queue[SLEEP_BLOCKED];
            queue[SLEEP_BLOCKED] = process;
  
        }else{  
            
            //add to middle
            while(iterator->next){         
                
                if(tick >= iterator->sleep_duration && tick < iterator->next->sleep_duration){
                    
                    old_next = iterator->next;
                    iterator->next = process;
                    process->next = old_next;
                    break;
                }
                iterator = iterator->next;
                total = total + iterator->sleep_duration;
            }
            //add to end
            if(!iterator->next){
                iterator->next = process;
            }
        }   
    }
    //kprintf("duration = %d\n",process->sleep_duration);

    //printQueue(queue[SLEEP_BLOCKED]); //this print must be here, must check
    //refreshQueue(queue[SLEEP_BLOCKED]);
}
