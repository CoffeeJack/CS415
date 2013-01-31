/*********************
 * CPSC 415
 * Due: Oct 9, 2012
 * Assignment 01
 * By: Vincent Tsui
 * ID: 67221069
 *********************/

/* initialize.c - initproc */

#include <i386.h>
#include <xeroskernel.h>
#include <xeroslib.h>

extern	int	entry( void );  /* start of kernel image, use &start    */
extern	int	end( void );    /* end of kernel image, use &end        */
extern  long	freemem; 	/* start of free memory (set in i386.c) */
extern char	*maxaddr;	/* max memory address (set in i386.c)	*/

/************************************************************************/
/***				NOTE:				      ***/
/***								      ***/
/***   This is where the system begins after the C environment has    ***/
/***   been established.  Interrupts are initially DISABLED.  The     ***/
/***   interrupt table has been initialized with a default handler    ***/
/***								      ***/
/***								      ***/
/************************************************************************/

/*------------------------------------------------------------------------
 *  The init process, this is where it all begins...
 *------------------------------------------------------------------------
 */
void initproc( void )				/* The beginning */
{
	kprintf( "\n\nCPSC 415, 2012W1 \n32 Bit Xeros 1.1\nLocated at: %x to %x\n", &entry, &end ); 

        /* Your code goes here */
        initevec();
        
        kmeminit();
        
        //DEBUG CODE
        //kprintf("start of kernel = %d\n",&start);
        //kprintf("end of kernel = %d\n",&end);
        
        di_init();
        
        dispinit(); //initialize dispatcher
        
        contextinit(); //initialize context switcher

        create(root,8000);//create root process
        //create(test_root,8000);//create root process
        create(idleproc,8000);//create idle process               

        dispatch(); //call dispatcher
        
        kprintf("all process stopped\n");
        
        /* This code should never be reached after you are done */
	for(;;); /* loop forever */
}


