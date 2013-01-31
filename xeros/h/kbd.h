/*********************
 * CPSC 415
 * Due: Nov 30, 2012
 * Assignment 03
 * By: Vincent Tsui
 * ID: 67221069
 *********************/

#ifndef KBD_H
#define	KBD_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* KBD_H */

#define KEY_UP   0x80            /* If this bit is on then it is a key   */
                                 /* up event instead of a key down event */

/* Control code */
#define LSHIFT  0x2a
#define RSHIFT  0x36
#define LMETA   0x38

#define LCTL    0x1d
#define CAPSL   0x3a


/* scan state flags */
#define INCTL           0x01    /* control key is down          */
#define INSHIFT         0x02    /* shift key is down            */
#define CAPSLOCK        0x04    /* caps lock mode               */
#define INMETA          0x08    /* meta (alt) key is down       */
#define EXTENDED        0x10    /* in extended character mode   */

#define ENTER           0x0A

#define EXTESC          0xe0    /* extended character escape    */
#define NOCHAR  256

#define BUFF_TYPE sizeof(int)

#define EOF 0x04

static  int     state; /* the state of the keyboard */
static int kernel_kb_buffer_count;
static int transfer_count;
static char* p_read_buff;
static int p_read_buff_length;
static unsigned char eof;
static int kb_type;
static int kb_eof_flag;

//Prototypes
int kbd_reset(void);
int kbd_open(struct pcb* process, int dev_num);
int kbd_close(void);
int kbd_int_read(void);
int kbd_ioctl(struct pcb* process, int fd, unsigned long command, int* args);
int kbd_check_error(struct pcb* process, int dev_num);
static int extchar(unsigned char code);
unsigned int kbtoa( unsigned char code );
int kbd_read(struct pcb* process, int fd, void *buff, int bufflen);
int kbd_write(struct pcb* process, int fd, void *buff, int bufflen);
unsigned char read_char(void);
int store_char(unsigned char value);
int transfer_char(void);
int flush_buffer(void);