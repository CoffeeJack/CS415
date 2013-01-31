/*********************
 * CPSC 415
 * Due: Nov 30, 2012
 * Assignment 03
 * By: Vincent Tsui
 * ID: 67221069
 *********************/

/* create.c : create a process
 */

#include <xeroskernel.h>

/* Your code goes here. */

/************************************
 * function name: create
 * require: func = valid pointer to a function, stack = real number > 0
 * return: pid of the generated process, -1 if creation fails
 * comment: creates a new process by specifying the address of code + stack size
*************************************/
extern int create( void (*func)(), int stack ){
    
    //kprintf("creating...\n");
    
    //allocate stack
    int i=0;
    int j=0;
    unsigned char* PC;
    unsigned char* pProcessStackBase = kmalloc(stack);
    unsigned char* pProcessStack;
    struct memHeader* pMH = pProcessStackBase - 16;
    int safety = 128;
    //unsigned short CS = getCS();//get Code Segment (CS) segment selector
    struct pcb new_process;
    struct CPU cpu_state;
    
    if(pProcessStackBase <= 0) return -1; //no suitable memory found, process not created  
    
    pProcessStack = pProcessStackBase + pMH->size - safety;
    
//DEBUG CODE  
    //DEBUG CODE     
//    kprintf("create function at %d\n",(void*)create);
//    kprintf("function pointer at %d\n",func);
//    kprintf("allocated stack base at %d\n",pProcessStackBase);
//    kprintf("memory header at %d\n",pMH);
    //kprintf("size allocated = %d\n",pMH->size);
    //kprintf("process stack at = %d\n",pProcessStack); 
//    kprintf("process code at = %d\n",CS);    

    //get initial context for process
    //cpu_state = get_context(func);

    cpu_state.edi = 0;
    cpu_state.esi = 0;
    cpu_state.ebp = 0;
    cpu_state.esp = 0;
    cpu_state.ebx = 0;
    cpu_state.edx = 0;
    cpu_state.ecx = 0;
    cpu_state.eax = 0;
    cpu_state.iret_cs = getCS();
    cpu_state.iret_eip = func;
    cpu_state.eflags = 0x00003200; //will enable preemption
    //cpu_state.eflags = 0; 
    

    //push context onto stack
    pProcessStack = push_context(pProcessStack,cpu_state);
    
    new_process.pid = pid_count;
    //new_process.parent_pid = 0;
    new_process.state = READY;
    new_process.next =  NULL;
    new_process.pArgs = 0;
    new_process.pProcessStackBase = pProcessStackBase;
    new_process.pProcessStack = pProcessStack;
    new_process.cpu_state = cpu_state;
    new_process.msg_queue = NULL;
    new_process.return_code = NULL_RC;
    new_process.sig_register_mask = 0;
    new_process.sig_waiting_mask = 0;
    new_process.sig_ignore_mask = ~0; //32 bits of 1's
    new_process.memory_size = pMH->size - safety;
    
    //DEBUG CODE 
    //kprintf("val of initial p stack %d\n",new_process.pProcessStack);

    //check PCB table for space
    
    if(func==&idleproc){
        i = PCB_TABLE_SIZE-1;
        
        pcb_table[i] = new_process;
        pcb_table[i].table_index = i;
    } 
    else{
        
        for(i=0;i<PCB_TABLE_SIZE-1;i++){
            if(pcb_table[i].state==DEAD){      
                pcb_table[i] = new_process;
                break;
            }
        }

        if(i>=PCB_TABLE_SIZE-1){
             return -1; //PCB table full, return -1 as error
        }

        pcb_table[i].table_index = i;
    }    

    //set all initial signal handlers to NULL
    for(j=0;j<SIG_TABLE_SIZE;j++){
        pcb_table[i].sig_handler_table[j] = NULL;
    }

    //set all initial file descriptor to null
    for(j=0;j<DEVICES_PER_PROCESS;j++){
        pcb_table[i].device_indices[j] = NULL_DEVICE;
    }
    
    //kprintf("stack = %d\n",stack);

    //kprintf("i=%d\n",i);
    
    //add new process to ready queue
    
    //DEBUG CODE 
    //kprintf("addr of new PCB = %d\n",&pcb_table[i]);
    
    if(i!=PCB_TABLE_SIZE-1) //prevents the idle process ever from entering ready queue
    ready(&pcb_table[i]);    

    //DEBUG CODE 
    printQueue(queue[READY]);
    
    pid_count++; //increment pid_count
    
    return new_process.pid;
}

/************************************
 * function name: push_context
 * require: stack != NULL, cpu_state = defined
 * return: new process stack
 * comment: push context onto new process stack
*************************************/

unsigned char* push_context(unsigned char* stack, struct CPU cpu_state){
    unsigned char* PC;
    
    //push state onto stack
    __asm__ __volatile__("movl %%esp, %0":"=g"(PC)::);
    
    __asm__ __volatile__("cli":::);
    __asm__ __volatile__("movl %0, %%esp"::"g"(stack):);    
    __asm__ __volatile__("push %0"::"g"(&sysstop):); //push return address to point to sysstop
    __asm__ __volatile__("push %0"::"g"(cpu_state.eflags):);
    __asm__ __volatile__("push %0"::"g"(cpu_state.iret_cs):);
    __asm__ __volatile__("push %0"::"g"(cpu_state.iret_eip):);
    __asm__ __volatile__("push %0"::"g"(cpu_state.eax):);
    __asm__ __volatile__("push %0"::"g"(cpu_state.ecx):);
    __asm__ __volatile__("push %0"::"g"(cpu_state.edx):);
    __asm__ __volatile__("push %0"::"g"(cpu_state.ebx):);
    __asm__ __volatile__("push %0"::"g"(cpu_state.esp):);
    __asm__ __volatile__("push %0"::"g"(cpu_state.ebp):);
    __asm__ __volatile__("push %0"::"g"(cpu_state.esi):);
    __asm__ __volatile__("push %0"::"g"(cpu_state.edi):);

    //get new position of stack pointer
    
    __asm__ __volatile__("movl %%esp, %0":"=g"(stack)::);
    //kprintf("new SP at = %d\n",stack);
    
    __asm__ __volatile__("movl %0, %%esp"::"g"(PC):); 

    return stack;    
}

/************************************
 * function name: get_context
 * require: func = value function address.
 * return: 
 * comment: set and get the initial context for a process
*************************************/

struct CPU get_context(void (*func)()){

    struct CPU cpu_state;
    unsigned short CS = getCS();//get Code Segment (CS) segment selector

    cpu_state.edi = 0;
    cpu_state.esi = 0;
    cpu_state.ebp = 0;
    cpu_state.esp = 0;
    cpu_state.ebx = 0;
    cpu_state.edx = 0;
    cpu_state.ecx = 0;
    cpu_state.eax = 0;
    cpu_state.iret_cs = CS;
    cpu_state.iret_eip = func;
    cpu_state.eflags = 0x00003200; //will enable preemption
    //cpu_state.eflags = 0; 

    return cpu_state;
}

