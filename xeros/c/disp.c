/*********************
 * CPSC 415
 * Due: Nov 30, 2012
 * Assignment 03
 * By: Vincent Tsui
 * ID: 67221069
 *********************/

/* disp.c : dispatcher
 */

#include <xeroskernel.h>

/* Your code goes here */
extern char	*maxaddr;

/************************************
 * function name: dispinit
 * require: 
 * return:
 * comment: Initialized PCB table and queues
*************************************/

extern void dispinit(void){
    
    //initialize all pcb
    int i,j;
    for(i = 0; i < PCB_TABLE_SIZE; i++){
        
        pcb_table[i].next = NULL;
        pcb_table[i].state = DEAD;
        pcb_table[i].msg_queue = NULL;
         
    }
    
    //initialize all queues
    for(i = 0; i < NUM_OF_QUEUES; i++){
        queue[i] = NULL;
    }
    
}

/************************************
 * function name: dispatch
 * require: 
 * return:
 * comment: dispatcher code, should loop indefinitely since root process should not call sysstop
*************************************/

extern void dispatch(void){
    
    //kprintf("in dispatch\n");
           
    struct pcb* process = next();    

    //DEBUG CODE
    //kprintf("pid = %d\n",process->pid);
    
    for( ;; ) {
        
        int request = contextswitch( process );
        struct pcb* old_process;
        int sig_mask = 0;
        int sig_index = 0;
        //kprintf("request = %d\n",request);

        switch( request ) {
            case( CREATE ): 
                
                //size = getArg(process, 0);
                //func_addr = getArg(process, 4);
                create((void*)getArg(process, 4),getArg(process, 0));
   
            break;     
                
            case( YIELD ):  
                
                //WARNING: Do not call yield when preemption timer is on
                //Will cause faults!
                
                old_process = process; //save old process value
                process = next();
                if(!process) process = &pcb_table[PCB_TABLE_SIZE-1];
                
                if(old_process->pid!=IDLE_PROC_PID)
                ready(old_process); //put old process back on ready queue        
                
            break;
            case( STOP ): 
                //kprintf("in stop\n");
                old_process = process;
                process = block(process,DEAD);
                
                //check processes that are sending to this process  
                free_senders(old_process->msg_queue);
                
                //check for processes are receiving from this process
                free_receivers(old_process);                          
                      
                kfree(old_process->pProcessStackBase); //free memory
                
                break;
                
            case( GETPID ): 
                process->return_code = process->pid;
            break; 
            
            case ( PUTS ):
                //str =  getArg(process, 0);
                kprintf(getArg(process, 0));
            break;
            
            case ( SEND ):
                //dest_pid = getArg(process, 8);
                //buffer = getArg(process, 4);
                //buffer_len = getArg(process, 0);   
                process->return_code = send(process,getArg(process, 8),getArg(process, 4),getArg(process, 0));
                               
                //kprintf("return code = %d\n", process->return_code);
                
                if(process->return_code==-3){
                    old_process = process;
                    process = next();
                    if(!process) process = &pcb_table[PCB_TABLE_SIZE-1];
                    else if(old_process==process){
                        process = &pcb_table[PCB_TABLE_SIZE-1];
                        queue[READY] = NULL;
                    }
                    old_process->next = NULL;
                }
                
            break;                
                
            case ( RECEIVE ):
                //from_pid = getArg(process, 8);
                //buffer = getArg(process, 4);
                //buffer_len = getArg(process, 0);
                //kprintf("from pid = %d\n",*((int*)getArg(process, 8)));
                //while(1);
                process->return_code = recv(process,getArg(process, 8),getArg(process, 4),getArg(process, 0));
                
                //kprintf("return code = %d\n", process->return_code);
                
                if(process->return_code==-3){
                    
                    old_process = process;
                    process = next();
                    if(!process) process = &pcb_table[PCB_TABLE_SIZE-1];
                    else if(old_process==process){
                        process = &pcb_table[PCB_TABLE_SIZE-1];
                        queue[READY] = NULL;
                    }
                    old_process->next = NULL;
                }
            break; 
            
            case ( TIMER_INT ):
                //kprintf("Timer\n");
                old_process = process; //save old process value
                process = next();
                if(!process) process = &pcb_table[PCB_TABLE_SIZE-1];           

                //check processes for pending signals, etc
                checkPCB();
                
                if(old_process->pid!=IDLE_PROC_PID)
                ready(old_process); //put old process back on ready queue

                tick();
                end_of_intr();
            break;
            
            case ( SLEEP ):
                //process->state = SLEEP_BLOCKED;
                old_process = process;                                
                process = block(process,SLEEP_BLOCKED);
                
                //sleep_duration = getArg(old_process, 0);
                sleep(old_process,getArg(old_process, 0));
                
            break;
            
            case ( SIGHANDLER ):
                process->return_code = sighandler(process, getArg(process, 8),getArg(process, 4),getArg(process, 0));
            break;
            
            case ( SIGRETURN ):
                
                //reset ignore mask
                process->sig_ignore_mask =  ~0;

                //restore old stack pointer
                process->pProcessStack = getArg(process, 0);
            break;
            
            case ( KILL ):
                signal(process, getArg(process, 4),getArg(process, 0));
            break;
            
            case ( SIGWAIT ):
                old_process = process;
                process = block(process,SIG_BLOCKED);
                queue[SIG_BLOCKED] = addtoqueue(queue[SIG_BLOCKED],old_process);
                
            break;
            
            case ( OPEN ):
                process->return_code = di_open(process, getArg(process, 0));
            break;
            
            case ( CLOSE ):
                process->return_code = di_close(process, getArg(process, 0));
            break;
            
            case ( WRITE ):
                process->return_code = di_write(process, getArg(process, 8),getArg(process, 4),getArg(process, 0));
            break;
            
            case ( READ ):
                
                process->return_code = di_read(process, getArg(process, 8),getArg(process, 4),getArg(process, 0));
                
                if(process->return_code>=0){
                    old_process = process;
                    process = block(process,READ_BLOCKED);
                    queue[READ_BLOCKED] = addtoqueue(queue[READ_BLOCKED],old_process);
                }

            break;
            
            case ( IOCTL ):                
                process->return_code = di_ioctl(process, getArg(process, 8),getArg(process, 4),getArg(process, 0));
            break;
            
            case ( KBD_INT ):
                //kprintf("kb\n");
                kbd_int_read();
                end_of_intr();
            break;    
        }          
    }
}

/************************************
 * function name: next
 * require: at least 1 process in ready queue
 * return:
 * comment: if only 1 process on ready queue, it would keep returning that one process
*************************************/

struct pcb* next(){
    
    struct pcb* process;
    //kprintf("process state = %d\n",queue[READY]->state);
    
    if(queue[READY]){
        if(queue[READY]->next){

            queue[READY] = queue[READY]->next;

        }
    }else{
        //kprintf("empty ready queue\n");
        return NULL;
    }

    return queue[READY];
}

/************************************
 * function name: 
 * require: 
 * return:
 * comment: 
*************************************/

struct pcb* block(struct pcb* process, int state){
    
    struct pcb* old_process = process;
    
    process = next();
    
    if(!process) process = &pcb_table[PCB_TABLE_SIZE-1];
    else if(old_process==process){
        process = &pcb_table[PCB_TABLE_SIZE-1];
        queue[READY] = NULL;
    } 

    old_process->state = state;
    old_process->next = NULL;

    return process;
}
/************************************
 * function name: ready
 * require: new_process = valid pointer to a PCB in PCB table
 * return:
 * comment:  puts new process at the end of the ready queue
*************************************/

void ready(struct pcb* new_process){
    struct pcb* iterator;
    
    //kprintf("readying\n");
    
    if(new_process){

        if(new_process->state!=READY){
            //kprintf("dequeue\n");
            if(queue[new_process->state])queue[new_process->state] = dequeue(queue[new_process->state],new_process);
        }        
        new_process->state = READY;
    }

    //DEBUG CODE
    //kprintf("iterator initially at: %d\n",iterator);     

    if(!queue[READY]){
	//kprintf(" in here\n");
        queue[READY] = new_process;
    }
    else{
	iterator = queue[READY];

	//kprintf("here\n");
	
        while(iterator->next){

            iterator = iterator->next;
	    
        }	

        iterator->next = new_process;
        new_process->next = NULL;
    }

    

}

/************************************
 * function name: printQueue
 * require: 
 * return:
 * comment: used for debug
*************************************/

void printQueue(struct pcb* iterator){
    //iterator = queue[READY];

    kprintf("printing queue...\n");
    
    if(!iterator){
        kprintf("empty queue!\n");
        return;
    }
    
    while(iterator->next){
        kprintf("pid = %d, ",iterator->pid);
        if(iterator==iterator->next)break;
        iterator = iterator->next;
    }
    kprintf("pid = %d\n",iterator->pid);
}

/************************************
 * function name: refreshQueue
 * require: 
 * return:
 * comment: used for debug
*************************************/

void refreshQueue(struct pcb* iterator){
    //iterator = queue[READY];

    //kprintf("printing queue...\n");
    
    if(!iterator){
        //kprintf("empty queue!\n");
        return;
    }
    
    while(iterator->next){
        //kprintf("pid = %d\n",iterator->pid);
        if(iterator==iterator->next)break;
        iterator = iterator->next;
    }
    //kprintf("pid = %d\n",iterator->pid);
}

/************************************
 * function name: getArg
 * require: 
 * return:
 * comment: fetches arguments pushed onto the process stack
*************************************/

int getArg(struct pcb* process, int addr_start){
    
    int arg_offset=12; //12 bytes offset to reach start of Args 
    
    return (*(process->pArgs+arg_offset+addr_start+3))*(16777216) +
    (*(process->pArgs+arg_offset+addr_start+2))*(65536) +
    (*(process->pArgs+arg_offset+addr_start+1))*(256) + 
    (*(process->pArgs+arg_offset+addr_start));
            
}

/************************************
 * function name: dequeue
 * require: 
 * return:
 * comment: remove a process from some queue
*************************************/

struct pcb* dequeue(struct pcb* head, struct pcb* process){
    struct pcb* iterator;    
    
    if(process==head){
        //kprintf("haha\n");
        
        head = head->next;
        //if(head==NULL) kprintf("null\n");        
        //process->next = NULL;
        return head;
    }else{
        iterator = head;
        
        while(iterator->next){
        
            if(iterator->next==process){
                iterator->next = iterator->next->next;
                //process->next = NULL;
                return head;
            }
            iterator = iterator->next;
        } 
        
        if(iterator->next==process){
            iterator->next = NULL;
            return head;
        }
    }
    kprintf("not found!\n");
}

/************************************
 * function name: addtoqueue
 * require: 
 * return:
 * comment: add a process from some queue
*************************************/

struct pcb* addtoqueue(struct pcb* head, struct pcb* process){
    
    struct pcb* iterator = head;
    
    if(head==NULL){
        //kprintf("null head\n");
        head = process;
        return head;
    }
    else{
        kprintf("adding to queue\n");
        
        while(iterator->next){
            iterator = iterator->next;
        }

        iterator->next = process;
        return head;
    }
    
}

/************************************
 * function name: countQueue
 * require: 
 * return:
 * comment: counts the number of processes in a queue
*************************************/

int countQueue(struct pcb* head){
    int count = 0;
    struct pcb* iterator = head;
    
    if(!head) return 0;
    
    while(iterator->next){
        
        count++;
        
        iterator = iterator->next;
    }
    
    return count;
}

/************************************
 * function name: free_senders
 * require: 
 * return:
 * comment: before process dies, use this to check if any process is sending to it
*************************************/

void free_senders(struct pcb* iterator){
    //iterator = process->msg_queue;
                
    //kprintf("process dying pid = %d\n",process->pid);
    //printQueue(iterator);

    //check processes that are sending to this process
    if(iterator){
        if(iterator->state==SEND_BLOCKED){
            iterator->return_code = -1;
            ready(iterator);
        }

        while(iterator->next){
            if(iterator->state==SEND_BLOCKED){
                iterator->return_code = -1;
                ready(iterator);
            }            
            iterator = iterator->next;
        }       
    } 
}

/************************************
 * function name: free_receivers
 * require: 
 * return:
 * comment: before process dies, use this to check if any process is receiving from it
*************************************/

void free_receivers(struct pcb* process){
    
    struct pcb* iterator;
    int i;
    
    for(i=0;i<PCB_TABLE_SIZE-1;i++){
        iterator = pcb_table[i].msg_queue;

        //kprintf("process index %d with msg queue %d\n",i,iterator);

        if(iterator && process){    

            if(iterator->pid==process->pid){

                //kprintf("process to unlock = %d\n",pcb_table[i].pid);
                if(pcb_table[i].state==RECV_BLOCKED){
                    pcb_table[i].return_code = -1;
                    ready(&pcb_table[i]);
                }

            }
            else{
                while(iterator->next){

                    if(iterator->pid==process->pid){
                        if(pcb_table[i].state==RECV_BLOCKED){
                            pcb_table[i].return_code = -1;
                            ready(&pcb_table[i]);
                        }
                        break;
                    }

                    iterator = iterator->next;
                }
            }

        }

    }
}

/************************************
 * function name: 
 * require: 
 * return:
 * comment: 
*************************************/

void addNewContext(struct pcb* process, int sig_index){
    
    unsigned char* PC;
    unsigned char* old_esp = process->pProcessStack;
    
    __asm__ __volatile__("movl %%esp, %0":"=g"(PC)::);
    
    __asm__ __volatile__("movl %0, %%esp"::"g"(process->pProcessStack):);
    __asm__ __volatile__("push %0"::"g"(old_esp):);
    __asm__ __volatile__("push %0"::"g"(old_esp):); //context and old esp the same?
    __asm__ __volatile__("push %0"::"g"(process->sig_handler_table[sig_index]):);    
    __asm__ __volatile__("push %0"::"g"(&sysstop):);
    __asm__ __volatile__("push %0"::"g"(process->cpu_state.eflags):);
    //__asm__ __volatile__("push %0"::"g"(0):);
    __asm__ __volatile__("push %0"::"g"(process->cpu_state.iret_cs):);
    __asm__ __volatile__("push %0"::"g"(sigtramp):);
    __asm__ __volatile__("push %0"::"g"(process->cpu_state.eax):);
    __asm__ __volatile__("push %0"::"g"(process->cpu_state.ecx):);
    __asm__ __volatile__("push %0"::"g"(process->cpu_state.edx):);
    __asm__ __volatile__("push %0"::"g"(process->cpu_state.ebx):);
    __asm__ __volatile__("push %0"::"g"(process->cpu_state.esp):);
    __asm__ __volatile__("push %0"::"g"(process->cpu_state.ebp):);
    __asm__ __volatile__("push %0"::"g"(process->cpu_state.esi):);
    __asm__ __volatile__("push %0"::"g"(process->cpu_state.edi):);
    __asm__ __volatile__("movl %%esp, %0":"=g"(process->pProcessStack)::);
    
    __asm__ __volatile__("movl %0, %%esp"::"g"(PC):);
    
}

/************************************
 * function name: 
 * require: 
 * return:
 * comment: 
*************************************/

void checkPCB(void){
    
    int i;
    
    for(i = 0; i < PCB_TABLE_SIZE; i++){
        if(pcb_table[i].state!=DEAD){
            checkSignals(&pcb_table[i]);
        }
    }
}

/************************************
 * function name: 
 * require: 
 * return:
 * comment: 
*************************************/

void checkSignals(struct pcb* process){
    int sig_index = 0;
    int sig_mask = process->sig_waiting_mask & process->sig_register_mask;
                
    if(sig_mask){
        //check if ignored
        sig_mask = sig_mask & process->sig_ignore_mask;
        
//        kprintf("sig ignore mask = %d\n",process->sig_ignore_mask);
//        kprintf("sig mask after ignore = %d\n",sig_mask);
        
        if(sig_mask){
            //signal needs to be serviced
            while(sig_mask >> sig_index){
                sig_index++;
            }

            //remember to minus 1
            sig_index--;
            
            //kprintf("sig index = %d\n",sig_index);

            //clear waiting signal
            process->sig_waiting_mask = process->sig_waiting_mask ^ (1 << (sig_index));
            
            //kprintf("mask before clear = %b\n",process->sig_ignore_mask);
            
            //add ignore mask
            process->sig_ignore_mask = ~0;
            process->sig_ignore_mask = process->sig_ignore_mask << (sig_index + 1);
            
            //kprintf("mask after clear = %b\n",process->sig_ignore_mask);

            addNewContext(process, sig_index);
        }
    }
}

/************************************
 * function name: 
 * require: 
 * return:
 * comment: 
*************************************/

void delay(int count){
    int i;
    
    for(i=0;i<count;i++);
}

/************************************
 * function name: 
 * require: 
 * return:
 * comment: 
*************************************/

int check_address(struct pcb* process, void* addr){
    
    if(!addr) return -1;
    
    if(addr<0) return -2;
    
    if(addr>maxaddr) return -3;
    
    if(process){
        if(addr < process->pProcessStackBase) return -4;

        if(addr > process->pProcessStackBase + process->memory_size) return -5;
    }
}