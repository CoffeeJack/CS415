/*********************
 * CPSC 415
 * Due: Nov 30, 2012
 * Assignment 03
 * By: Vincent Tsui
 * ID: 67221069
 *********************/

/* user.c : User processes
 */

#include <xeroskernel.h>

/* Your code goes here */

/************************************
 * function name: root
 * require: 
 * return:
 * comment: see A03 Section 2.5 "A Test Program" 
*************************************/

extern void root( void ){    
    
    int pid = sysgetpid();
    char* str;
    void(** old_handler)(void*);
    int fd=0, fd2=0;
    int rc=1;
    int buffer_a[10] = {};
    int buffer_b[3][10];    
    
    //#1
    sysputs("Step 01:\n");
    sprintf(str,"PID %d: Root Process Created!!\n",pid);   
    sysputs(str);
    sysputs("\n---------------\n");
    
    //#2
    sysputs("Step 02:\n");      
    fd = sysopen(KEYBOARD_WITH_ECHO);
    
    sprintf(str,"open 1 (KB WITH ECHO) return code = %d\n",fd);
    sysputs(str);
    sysputs("\n---------------\n");
    
    //#3
    sysputs("Step 03:\n");   
    sprintf(str,"\nread 1 return code = %d\n",sysread(fd,&buffer_a,sizeof(buffer_a)));
    sysputs(str);
    sysputs("\n---------------\n");
    
    //#4
    sysputs("Step 04:\n");   
    sprintf(str,"open 2 (KB W/O ECHO)return code = %d\n",sysopen(KEYBOARD_WITHOUT_ECHO));
    sysputs(str);
    sysputs("\n---------------\n");
    
    //#5
    sysputs("Step 05:\n");   
    sprintf(str,"open 3 (KB WITH ECHO)return code = %d\n",sysopen(KEYBOARD_WITH_ECHO));
    sysputs(str);
    sysputs("\n---------------\n");
    
    //#6
    sysputs("Step 06:\n");   
    sprintf(str,"close 1 return code = %d\n",sysclose(fd));
    sysputs(str);
    sysputs("\n---------------\n");
    
    //#7
    sysputs("Step 07:\n");   
    fd = sysopen(KEYBOARD_WITHOUT_ECHO);
    
    sprintf(str,"open 4 (KB W/O ECHO)return code = %d\n",fd);
    sysputs(str);
    sysputs("\n---------------\n");

    //#8
    sysputs("Step 08:\n");   
    sprintf(str,"\nread 2 return code = %d\n",sysread(fd,&buffer_b[0],sizeof(buffer_b[0])));
    sysputs(str);
    
    sprintf(str,"\nread 3 return code = %d\n",sysread(fd,&buffer_b[1],sizeof(buffer_b[1])));
    sysputs(str);
    
    sprintf(str,"\nread 4 return code = %d\n",sysread(fd,&buffer_b[2],sizeof(buffer_b[2])));
    sysputs(str);
    
    printChars(buffer_b,30);
    sysputs("\n---------------\n");
    
    //#9
    sysputs("Step 09:\n");   
    while(rc>0)
    rc = sysread(fd,&buffer_a,sizeof(buffer_a));
    
    sprintf(str,"\nread 5 return code = %d\n",rc);
    sysputs(str);
    sysputs("\n---------------\n");
    
    //#10
    sysputs("Step 10:\n");   
    sprintf(str,"close 2 return code = %d\n",sysclose(fd));
    sysputs(str); 
    
    fd = sysopen(KEYBOARD_WITH_ECHO);
    
    sprintf(str,"open 5 (KB WITH ECHO) return code = %d\n",fd);
    sysputs(str);
    sysputs("\n---------------\n");
    
    //#11
    sysputs("Step 11:\n");   
    sprintf(str,"create handler 1 return code = %d\n",syssighandler(18,my_handler,NULL));
    sysputs(str); 
    sysputs("\n---------------\n");
    
    //#12
    sysputs("Step 12:\n");   
    syscreate(sig_process_A,8000);
    sysputs("\n---------------\n");
    
    //#13, 14
    sysputs("Step 13, 14:\n");   
    sprintf(str,"\nread 6 return code = %d\n",sysread(fd,&buffer_a,sizeof(buffer_a)));
    sysputs(str);    
    
    sysputs("\n---------------\n");
    
    //#15
    sysputs("Step 15:\n");   
    syscreate(sig_process_B,8000);
    sysputs("\n---------------\n");

    //#16
    sysputs("Step 16:\n"); 
    old_handler = 10000; //initialize to some random value for it to work
    sprintf(str,"unset old handler = %d\n",*old_handler);
    sysputs(str);    
    
    sprintf(str,"create handler 2 return code = %d\n",syssighandler(18,my_other_handler,old_handler));
    sysputs(str);
    
    sprintf(str,"retrieved old handler = %d\n",*old_handler);
    sysputs(str);    
    sysputs("\n---------------\n");
    
    //#17, 18
    sysputs("Step 17, 18:\n");   
    sprintf(str,"\nread 7 return code = %d\n",sysread(fd,&buffer_a,sizeof(buffer_a)));
    sysputs(str);
    sysputs("\n---------------\n");
    
    //#19
    sysputs("Step 19:\n");   
    sprintf(str,"create handler 3 return code = %d\n",syssighandler(20,*old_handler,old_handler));
    sysputs(str); 
    sysputs("\n---------------\n");
    
    //#20
    sysputs("Step 20:\n");   
    syscreate(sig_process_C,16000);    
    sysputs("\n---------------\n");
    
    //#21
    sysputs("Step 21:\n");   
    rc = sysread(fd,&buffer_a,sizeof(buffer_a));
    
    sprintf(str,"\nread 8 return code = %d\n",rc);
    sysputs(str);
    sysputs("\n---------------\n");
    
    //#22, 23
    sysputs("Step 22,23:\n");   
    rc = 1;
    while(rc>0){
        rc = sysread(fd,&buffer_a,sizeof(buffer_a));
        
        if(rc >= 0 && rc < sizeof(buffer_a)){
            sprintf(str,"\nread 8 return code = %d\n",rc);
            sysputs(str);
        }
    }
    sysputs("\n---------------\n");

    //#24
    sysputs("Step 24:\n");   
    rc = sysread(fd,&buffer_a,sizeof(buffer_a));
    
    sprintf(str,"\nread 9 return code = %d\n",rc);
    sysputs(str);
    sysputs("\n---------------\n");
    
    //#25
    sysputs("Step 25:\n");   
    sysputs("Root ending...\n");    
    
    //sysstop();
    
}

extern void test_root( void ){    
    
    int pid = sysgetpid();
    char* str;
    int fd;
    int buffer_a[10];
    
    //#1
    sysputs("Step 01:\n");
    sprintf(str,"PID %d: Root Process Created!!\n",pid);   
    sysputs(str);
    sysputs("\n---------------\n");  
    
    sprintf(str,"close return code = %d\n",sysclose(fd));
    sysputs(str); 
    
    //#25
    sysputs("Step 25:\n");   
    sysputs("Root ending...\n");    
    
    while(1);
    //sysstop();
    
}

/************************************
 * function name: printChars
 * require: buffer != NULL, buffer_len > 0
 * return:
 * comment: Used in step #8. The reason for int array and no char array is because of __stack_chk_error from BOCH
*************************************/

void printChars(int buffer[], int buffer_len){
    
    int i;
    //char str[buffer_len];
    
    for(i=0;i<buffer_len;i++){
        kprintf("%c",buffer[i]);
    }
    sysputs("\n");
    
}

/************************************
 * function name: sig_process_A
 * require: 
 * return:
 * comment: Used in Step 12. It signals the process twice. One should fail because signal number not set, while other should succeed.
*************************************/

extern void sig_process_A(void){
    
    char* str;
    
    sysputs("Step 12 continued...:\n"); 
    sysputs("in signaling process A\n");
    
    syssleep(1000);
    
    sprintf(str,"syskill 1 return code = %d\n",syskill(1,20));
    sysputs(str);
    
    sprintf(str,"syskill 2 return code = %d\n",syskill(1,18));
    sysputs(str); 
    
    sysputs("signaling process A exiting...\n");
}

/************************************
 * function name: sig_process_B
 * require: 
 * return:
 * comment: Used in step 15. Signals the root process.
*************************************/

extern void sig_process_B(void){
    
    char* str;
    
    sysputs("Step 15 continued...:\n"); 
    sysputs("in signaling process B\n");
    
    syssleep(5000);
    
    sprintf(str,"syskill 3 return code = %d\n",syskill(1,18));
    sysputs(str);
    
    sysputs("signaling process B exiting...\n"); 
}

/************************************
 * function name: sig_process_C
 * require: 
 * return:
 * comment: Used in step 15, Signals the root process.
*************************************/

extern void sig_process_C(void){
    
    char* str;
    
    sysputs("Step 20 continued...:\n"); 
    sysputs("in signaling process C\n");
    
    syssleep(5000);
    
    sprintf(str,"syskill 4 return code = %d\n",syskill(1,20));
    sysputs(str);
    
    sysputs("signaling process C exiting...\n"); 
}

/************************************
 * function name: my_handler
 * require: 
 * return:
 * comment: Handler 1 to wake process
*************************************/

extern void my_handler(void){
    sysputs("in sig 1 (18) handler!\n");
}

/************************************
 * function name: my_other_handler
 * require: 
 * return:
 * comment: Handler 2 to wake process
*************************************/

extern void my_other_handler(void){
    sysputs("In sig 2 (20) handler!\n");
}

/************************************
 * function name: idleproc
 * require: 
 * return:
 * comment: this process will run when no process is in ready queue
*************************************/

extern void idleproc( void ){
    
    int count = 0;
    
    while(1){
        count++;
        //if(count%500==0)sysputs("in idle\n");
        __asm__ __volatile__("hlt");
    }
}
