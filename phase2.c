/* ------------------------------------------------------------------------
   phase2.c
   Applied Technology
   College of Applied Science and Technology
   The University of Arizona
   CSCV 452

   ------------------------------------------------------------------------ */
#include <stdlib.h>
#include <phase1.h>
#include <phase2.h>
#include <usloss.h>
#include <stdio.h>
#include "message.h"

/* ------------------------- Prototypes ----------------------------------- */
int start1 (char *);
extern int start2 (char *);

//We need to define the variable such as system call array of function pointers 
//If we don’t define this in phase2.c we will get complaints from the compiler
void (*sys_vec[MAXSYSCALLS])(sysargs *args); 

void clock_handler2(int dev, void *unit);

/* -------------------------- Globals ------------------------------------- */
int debugflag2 = 0;
int nextMailboxID = 0;
int clock;

/* the mail boxes */
mail_box MailBoxTable[MAXMBOX];

/* An error method to handle invalid syscalls */ 
static void nullsys(sysargs *args){ 
	printf("nullsys(): Invalid syscall %d. Halting...\n", args->number); 
	halt(1);
}


/* -------------------------- Functions -----------------------------------
  Below I have code provided to you that calls

  check_kernel_mode
  enableInterrupts
  disableInterupts
  
  These functions need to be redefined in this phase 2,because
  their phase 1 definitions are static 
  and are not supposed to be used outside of phase 1.  */

/* ------------------------------------------------------------------------
   Name - start1
   Purpose - Initializes mailboxes and interrupt vector.
             Start the phase2 test process.
   Parameters - one, default arg passed by fork1, not used here.
   Returns - one to indicate normal quit.
   Side Effects - lots since it initializes the phase2 data structures.
   ----------------------------------------------------------------------- */
int start1(char *arg)
{
   int kid_pid, status; 

   if (DEBUG2 && debugflag2)
      console("start1(): at beginning\n");

   check_kernel_mode("start1");

   clock = MBoxCreate(0, 50);
   int_vec[CLOCK_DEV] = clock_handler2;

   /* Disable interrupts */
   disableInterrupts();

   /* Initialize the mail box table, slots, & other data structures.
    * Initialize int_vec and sys_vec, allocate mailboxes for interrupt
    * handlers.  Etc... */

   // First step: Initialize the mailbox table
   for (int i = 0; i < MAXMBOX; i++)
   {
      MailBoxTable[i].status   = READY;
      MailBoxTable[i].my_slots = NULL;
      sys_vec[i] = nullsys;
   }

   enableInterrupts();

   /* Create a process for start2, then block on a join until start2 quits */
   if (DEBUG2 && debugflag2)
      console("start1(): fork'ing start2 process\n");
   kid_pid = fork1("start2", start2, NULL, 4 * USLOSS_MIN_STACK, 1);
   if ( join(&status) != kid_pid ) {
      console("start2(): join returned something other than start2's pid\n");
   }

   return 0;
} /* start1 */


/* ------------------------------------------------------------------------
   Name - MboxCreate
   Purpose - gets a free mailbox from the table of mailboxes and initializes it 
   Parameters - maximum number of slots in the mailbox and the max size of a msg
                sent to the mailbox.
   Returns - -1 to indicate that no mailbox was created, or a value >= 0 as the
             mailbox id.
   Side Effects - initializes one element of the mail box array. 
   ----------------------------------------------------------------------- */
   int MboxCreate(int slots, int slot_size)
   {
      return 0;
   } /* MboxCreate */


/* ------------------------------------------------------------------------
   Name - MboxReceive
   Purpose - Get a msg from a slot of the indicated mailbox.
             Block the receiving process if no msg available.
   Parameters - mailbox id, pointer to put data of msg, max # of bytes that
                can be received.
   Returns - actual size of msg if successful, -1 if invalid args.
   Side Effects - none.
   ----------------------------------------------------------------------- */
   int MboxReceive(int mbox_id, void *msg_ptr, int msg_size)
   {
      return 0;
   } /* MboxReceive */


/* ------------------------------------------------------------------------
   Name - MboxSend
   Purpose - Put a message into a slot for the indicated mailbox.
             Block the sending process if no slot available.
   Parameters - mailbox id, pointer to data of msg, # of bytes in msg.
   Returns - zero if successful, -1 if invalid args.
   Side Effects - none.
   ----------------------------------------------------------------------- */
   int MboxSend(int mbox_id, void *msg_ptr, int msg_size)
   {
      return 0;
   } /* MboxSend */



/* ------------------------------------------------------------------------
   Name         -  MboxCondReceive
   Purpose      -  Conditionally receives a message from the mailbox. 
   Parameters   -  mailbox id, pointer to data of msg, # of bytes in msg.
   Returns      -  0 if successful, 
                   1 if no msg available
                  -1 if illegal args
                  -2 if the mailbox is empty
                  3 if process is zap’d.
   Side Effects - initializes one element of the mail box array. 
   Worked on by Arianna Boatner
   ----------------------------------------------------------------------- */
int MboxCondReceive(int mailboxID, void *message, int max_message_size)
{
   // Call this since all the functions start with checking for the kernel mode 
   check_kernel_mode("MboxCondReceive");

   // Disable interrupts
   disableInterrupts();

   //Access the mailbox
   int mail_idx = mailboxID % MAXMBOX;
   struct mailbox *mBox = &MailBoxTable[mail_idx];

   // Return 1 if no message is available
   if (message == NULL)
   {
      return 1;
   }
   
   // Return -1 if there are any illegal arguments
   if ( mailboxID > MAXMBOX || mailboxID <= -1 ) {
      return -1;
   }

   // If the index of the mailbox is not blocked or null then receive it!
   if (mBox->status == BLOCKED && mBox != NULL){
      if (mBox->slots_used != 0){
         return MboxReceive(mailboxID, message, max_message_size);
      }
      else { // Mailbox is empty
         return -2;
      }
   } 

   // Take care of the zero slot mailbox
   if (mBox->slots_used == 0) {
      struct mbox_proc mboxProc;
      mboxProc.next = NULL;
      mboxProc.pid = getpid();
   }

   return 0;
} /* MboxCondReceive */



/* ------------------------------------------------------------------------
   Name         - MboxCondSend
   Purpose      - Conditionally sends a message to a mailbox.
   Parameters   - mailbox id, pointer to data of msg, # of bytes in msg.

   Returns      - 0 if successful
                  1 if mailbox full
                 -1 if illegal args
                  3 process is zap’d.
                 -2 if the mailbox is full, the message is not sent,
                  or no mailbox slots are available in the system

   Side Effects - none.
   Worked on by Arianna Boatner
   ----------------------------------------------------------------------- */
   int MboxCondSend(int mailboxID, void *message, int message_size)
   {
      // Call this since all the functions start with checking for the kernel mode 
      check_kernel_mode("MboxCondSend");

      // Disable interrupts
      disableInterrupts();

      // Return 1 if no message is available
      if (message == NULL)
      {
         printf("No message is available.");
         return 1;
      }
   
      //Access the mailbox
      int mail_idx = mailboxID % MAXMBOX;
      struct mailbox *mBox = &MailBoxTable[mail_idx];

      // Return -1 if there are any illegal arguments
      if ( mBox->status == INACTIVE || message_size > mBox->max_slot_size ||
         mailboxID > MAXMBOX || mailboxID <= -1 || message_size < 0  ) {
         printf("There was illegal arguments. Returning -1");
         return -1;
      }

      // Return -2 if the mailbox is full, the message is not sent,
      // or no mailbox slots are available in the system
      if (check_message_availability() == 0)
      {
         printf("Mailbox is full. Returning -2");
         return -2;
      }

      //Check for illegal arguments
      if ( message_size < 0 || mBox->status == INACTIVE || message_size > mBox->slotSize) {
         if (DEBUG2 && debugflag2){ 
            console("MboxSend(): called with and invalid argument, returning -1\n", mailboxID);
            enableInterrupts(); // enable interrupts
            return -1;
         }
      }

      return MboxSend(mailboxID, message, message_size);

   } /* MboxCondSend */



/* ------------------------------------------------------------------------
   Name         - check_message_availability
   Purpose      - Check if there is an available slot in the Mailbox 
                  global array
   Parameters   - None
   Returns      - 1 if there is a message slot available, 0 otherwise
   Side Effects - none.
   Worked on by Arianna Boatner
   ----------------------------------------------------------------------- */
   int check_message_availability()
   {
      // Call this since all the functions start with checking for the kernel mode 
      check_kernel_mode("check_message_availability");

      for (int i = 0; i < MAXMBOX; ++i)
      {
         // This means a message is available, so return 1
         if (MailBoxTable[i].status == READY)
         {
            return 1; 
         }  
      }

      // At this point, a message is not available so return 0
      return 0;
   }


/* ------------------------------------------------------------------------
   Name         - check_kernel_mode
   Purpose      - Check if we are in kernel mode
   Parameters   - Name
   Returns      - None
   Side Effects - none.
   Worked on by Arianna Boatner
   ----------------------------------------------------------------------- */
void check_kernel_mode(char *name)
{
   /* test if in kernel mode; halt if in user mode */   
   if((PSR_CURRENT_MODE & psr_get()) == 0) {
       // In user mode
       console("User mode is on\n");
       halt(1);
    } 
    else
    {
       /* We ARE in kernel mode */
      psr_set( psr_get() & ~PSR_CURRENT_INT );
   }
}

/* ------------------------------------------------------------------------
   Name         - enableInterrupts
   Purpose      - Enables the interrupts
   Parameters   - Name
   Returns      - None
   Side Effects - none.
   Worked on by Arianna Boatner
   ----------------------------------------------------------------------- */
 void enableInterrupts()
{
   /* turn the interrupts OFF iff we are in kernel mode */
   if((PSR_CURRENT_MODE & psr_get()) == 0) {
      //not in kernel mode
      console("Not in kernel mode\n");
      halt(1);
   } else
      /* We ARE in kernel mode */
     psr_set( psr_get() | PSR_CURRENT_INT );

} /* enableInterrupts */


/* ------------------------------------------------------------------------
   Name         - disableInterrupts
   Purpose      - Disables the interrupts
   Parameters   - Name
   Returns      - None
   Side Effects - none.
   Worked on by Arianna Boatner
   ----------------------------------------------------------------------- */
  void disableInterrupts()
 {
   /* turn the interrupts OFF iff we are in kernel mode */
   if((PSR_CURRENT_MODE & psr_get()) == 0) {
      //not in kernel mode
      console("Kernel Error: Not in kernel mode, may not disable interrupts\n");
      halt(1);
   } else
      /* We ARE in kernel mode */
      psr_set( psr_get() & ~PSR_CURRENT_INT );
} /* disableInterrupts */



/* ------------------------------------------------------------------------
   Name         - check_io
   Purpose      - Handles I/O interrupt handling
   Parameters   - None
   Returns      - 1 if one process is blocked on an I/O mailbox, 0 otherwise.
   Side Effects - none.
   Worked on by Arianna Boatner
   ----------------------------------------------------------------------- */
   int check_io(){						
      for (int i = 0; i < 7; i++){
         if (MailBoxTable[i].slots_used > 0){
            return 1;
         }
      }
   } /* check_io */



/* ------------------------------------------------------------------------
   Name         - clock_handler2
   Purpose      - Conditionally sends to the clock I/O mailbox every 
                  5th clock interrupt .
   Parameters   - dev  - The device we are handling
                  unit - The unit we are handling 
   Returns      - none.
   Side Effects - none.
   Worked on by Arianna Boatner
   ----------------------------------------------------------------------- */
void clock_handler2(int dev, void *unit)
{
   if (DEBUG2 && debugflag2)
      console("clock_handler(): handler called\n");
} /* clock_handler */

// /* ------------------------------------------------------------------------
//    Name         - disk_handler
//    Purpose      - Conditionally sends the content of the status register
//                   to the appropriate I/O mailbox.
//    Parameters   - First parameter in function is int, second parameter 
//                   is a general pointer 
//    Returns      - None
//    Side Effects - None.
//    Worked on by Arianna Boatner
//   ----------------------------------------------------------------------- */
   void disk_handler(int dev, void *punit)
   {  
      int status;
      int result;
      int unit = (int)punit;

      if (DEBUG2 && debugflag2)
         console("disk_handler(): handler called\n");
      
      /* Sanity checks */ 
      //make sure the arguments dev and unit are OK, e.g.   
      //more checking if you see necessary
   
      device_input(DISK_DEV, unit, &status);
   
      //result = MboxCondSend(disk_mbox[unit], &status, sizeof(status));
      //should do some checking on the returned result value

   } /* disk_handler */


   void term_handler(int dev, void *unit)
   {
      if (DEBUG2 && debugflag2)
         console("term_handler(): handler called\n");
   } /* term_handler */


   void syscall_handler(int dev, void *unit)
   {
      if (DEBUG2 && debugflag2)
         console("syscall_handler(): handler called\n");
   } /* syscall_handler */


// int waitdevice(int type, int unit, int ∗status) { 
//    int result = 0;	/* Sanity checks */ 
//    //more code logic could be inserted before the below checking 
//    switch (type) {		case CLOCK_DEV : 
//          //more code for communication with clock device 
//          result = MboxReceive(clock mbox[unit], status, sizeof(int)); // This was added in - following given line below
//          break;		case DISK_DEV: 
//          result = MboxReceive(disk mbox[unit], status, sizeof(int)); // This is given to us
//          break;		case TERM_DEV: 
//          //more code logic 
//          result = MboxReceive(term mbox[unit], status, sizeof(int)); // This was added in - following given line above
//          break; 
//          default: 
//          printf("waitdevice(): bad type (%d). Halting...\n", type); 
//          halt(1); 
//       } 
//       if (result == -3)	    /* we were zap’d! */ 
// 	   {
//          return -1;
//       } 
//  	   return 0; 
// } /* disk_handler */
