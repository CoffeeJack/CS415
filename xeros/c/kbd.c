/*********************
 * CPSC 415
 * Due: Nov 30, 2012
 * Assignment 03
 * By: Vincent Tsui
 * ID: 67221069
 *********************/
#include <xeroskernel.h>
#include <kbd.h>

///*  Normal table to translate scan code  */
unsigned char   kbcode[] = { 0,
          27,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',
         '0',  '-',  '=', '\b', '\t',  'q',  'w',  'e',  'r',  't',
         'y',  'u',  'i',  'o',  'p',  '[',  ']', '\n',    0,  'a',
         's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', '\'',
         '`',    0, '\\',  'z',  'x',  'c',  'v',  'b',  'n',  'm',
         ',',  '.',  '/',    0,    0,    0,  ' ' };

/* captialized ascii code table to tranlate scan code */
unsigned char   kbshift[] = { 0,
           0,  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',
         ')',  '_',  '+', '\b', '\t',  'Q',  'W',  'E',  'R',  'T',
         'Y',  'U',  'I',  'O',  'P',  '{',  '}', '\n',    0,  'A',
         'S',  'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  '"',
         '~',    0,  '|',  'Z',  'X',  'C',  'V',  'B',  'N',  'M',
         '<',  '>',  '?',    0,    0,    0,  ' ' };
/* extended ascii code table to translate scan code */
unsigned char   kbctl[] = { 0,
           0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
           0,   31,    0, '\b', '\t',   17,   23,    5,   18,   20,
          25,   21,    9,   15,   16,   27,   29, '\n',    0,    1,
          19,    4,    6,    7,    8,   10,   11,   12,    0,    0,
           0,    0,   28,   26,   24,    3,   22,    2,   14,   13 };

/************************************
 * function name: kbd_reset
 * require: 
 * return:
 * comment: Resets the keyboard, setting initial values
*************************************/

int kbd_reset(void){
    
    int i;
    
    //reset buffer count
    kernel_kb_buffer_count = 0;
    
    //empty keyboard buffer
    for(i=0;i<KERNEL_KB_BUFFER_SIZE;i++) kernel_kb_buffer[i] = 0;
    
    //reset process buffer pointer
    p_read_buff = NULL;
    
    //reset process buffer length
    p_read_buff_length = 0;
    
    //reset count of number of characters transfered
    transfer_count = 0;
    
    //set EOF value
    eof = EOF; 
    
    //sets KB type to null device
    kb_type = NULL_DEVICE;
    
    //clears the EOF flag, so new data can be read
    kb_eof_flag = 0;
    
    return 0;
}

/************************************
 * function name: kbd_open
 * require: process not null, dev_num is valid integer
 * return: negative number if invalid parameters, 0 otherwise
 * comment: Opens keyboard via port 64
*************************************/

int kbd_open(struct pcb* process, int dev_num){  
    
    int error_flag = 0;
    
    //kprintf("in kbd open\n");
//    kprintf("dev num = %d\n",dev_num);
    
    error_flag = kbd_check_error(process,dev_num);
    
    if(error_flag<0) return error_flag;
    
    __asm__ __volatile__("pusha"); //saves registers
    __asm__ __volatile__("movl $0XAE, %%eax":::); //enable keyboard code = AE
    __asm__ __volatile__("out %%al, $0X64":::); //port 64
    __asm__ __volatile__("popa"); //restore registers
    
    enable_irq(1, 0);

    kbd_reset();

    kb_type = dev_num;    
    
    return 0;
}

/************************************
 * function name: kbd_close
 * require: 
 * return:
 * comment: Close keyboard via port 64
*************************************/

int kbd_close(void){
    
    //kprintf("in kbd close\n");
    
    __asm__ __volatile__("pusha"); //saves registers
    __asm__ __volatile__("movl $0XAD, %%eax":::); //disable keyboard code = AD
    __asm__ __volatile__("out %%al, $0X64":::); //port 64
    __asm__ __volatile__("popa"); //restore registers
    
    kbd_reset();  
    
    return 0;
}

/************************************
 * function name: kbd_read
 * require: 
 * return: negative value if invalid inputs (null buffer, etc)
 * comment: designate a location for read characters to transfer to
*************************************/

int kbd_read(struct pcb* process, int fd, void *buff, int bufflen){
    
    //kprintf("in kbd read\n");    
    if(kb_eof_flag){
        if(queue[READ_BLOCKED]){
            //set return code
            queue[READ_BLOCKED]->return_code = 0;                        

            //reset transfer count
            transfer_count = 0;

            //free READ_BLOCKED process
            ready(queue[READ_BLOCKED]);
        }
    }
    
    if(check_address(process,buff)<0) return -1;
    
    if(bufflen < 0) return -2;
    
    p_read_buff = buff;
    p_read_buff_length = bufflen;
    
    //check and see if characters are waiting in kernel buffer, if yet transfer

    return 0;
}

/************************************
 * function name: kbd_write
 * require: 
 * return: -1
 * comment: Always return -1, because you cannot write to a keyboard
*************************************/

int kbd_write(struct pcb* process, int fd, void *buff, int bufflen){
    
    //writes are not allowed for a keyboard
    return -1;
}

/************************************
 * function name: kbd_ioctl
 * require: process != NULL, args != NULL
 * return: -1 if command is invalid
 * comment: Used to change the EOF value
*************************************/

int kbd_ioctl(struct pcb* process, int fd, unsigned long command, int* args){
    
    //kprintf("in ioctl\n");
    
    if(command!=KB_IOCTL_NUM) return -1; //only allows 1 command
    
    //kprintf("arg = %c\n",args);
    
    eof = args; //sets new EOF value
    
    return 0;
}

/************************************
 * function name: kbd_check_error
 * require: 
 * return: negative value if constraints violated
 * comment: Used to check if keyboard constraints are satisfied. No 2 KB opened, etc
*************************************/
int kbd_check_error(struct pcb* process, int dev_num){
    
    int i;
    
    for(i = 0; i < DEVICES_PER_PROCESS; i++){
        
        //check if process has a conflicting device opened
        if(dev_num==KEYBOARD_WITHOUT_ECHO){
            if(process->device_indices[i]==KEYBOARD_WITH_ECHO){
                return -5;
            }
        }
        
        if(dev_num==KEYBOARD_WITH_ECHO){
            if(process->device_indices[i]==KEYBOARD_WITHOUT_ECHO) return -6;
        }
    }

    return 0;    
}

/************************************
 * function name: kbd_int_read
 * require: 
 * return:
 * comment: This gets called whenever there is a keyboard interrupt
 *          It reads a character from keyboard, then process it accordingly
*************************************/
int kbd_int_read(void){
    
    unsigned char value = read_char();

    //kprintf("value = %c, value in binary = %b, size = %d\n",value,value,sizeof(value)); //size in bytes
    
    if(!kb_eof_flag){
        //avoids the keyboard UP
        if(value){ 
            if(value != eof)store_char(value);
            if(p_read_buff) transfer_char();
            if(kb_type==KEYBOARD_WITH_ECHO) kprintf("%c",value);
        }
    }else{
        if(queue[READ_BLOCKED]){
            //set return code
            queue[READ_BLOCKED]->return_code = 0;                        

            //reset transfer count
            transfer_count = 0;

            //free READ_BLOCKED process
            ready(queue[READ_BLOCKED]);
        }
    }
}

/************************************
 * function name: read_char
 * require: 
 * return: a character
 * comment: Reads a keyboard value from port 60
*************************************/
unsigned char read_char(void){
    
    //kprintf("in read char\n");
    
    unsigned char value=0;
    
    __asm__ __volatile__("pusha"); //saves registers
    __asm__ __volatile__("in $0x60, %%al":::); //read from port 60
    __asm__ __volatile__("movb %%al, %0":"=g"(value)::); //transfer value
    __asm__ __volatile__("popa"); //restore registers
    
    value = kbtoa(value);
    
    //puts process back on ready queue if EOF or ENTER key encountered
    if(value==ENTER || value==eof){
        if(queue[READ_BLOCKED]){
        //kprintf("read blocked process pid = %d\n",queue[READ_BLOCKED]->pid);
        
        //if EOF, set flag
        if(value==eof) kb_eof_flag=1;    
            
        //set return code
        queue[READ_BLOCKED]->return_code = BUFF_TYPE * transfer_count;                        
        
        //reset transfer count
        transfer_count = 0;
        
        //free READ_BLOCKED process
        ready(queue[READ_BLOCKED]);
        }
    }
    
    return value;
}

/************************************
 * function name: store_char
 * require: 
 * return: -1 if buffer is full, else 0
 * comment: Store characters read into kernel buffer.
*************************************/
int store_char(unsigned char value){
    
    int count;
    
    if(kernel_kb_buffer_count<KERNEL_KB_BUFFER_SIZE){
        if(value){
            kernel_kb_buffer[kernel_kb_buffer_count] = value;
            kernel_kb_buffer_count++;
        }
    }else{
        //buffer is full
//        for(count=0;count<KERNEL_KB_BUFFER_SIZE;count++){
//            kprintf("%c",kernel_kb_buffer[count]);
//        }
//        kprintf("\n");
        
        return -1;
        //kernel_kb_buffer_count=0;
    }
    
    return 0;
}

/************************************
 * function name: transfer_char
 * require: kbd_read has been called first, flags should be set
 * return:
 * comment: move characters in kernel buffer to process buffer
*************************************/
int transfer_char(void){    
    
    int buffer_fullness;
    
    if(p_read_buff_length<kernel_kb_buffer_count*BUFF_TYPE){
        buffer_fullness = p_read_buff_length;
    }else{
        buffer_fullness = kernel_kb_buffer_count*BUFF_TYPE;
    }
    
    blkcopy(p_read_buff,kernel_kb_buffer,buffer_fullness);
    
    //increment pointer
    p_read_buff += buffer_fullness;
    
    //increment count
    transfer_count += buffer_fullness/BUFF_TYPE;               
    
    //check if process buff is full, if yes add it back to ready queue
    if(transfer_count*BUFF_TYPE >= p_read_buff_length){
        
        if(queue[READ_BLOCKED]){
            //set return code
            queue[READ_BLOCKED]->return_code = BUFF_TYPE * transfer_count;
            
            //reset transfer count
            transfer_count = 0;
            
            //put in ready
            ready(queue[READ_BLOCKED]);    
        }
    }
    
    //flush buffer
    flush_buffer();
    
    return 0;
}

/************************************
 * function name: flush_buffer
 * require: 
 * return:
 * comment: flushes the kernel buffer
*************************************/
int flush_buffer(){
    int i;
    
    //reset buffer count
    kernel_kb_buffer_count = 0;
    
    //empty keyboard buffer
    for(i=0;i<KERNEL_KB_BUFFER_SIZE;i++) kernel_kb_buffer[i] = 0;
}

/************************************
 * function name: extchar
 * require: 
 * return:
 * comment: function provided by instructor, DO NOT CHANGE!
*************************************/
static int extchar(unsigned char   code)
{
        state &= ~EXTENDED;
}

/************************************
 * function name: kbtoa
 * require: 
 * return:
 * comment: function provided by instructor, DO NOT CHANGE!
*************************************/
unsigned int kbtoa( unsigned char code )
{
    unsigned int    ch;

    if (state & EXTENDED) return extchar(code);

    if (code & KEY_UP) {

        switch (code & 0x7f) {

            case LSHIFT:

            case RSHIFT:
                state &= ~INSHIFT;
            break;

            case CAPSL:
                kprintf("Capslock off detected\n");
                state &= ~CAPSLOCK;
            break;

            case LCTL:
                state &= ~INCTL;
            break;

            case LMETA:
                state &= ~INMETA;
            break;
        }

      return NOCHAR;
    }


    /* check for special keys */
    switch (code) {
        case LSHIFT:
        case RSHIFT:
            state |= INSHIFT;
            kprintf("shift detected!\n");
            return NOCHAR;

        case CAPSL:
             state |= CAPSLOCK;
            kprintf("Capslock ON detected!\n");
            return NOCHAR;

        case LCTL:
            state |= INCTL;
            return NOCHAR;

        case LMETA:
            state |= INMETA;
            return NOCHAR;

        case EXTESC:
            state |= EXTENDED;
            return NOCHAR;
    }
  
    ch = NOCHAR;
  
    if (code < sizeof(kbcode)){
        if ( state & CAPSLOCK ) ch = kbshift[code];
        else ch = kbcode[code];
    }
    if (state & INSHIFT) {
        if (code >= sizeof(kbshift)) return NOCHAR;
        if ( state & CAPSLOCK ) ch = kbcode[code];
        else ch = kbshift[code];
    }
    if (state & INCTL) {
        if (code >= sizeof(kbctl)) return NOCHAR;
        ch = kbctl[code];
    }
    if (state & INMETA) ch += 0x80;
    
    return ch;
}