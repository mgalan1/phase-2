#define DEBUG2 1

// Added in by Arianna 
// This struct is used to keep track of blocked processes on a mailbox
struct mbox_proc {
   int    mbox_id;
   int    pid;
   struct mbox_proc *next;
};

struct mailbox {
   int           mbox_id;
   int           status;     
   int           num_slots;
   int           max_slot_size;
   int           slots_used;
   int           slotSize;
   int           sentBlockCount;
   int           receivedBlockCount;
   struct        mail_slot *my_slots;
   struct        mail_slot *block_procs;
   /* other items as needed... */
};

struct mail_slot {
   int       mbox_id;
   int       status;
   struct    mail_slot *next_slot;
   char      message[MAX_MESSAGE];
   /* other items as needed... */
};

struct psr_bits {
    unsigned int cur_mode:1;
    unsigned int cur_int_enable:1;
    unsigned int prev_mode:1;
    unsigned int prev_int_enable:1;
    unsigned int unused:28;
};

union psr_values {
   struct psr_bits bits;
   unsigned int integer_part;
};

typedef struct mailbox m_box;
typedef struct mail_slot m_slot;
typedef struct mbox_proc m_proc;

// These are pointers to the struct themselves, which we may or may not use later but I just replaced them with regular
// typedef names so we could initialize the global struct arrays in phase2.c without using
// struct mail_box somemailbox;
// Instead, we can use just
// m_box somemailbox;

// typedef struct mail_slot *slot_ptr;
// //typedef struct mailbox   mail_box;
// typedef struct mailbox   *mail_box_ptr;
// typedef struct mbox_proc *mbox_proc_ptr;

/* Some useful constants */
#define INACTIVE -1
#define READY     0
#define BLOCKED   1
#define RELEASED  2
#define OCCUPIED  3