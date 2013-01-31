/*********************
 * CPSC 415
 * Due: Nov 30, 2012
 * Assignment 03
 * By: Vincent Tsui
 * ID: 67221069
 *********************/

/* ctsw.c : context switcher
 */

#include <xeroskernel.h>

/* Your code goes here - You will need to write some assembly code. You must
   use the gnu conventions for specifying the instructions. (i.e this is the
   format used in class and on the slides.) You are not allowed to change the
   compiler/assembler options or issue directives to permit usage of Intel's
   assembly language conventions.
*/

extern long	initsp;
static unsigned char* pKernelStack;

extern void set_evec(unsigned int xnum, unsigned long handler);

/************************************
 * function name: contextinit
 * require: 
 * return:
 * comment: sets to entry point of the context switcher
*************************************/

extern void contextinit(void){
    
    //verify kernel stack pointer
    //DEBUG CODE 
    int SP ;
    __asm__ __volatile__ ("movl %%esp, %0;": "=g"(SP) : : );    
    kprintf("kernel stack initially at %d\n",SP);
    
    //set the entry point interrupt
    
    long x, y, z;
    asm("movl $_timer_entry_point, %%eax\n"
        "movl %%eax, %0": "=g" (x) : : "%eax");
    
    asm("movl $_syscall_entry_point, %%eax\n"
    "movl %%eax, %0": "=g" (y) : : "%eax");
    
    asm("movl $_keyboard_entry_point, %%eax\n"
    "movl %%eax, %0": "=g" (z) : : "%eax");
    
    //kprintf("entry point: %d\n",x);
    my_set_evec(TIMER_INT, x); //timer
    my_set_evec(SYS_CALL, y); //syscall
    my_set_evec(KBD_INT,z); //keyboard
    
    initPIT(DIVISOR); //timer interrupt will occur once every 1/DIVISOR seconds
}

/************************************
 * function name: contextswitch
 * require: process = valid pointer to a PCB
 * return: returns one of the conditions in dispatcher. See kernel.h
 * comment: 
*************************************/

int contextswitch(struct pcb* process){
    
    pcb_table_tracker=process->table_index; //used to get the right PCB for context switch
    
    //DEBUG CODE 
    //kprintf("in context switch\n");
    //kprintf("entering indexing pid = %d\n",pcb_table_tracker);
//    kprintf("entering stack = %d\n",pcb_table[pcb_table_tracker].pProcessStack);
    //kprintf("return code = %d\n",pcb_table[pcb_table_tracker].return_code);
    
    __asm__ __volatile__("pushf"); //push flags
    
    __asm__ __volatile__("pusha"); //push registers 
    
    __asm__ __volatile__("movl %%esp, %0\n" : "=g" (pKernelStack)); //save kernel stack pointer
    //kprintf("kernel stack pointer saved = %d\n",pKernelStack);
    
    __asm__ __volatile__("movl %0, %%esp\n" : : "g" (pcb_table[pcb_table_tracker].pProcessStack));//switch to process stack
    //kprintf("process stack pointer loaded = %d\n",pcb_table[pcb_table_tracker].pProcessStack);
    
    //save return value
    if(pcb_table[pcb_table_tracker].return_code!=NULL_RC){
        __asm__ __volatile__("movl %0, 28(%%esp)": :"g"(pcb_table[pcb_table_tracker].return_code):);
    }
    
    __asm__ __volatile__("popa"); //pop process stack

    //__asm__ __volatile__("movl %0, %%eax": :"g"(pcb_table[pcb_table_tracker].return_code):);
    
    __asm__ __volatile__("iret");

    
    /*********************************************/
    
    __asm__ __volatile__("_keyboard_entry_point:":::"%eax");
    
    __asm__ __volatile__("cli":::); //ignore all incoming interrupts now
    
    __asm__ __volatile__("pusha":::"%eax"); //push stack registers 
    
     __asm__ __volatile__("movl %1, %0" : "=g"(request):"g"(KBD_INT):"%eax");
    
    __asm__ __volatile__("jmp _common_entry_point":::);
    
    
    /*********************************************/
    
    __asm__ __volatile__("_timer_entry_point:":::"%eax");
    
    __asm__ __volatile__("cli":::); //ignore all incoming interrupts now
    
    __asm__ __volatile__("pusha":::"%eax"); //push stack registers 
    
     __asm__ __volatile__("movl %1, %0" : "=g"(request):"g"(TIMER_INT):"%eax");
    
    __asm__ __volatile__("jmp _common_entry_point":::);
    
    /*********************************************/
    
    __asm__ __volatile__("_syscall_entry_point:":::"%eax");
    
    __asm__ __volatile__("cli":::); //ignore all incoming interrupts now
    
    __asm__ __volatile__("movl %%eax, %0" : "=g"(request)::"%eax");//save system call arguments
    
    __asm__ __volatile__("movl %%esp, %0\n" : "=g" (pcb_table[pcb_table_tracker].pArgs)::"%eax"); //save arguments stack pointer
    
    __asm__ __volatile__("pusha":::); //push stack registers  
    
    /*********************************************/
    
    __asm__ __volatile__("_common_entry_point:":::"%eax");
    //DEBUG CODE 
    //index = process->pid;
    //kprintf("index = %d\n",pcb_table_tracker);

    __asm__ __volatile__("movl %%esp, %0\n" : "=g" (pcb_table[pcb_table_tracker].pProcessStack)::"%eax"); //save process stack pointer
    //kprintf("process stack pointer saved = %d\n",pcb_table[pcb_table_tracker].pProcessStack);
    
    //DEBUG CODE
    //kprintf("exiting indexing pid = %d\n",pcb_table_tracker);
    
    __asm__ __volatile__("movl %0, %%esp\n" : : "g" (pKernelStack):"%eax");//switch to kernel stack
    
    __asm__ __volatile__("popa"); //restore kernel registers 
    
    __asm__ __volatile__("popf"); //restore kernel flags
    
    //DEBUG CODE 
    //kprintf("debug = %d\n",request);
    
    
    return request;    
}

