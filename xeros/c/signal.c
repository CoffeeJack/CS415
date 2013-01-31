/*********************
 * CPSC 415
 * Due: Nov 30, 2012
 * Assignment 03
 * By: Vincent Tsui
 * ID: 67221069
 *********************/
#include <xeroskernel.h>
#include <stdarg.h>

//signal.c

/*your code here*/

/************************************
 * function name: sigtramp
 * require: 
 * return: NEVER! sigreturn should act according and should never return!
 * comment: cntx and osp are the same for simplicity
*************************************/

void sigtramp(void (*handler)(void *), void *cntx, void *osp){
    
    //kprintf("in sigtramp\n");
    
    //while(1);
    
    handler(cntx);
    
    sigreturn(osp);
}

/************************************
 * function name: signal
 * require: 
 * return: -18 if target process doesn't exist, -3 if target signal isn't registered, otherwise 0
 * comment: signals a process. If process isn't on ready queue during signal, it gives that process a return code of -128
*************************************/

int signal(struct pcb* process, int pid, int sig_no){
    
    struct pcb* target_process;
    
//    kprintf("in signal\n");
//    kprintf("pid = %d, sig num = %d\n",pid,sig_no);
    
    target_process = findProcess(pid);
    
    if(0>target_process){
       process->return_code = -18;
       return -18; //invalid PID, process not found 
    } 
    
    if(sig_no < 0 || sig_no > SIG_TABLE_SIZE-1){
       process->return_code = -3; 
       return -3; //invalid signal number 
    }
    
    //check if target signal is installed
    if(!target_process->sig_handler_table[sig_no]){
       process->return_code = -3; 
       return -3; //invalid signal number
    }
    
    //register signal to be waiting
    target_process->sig_waiting_mask = (1 << sig_no) | target_process->sig_waiting_mask;
    
    //kprintf("wait mask = %b\n",target_process->sig_waiting_mask);
    
    //check if process is blocked
    if(target_process->state!=RUNNING || target_process->state != READY){
        target_process->return_code = -128;
        ready(target_process);//puts it back on ready queue
    }
    
    process->return_code = 0;
    return 0;
}

/************************************
 * function name: sighandler
 * require: 
 * return: negative number if invalid inputs, else 0
 * comment: Installs a new signal handler, and pass back the old signal handler through a double pointer
*************************************/

int sighandler(struct pcb* process, int signal, void (*newhandler)(void *), void (** oldhandler)(void *)){
    
    //kprintf("%d %d %d\n",signal,newhandler,oldhandler);
    
    if(signal<0 || signal > SIG_TABLE_SIZE-1){
        return -1; //invalid signal number
    } 
    
    if(check_address(NULL,newhandler)<0){
        //kprintf("signal handler chk addr error = %d\n",check_address(process,newhandler));
        return -2; //invalid handler
    } 
    
//    kprintf("K 1 old address = %d\n",oldhandler);
//    kprintf("K 1 *old address = %d\n",*oldhandler);
//    kprintf("K 1 **old address = %d\n",**oldhandler);
    
    //save old handler
    *oldhandler = process->sig_handler_table[signal];
    //oldhandler = process->sig_handler_table[signal];
    
//    kprintf("K 2 old address = %d\n",oldhandler);
//    kprintf("K 2 *old address = %d\n",*oldhandler);
//    kprintf("K 2 **old address = %d\n",**oldhandler);
    
    //assign new handler
    process->sig_handler_table[signal] = newhandler;
    
    //set register mask
    process->sig_register_mask = (1 << signal) | process->sig_register_mask;
    
    //clear wait mask
    //process->sig_waiting_mask = process->sig_waiting_mask & ~(1 << signal);
    
    //kprintf("handler mask = %b\n",process->sig_register_mask);
    
    return 0; //every okay!
}
