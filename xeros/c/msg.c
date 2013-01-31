/*********************
 * CPSC 415
 * Due: Nov 6, 2012
 * Assignment 02
 * By: Vincent Tsui
 * ID: 67221069
 *********************/

/* msg.c : messaging system (assignment 2)
 */

#include <xeroskernel.h>

/* Your code goes here */

/************************************
 * function name: send
 * require: 
 * return: size transfered in bytes, or negative value if error
 * comment: send a msg to some other process
*************************************/

int send(struct pcb* this_process, int dest_pid, void *buffer, int buffer_len ){
    
    struct pcb* dest_process = findProcess(dest_pid);
    struct pcb* expected_sender;
    int buffer_limit;
    
    if(dest_process==-1) return -1; //no matching pid
    
    if(this_process->pid==dest_process->pid) return -2; //sending to itself
    
    expected_sender = dest_process->msg_queue;
    
    //kprintf("sending %d to %d\n",this_process->pid,dest_pid);
        
    while(expected_sender){
        
        //matching receiver found
        if(expected_sender->pid==this_process->pid && dest_process->state==RECV_BLOCKED){
            
            //kprintf("match again!\n");
            
            if(dest_process->buffer.size < buffer_len) buffer_limit = dest_process->buffer.size;
            else buffer_limit = buffer_len;
            
            //copy data            
            blkcopy(dest_process->buffer.addr,buffer,buffer_limit);
            
            //dequeue expected sender from sender queue
            dest_process->msg_queue = dequeue(dest_process->msg_queue,expected_sender);            
            
            //unblock destination process
            ready(dest_process);
            
            //return bytes
            return buffer_limit;
            
        }
        expected_sender = expected_sender->next;
    }
    
    
    //no match, receiver is not receiving yet
    if(dest_process->buffer.ipc_pid==-1){
        //destination is in receive from all mode
        
        if(dest_process->buffer.size < buffer_len) buffer_limit = dest_process->buffer.size;
        else buffer_limit = buffer_len;

        //copy data            
        blkcopy(dest_process->buffer.addr,buffer,buffer_limit);          

        //unblock destination process
        ready(dest_process);       
        
        //return bytes
        return buffer_limit;
        
    }
    
    
    this_process->buffer.ipc_pid = dest_process->pid;
    this_process->buffer.addr = buffer;
    this_process->buffer.size = buffer_len;
    
    this_process->state = SEND_BLOCKED;
    
    //add this process to receiver's sender queue
    dest_process->msg_queue = addtoqueue(dest_process->msg_queue,this_process);
    
    //remember to set this_process->next to NULL after calling next()
    
    return -3; //destination process not yet receiving
}

/************************************
 * function name: recv
 * require: 
 * return: size transfered in bytes, or negative value if error
 * comment: receive a msg to some other process
*************************************/

int recv(struct pcb* this_process, int *from_pid, void *buffer, int buffer_len ){
    
    struct pcb* src_process;
    int buffer_limit;
    
    //kprintf("in receive\n");
    
    if(*from_pid==0){
        
        if(this_process->msg_queue){
            kprintf("send queue is not empty!\n");
            *from_pid = this_process->msg_queue->pid; 
        }                
        
        kprintf("pid not specified, receiving from anyone\n");
    }
     
    if(*from_pid!=0){
        src_process = findProcess(*from_pid);
        kprintf("receiving from pid %d to %d\n",*from_pid,this_process->pid);
    }
    
    if(src_process==-1) return -1; //no matching pid
    
    if(this_process->pid==src_process->pid) return -2; //sending to itself
    
    //receiving from a process that's receiving from you
    if(src_process){
       if(src_process->state==RECV_BLOCKED){
           if(src_process->buffer.ipc_pid=this_process->pid) return -4; 
       } 
    }
    
    
    if(src_process->state==SEND_BLOCKED && src_process->buffer.ipc_pid==this_process->pid && *from_pid!=0){        
        
        //match found, proceed with data transfer
        if(src_process->buffer.size > buffer_len) buffer_limit = buffer_len;
        else buffer_limit = src_process->buffer.size;
        
        //copy data
        blkcopy(buffer,src_process->buffer.addr,buffer_limit);    
        
        printQueue(this_process->msg_queue);
        
        //dequeue src process from sender list        
        this_process->msg_queue = dequeue(this_process->msg_queue,src_process);
        
        //update the pointer to from pid
        //blkcopy(*from_pid,&(src_process->pid),sizeof(int));
        
        printQueue(this_process->msg_queue);
        
        //while(1);
        
        //unblock sending process, put it back on ready queue
        ready(src_process);

        //return received bytes
        return buffer_limit;
        
    }else{
        
        if(*from_pid!=0){
            
            //add source process to send queue
            this_process->msg_queue = addtoqueue(this_process->msg_queue,src_process);        

            //printQueue(this_process->msg_queue);        

            this_process->buffer.ipc_pid = src_process->pid;

        }else{
            kprintf("no senders\n");
            this_process->buffer.ipc_pid = -1;
        }
        
        this_process->buffer.addr = buffer;
        this_process->buffer.size = buffer_len;

        this_process->state = RECV_BLOCKED;        
        
        return -3; //source process not yet sending
        
    }
}

/************************************
 * function name: findProcess
 * require: 
 * return: a process
 * comment: 
*************************************/

struct pcb* findProcess(int pid){
    int i;
    
    for(i=0;i<PCB_TABLE_SIZE;i++){
        if(pcb_table[i].pid==pid && pcb_table[i].state!=DEAD) return &pcb_table[i];
    }
    
    return -1; //no matching process
}
