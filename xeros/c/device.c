/*********************
 * CPSC 415
 * Due: Nov 30, 2012
 * Assignment 03
 * By: Vincent Tsui
 * ID: 67221069
 *********************/
#include <xeroskernel.h>
#include <kbd.h>
#include <stdarg.h>

//device.c

/************************************
 * function name: di_init
 * require: 
 * return:
 * comment: Initializes the device independent interface with device handlers
*************************************/

void di_init(){
    
    //map device keyboard with echo
    device_table[KEYBOARD_WITH_ECHO].dvopen = kbd_open;
    device_table[KEYBOARD_WITH_ECHO].dvclose = kbd_close;
    device_table[KEYBOARD_WITH_ECHO].dvread = kbd_read;
    device_table[KEYBOARD_WITH_ECHO].dvwrite = kbd_write;
    device_table[KEYBOARD_WITH_ECHO].dvioctl = kbd_ioctl;
    
     //map device keyboard without echo
    device_table[KEYBOARD_WITHOUT_ECHO].dvopen = kbd_open;
    device_table[KEYBOARD_WITHOUT_ECHO].dvclose = kbd_close;
    device_table[KEYBOARD_WITHOUT_ECHO].dvread = kbd_read;
    device_table[KEYBOARD_WITHOUT_ECHO].dvwrite = kbd_write;
    device_table[KEYBOARD_WITHOUT_ECHO].dvioctl = kbd_ioctl;
}

/************************************
 * function name: di_open
 * require: 
 * return: negative value if device is already opened, or device doesn't exist, etc
 * comment: common interface to open a device
*************************************/

int di_open(struct pcb* process, int device_no){

    kprintf("in di open with device no %d\n",device_no);
    
    int error_flag = 0;
    int free_fd = 0;
    
    if(device_no < 0 || device_no >= NUM_OF_DEVICES) return -1; //device number not in range
    
    //check for conflicts
    if(di_check_self(process,device_no)<0) return -2;
    if(di_check_others(device_no)<0) return -3;
    
    //find available space in file description table
    for(free_fd = 0; free_fd < DEVICES_PER_PROCESS; free_fd++){
        
        //free FD found
        if(process->device_indices[free_fd]==NULL_DEVICE){
            
            //kprintf("free fd = %d\n",free_fd);
            
            //assign FD
            process->device_indices[free_fd] = device_no;            
            
            //call attached open method
            error_flag = device_table[device_no].dvopen(process, device_no);
            
            if(error_flag<0){
                //roll back
                process->device_indices[free_fd] = NULL_DEVICE;
                
                return error_flag; 
            }
            
            return free_fd;
        }
    }
    
    return -4; //FD for process full
    
}

/************************************
 * function name: di_close
 * require: 
 * return: negative if invalid file descriptor, else 0
 * comment: closes a device based on FILE DESCRIPTOR, not DEVICE NUMBER
*************************************/

int di_close(struct pcb* process, int fd){
    
    kprintf("in di close with fd %d\n",fd);
    
    if(fd < 0 || fd >= DEVICES_PER_PROCESS) return -1; //file descriptor out of range
    
    //check and see if FD is valid
    if(process->device_indices[fd]==NULL_DEVICE) return -1; //invalid FD
    
    device_table[process->device_indices[fd]].dvclose(); //call the close method
    
    process->device_indices[fd] = NULL_DEVICE; //clear FD
    
//    kprintf("fd = %d\n",fd);
//    kprintf("fd after close = %d\n",process->device_indices[fd]);
    
    return 0;
    
}

/************************************
 * function name: di_write
 * require: 
 * return:
 * comment: common interface for write. Should always return -1 if keyboard
*************************************/

int di_write(struct pcb* process, int fd, void *buff, int bufflen){    
    
    if(fd < 0 || fd >= DEVICES_PER_PROCESS) return -1; //file descriptor out of range
    
    //check and see if FD is valid
    if(process->device_indices[fd]==NULL_DEVICE) return -1; //invalid FD
    
    //call the write method, should return number of bytes written
    return device_table[process->device_indices[fd]].dvwrite(process,fd,buff,bufflen); 
  
}

/************************************
 * function name: di_read
 * require: 
 * return:
 * comment: common interface for device read
*************************************/

int di_read(struct pcb* process, int fd, void *buff, int bufflen){
    
    if(fd < 0 || fd >= DEVICES_PER_PROCESS) return -1; //file descriptor out of range

    //check and see if FD is valid
    if(process->device_indices){
       if(process->device_indices[fd]==NULL_DEVICE) return -1; //invalid FD
    
       //call the read method, should return number of bytes read
       return device_table[process->device_indices[fd]].dvread(process,fd,buff,bufflen);    
    } 
    else{
       return -1; //null FD
    }  
}

/************************************
 * function name: di_ioctl
 * require: 
 * return:
 * comment: common interface for device settings/commands
*************************************/

int di_ioctl(struct pcb* process, int fd, unsigned long command, int* args){
    if(fd < 0 || fd >= DEVICES_PER_PROCESS) return -1; //file descriptor out of range
    
    //check and see if FD is valid
    if(process->device_indices[fd]==NULL_DEVICE) return -1; //invalid FD
    
    //call the ioctl method, should return 0 for success
    return device_table[process->device_indices[fd]].dvioctl(process,fd,command,args); 
}

/************************************
 * function name: di_check_self
 * require: 
 * return: negative value if fail
 * comment: checks a process to see if it already has a certain device open
*************************************/

int di_check_self(struct pcb* process, int device_no){
    
    int i;
    
    for(i = 0; i < DEVICES_PER_PROCESS; i++){
        //check if device is already opened by the process
        
        if(process->device_indices[i] == device_no) return -1; //device already opened by the process
        
    }
    return 0;
}

/************************************
 * function name: di_check_others
 * require: 
 * return: negative value if fail
 * comment: checks other processes if they have the same device opened
*************************************/

int di_check_others(int device_no){
    
    int i, j;
    
    for(i = 0; i < PCB_TABLE_SIZE; i++){
        
        if(pcb_table[i].state!=DEAD){
            
            for(j = 0; j < DEVICES_PER_PROCESS; j++){

                if(pcb_table[i].device_indices[j] == device_no) return -1; //some other process already has it opened

            }
        }
    }
    return 0;
}