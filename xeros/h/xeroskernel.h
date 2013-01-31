/*********************
 * CPSC 415
 * Due: Nov 30, 2012
 * Assignment 03
 * By: Vincent Tsui
 * ID: 67221069
 *********************/

/* xeroskernel.h - disable, enable, halt, restore, isodd, min, max */

/* Symbolic constants used throughout Xinu */

typedef	char		Bool;		/* Boolean type			*/
#define	FALSE		0		/* Boolean constants		*/
#define	TRUE		1
#define	EMPTY		(-1)		/* an illegal gpq		*/
#define	NULL		0		/* Null pointer for linked lists*/
#define	NULLCH		'\0'		/* The null character		*/


/* Universal return constants */

#define	OK		 1		/* system call ok		*/
#define	SYSERR		-1		/* system call failed		*/
#define	EOF		-2		/* End-of-file (usu. from read)	*/
#define	TIMEOUT		-3		/* time out  (usu. recvtim)	*/
#define	INTRMSG		-4		/* keyboard "intr" key pressed	*/
					/*  (usu. defined as ^B)	*/
#define	BLOCKERR	-5		/* non-blocking op would block	*/

//list of system call request

#define CREATE 0
#define YIELD 1
#define STOP 2
#define GETPID 3
#define PUTS 4
#define SEND 5
#define RECEIVE 6
#define SLEEP 7
#define SIGHANDLER 8
#define SIGRETURN 9
#define KILL 10
#define SIGWAIT 11
#define OPEN 12
#define CLOSE 13
#define WRITE 14
#define READ 15
#define IOCTL 16

//list of interrupt numbers

#define TIMER_INT 32
#define SYS_CALL 69
#define KBD_INT 33

//list of process states

#define READY 0
#define RUNNING 1
//#define BLOCKED 2
#define DEAD 3
#define SEND_BLOCKED 4
#define RECV_BLOCKED 5
#define SLEEP_BLOCKED 6
#define SIG_BLOCKED 7
#define READ_BLOCKED 8

#define NUM_OF_QUEUES 10

//static data structures

#define PCB_TABLE_SIZE 32
#define SIG_TABLE_SIZE 32
#define NUM_OF_DEVICES 2
#define DEVICES_PER_PROCESS 4
#define KERNEL_KB_BUFFER_SIZE 4

//timer constants

#define DIVISOR 100
#define QUANTUM_SIZE 10 //10 ms quantum

//kernel processes

#define ROOT_PROC_PID 1
#define IDLE_PROC_PID 2

//unused values
#define NULL_RC -99
#define NULL_DEVICE 5

//devices
#define KEYBOARD_WITHOUT_ECHO 0
#define KEYBOARD_WITH_ECHO 1

#define KB_IOCTL_NUM 49

/* Functions defined by startup code */


void bzero(void *base, int cnt);
void bcopy(const void *src, void *dest, unsigned int n);
int kprintf(char * fmt, ...);
void lidt(void);
void init8259(void);
void disable(void);
void outb(unsigned int, unsigned char);
unsigned char inb(unsigned int);

//Memory header
struct memHeader {
  unsigned long size;
  struct memHeader *prev;
  struct memHeader *next;
  unsigned char* sanityCheck;
  unsigned char dataStart[0];
};

//CPU State
struct CPU{
    unsigned int   edi;
    unsigned int   esi;
    unsigned int   ebp;
    unsigned int   esp;
    unsigned int   ebx;
    unsigned int   edx;
    unsigned int   ecx;
    unsigned int   eax;
    unsigned int   iret_eip;
    unsigned int   iret_cs;
    unsigned int   eflags;
};

//IPC
struct msg_buffer{
    int ipc_pid;
    void* addr;
    int size;
};

//Device
struct devsw {
    int dvnum;
    char *dvname;
    int (*dvinit)();
    int (*dvopen)();
    int (*dvclose)();
    int (*dvread)();
    int (*dvwrite)();
    int (*dvseek)();
    int (*dvgetc)();
    int (*dvputc)();
    int (*dvioctl)();
    void *dvcsr;
    void *dvivec;
    void *dvovec;
    int (*dviint)();
    int (*dvoint)();
    void *dvioblk;
    int dvminor;
};


//PCB
struct pcb{
    int pid;
    int state;
    int table_index;
    struct CPU cpu_state;
    struct pcb* next;
    unsigned char* pArgs;
    unsigned char* pProcessStack;
    unsigned char* pProcessStackBase;
    int memory_size;
    int return_code;
    struct pcb* msg_queue;
    struct msg_buffer buffer;
    unsigned int sleep_duration;
    void* sig_handler_table[SIG_TABLE_SIZE];
    int sig_register_mask;
    int sig_waiting_mask;
    int sig_ignore_mask;
    int device_indices[DEVICES_PER_PROCESS];
};

//misc global variables

struct pcb pcb_table[PCB_TABLE_SIZE];
struct devsw device_table[NUM_OF_DEVICES];
struct pcb* queue[NUM_OF_QUEUES];
static int pid_count = 1;
static int request;
static int pcb_table_tracker;
static unsigned int kernel_kb_buffer[KERNEL_KB_BUFFER_SIZE];

//prototype for evec.c
void my_set_evec(unsigned int xnum, unsigned long handler);

/* Prototypes for mem.c*/

extern void kmeminit( void );
extern void *kmalloc( int size );
extern void kfree( void *ptr );
void defrag(void);
extern void printFreeMemList(void);

//Prototypes for dispatch.c

extern void dispinit(void);
extern void dispatch(void);
struct pcb* next();
struct pcb* block(struct pcb* process, int state);
void ready(struct pcb* new_process);
void printQueue(struct pcb* iterator);
void refreshQueue(struct pcb* iterator);
int getArg(struct pcb* process, int addr_start);
struct pcb* dequeue(struct pcb* head, struct pcb* process);
struct pcb* addtoqueue(struct pcb* head, struct pcb* process);
int countQueue(struct pcb* head);
void free_senders(struct pcb* iterator);
void free_receivers(struct pcb* process);
void addNewContext(struct pcb* process, int sig_index);
void checkSignals(struct pcb* process);
void checkPCB(void);
void delay(int count);
int check_address(struct pcb* process, void* addr);

//Prototypes for ctsw.c

extern void contextinit(void);
int contextswitch(struct pcb* process);

//Prototypes for create.c

extern int create( void (*func)(), int stack );
extern unsigned short getCS( void );
unsigned char* push_context(unsigned char* stack, struct CPU cpu_state);
struct CPU get_context(void (*func)());
int init_tables(int index);
int issue_to_pcb_table(struct pcb process, void (*func)());
struct pcb init_pcb(unsigned char* stack_start, struct CPU cpu_state);

//Prototypes for syscall.c

extern int syscall( int call, ... );
extern int syscreate( void (*func)(), int stack );
extern void sysyield( void );
extern void sysstop( void );
extern unsigned int sysgetpid( void );
extern void sysputs( char *str );
extern int syssend( int dest_pid, void *buffer, int buffer_len );
extern int sysrecv( int *from_pid, void *buffer, int buffer_len );
extern unsigned int syssleep( unsigned int milliseconds );
int syssighandler(int signal, void (*newhandler)(void *), void (** oldhandler)(void *));
int syskill(int PID, int signalNumber);
int syssigwait(void);
extern int sysopen(int device_no);
extern int sysclose(int fd);
extern int syswrite(int fd, void *buff, int bufflen);
extern int sysread(int fd, void *buff, int bufflen);
extern int sysioctl(int fd, unsigned long command, ...);

//Prototypes for user.c
extern void root( void );
extern void test_root( void );
extern void idleproc( void );
extern void my_handler(void);
extern void my_other_handler(void);
extern void sig_process_A(void);
extern void sig_process_B(void);
extern void sig_process_C(void);
void printChars(int buffer[], int buffer_len);

//Prototypes for msg.c
int send( struct pcb* this_process, int dest_pid, void *buffer, int buffer_len );
int recv( struct pcb* this_process, int *from_pid, void *buffer, int buffer_len );
struct pcb* findProcess(int pid);

//Prototypes for sleep.c
void sleep(struct pcb* process, unsigned int milliseconds);
void tick();

//Prototypes for signal.c
void sigreturn(void *old_sp);
void sigtramp(void (*handler)(void *), void *cntx, void *osp);
int signal(struct pcb* process, int pid, int sig_no);
int sighandler(struct pcb* process, int signal, void (*newhandler)(void *), void (** oldhandler)(void *));

//Prototypes for device.c
int di_open(struct pcb* process, int device_no);
int di_close(struct pcb* process, int fd);
int di_write(struct pcb* process, int fd, void *buff, int bufflen);
int di_read(struct pcb* process, int fd, void *buff, int bufflen);
int di_ioctl(struct pcb* process, int fd, unsigned long command, int* args);
int di_check_self(struct pcb* process, int device_no);
int di_check_others(int device_no);
