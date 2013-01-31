/*********************
 * CPSC 415
 * Due: Nov 30, 2012
 * Assignment 03
 * By: Vincent Tsui
 * ID: 67221069
 *********************/

/* syscall.c : syscalls
 */

#include <xeroskernel.h>
#include <stdarg.h>

/* Your code goes here */

/************************************
 * function name: syscall
 * require: call be one of CREATE, YIELD, STOP
 * return:
 * comment: pushes arguments onto stack if any, then make int call to context switcher
*************************************/

extern int syscall( int call, ... ){
    
    va_list arg_list;
    int arg[32];
    
    //kprintf("call = %d\n",call);
    //kprintf("va list = %d\n",arg_list);
    
    va_start(arg_list, call);
    
    switch(call){
        case CREATE:
            arg[0] = va_arg(arg_list, int); //function
            arg[1] = va_arg(arg_list, int); //size
            __asm__ __volatile__("push %0"::"g"(arg[0]):"%eax"); //push arg to stack
            __asm__ __volatile__("push %0"::"g"(arg[1]):"%eax"); //push arg to stack

        break;
        
        case PUTS:
            arg[0] = va_arg(arg_list, char*); //string
            __asm__ __volatile__("push %0"::"g"(arg[0]):"%eax");
        break;
        
        case SEND:
            arg[0] = va_arg(arg_list, int);
            arg[1] = va_arg(arg_list, void*);
            arg[2] = va_arg(arg_list, int);
            __asm__ __volatile__("push %0"::"g"(arg[0]):"%eax");
            __asm__ __volatile__("push %0"::"g"(arg[1]):"%eax");
            __asm__ __volatile__("push %0"::"g"(arg[2]):"%eax");
        break;
        
        case RECEIVE:
            arg[0] = va_arg(arg_list, int*); //pid
            arg[1] = va_arg(arg_list, void*); //buffer addr
            arg[2] = va_arg(arg_list, int); //buffer size
            __asm__ __volatile__("push %0"::"g"(arg[0]):"%eax");
            __asm__ __volatile__("push %0"::"g"(arg[1]):"%eax");
            __asm__ __volatile__("push %0"::"g"(arg[2]):"%eax");
        break;
        
        case SLEEP:
            arg[0] = va_arg(arg_list, unsigned int); //string
            __asm__ __volatile__("push %0"::"g"(arg[0]):"%eax");
        break;
        
        case SIGHANDLER:
            arg[0] = va_arg(arg_list, int);   //signal number
            arg[1] = va_arg(arg_list, void*); //new handler
            arg[2] = va_arg(arg_list, void*); //old handler
            __asm__ __volatile__("push %0"::"g"(arg[0]):"%eax");
            __asm__ __volatile__("push %0"::"g"(arg[1]):"%eax");
            __asm__ __volatile__("push %0"::"g"(arg[2]):"%eax");
        break;
        
        case SIGRETURN:
            arg[0] = va_arg(arg_list, void*);   //old stack pointer
            __asm__ __volatile__("push %0"::"g"(arg[0]):"%eax");
        break;
        
        case KILL:
            arg[0] = va_arg(arg_list, int);   //PID
            arg[1] = va_arg(arg_list, int);   //Sig Number
            __asm__ __volatile__("push %0"::"g"(arg[0]):"%eax");
            __asm__ __volatile__("push %0"::"g"(arg[1]):"%eax");
        break;
        
        case OPEN:
            arg[0] = va_arg(arg_list, int); //device number
            __asm__ __volatile__("push %0"::"g"(arg[0]):"%eax");
        break;
        
        case CLOSE:
            arg[0] = va_arg(arg_list, int); //file descriptor
            __asm__ __volatile__("push %0"::"g"(arg[0]):"%eax");
        break;
        
        case WRITE:
            arg[0] = va_arg(arg_list, int); //fd
            arg[1] = va_arg(arg_list, void*); //buffer addr
            arg[2] = va_arg(arg_list, int); //buffer size
            __asm__ __volatile__("push %0"::"g"(arg[0]):"%eax");
            __asm__ __volatile__("push %0"::"g"(arg[1]):"%eax");
            __asm__ __volatile__("push %0"::"g"(arg[2]):"%eax");
        break;
        
        case READ:
            arg[0] = va_arg(arg_list, int); //fd
            arg[1] = va_arg(arg_list, void*); //buffer addr
            arg[2] = va_arg(arg_list, int); //buffer size
            __asm__ __volatile__("push %0"::"g"(arg[0]):"%eax");
            __asm__ __volatile__("push %0"::"g"(arg[1]):"%eax");
            __asm__ __volatile__("push %0"::"g"(arg[2]):"%eax");
        break;
        
        case IOCTL:
            arg[0] = va_arg(arg_list, int); //fd
            arg[1] = va_arg(arg_list, unsigned long); //command
            arg[2] = va_arg(arg_list, int*); //misc arguments
            __asm__ __volatile__("push %0"::"g"(arg[0]):"%eax");
            __asm__ __volatile__("push %0"::"g"(arg[1]):"%eax");
            __asm__ __volatile__("push %0"::"g"(arg[2]):"%eax");
        break;
        
        default:
        break;
    }

    va_end(arg_list);    

    __asm__ __volatile__("movl %0, %%eax"::"g"(call):);

//DEBUG CODE
//not used, using C loops between ASM code will screw with the stack pointer        
    
//    __asm__ __volatile__("push %0"::"g"(arg[0]):"%eax"); //push arg to stack
//    __asm__ __volatile__("push %0"::"g"(arg[1]):"%eax"); //push arg to stack
//    __asm__ __volatile__("push %0"::"g"(arg[2]):"%eax"); //push arg to stack
    
    __asm__ __volatile__("int %0"::"g"(SYS_CALL):"%eax"); //go to context switcher
}

/************************************
 * function name: syscreate
 * require: func = valid pointer to a function, stack > 0 && stack < maxaddr
 * return:
 * comment: make system call to create new process
*************************************/

extern int syscreate( void (*func)(), int stack ){
    
    //DEBUG CODE
    //kprintf("func addr original = %d\n",func);
    
    //kprintf("syscreate function at %d\n",(void*)syscreate);
    
    syscall(CREATE,func,stack);
    
}

/************************************
 * function name: sysyield
 * require: 
 * return:
 * comment: make system call to yield current process for another on the ready queue
*************************************/

extern void sysyield( void ){
    
    syscall(YIELD);    
}

/************************************
 * function name: sysstop
 * require: 
 * return:
 * comment: make system call to stop/kill current process
*************************************/

extern void sysstop( void ){
    return syscall(STOP);
}

/************************************
 * function name: sysgetpid
 * require: 
 * return: pid of the process that calls this function
 * comment: 
*************************************/

extern unsigned int sysgetpid( void ){
    return syscall(GETPID);
}

/************************************
 * function name: sysputs
 * require: 
 * return:
 * comment: works like kprintf, except on process side.  Use sprintf to format string if needed
*************************************/

extern void sysputs( char *str ){
    return syscall(PUTS,str);
}

/************************************
 * function name: syssend
 * require: 
 * return: number of bytes transfered, or negative value if error
 * comment: allow process to send msg to other process
*************************************/

extern int syssend( int dest_pid, void *buffer, int buffer_len ){
    return syscall(SEND,dest_pid, buffer, buffer_len);
    //return 0;
}

/************************************
 * function name: sysrecv
 * require: 
 * return: number of bytes transfered, or negative value if error
 * comment: allow process to receive from other process
*************************************/

extern int sysrecv( int *from_pid, void *buffer, int buffer_len ){
    return syscall(RECEIVE,from_pid, buffer, buffer_len);
    //return 0;
}

/************************************
 * function name: syssleep
 * require: 
 * return:
 * comment: puts process to sleep for duration specified
*************************************/

extern unsigned int syssleep( unsigned int milliseconds ){
    return syscall(SLEEP, milliseconds);
}

int syssighandler(int signal, void (*newhandler)(void *), void (** oldhandler)(void *)){
    return syscall(SIGHANDLER,signal,newhandler,oldhandler);
}

int syskill(int PID, int signalNumber){
    return syscall(KILL,PID,signalNumber);
}

int syssigwait(void){
    return syscall(SIGWAIT);
}

void sigreturn(void *old_sp){
    
    syscall(SIGRETURN,old_sp);
    
}

extern int sysopen(int device_no){
    return syscall(OPEN,device_no);
}

extern int sysclose(int fd){
    return syscall(CLOSE,fd);
}

extern int syswrite(int fd, void *buff, int bufflen){
    return syscall(WRITE,fd,buff,bufflen);
}

extern int sysread(int fd, void *buff, int bufflen){
    return syscall(READ,fd,buff,bufflen);
}

extern int sysioctl(int fd, unsigned long command, ...){
    
    va_list arg_list;
    int arg[32];
    int i = 0;
    
    va_start(arg_list, command);
    while(arg[i] = va_arg(arg_list, int)) i++;
    va_end(arg_list);
    
    return syscall(IOCTL,fd,command,*arg);
}