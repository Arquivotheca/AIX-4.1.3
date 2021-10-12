/* @(#)17        1.3  src/bos/kernext/dartic/darticdd.c, dd_artic, bos411, 9428A410j 6/10/94 08:30:25
 *
 * COMPONENT_NAME: dd_artic -- ARTIC diagnostic driver.
 *
 * FUNCTIONS: darticconfig, darticopen, darticclose, darticioctl, darticopen,
 *            darticintr, darticmpx and several supporting routines
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 *      ARTIC Diag device driver
 *
 *      The ARTIC Diag device driver is used to facilitate downloading to
 *      and communications with IBM ARTIC adapters.
 *      The device driver provides an ioctl interface that
 *      allows reading/writing ARTIC adapter memory, and interfacing with
 *      the ARTIC diagnostic microcode and the adapter's firmware.
 */


/*
 *      AIX Include files
 */

#include        <sys/types.h>
#include        <sys/param.h>
#include        <sys/signal.h>
#include        <sys/conf.h>
#include        <sys/proc.h>
#include        <sys/sysmacros.h>
#include        <sys/errno.h>
#include        <sys/systm.h>
#include        <sys/uio.h>
#include        <sys/device.h>
#include        <sys/adspace.h>
#include        <sys/ioctl.h>
#include        <sys/ioacc.h>
#include        <sys/iocc.h>
#include        <sys/pin.h>
#include        <sys/malloc.h>
#include        <sys/lockl.h>
#include        <sys/watchdog.h>
#include        <sys/poll.h>
#include        <sys/sleep.h>
#include        <sys/xmem.h>
#include        <sys/dma.h>

/**
 *      ARTIC Diag Include files
 */

#include        <sys/dartic.h>
#include        <sys/darticdd.h>


/**--------------------*
 |   Global Variables  |
 *--------------------*/

/**
 *      The adapter table is used to hold information about each adapter installed
 *      that is being used for ARTIC Diag.  The table is defined in articdd.h, and
 *      contains information about the functional state of the adapter, as well as
 *      the operating parameters for the adapter and the RCM running on it.
 */

DARTIC_Adapter      artic_adapter[MAXADAPTERS];

/**
 *      The DMA table stores relevant information during a Busmaster
 *      transfer by one of the Busmaster ARTIC adapters
 *
 */
DARTIC_DMA          artic_dma[MAXADAPTERS];

/**
 *      A process table entry is allocated for a process when articopen is called.
 *      Only one open per process is allowed.  The DARTIC_Proc structure contains the
 *      pointers to the per-adapter pointer arrays to linked lists of ProcReg
 *      structures.
 *
 *      The process table uses a primitive hash method for access.  The artic_procptr
 *      array of pointers point to the first structure of a linked list.  Criteria
 *      for being on the list are the last log2(ARTICPA_SIZE) bits of the Process ID.
 *      Thus to find the index of the pointer for a process, use:
 *
 *                      #define ARTICPA_SIZE     some_power_of_2
 *                      #define ARTICPA_MASK     (ARTICPA_SIZE - 1)
 *
 *                              index = PID & ARTICPA_MASK;
 */

#define ARTICPA_SIZE       128
#define ARTICPA_MASK       127

struct DARTIC_Proc  *artic_procptr[ARTICPA_SIZE];

/**
 *      The artic_intr_count array is used to count interrupts from adapter
 *      tasks.  There is one counter for each task/adapter combination.
 *      These counters are incremented by the interrupt handler ricdintr
 *      once the task and adapter numbers have been validated.
 *      This driver only supports Task 0.
 *
 *                              Interrupt Register
 *                              Interrupt Wait
 *                              Interrupt Deregister                                                                    */

ulong   artic_intr_count[MAXADAPTERS];

/**
 * wakeup cookies for e_sleep, e_wakeup.
 */

int             artic_wu_cookie[MAXADAPTERS];

/**  The config_lock lock is used to serialize access to the darticconfig
 *   driver function.  It is unlikely, but since it can be entered by
 *   issuing mkdev from the command line, we must guard against it.             */

lock_t  config_lock = LOCK_AVAIL;


/**  CPUPAGE locks.  These locks are used by the device driver on a per-
 *   adapter basis to serialize access to the adapter's I/O space and
 *   shared memory.                                                                                  */

lock_t  cpupage_lock[MAXADAPTERS];    /* One lock per adapter */
lock_t  pcselect_lock[MAXADAPTERS];   /* for access to PC select byte */


/**      Process Table lock.  This lock is used by the device driver to serialize
 *      access to the artic_procptr array
 */

lock_t  proc_lock = LOCK_AVAIL;                 /* one lock for all processes   */


/**  Jumpbuffers.  These are used with the kernel service "setjmpx" to allow
 *      for unforseen exceptions.  When an exception occurs (like a memory
 *      violation), the default action taken by the kernel is to "assert", which
 *      a nice name for system crash.  By Issuing setjmpx, the kernel will transfer
 *      control to where setjmpx (setjmp/longjmp style) return (now with a
 *      non-zero value).  This way the code can take appropriate action, instead
 *      of allowing AIX to crash.                                                                                       */

label_t config_jumpbuffer;           /* darticconfig jump buffer              */
label_t oaintr_jumpbuffer;           /* darticintr jump buffer                */
label_t open_jumpbuffer;             /* darticopen jump buffer                */

label_t artic_jumpbuffers[MAXADAPTERS];  /* per-adapter jump buffers          */
label_t artic_ioctljumpb[MAXADAPTERS];   /* per-adapter jump buffers          */


/**
 *      artic_switchtable is where we build our switch table prior to passing it
 *      to the kernel via "devswadd".  This is used to install the device driver
 *      entry points into the kernel's device switch tables.
 */

struct devsw    artic_switchtable;


/**  The Interrupt Plus Structure.  This structure is allocated one-per-adapter,
 *      and is used to bind the interrupt handler to the interrupt level selected.
 *      By enclosing the system "intr" structure within our own structure (struct
 *      DARTIC_IntrPlus), we can use the same interrupt handler for all our adapters.
 *      The DARTIC_IntrPlus structure consists of the system intr structure with an
 *      integer that contains the adapter number (card number).  This binding
 *      is done with the kernel service "i_init" in driver function articconfig.*/

struct DARTIC_IntrPlus      artic_intr[MAXADAPTERS];   /* one per adapter     */


/**      Device Driver Load Count.  This counter is used to count the number of
 *      times articconfig was called.  It should be called once-per-adapter,
 *      and artic_loadcount will be incremented accordingly.   When articconfig
 *      is called to terminate the device (once per adapter), artic_loadcount
 *      will be decremented accordingly.  When the loadcount reaches 0, the
 *      device driver is removed from the device switch table.
 */

int     artic_loadcount = 0;     /* device driver instance counter           */

/**
 *      External Function Declarations
 */

extern  int             nodev();

/**
 *      Function Declarations
 */

void                    intboard();
void                    cleanup_for_return();

int                     darticconfig();
int                     darticclose();
int                     darticioctl();
int                     darticopen();
int                     darticintr();
int                     darticmpx();


struct DARTIC_Proc *remove_articproc();
struct DARTIC_Proc *lookup_articproc();

uchar   setCPUpage();
uchar   readdreg();
ushort  to16local();
char   *inteltolocal();
void    artic_time_func();
void    artic_stime_func();
int     artic_read_mem();

/**
 *      Adapter Memory determination masks (used with initreg1)
 */

#define MEMMASK                 0x28   /* Mask for memory size bits */
#define HALFMEG                 0x20   /* 512 K                     */
#define ONEMEG                  0x00   /* 1 MEG                     */
#define TWOMEG                  0x28   /* 2 MEG                     */

/*
 *
 *      darticconfig
 *
 *
 *      darticconfig is the ARTIC Diag device driver configuration routine.  It is
 *      called by the device configuration methods "cfgdgric" and "ucfgdgric"  via
 *      the sysconfig system call.  cfgdgric calls it to configure the device, and
 *      cfgdgric calls it to unconfigure  the device.  It does the following
 *      during configuration:
 *
 *              1.      Installs driver entry points into the system device switch tables.
 *
 *              2.      Programs the adapter to operating parameters passed by cfgdgric.
 *
 *              3.      Binds the interrupt handler to the interrupt level.
 *
 *              4.      Pins the program module into kernel memory.
 *
 *      It does the following during "unconfiguration":
 *
 *              1.      Deinstalls the driver entry points.
 *
 *              2.      Unbinds the interrupt handler.
 *
 *              3.      Unpins the program module.
 *
 */



#define         DARTIC_INVALID_IO_ADDR                      112
#define         DARTIC_INVALID_INTR_LEVEL                   113
#define         DARTIC_INVALID_CARDNUMBER                   114
#define         DARTIC_SETJMPX_FAILURE                      115
#define         DARTIC_LOCKL_FAILURE                        116
#define         DARTIC_I_CLEAR_FAILURE                      117
#define         DARTIC_I_INIT_FAILURE                       118
#define         DARTIC_UNKNOWN_ADAPTER                      119
#define         DARTIC_UNKNOWN_EXCEPTION                    120
#define         DARTIC_PINCODE_FAILURE                      121
#define         DARTIC_UIOMOVE_FAILURE                      122
#define         DARTIC_INVALID_ADAPTER                      123
#define         DARTIC_DEVSWADD_FAILURE                     124
#define         DARTIC_INVALID_WINDOW_SIZE                  125
#define         DARTIC_DEVSWDEL_FAILURE                     126
#define         DARTIC_UNPINCODE_FAILURE                    127

char    sccsver[64];            /* to hold SCCS identifier string     */

dev_t   articdevicenumber;      /* to hold major/minor device number    */


darticconfig(devno,cmd,uiop)
dev_t       devno;          /* device number (major and minor)      */
int         cmd;            /*      command value                   */
char       *uiop;           /* pointer to uio structure (DDS)       */
{
        DARTIC_Adapter  devinfo;              /*  The device dependent struct */
        ulong       majornum = major(devno);  /*  Major Device Number         */
        int         cardnumber = minor(devno);/*  from minor number           */
        char       *busio_addr;               /*  used to hold MCA I/O address*/
        int         pos2;                     /*  Programmable Option Select  */
        int         pos3;                     /*   these are used to program  */
        int         pos4;                     /*    the ARTIC adapter values   */
        int         pos5;                     /*     from the DDS             */
        int         retval;                   /* for return values            */
        int         i,j;                      /* traditional indices          */
        int         amemsize;                 /* read from initreg1           */
        int         adaptmemsize;             /* actual adapter memory size   */
        int         maxpage;                  /* Max page value for mem size  */
        uchar       cc_enable;                /* Used to enable channel check */

        /*
         *      Save major/minor device number
         */

        articdevicenumber = devno;

#ifdef DARTIC_DEBUG
        printf("darticconfig: Entered. ");
        printf(" cmd = %d maj = %d min = %d\n", cmd,majornum,cardnumber);
        printf("           :\n");
#endif

        /*
         *      Obtain lock word for darticconfig - this serializes access to this
         *      function.
         */

        retval = lockl(&config_lock, LOCK_SIGRET);

        if (retval)
        {
#ifdef DARTIC_DEBUG
                printf("darticconfig: lockl return %d\n",retval);
#endif

                return(DARTIC_LOCKL_FAILURE);       /* return error code    */
        }

        if (retval = setjmpx(&config_jumpbuffer))
        {
#ifdef DARTIC_DEBUG
                printf("ricdconfig: setjmpx return %d\n",retval);
#endif
                unlockl(&config_lock);            /* release lock word    */

                return(DARTIC_SETJMPX_FAILURE);     /* return error code    */
        }

        /*
         *      Switch on value of cmd
         *
         *      CFG_INIT is the value passed when this function is called via device
         *      method cfgt1pmd.
         *
         *      CFG_TERM is the value passed when this function is called via device
         *      method ucfgdevice
         */

        switch (cmd)
        {
                case    CFG_INIT:                       /* initialize the driver */
                {
                        /*
                         *      If artic_loadcount is 0, then this is the first time ricdconfig
                         *      has been called since IPL.  We do the following operations
                         *      during the first execution ONLY:
                         *
                         *      1.      Pin code and data associated with this module.
                         *      2.      Initialize the adapter table by setting the field "state"
                         *              to DARTIC_EMPTY.
                         *
                         */

#ifdef DARTIC_DEBUG
                                printf("darticconfig: CFG_INIT entered\n");
#endif
                        if (artic_loadcount == 0)
                        {
                                /*      Pin code in kernel memory */

                                retval = pincode((void *)darticintr);

                                if (retval)             /* pincode Failed       */
                                {
#ifdef DARTIC_DEBUG
                                        printf("darticconfig: pincode() failed, retval = %d\n",
                                                  retval);
#endif
                                        clrjmpx(&config_jumpbuffer);    /* clear exception stack */
                                        unlockl(&config_lock);                  /* release lock word     */
                                        return(DARTIC_PINCODE_FAILURE);             /* return failure code   */
                                }

                                /*      Initialize adapter table */

                                for (i=0 ; i < MAXADAPTERS ; i++)
                                {
                                        /* No adapter present */

                                        artic_adapter[i].state       = DARTIC_EMPTY;

                                        /* -1 indicates no valid value present */

                                        artic_adapter[i].slot        = -1;
                                        artic_adapter[i].cardnumber  = -1;

                                        artic_adapter[i].maxpri      = DEF_MAXPRI;   /* defaults     */
                                        artic_adapter[i].maxtimer    = DEF_MAXTIME;
                                        artic_adapter[i].maxqueue    = DEF_MAXQUEUE;
                                        artic_adapter[i].maxtask     = DEF_MAXTASK;
                                        artic_dma[i].channel_id      = DMA_FAIL;

                                        /*      Zero the interrupt counters  */

                                         artic_intr_count[i] = 0;                        /* no intrs yet */
                                         artic_wu_cookie[i]  = EVENT_NULL;       /* for e_sleep  */

                                        /*      Initialize the lock words       */

                                        cpupage_lock[i]  = LOCK_AVAIL;                  /* lock available */
                                        pcselect_lock[i] = LOCK_AVAIL;
                                }

                                /*      Initialize artic_procptr queue heads */

                                for (i = 0 ; i < ARTICPA_SIZE ; i++)
                                {
                                        artic_procptr[i] = PANULL;         /* NULL pointer */
                                }
                        }

                        /*
                         *      COPY THE DEVICE DEPENDENT STRUCTURE (DDS) FROM USER SPACE.
                         *      The DDS is of type DARTIC_Adapter, and will be copied to
                         *      the global table artic_adapter if valid.
                         */

                        retval = uiomove(&devinfo, sizeof(DARTIC_Adapter), UIO_WRITE, uiop);

                        if (retval)             /* uiomove Failed       */
                        {
#ifdef DARTIC_DEBUG
                                printf("darticconfig: uiomove failed, retval = %d\n",
                                          retval);
#endif

                                /*
                                 * cleanup_for_return does unpin (if needed), clrjmpx, and
                                 *      unlockl(&config_lock).
                                 */

                                cleanup_for_return();

                                return(DARTIC_UIOMOVE_FAILURE);             /* return failure code          */
                        }


                        /*
                         *      PARAMETER VALIDITY CHECKS:
                         *
                         *      1)      Adapter number (cardnumber) must be in the range:
                         *
                         *                      0 <= cardnumber < MAXADAPTERS
                         *
                         */

                        /* adapter type check */


                        /*      cardnumber range check */

                        if ( devinfo.cardnumber < 0 || devinfo.cardnumber >= MAXADAPTERS)
                        {
#ifdef DARTIC_DEBUG
                                printf("darticconfig: Invalid cardnumber: %X\n",
                                         devinfo.cardnumber);
#endif

                                /*
                                 * cleanup_for_return does unpin (if needed),clrjmpx and
                                 *  unlockl.
                                 */

                                cleanup_for_return();

                                return(DARTIC_INVALID_CARDNUMBER);  /*  return failure code     */
                        }

                        /* save this value for easy use */

                        cardnumber = devinfo.cardnumber;


                        /*
                         *      If the debug flag is set, print out some stats
                         */

#ifdef DARTIC_DEBUG
                         printf("           : module_id   = %8X  bus_id    = %8X\n",
                                                 devinfo.module_id,ARTIC_BUS_ID | devinfo.bus_id  );
                         printf("           : basemem     = %8X  baseio    = %8X\n",
                                                 devinfo.basemem ,devinfo.baseio  );
                         printf("           : dmalevel    = %8X  intlevel  = %8X\n",
                                                 devinfo.dmalevel ,devinfo.intlevel);
                         printf("           : windowsize  = %8X  slot      = %8X\n",
                                                 devinfo.windowsize, devinfo.slot);
                         printf("           : dma memory  = %8X  cardnum   = %8X\n",
                                                 devinfo.dmamem, devinfo.cardnumber);

#endif

                        /*
                         *      If artic_loadcount is 0, then this is the first time darticconfig
                         *      has been called since IPL.  Do the following in this case:
                         *
                         *        1.    Populate artic_switchtable with driver entry points.
                         *        2.    Call devswadd with artic_switchtable
                         */

                        if (artic_loadcount == 0)
                        {

                                artic_switchtable.d_open           = darticopen;    /* Entry Points */
                                artic_switchtable.d_close          = darticclose;
                                artic_switchtable.d_ioctl          = darticioctl;
                                artic_switchtable.d_config         = darticconfig;
                                artic_switchtable.d_select         = nodev;
                                artic_switchtable.d_mpx            = darticmpx;

                                artic_switchtable.d_read           = nodev;        /* No Entry Point       */
                                artic_switchtable.d_write          = nodev;
                                artic_switchtable.d_strategy       = nodev;
                                artic_switchtable.d_revoke         = nodev;
                                artic_switchtable.d_print          = nodev;
                                artic_switchtable.d_dump           = nodev;

                                retval = devswadd(devno,&artic_switchtable);       /* add to table */

                                if (retval)
                                {
#ifdef DARTIC_DEBUG
                                        printf("darticconfig: devswadd failed with %d return\n",
                                                 retval);
#endif
                                   /*
                                    * cleanup_for_return does unpin (if needed),clrjmpx and
                                    *  unlockl.
                                    */

                                        cleanup_for_return();

                                        return(DARTIC_DEVSWADD_FAILURE);        /* return error code */
                                }
                                else
                                {
#ifdef DARTIC_DEBUG
                                        printf("darticconfig: devswadd succeeded.\n");
                                        printf("darticconfig: devno = %X\n",articdevicenumber);
#endif
                                }
                        }

                        /*
                         *      Attach to IO bus so we can access POS registers
                         */

                        busio_addr = IOCC_ATT(ARTIC_BUS_ID | devinfo.bus_id,
                                              SLOTADDR(devinfo.slot));
                        /*
                         *      Construct POS register values
                         */

                        /*      POS2 - adapter enable, base I/O address, interrupt level*/

                        pos2  = (uchar) CARD_ENABLE;            /* initial value                */

                        if (devinfo.basetype == MULTIPORT_2)
                        { /* Multiport/2 or X.25 card */

                        switch (devinfo.baseio)                 /* base io value                */
                          {
                                case 0x2A0:
                                {
                                        pos2 |= (uchar) (BASEIO_2A0 << 4);
                                        break;
                                }
                                case 0x6A0:
                                {
                                        pos2 |= (uchar) (BASEIO_6A0 << 4);
                                        break;
                                }
                                case 0xAA0:
                                {
                                        pos2 |= (uchar) (BASEIO_AA0 << 4);
                                        break;
                                }
                                case 0xEA0:
                                {
                                        pos2 |= (uchar) (BASEIO_EA0 << 4);
                                        break;
                                }
                                case 0x12A0:
                                {
                                        pos2 |= (uchar) (BASEIO_12A0 << 4);
                                        break;
                                }
                                case 0x16A0:
                                {
                                        pos2 |= (uchar) (BASEIO_16A0 << 4);
                                        break;
                                }
                                case 0x1AA0:
                                {
                                        pos2 |= (uchar) (BASEIO_1AA0 << 4);
                                        break;
                                }
                                case 0x1EA0:
                                {
                                        pos2 |= (uchar) (BASEIO_1EA0 << 4);
                                        break;
                                }
                                case 0x22A0:
                                {
                                        pos2 |= (uchar) (BASEIO_22A0 << 4);
                                        break;
                                }
                                case 0x26A0:
                                {
                                        pos2 |= (uchar) (BASEIO_26A0 << 4);
                                        break;
                                }
                                case 0x2AA0:
                                {
                                        pos2 |= (uchar) (BASEIO_2AA0 << 4);
                                        break;
                                }
                                case 0x2EA0:
                                {
                                        pos2 |= (uchar) (BASEIO_2EA0 << 4);
                                        break;
                                }
                                case 0x32A0:
                                {
                                        pos2 |= (uchar) (BASEIO_32A0 << 4);
                                        break;
                                }
                                case 0x36A0:
                                {
                                        pos2 |= (uchar) (BASEIO_36A0 << 4);
                                        break;
                                }
                                case 0x3AA0:
                                {
                                        pos2 |= (uchar) (BASEIO_3AA0 << 4);
                                        break;
                                }
                                case 0x3EA0:
                                {
                                        pos2 |= (uchar) (BASEIO_3EA0 << 4);
                                        break;
                                }
                                default:
                                {

                                        /*  If we don't recognize the I/O address, this is a fatal
                                         *      error.  If artic_loadcount == 0, then remove the
                                         *      entry points from the switch table before cleaning
                                         *      up and returning.
                                         */

                                        retval = DARTIC_INVALID_IO_ADDR;    /* invalid I/O address */

                                        if (artic_loadcount == 0)                  /* sole owner of device */
                                        {
                                                /* remove device from kernel switchtable */

                                                retval = devswdel(devno);

                                                if (retval)             /* removal Failed!      */
                                                {
                                                        /* change return code */
                                                        retval = DARTIC_DEVSWDEL_FAILURE;
                                                }
                                        }

                                        /*
                                         *      cleanup_for_return does unpin (if needed),clrjmpx and
                                         *      unlockl.
                                         */

                                        cleanup_for_return();

                                        return(retval);
                                }
                           }
                        }
                        else
                        { /* Portmaster or Sandpiper 5 card */
                                switch (devinfo.baseio)         /* base io value                */
                                {
                                        case 0x2A0:
                                        {
                                                pos2 |= (uchar) (BASEIO_2A0 << 4);
                                                break;
                                        }
                                        case 0x6A0:
                                        {
                                                pos2 |= (uchar) (BASEIO_6A0 << 4);
                                                break;
                                        }
                                        case 0xAA0:
                                        {
                                                pos2 |= (uchar) (BASEIO_AA0 << 4);
                                                break;
                                        }
                                        case 0xEA0:
                                        {
                                                pos2 |= (uchar) (BASEIO_EA0 << 4);
                                                break;
                                        }
                                        case 0x12A0:
                                        {
                                                pos2 |= (uchar) (BASEIO_12A0 << 4);
                                                break;
                                        }
                                        case 0x16A0:
                                        {
                                                pos2 |= (uchar) (BASEIO_16A0 << 4);
                                                break;
                                        }
                                        case 0x1AA0:
                                        {
                                                pos2 |= (uchar) (BASEIO_1AA0 << 4);
                                                break;
                                        }
                                        case 0x1EA0:
                                        {
                                                pos2 |= (uchar) (BASEIO_1EA0 << 4);
                                                break;
                                        }
                                        default:
                                        {

                                                /*  If we don't recognize the I/O address, this is a fatal
                                                 *      error.  If artic_loadcount == 0, then remove the
                                                 *      entry points from the switch table before cleaning
                                                 *      up and returning.
                                                 */

                                                retval = DARTIC_INVALID_IO_ADDR;    /* invalid I/O address */

                                                if (artic_loadcount == 0)                  /* sole owner of device */
                                                {
                                                        /* remove device from kernel switchtable */

                                                        retval = devswdel(devno);

                                                        if (retval)             /* removal Failed!      */
                                                        {
                                                                /* change return code */
                                                                retval = DARTIC_DEVSWDEL_FAILURE;
                                                        }
                                                }

                                                /*
                                                 *      cleanup_for_return does unpin (if needed),clrjmpx and
                                                 *      unlockl.
                                                 */

                                                cleanup_for_return();

                                                return(retval);
                                        }
                                }
                        }


                        /*
                         *      Check Validity of Interrupt Level.  Determine value to
                         *      program POS registers with.
                         */

                        switch (devinfo.intlevel)               /* figure out interrupt value   */
                        {
                                case    3:
                                {
                                        pos2 |= (uchar) (INTLEVEL_3 << 1);
                                        break;
                                }
                                case    4:
                                {
                                        pos2 |= (uchar) (INTLEVEL_4 << 1);
                                        break;
                                }
                                case    7:
                                {
                                        pos2 |= (uchar) (INTLEVEL_7 << 1);
                                        break;
                                }
                                case    9:
                                {
                                        pos2 |= (uchar) (INTLEVEL_9 << 1);
                                        break;
                                }
                                case    10:
                                {
                                        pos2 |= (uchar) (INTLEVEL_10 << 1);
                                        break;
                                }
                                case    11:
                                {
                                        pos2 |= (uchar) (INTLEVEL_11 << 1);
                                        break;
                                }
                                case    12:
                                {
                                        pos2 |= (uchar) (INTLEVEL_12 << 1);
                                        break;
                                }
                                default:
                                {
                                        retval = DARTIC_INVALID_INTR_LEVEL; /* invalid interrupt lev*/

                                        if (artic_loadcount == 0) /* sole owner of device */
                                        {
                                                /* remove device from kernel switchtable */

                                                retval = devswdel(devno);

                                                if (retval)    /* removal Failed! */
                                                {
                                                        /* change return code */
                                                        retval = DARTIC_DEVSWDEL_FAILURE;
                                                }
                                        }

                                        /*
                                         *      cleanup_for_return does unpin (if needed),clrjmpx and
                                         *      unlockl.
                                         */

                                        cleanup_for_return();

                                        return(retval);
                                }
                        }


                        /*
                         *      POS 3,4,5 - These registers are defined one way
                         *      for the MP2 and X25 cards and another for the
                         *      Portmaster and Sandpiper 5. The base card type
                         *      is passed to us via the DDS structure.  We
                         *      program the POS registers based on the adapter
                         *      base type.
                         *
                         *
                         *              POS3            -  base memory address
                         *      POS4,POS5       -  base memory address continured, window size
                         */
                        if (devinfo.basetype == MULTIPORT_2)
                        {   /* Multiport/2 or X.25 card */
                            pos3 = (uchar) ((devinfo.basemem   >> 13) & 0x7F);
                            pos4  = (uchar) ((devinfo.basemem  >> 20) & 0x0F);
                            pos4 |= (uchar) EXTCLKX25BRD ;

                            switch (devinfo.windowsize)
                            {
                               case 0x2000:
                               {
                                       pos5 = (uchar) (POSWIN_8K);
                                       break;
                               }
                               case 0x4000:
                               {
                                       pos5 = (uchar) (POSWIN_16K);
                                       break;
                               }
                               case 0x8000:
                               {
                                       pos5 = (uchar) (POSWIN_32K);
                                       break;
                               }
                               case 0x10000:
                               {
                                       pos5 = (uchar) (POSWIN_64K);
                                       break;
                               }
                               default:
                               {
                                       retval = DARTIC_INVALID_WINDOW_SIZE; /* Bad window size */

                                       if (artic_loadcount == 0)          /* sole owner of device */
                                       {
                                               /* remove device from kernel switchtable */

                                               retval = devswdel(devno);

                                               if (retval)             /* removal Failed!      */
                                               {
                                                       /* change return code */
                                                       retval = DARTIC_DEVSWDEL_FAILURE;
                                               }
                                       }

                                       /*
                                        *      cleanup_for_return does unpin (if needed),clrjmpx
                                        *      and unlockl.
                                        */

                                       cleanup_for_return();

                                       return(retval);
                               }
                            }
                        }
                        else  /* Portmaster or Sandpiper base card */
                        {
                            pos3 = (uchar) ((devinfo.basemem    >> 13) & 0xFF);
                            pos4  = (uchar) ((devinfo.basemem   >> 21) & 0x07);
                            pos4 |= (uchar) ((devinfo.basemem   >> 27) & 0x18);

                            switch (devinfo.windowsize)
                            {
                               case 0x2000:
                               {
                                       pos4 |= (uchar) (POSWIN_8K << 5);
                                       break;
                               }
                               case 0x4000:
                               {
                                       pos4 |= (uchar) (POSWIN_16K << 5);
                                       break;
                               }
                               case 0x8000:
                               {
                                       pos4 |= (uchar) (POSWIN_32K << 5);
                                       break;
                               }
                               case 0x10000:
                               {
                                       pos4 |= (uchar) (POSWIN_64K << 5);
                                       break;
                               }
                               default:
                               {
                                       retval = DARTIC_INVALID_WINDOW_SIZE; /* Bad window size */

                                       if (artic_loadcount == 0)          /* sole owner of device */
                                       {
                                               /* remove device from kernel switchtable */

                                               retval = devswdel(devno);

                                               if (retval)             /* removal Failed!      */
                                               {
                                                       /* change return code */
                                                       retval = DARTIC_DEVSWDEL_FAILURE;
                                               }
                                       }

                                       /*
                                        *      cleanup_for_return does unpin (if needed),clrjmpx
                                        *      and unlockl.
                                        */

                                       cleanup_for_return();

                                       return(retval);
                               }
                            }

                            /* POS5 -  DMA arbitration level,  "fairness enable" bit */
                            /* Enable data parity if we're a Sandpiper               */
                            if (devinfo.basetype == SP5_ADAPTER)
                               pos5 = ((devinfo.dmalevel  << 1) & 0x1E) |
                                        FAIRNESS_ENABLE | PARITY_ENABLE;
                            else
                               pos5 = ((devinfo.dmalevel  << 1) & 0x1E) |
                                        FAIRNESS_ENABLE ;

                        }


                        /*      Now put the calculated values into the registers. */

                        BUSIO_PUTC(busio_addr + 2,pos2);

                        BUSIO_PUTC(busio_addr + 3,pos3);

                        BUSIO_PUTC(busio_addr + 4,pos4);

                        BUSIO_PUTC(busio_addr + 5,pos5);

                        /*      Save POS register values for subsequent re-programming  */

                        artic_adapter[cardnumber].pos2 = pos2;
                        artic_adapter[cardnumber].pos3 = pos3;
                        artic_adapter[cardnumber].pos4 = pos4;
                        artic_adapter[cardnumber].pos5 = pos5;
#ifdef DARTIC_DEBUG
                        printf("darticconfig: POS 2 = %02X\n",pos2);
                        printf("darticconfig: POS 3 = %02X\n",pos3);
                        printf("darticconfig: POS 4 = %02X\n",pos4);
                        printf("darticconfig: POS 5 = %02X\n",pos5);
#endif

                        IOCC_DET(busio_addr);   /* Detach from MCA bus  */

                        /*
                         *      Copy DDS values into the adapter table.
                         */

                        artic_adapter[cardnumber].state            = DARTIC_NOTREADY;
                        artic_adapter[cardnumber].cardnumber       = cardnumber;
                        artic_adapter[cardnumber].adaptertype      = devinfo.adaptertype;
                        artic_adapter[cardnumber].basetype         = devinfo.basetype;
                        artic_adapter[cardnumber].slot             = devinfo.slot;
                        artic_adapter[cardnumber].intlevel         = devinfo.intlevel;
                        artic_adapter[cardnumber].windowsize       = devinfo.windowsize;
                        artic_adapter[cardnumber].dmalevel         = devinfo.dmalevel;
                        artic_adapter[cardnumber].debugflag        = devinfo.debugflag;
                        artic_adapter[cardnumber].module_id        = devinfo.module_id;
                        artic_adapter[cardnumber].basemem          = devinfo.basemem;
                        artic_adapter[cardnumber].dmamem           = devinfo.dmamem;
                        artic_adapter[cardnumber].baseio           = devinfo.baseio;
                        artic_adapter[cardnumber].bus_id           = devinfo.bus_id;

                        delay(HZ);      /* delay for 1 second   */


                        /*
                         *      Poll PROMREADY bit to see if adapter passed self-test.
                         *      function "isbrdready" returns DARTIC_READY or DARTIC_NOTREADY.
                         */

                        for  (i = 0 ; i < 30 ; i++)
                        {
                                if (isbrdready(cardnumber) == DARTIC_READY)
                                {
#ifdef DARTIC_DEBUG
                                        printf("darticconfig: isbrdready returned DARTIC_READY iteration  %d\n",i);
#endif
                                        artic_adapter[cardnumber].state = DARTIC_READY;
                                        break;
                                }
                                else                            /* not ready yet                */
                                {
#ifdef DARTIC_DEBUG
                                        printf("darticconfig: isbrdready iteration %d\n",i);
#endif

                                        delay(HZ);              /* sleep for 1 second   */
                                }
                        }

#ifdef DARTIC_DEBUG
                        printf("darticconfig: artic_adapter[%d].state = %d\n",
                                        cardnumber,artic_adapter[cardnumber].state);
#endif
                        /*
                         *      If adapter is still not ready, then issue a hardware
                         *      reset to the adapter.  IF still not ready, then mark
                         *      the adapter as DARTIC_BROKEN.
                         */

                        if (artic_adapter[cardnumber].state != DARTIC_READY)
                        {
                                /*      Still Not Ready!  Issue hardware reset! */

                                if (boardreset(cardnumber))
                                {
                                        artic_adapter[cardnumber].state = DARTIC_BROKEN;
#ifdef DARTIC_DEBUG
                                        printf("darticconfig: boardreset FAILED.  Marking adapter as BROKEN.\n");
#endif

                                        retval = DARTIC_SELFTEST_FAILURE;

                                        if (artic_loadcount == 0)          /* sole owner of device */
                                        {
                                                /* remove device from kernel switchtable */

                                                devswdel(devno);
                                        }

                                        /*
                                         *      cleanup_for_return does unpin (if needed),clrjmpx
                                         *      and unlockl.
                                         */

                                        cleanup_for_return();

                                        return(retval);
                                }
                        }

                        /*
                         *      Determine the adapter memory size
                         *      1.      read initreg1 and mask with 0x28
                         *      2.      switch on value and adapter type.
                         *      3.      Upper 64K of Multiport/2 and x25 adapters unreadable for
                         *              1 Meg versions.
                         */

                        amemsize = MEMMASK & readdreg(cardnumber,INITREG1);

                        switch (amemsize)
                        {
                                case ONEMEG:
                                {
                                        adaptmemsize = 0x100000;                /* access 1Meg  */
                                        break;
                                }

                                case TWOMEG:
                                {
                                        adaptmemsize = 0x200000;                /* access 2 Meg */
                                        break;
                                }

                                case HALFMEG:
                                {
                                        adaptmemsize = 0x80000;                 /* Only access 512K     */
                                        break;
                                }

                                /* Use 512 K as the default */
                                default:
                                {
                                        adaptmemsize = 0x80000;                 /* Only access 512K     */
                                        break;
                                }
                        }

                        /*
                         *      Now set maxpage based on chosen window size
                         */

                        switch (devinfo.windowsize)
                        {
                                case 0x2000:
                                {
                                        switch (adaptmemsize)
                                        {
                                                case    0x200000:
                                                        maxpage = 0xff;
                                                        break;
                                                case    0x100000:
                                                        maxpage = 0x7f;
                                                        break;
                                                case    0xF0000:
                                                        maxpage = 0x77;
                                                        break;
                                                case    0x80000:
                                                        maxpage = 0x3f;
                                                        break;
                                        }
                                        break;
                                }
                                case 0x4000:
                                {
                                        switch (adaptmemsize)
                                        {
                                                case    0x200000:
                                                        maxpage = 0x7f;
                                                        break;
                                                case    0x100000:
                                                        maxpage = 0x3f;
                                                        break;
                                                case    0xF0000:
                                                        maxpage = 0x3b;
                                                        break;
                                                case    0x80000:
                                                        maxpage = 0x1f;
                                                        break;
                                        }
                                        break;
                                }
                                case 0x8000:
                                {
                                        switch (adaptmemsize)
                                        {
                                                case    0x200000:
                                                        maxpage = 0x3f;
                                                        break;
                                                case    0x100000:
                                                        maxpage = 0x1f;
                                                        break;
                                                case    0xF0000:
                                                        maxpage = 0x1d;
                                                        break;
                                                case    0x80000:
                                                        maxpage = 0x0f;
                                                        break;
                                        }
                                        break;
                                }
                                case 0x10000:
                                {
                                        switch (adaptmemsize)
                                        {
                                                case    0x200000:
                                                        maxpage = 0x1f;
                                                        break;
                                                case    0x100000:
                                                        maxpage = 0x0f;
                                                        break;
                                                case    0xF0000:
                                                        maxpage = 0x0e;
                                                        break;
                                                case    0x80000:
                                                        maxpage = 0x07;
                                                        break;
                                        }
                                        break;
                                }
                        }

                        artic_adapter[cardnumber].maxpage = maxpage;

#ifdef DARTIC_DEBUG
                        printf("darticconfig:  maxpage    = %X\n", maxpage);
#endif

                        /*
                         *      Bind interrupt handler to kernel
                         */

                        artic_intr[cardnumber].cardnumber = cardnumber;

                        artic_intr[cardnumber].artic_interrupt.next     = (struct intr *) 0;

                        artic_intr[cardnumber].artic_interrupt.handler  =  darticintr;

                        artic_intr[cardnumber].artic_interrupt.level    = devinfo.intlevel;

                        artic_intr[cardnumber].artic_interrupt.bid      = ARTIC_BUS_ID | devinfo.bus_id;

                        artic_intr[cardnumber].artic_interrupt.bus_type = BUS_MICRO_CHANNEL;

                        artic_intr[cardnumber].artic_interrupt.flags    = 0;

                        artic_intr[cardnumber].artic_interrupt.priority = INTCLASS3;

                        retval = i_init(&artic_intr[cardnumber]);

                        /*      If i_init failed, then clean up and return */

                        if (retval)
                        {
                                retval = DARTIC_I_INIT_FAILURE;     /* i_init failed                */

                                if (artic_loadcount == 0)          /* sole owner of device */
                                {
                                        /* remove device from kernel switchtable */

                                        retval = devswdel(devno);

                                        if (retval)             /* removal Failed!      */
                                        {
                                                /* change return code */
                                                retval = DARTIC_DEVSWDEL_FAILURE;
                                        }
                                }

                                /*
                                 *      cleanup_for_return does unpin (if needed),clrjmpx
                                 *      and unlockl.
                                 */

                                cleanup_for_return();

                                return(retval);
                        }


                        busio_addr = BUSIO_ATT(ARTIC_BUS_ID | devinfo.bus_id,
                                               devinfo.baseio);

                        /*
                         * Enable synchronous channel check for Portmasters and
                         * Sandpipers
                         */
                         if ((devinfo.basetype == SP5_ADAPTER) ||
                             (devinfo.basetype == PORTMASTER))
                         {
                            BUSIO_PUTC(busio_addr + PTRREG, PCPAR2);
                            cc_enable = BUSIO_GETC(busio_addr + DREG);
                            cc_enable |= 0x60;
                            BUSIO_PUTC(busio_addr + DREG, cc_enable);
                         }

                        /*
                         * Turn on RCM interrupts if we successfully attached
                         *      interrupt handler
                         */


                        BUSIO_PUTC(busio_addr + COMREG, INTENABLE);
                        BUSIO_DET (busio_addr);

#ifdef DARTIC_DEBUG
                        printf("darticconfig: Interrupts enabled.\n");
#endif

                        artic_loadcount++;                 /* increment the load counter */

                        break;
                }

                /*
                 *      Terminate device
                 */

                case    CFG_TERM:
                {

#ifdef DARTIC_DEBUG
                        printf("darticconfig: Load count = %d\n", artic_loadcount);
                        printf("darticconfig: Card number = %d\n", cardnumber);
#endif

                        /*
                         *      Range check the cardnumber
                         */

                        if (cardnumber < 0 || cardnumber >= MAXADAPTERS)
                        {
                                cleanup_for_return();
                                return(DARTIC_INVALID_ADAPTER);
                        }

                        retval = 0;        /* preset to 0 in case not used */

                        artic_loadcount--;   /* decrement the load counter   */

                        artic_adapter[cardnumber].state = DARTIC_NOTREADY;     /* mark as Gone */

                        /*
                         *      Unbind interrupt handler from kernel
                         */

                        i_clear(&artic_intr[cardnumber]);

                        if (artic_loadcount == 0)
                        {

                                /*
                                 *      Remove driver from kernel device switch table
                                 */

                                retval = devswdel(devno);

                                /*
                                 *      Unpin driver module
                                 */

                                unpincode((void *) darticconfig);
                        }

                        break;
                }

                default:
                {
                        break;
                }
        }


#ifdef DARTIC_DEBUG
        printf("darticconfig: exiting.\n");
#endif
        clrjmpx(&config_jumpbuffer);    /* clear exception stack        */
        unlockl(&config_lock);          /* relinquish lock word         */
        return (0);                     /* Return 0 for Success         */
}


/*
 *      cleanup_for_return
 *
 *      This function is used Exclusively in darticconfig of the device driver.
 *      It does the following:
 *
 *              1.      IF artic_loadcount == 0, this indicates that this invokation has
 *                      sole possition of the device.  That means we can finish cleanup
 *                      by unpinning the kernel memory that we are occupying.
 *
 *              2.      Clear our jump buffer from the exception stack by calling clrjmpx.
 */

void
cleanup_for_return()
{
        if (artic_loadcount == 0)                  /* If only user of device  */
        {
                unpincode((void *) darticconfig);/*   unpin the code        */
        }
        clrjmpx(&config_jumpbuffer);    /* clear exception stack */
        unlockl(&config_lock);          /* release lock word     */

        return;
}


/*
 *      darticioctl
 *
 *      darticioctl is the ARTIC Diag device driver ioctl function.  The ARTIC Diag
 *      ioctl is made up of sub-functions.  Following is a list of the sub-function
 *      symbols, and descriptions.
 *
 *      Symbol          Description
 *      ------          -----------
 *
 *      ICARESET        Reset
 *      ICAREADMEM      Read Memory
 *      ICAWRITEMEM     Write Memory
 *      ICAINTREG       Interrupt Register
 *      ICAINTWAIT      Interrupt Wait
 *      ICAINTDEREG     Interrupt Deregister
 *      ICAISSUECMD     Issue Command
 *      ICAGETPARMS     Get Parameters
 *      ICAGETBUFADDRS  Get Buffer Address
 *      ICAGETPRIMSTAT  Get Primary Status
 *      ICASENDCFG      Send Config Parameters
 *      ICAGETADAPTYPE  Return adapter's type (EIB+base card combination)
 *      ICADMASETUP     Set up for DMA transfer
 *      ICADMAREL       Clean up after DMA transfer
 */


darticioctl(device, cmd, arg,devflag,chan,ext)
dev_t                device;        /* major/minor device number       */
int                  cmd;           /* ioctl function value            */
ARTIC_IOCTL_UNION     *arg;         /* pointer to argument structures  */
ulong                devflag;       /* device flags, ignored           */
int                  chan,ext;      /* mpx channel and extension, ignored   */
{
        int     retval;             /* return value from subroutines  */

#ifdef DARTIC_DEBUG
         printf("darticioctl: Entered. cmd = %X arg = %X\n",cmd,arg);
#endif

        retval = 0;               /* preset return value to 0 (success)   */

        /*
         *      switch on the value of cmd, and call the specified ioctl sub-function.
         */

        switch (cmd)
        {

                /* ICAGETPARMS - This ioctl is used to retrieve the RCM operating
                                                 parameters (maxtask,maxpri,maxqueue,maxtime) from
                                                the artic_adapter table                                                            */

                case ICAGETPARMS:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl: case ICAGETPARMS entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICAGETPARMS
                         */

                        retval = artic_icagetparms(device, arg);

                        break;
                }

                /* ICARESET - This ioctl is used to issue a hardware reset
                   to the specified coprocessor                          */

                case ICARESET:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl: case ICARESET entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICARESET
                         */

                        retval = artic_icareset(device, arg);

                        break;
                }

                /* ICASENDCFG    - This ioctl is used to write RCM operating
                                                   parameters to the RCM Interface Block, in
                                                   coprocessor memory.                                                  */

                case ICASENDCFG:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl: case ICASENDCFG    entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICASENDCFG
                         */

                        retval = artic_icasendconfig(device, arg);
                        break;
                }

                /* ICAGETPRIMSTAT - This ioctl is used to retrieve a task's primary
                                                        status byte                                                                             */

                case ICAGETPRIMSTAT:
                {
#ifdef DARTIC_DEBUG
                        printf ("darticioctl: case ICAGETPRIMSTAT entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICAGETPRIMSTAT
                         */

                        retval = artic_icagetprimstat(device, arg);

                        break;
                }


                /* ICAREADMEM - This ioctl is used to read adapter memory */

                case ICAREADMEM:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl: case ICAREADMEM entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICAREADMEM
                         */

                        retval = artic_icareadmem(device, arg);

                        break;
                }

                /* ICAWRITEMEM - This ioctl is used to write adapter memory */

                case ICAWRITEMEM:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl: case ICAWRITEMEM entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICAWRITEMEM
                         */

                        retval = artic_icawritemem(device, arg);

                        break;
                }

                /* ICAGETBUFADDRS - This ioctl is used to retrieve  the
                        buffer control block contents   */

                case ICAGETBUFADDRS:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl: case ICAGETBUFADDRS entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICAGETBUFADDRS
                         */

                        retval = artic_icagetbufaddrs(device, arg);

                        break;
                }


                /* ICAISSUECMD - This is the "Issue Command" ioctl. */

                case ICAISSUECMD:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl: case ICAISSUECMD entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICAISSUECMD
                         */

                        retval = artic_icaissuecmd(device, arg);

                        break;
                }

                /* ICAINTREG - This is the "Interrupt Register" ioctl. */

                case ICAINTREG:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl: case ICAINTREG entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICAINTREG
                         */

                        retval = artic_icaintreg(device, arg);

                        break;
                }

                /* ICAINTWAIT - This is the "Interrupt Wait" ioctl. */

                case ICAINTWAIT:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl: case ICAINTWAIT entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICAINTWAIT
                         */

                        retval = artic_icaintwait(device, arg);

                        break;
                }

                /* ICAINTDEREG - This is the "Interrupt DeRegister" ioctl. */

                case ICAINTDEREG:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl: case ICAINTDEREG entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICAINTDEREG
                         */

                        retval = artic_icaintdereg(device, arg);

                        break;
                }

                /*      ICAIOREAD       - used to read from adapter I/O port            */

                case ICAIOREAD:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl: case ICAIOREAD entered.\n");
#endif

                        retval = artic_icaioread(device, arg);
                        break;
                }

                /*  ICAIOWRITE      - used to write to adapter I/O port */

                case ICAIOWRITE:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl: case ICAIOWRITE entered.\n");
#endif


                        retval = artic_icaiowrite(device, arg);
                        break;
                }

                case ICAPOSREAD:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl: case ICAPOSREAD entered.\n");
#endif

                        retval = artic_icaposread(device, arg);
                        break;
                }

                case ICAPOSWRITE:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl: case ICAPOSWRITE entered.\n");
#endif

                        retval = artic_icaposwrite(device, arg);
                        break;
                }


                case ICAGETADAPTYPE:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl: case ICAGETADAPTYPE entered.\n");
#endif


                        retval = artic_icagetadaptype(device, arg);
                        break;
                }


                case ICADMASETUP:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl: case ICADMASETUP entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICADMASETUP
                         */

                        retval = artic_icadmasetup(device, arg);
                        break;
                }

                case ICADMAREL:
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl: case ICADMAREL entered.\n");
#endif

                        /*
                         *      Call the subfunction for ICADMAREL
                         */

                        retval = artic_icadmarel(device, arg);
                        break;
                }

                default:                                                /* invalid command argument     */
                {
#ifdef DARTIC_DEBUG
                        printf("darticioctl - invalid command");
#endif
                        retval = EINVAL;
                        break;
                }
        }

#ifdef DARTIC_DEBUG
        printf("darticioctl: retval = %d, exiting.\n",retval);
#endif

        return(retval);         /* non-zero if error */
}



/*
 *      Empty function.  This allows us to declare the artic device as
 *      "multiplexed".  By doing this, we cause the device close routine
 *      to be called for each user-space close, instead of just the last on
 *      on the device.
 */


darticmpx()
{

        return(NO_ERROR);
}



/*
 *      darticintr
 *
 *      This is the interrupt handler for the ARTIC Diag device driver.
 *      It determines if the interrupt source was an ARTIC Diag adapter,
 *      and if so, determines the interrupting task's number.  The
 *      interrupt count for the task is incremented, and an e_wakeup
 *      is done on the counts address.
 */


darticintr(handler)
struct DARTIC_IntrPlus *handler;
{
        int     coprocnum;    /* coprocessor number goes here   */
        int     retval;       /* retval from setjmpx            */
        char   *busio_addr;   /* address used for adapter I/O   */
        uchar   taskregvalue; /* value read from task register  */

        /*
         *      Set up an exception handler.  setjmpx will return non-zero if
         *      is entered from an exception condition.   This will prevent
         *      the default action for unforseen exceptions, which is to "assert"
         *      (a nice name for system crash).
         */

        if (retval = setjmpx(&oaintr_jumpbuffer))
        {
#ifdef DARTIC_DEBUG
                printf("darticintr: setjmpx returned %d\n",retval);
#endif

                i_reset(handler);

                return(INTR_SUCC);
        }

#ifdef DARTIC_DEBUG
        printf("darticintr: Entered.\n");
#endif

        coprocnum = handler->cardnumber;           /* coprocessor number   */

        retval = INTR_FAIL;                        /* interrupt not for us */

        /*
         *      Attach to IO bus on MCA
         */

        busio_addr = BUSIO_ATT(ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
        artic_adapter[coprocnum].baseio);

        taskregvalue = BUSIO_GETC(busio_addr + TASKREG);

        BUSIO_DET(busio_addr);

        /*
         *      If value is not 0xFF, then we have an interrupting task .
         *
         *      Note: We acknowledge the interrupt by writing a 0xFF to
         *                the INTID byte of the IB (interface block).  This
         *                is done in the rcm_intr() and task_intr() subroutines.
         */

        switch (taskregvalue)
        {
                case    0xFF:
                {
                        break;          /* nothing from this coprocnum */
                }
                default:                /* interrupt from a task       */
                {

#ifdef DARTIC_DEBUG
                        printf("darticintr: Adapter %d Task %d Interrupt.\n",
                                                        coprocnum,(int)taskregvalue);
#endif

                        /* increment interrupt count    */

                        artic_intr_count[coprocnum]++;

                        /* Wakeup sleeping processes    */

                        e_wakeup(&artic_wu_cookie[coprocnum]);

                        ackRCMintr(coprocnum);    /* ack the intr         */
                        retval = INTR_SUCC;       /* interrupt was for us */

                        break;
                }
        }

        i_reset(handler);

        clrjmpx(&oaintr_jumpbuffer);

#ifdef DARTIC_DEBUG
        printf("darticintr: exiting.\n");
#endif

        return(retval);

}


/*
 *      ackRCMintr()
 *
 *      This routine acknowledges an interrupt from the coprocessor
 *      board by writing the value 0xFF to address 0x441 on the board.
 *      This address is the INTID byte of the IB (Interface Block).
 */

ackRCMintr(coprocnum)
int coprocnum;
{
        uchar   lastpage;
        uchar   page;
        char   *dpram;

        /*
         *      Set page register to show the Interface Block in the window.
         *      Save the last page value so we can restore it.
         */

#ifdef DARTIC_DEBUG
        printf("ackRCMintr: Entered.\n");
#endif

        page     = 0;
        lastpage = setCPUpage(coprocnum,page);

        /*      Ack the inter */

        dpram = BUSMEM_ATT(ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                           artic_adapter[coprocnum].basemem);

        ibINTID(dpram + IBADDR)   = (uchar) 0xFF;

        BUSMEM_DET(dpram);

        /*      restore page value */

        setCPUpage(coprocnum,lastpage);

#ifdef DARTIC_DEBUG
        printf("ackRCMintr: exiting.\n");
#endif

        return(NO_ERROR);
}



/*
 *      darticopen
 *
 *      darticopen is the device driver open function.  It is called when
 *      a user issues an open system call on the /dev/artic device.  darticopen
 *      allocates an DARTIC_Proc structure for the process, and places it on a
 *      linked list pointed to by artic_procptr[index], where index is
 *
 *                      index = PID % ARTICPA_SIZE.
 */

darticopen(devno,oflag,ext)
dev_t    devno;             /* major/minor device number    */
int      oflag;             /* open flags (ignored)         */
char    *ext;               /* extended parameters (ignored)*/
{
        int             i;          /* used for loop index                  */
        int             retval;     /* return value from kernel services    */
        int             index;      /* used to index artic_procptr            */
        struct DARTIC_Proc  *artic_ptr; /* pointer to per-process structure     */
        struct DARTIC_Proc  *lst_ptr; /* "current" pointer for linked list    */

#ifdef DARTIC_DEBUG
        printf("darticopen: Entered.\n");
#endif

        /*
         *      Determine if this process has already opened the device driver.
         *      If it has, then there will be an DARTIC_Proc structure already allocated
         *      for this process, and lookup_articproc will return a pointer.  If
         *      lookup_articproc returns PANULL (null pointer), then there is no DARTIC_Proc
         *      structure allocated for this process, and therefore it has not yet
         *      opened the driver.
         */

        artic_ptr = lookup_articproc();    /* uses kernel service getpid() */

        if (artic_ptr != PANULL)
        {
#ifdef DARTIC_DEBUG
                printf("darticopen: process %d already open error.\n",
                          getpid() );
#endif

                return(E_ICA_ALREADY_OPEN);       /* return error code    */
        }


        /*
         *      Allocate a new process structure from kernel memory
         */

        artic_ptr = xmalloc(sizeof(struct DARTIC_Proc), 1, kernel_heap);

        if (artic_ptr == PANULL)  /* unable to allocate memory */
        {
                return (E_ICA_XMALLOC_FAIL);
        }

        /*
         *      Initialize structure
         */


        artic_ptr->myPID    = getpid();   /* processes PID        */
        artic_ptr->next     = PANULL;             /* NULL link ptr        */
        artic_ptr->tocfcount = 0;                 /* no additional callout entries added
                                                                                                to kernel               */

#ifdef DARTIC_DEBUG
        printf("darticopen: artic_ptr = %X pid = %d\n",
                                        artic_ptr,artic_ptr->myPID);
#endif

        for (i=0 ; i<MAXADAPTERS ; i++)      /* NULL all ProcReg pointers    */
        {
                artic_ptr->prptr[i]    = PRNULL;      /* NULL ProcReg ptr     */
        }

        /*      The index to the pointer array is calculated by dividing the
         *      process ID by the array size (ARTICPA_SIZE) and taking the remainder:
         */

        index = getpid() & ARTICPA_MASK;   /* same as PID % ARTICPA_SIZE */

        /*
         *      Lock access to artic_procptr array while we add new structure
         *      to list
         */

        retval = lockl(&proc_lock, LOCK_SIGRET);

        if (retval)                     /* interrupted by a signal! */
        {
                xmfree(artic_ptr,kernel_heap);    /* free allocated memory        */

                return(EINTR);        /* return signal error code     */
        }

        lst_ptr = artic_procptr[index]; /* first element of linked list */

        if (lst_ptr == PANULL)
        {
#ifdef DARTIC_DEBUG
                printf("darticopen: lst_ptr = PANULL, list was empty\n");
#endif

                artic_procptr[index] = artic_ptr;    /* First on the list            */
        }
        else                                                            /* Find end of list                     */
        {
                while (lst_ptr->next != PANULL)      /* while link ptr is not null   */
                {
                        lst_ptr = lst_ptr->next;     /* next element in linked list  */

#ifdef DARTIC_DEBUG
                        printf("darticopen: lst_ptr = %X\n",lst_ptr);
#endif
                }
                lst_ptr->next = artic_ptr;             /* add new structure to list    */
        }

#ifdef DARTIC_DEBUG
        printf("darticopen: artic_ptr = %X\n",artic_ptr);
#endif

        /*
         *      Done with artic_procptr array.  Unlock it.
         */

        unlockl(&proc_lock);

#ifdef DARTIC_DEBUG
        printf("darticopen: Normal Exit.\n");
#endif

        return(NO_ERROR);                             /*      Success */
}

/*
 *      darticclose
 *
 *      darticclose is the device driver close function.  The close function
 *      is responsible for freeing any resources that the device driver has
 *      allocated on behalf of the process.  For ARTIC Diag, the process will have
 *      allocated memory to store the following:
 *
 *              DARTIC_Proc structure
 *              ProcReg structures
 */


darticclose(devno,flag,ext)
dev_t    devno;   /* major/minor device number  */
int      flag;    /* multiplexed channer number (ignored) */
caddr_t  ext;     /* extension parameter (ignored) */
{
        int              i,retval;   /* index, returned value from lockl   */
        struct DARTIC_Proc *artic_ptr;   /* pointer to per-process structure   */
        struct DARTIC_Proc *lst_ptr;   /* "current" pointer for linked list  */
        struct ProcReg   *prptr;     /* pointer to ProcReg for removal     */
        struct ProcReg   *lastptr;   /* pointer to ProcReg for removal     */

#ifdef DARTIC_DEBUG
        printf("darticclose: Entered.\n");
#endif

        /*
         *      Lock access to artic_procptr array so we can remove our DARTIC_Proc struct
         */

        retval = lockl(&proc_lock, LOCK_SIGRET);

        if (retval)                       /* interrupted by a signal! */
        {
                return(EINTR);            /* return signal error code */
        }

        /*
         *      Function remove_articproc is called to remove the structure from
         *      the linked list pointed to by artic_procptr.  It returns the pointer
         *      to the structure, so we can free allocated ProcReg memory before
         *      freeing the DARTIC_Proc memory.
         */

        artic_ptr = remove_articproc();       /* delete from artic_procptr list  */

        retval = 0;                      /* Preset to successful return..*/

        if (artic_ptr == PANULL)           /* could not find our struct    */
        {
#ifdef DARTIC_DEBUG
                printf("darticclose: could not find DARTIC_Proc.\n");
#endif

                retval = EBADF;          /* Could Not find our Structure */
        }
        else
        {
#ifdef DARTIC_DEBUG
                printf("darticclose: artic_ptr = %X pid = %d\n",
                          artic_ptr, artic_ptr->myPID);
#endif

                /*
                 *      Free any ProcReg structures that have been allocated for
                 *      this process
                 */

                for (i = 0 ; i<MAXADAPTERS ; i++)
                {
                        /* Interrupt Register ProcReg structs   */

                        prptr = artic_ptr->prptr[i];

                        while (prptr != PRNULL)
                        {
                                lastptr = prptr;   /* save current pointer */
                                prptr = prptr->next;/* get next link       */
                                xmfree(lastptr,kernel_heap); /* free current pointer    */
#ifdef DARTIC_DEBUG
                                printf("darticclose: IR xmfree (%X) \n",lastptr);
#endif
                        }

                }

                /*      check tocfcount to  see how many times timeoutcf(1) was called */

                for (i=0 ; i < artic_ptr->tocfcount ; i++)
                {
                        timeoutcf(-1);          /* decrement callout table entries      */
                }

                xmfree(artic_ptr,kernel_heap);    /* free allocated memory    */
        }

        unlockl(&proc_lock);           /* unlock proc_lock access lock */

#ifdef DARTIC_DEBUG
        printf("darticclose: Exit.  retval = %d\n",retval);
#endif

        return(retval);           /* return either 0 or EBADF */
}


/************************************
 *  DEVICE DRIVER SUPPORT FUNCTIONS *
 ************************************/



/*
 *      artic_put_pcselect
 *
 *      artic_put_pcselect is used to put a value in the PC Select
 *      byte of RCM's Interface Block.
 */

artic_put_pcselect(coprocnum, pcselect)
int     coprocnum;        /* coprocessor number          */
uchar   pcselect;         /* Value to put in PC select   */
{
        int     retval;
        char    *dpram;
        uchar   lastpage;

#ifdef DARTIC_DEBUG
        printf("artic_put_pcselect: Entered.\n");
#endif

        /*
         *      NOTE: CPUPAGE register should be locked prior to this call
         */

        /* position window over IB page 0 */

        lastpage = setCPUpage(coprocnum,(uchar) 0);

        /*      Attach to MCA memory bus */

        dpram = BUSMEM_ATT(ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                           artic_adapter[coprocnum].basemem);

        /*      Put the value in the PC Select byte */

        ibPC_SELECT(dpram + IBADDR) = pcselect;

#ifdef DARTIC_DEBUG
        printf("artic_put_pcselect: pcselect = %d\n",(int)pcselect);
#endif

        /* detach from MCA bus  */

        BUSMEM_DET(dpram);

        /*      replace CPUPAGE value   */

        setCPUpage(coprocnum,lastpage);

#ifdef DARTIC_DEBUG
        printf("artic_put_pcselect: exiting.\n");
#endif

        return(NO_ERROR);                                                       /* indicate success             */
}

/*
 *      artic_get_pcselect
 *
 *      artic_get_pcselect is used to retrieve the PC Select
 *      Byte from RCM's Interface Block.  The PCS is returned
 *      in *pcsptr.  This is because the PCS may have any
 *      value, and we need a way to indicate failure.
 */

artic_get_pcselect(coprocnum, pcsptr)
int coprocnum;                  /* coprocessor number           */
uchar   *pcsptr;                /* char for return value        */
{
        int      retval;
        char    *dpram;
        uchar    lastpage;

        /*      Lock the cpupage_lock to serialize access       */

        retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET);

        if (retval)
        {
#ifdef DARTIC_DEBUG
                printf("artic_get_pcselect: lockl failure - retval = %d.\n",
                                        retval);
#endif
                return(EINTR);                          /* lockl failure */
        }

        /*
         *      Set up a jumpbuffer for exceptions.
         */

        if (retval = setjmpx(&artic_jumpbuffers[coprocnum]))
        {
#ifdef DARTIC_DEBUG
                printf("artic_get_pcselect: setjmpx returned %d\n",retval);
#endif

                unlockl(&cpupage_lock[coprocnum]);                      /* release lock word    */

                return(EIO);                                                            /* return error code    */
        }

#ifdef DARTIC_DEBUG
        printf("artic_get_pcselect: Entered.\n");
#endif

        /* position window over IB page 0 */

        lastpage = setCPUpage(coprocnum,(uchar) 0);

        /*      Attach to MCA memory bus */

        dpram = BUSMEM_ATT(ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                           artic_adapter[coprocnum].basemem);

        /*      Retrieve the PC Select Byte     */

        *pcsptr = ibPC_SELECT(dpram + IBADDR);

        /* detach from MCA bus  */

        BUSMEM_DET(dpram);

        /*      replace CPUPAGE value   */

        setCPUpage(coprocnum,lastpage);

        /*      Clear exception stack           */

        clrjmpx(&artic_jumpbuffers[coprocnum]);

        /*      Unlock access to adapter        */

        unlockl(&cpupage_lock[coprocnum]);

#ifdef DARTIC_DEBUG
        printf("artic_get_pcselect: exiting.\n");
#endif

        return(NO_ERROR);                                                       /* indicate success             */
}


/*
 *      artic_put_cmd
 *
 *      artic_put_cmd is used to retrieve the Buffer Control
 *      Block for a specified task.
 *
 *      The return value is either 0 for success, EINTR
 *      if lockl fails, and EIO if an exception occurs
 *      (setjmpx returns non-zero).
 */

artic_put_cmd(coprocnum, cmdcode)
int      coprocnum;             /* coprocessor number       */
uchar   cmdcode;                /* command code for BCB     */
{
        int      retval;      /* return value from lockl      */
        char    *dpram;       /* pointer to adapter memory    */
        ushort   bcboffset;   /* offset of BCB                */
        uchar    lastvalue;   /* previous value in CPUPAGE    */

#ifdef DARTIC_DEBUG
        printf("artic_put_cmd: Entered.\n");
#endif

        /*      Lock the cpupage_lock to serialize access       */

        if (retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET))
        {
#ifdef DARTIC_DEBUG
                printf("artic_put_cmd: lockl failure - retval = %d.\n",
                                        retval);
#endif
                return(EINTR);          /* lockl failure due to signal */
        }

        /*
         *      Set up a jumpbuffer for exceptions.
         */

        if (retval = setjmpx(&artic_jumpbuffers[coprocnum]))
        {
#ifdef DARTIC_DEBUG
                printf("artic_put_cmd: setjmpx returned %d\n",retval);
#endif

                unlockl(&cpupage_lock[coprocnum]);  /* release lock word    */

                return(EIO);                                                            /* return error code    */
        }

        /* position window over IB page 0 */

        lastvalue = setCPUpage(coprocnum,(uchar) 0);

        /*      Attach to MCA memory bus */

        dpram = BUSMEM_ATT(ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                           artic_adapter[coprocnum].basemem);

        /*      Get offset of specified BCB     */

        bcboffset       = (ulong) to16local(ibBCB_OFF(dpram + IBADDR));


        /*
         *      Put command code in the BCB
         */

#ifdef DARTIC_DEBUG
        printf("*** artic_put_cmd: Putting %d in BCB ***\n", cmdcode);
#endif
        bcbCMD(dpram + bcboffset) = cmdcode;

        BUSMEM_DET(dpram);                      /* detach from MCA bus  */

        /*      Restore CPUPAGE register to last value                  */

        setCPUpage(coprocnum,lastvalue);

        /*      Clear jumpbuffer from exception stack                   */

        clrjmpx(&artic_jumpbuffers[coprocnum]);

        /*      Unlock to allow access to cpupage register              */

        unlockl(&cpupage_lock[coprocnum]);

        /*      return 0 to indicate success                            */

#ifdef DARTIC_DEBUG
        printf("artic_put_cmd: Exiting.\n");
#endif

        return(NO_ERROR);
}

/*
 *      artic_read_mem
 *
 *      This function is used to read data to user-space from
 *      coprocessor adapter memory.  Addresses are always in
 *      page/offset format.
 */

artic_read_mem(coprocnum, destbuff,page,offset,length)
int    coprocnum;             /* coprocessor number         */
char   *destbuff;             /* destination in user-space  */
uchar  page;                  /* destination page           */
ushort offset;                /* destination offset         */
ulong  length;                /* # bytes to transfer        */
{
        int     i;              /* loop index                   */
        int     retval;         /* return value from lockl      */
        char    *dpram;         /* pointer to adapter memory    */
        char    *busio_addr;    /* pointer to adapter I/O       */
        uchar    lastvalue;     /* previous value in CPUPAGE    */

#ifdef DARTIC_DEBUG
        printf("artic_read_mem: Entered.\n");
        printf("           : coprocnum = %d\n",coprocnum);
        printf("           : destbuff   = %X\n",destbuff  );
        printf("           : page      = %X\n",(ulong)page);
        printf("           : offset    = %X\n",(ulong)offset);
        printf("           : length    = %d\n",length);
#endif

        /*      Lock the cpupage_lock to serialize access       */

        if (retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET))
        {
#ifdef DARTIC_DEBUG
                printf("artic_read_mem: lockl failure - retval = %d.\n",
                                        retval);
#endif
                return(EINTR);          /* lockl failure due to signal */
        }

        /*
         *      Set up a jumpbuffer for exceptions.
         */

        if (retval = setjmpx(&artic_jumpbuffers[coprocnum]))
        {
#ifdef DARTIC_DEBUG
                printf("artic_read_mem: setjmpx returned %d\n",retval);
#endif

                unlockl(&cpupage_lock[coprocnum]);  /* release lock word    */

                return(EIO);                                                            /* return error code    */
        }

        retval = NO_ERROR;              /* preset return value to success */

        /* position window over destination page        */

        lastvalue = setCPUpage(coprocnum,page);

        /*      Attach to MCA memory and I/O bus */

        dpram           = BUSMEM_ATT(ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                                     artic_adapter[coprocnum].basemem);
        busio_addr  = BUSIO_ATT( ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                                 artic_adapter[coprocnum].baseio);

        i = 0;
        while (i < length)
        {
                /* fetch coproc byte and put in user memory                     */

                subyte(destbuff + i,*(dpram + offset));

                offset++;                       /* increment source      offset         */
                i++;                            /* increment destination offset */

                /* check window wrap    */

                if (offset >= artic_adapter[coprocnum].windowsize && i < length)
                {
                        page++;                 /* increment page number        */
                        offset = 0;             /* reset offset                         */

                        /*      Check page Range        */

                        if (page > artic_adapter[coprocnum].maxpage)
                        {
                                retval = E_ICA_INVALID_PAGE;
#ifdef DARTIC_DEBUG
                                printf("artic_read_mem: Invalid Page %d\n",page);
#endif
                                break;
                        }

                        /* reprogram cpupage register   */

#ifdef DARTIC_DEBUG
                        printf("artic_read_mem: setting page to %d\n",page);
#endif

                        BUSIO_PUTC(busio_addr + CPUPAGE, page);
                }
        }

        /*      detach from MCA bus     */

        BUSIO_DET(busio_addr);
        BUSMEM_DET(dpram);

        /*      Restore CPUPAGE register to last value                  */

        setCPUpage(coprocnum,lastvalue);

        /*      Clear jumpbuffer from exception stack                   */

        clrjmpx(&artic_jumpbuffers[coprocnum]);

        /*      Unlock to allow access to cpupage register              */

        unlockl(&cpupage_lock[coprocnum]);

        /*      return 0 to indicate success                                    */

#ifdef DARTIC_DEBUG
        printf("artic_read_mem: exiting.\n");
#endif

        return(retval);
}

/*
 *      artic_write_mem
 *
 *      This function is used to write data from user-space to
 *      coprocessor adapter memory.  Addresses are always in
 *      page/offset format.
 */

artic_write_mem(coprocnum, srcbuff,page,offset,length)
int     coprocnum;             /* coprocessor number           */
char   *srcbuff;               /* source buffer in user-space  */
uchar   page;                  /* destination page             */
ushort  offset;                /* destination offset           */
int     length;                /* # bytes to transfer          */
{
        int    i;              /* loop index                   */
        int    retval;         /* return value from lockl      */
        char   *dpram;         /* pointer to adapter memory    */
        char   *busio_addr;    /* pointer to adapter I/O       */
        uchar  lastvalue;      /* previous value in CPUPAGE    */

#ifdef DARTIC_DEBUG
        printf("artic_write_mem: Entered.\n");
        printf("            : coprocnum = %d\n",coprocnum);
        printf("            : srcbuff   = %X\n",srcbuff  );
        printf("            : page      = %X\n",(ulong)page);
        printf("            : offset    = %X\n",(ulong)offset);
        printf("            : length    = %d\n",length);
#endif

        /*      Lock the cpupage_lock to serialize access       */

        if (retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET))
        {
#ifdef DARTIC_DEBUG
                printf("artic_write_mem: lockl failure - retval = %d.\n",
                                        retval);
#endif
                return(EINTR);          /* lockl failure due to signal */
        }

        /*
         *      Set up a jumpbuffer for exceptions.
         */

        if (retval = setjmpx(&artic_jumpbuffers[coprocnum]))
        {
#ifdef DARTIC_DEBUG
                printf("artic_write_mem: setjmpx returned %d\n",retval);
#endif

                unlockl(&cpupage_lock[coprocnum]);  /* release lock word    */

                return(EIO);                                                            /* return error code    */
        }

        retval = NO_ERROR;              /* preset to success    */

        /* position window over destination page        */

        lastvalue = setCPUpage(coprocnum,page);

        /*      Attach to MCA memory and I/O bus */

        dpram           = BUSMEM_ATT(ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                                     artic_adapter[coprocnum].basemem);
        busio_addr  = BUSIO_ATT( ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                                 artic_adapter[coprocnum].baseio);

        i = 0;
        while (i < length)
        {
                /* fetch user byte and put in coproc memory                     */

                *(dpram + offset) = fubyte(srcbuff + i);

                offset++;       /* increment destination offset */
                i++;            /* increment source offset      */

                /* check window wrap    */

                if (offset >= artic_adapter[coprocnum].windowsize && i < length)
                {
                        page++;        /* increment page number */
                        offset = 0;    /* reset offset          */

                        /*      Check page Range        */

                        if (page > artic_adapter[coprocnum].maxpage)
                        {
                                retval = E_ICA_INVALID_PAGE;
#ifdef DARTIC_DEBUG
                                printf("artic_write_mem: Invalid Page %d\n",page);
#endif
                                break;
                        }

                        /* reprogram cpupage register   */

                        BUSIO_PUTC(busio_addr + CPUPAGE, page);
                }
        }

        /*      detach from MCA bus     */

        BUSIO_DET(busio_addr);
        BUSMEM_DET(dpram);

        /*      Restore CPUPAGE register to last value                  */

        setCPUpage(coprocnum,lastvalue);

        /*      Clear jumpbuffer from exception stack                   */

        clrjmpx(&artic_jumpbuffers[coprocnum]);

        /*      Unlock to allow access to cpupage register              */

        unlockl(&cpupage_lock[coprocnum]);

        /*      return 0 to indicate success                                    */

#ifdef DARTIC_DEBUG
        printf("artic_write_mem: exiting.\n");
#endif

        return(retval);
}

/*
 *      artic_get_bcb
 *
 *      artic_get_bcb is used to retrieve the Buffer Control
 *      Block for task 0.
 *
 *      The return value is either 0 for success, EINTR
 *      if lockl fails, and EIO if an exception occurs
 *      (setjmpx returns non-zero).
 */

artic_get_bcb(coprocnum, bcbptr)
int                  coprocnum;  /* coprocessor number    */
ICAGETBUFADDRS_PARMS *bcbptr;    /* char for return value */
{
        int      retval;     /* return value from lockl      */
        char    *dpram;      /* pointer to adapter memory    */
        uchar    lastvalue;  /* previous value in CPUPAGE    */
        ushort   bcboffset;  /* offset of specified BCB      */

#ifdef DARTIC_DEBUG
        printf("artic_get_bcb: Entered.\n");
#endif

        /*      Lock the cpupage_lock to serialize access       */

        if (retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET))
        {
#ifdef DARTIC_DEBUG
                printf("artic_get_bcb: lockl failure - retval = %d.\n",
                                        retval);
#endif
                return(EINTR);          /* lockl failure due to signal */
        }

        /*
         *      Set up a jumpbuffer for exceptions.
         */

        if (retval = setjmpx(&artic_jumpbuffers[coprocnum]))
        {
#ifdef DARTIC_DEBUG
                printf("artic_get_bcb: setjmpx returned %d\n",retval);
#endif

                unlockl(&cpupage_lock[coprocnum]);  /* release lock word    */

                return(EIO);                                                            /* return error code    */
        }

        /* position window over IB page 0 */

        lastvalue = setCPUpage(coprocnum,(uchar) 0);

        /*      Attach to MCA memory bus */

        dpram = BUSMEM_ATT(ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                           artic_adapter[coprocnum].basemem);

        /*      Get offset of specified BCB     */

        bcboffset       = (ulong) to16local(ibBCB_OFF(dpram + IBADDR));


#ifdef DARTIC_DEBUG
        printf("          : bcboffset = %X\n",bcboffset);
#endif

        /*
         *      Retrieve the BCB, swapping bytes for offset and length fields.
         */

        /*      Output buffer */

        bcbptr->ob.length = to16local(bcbOUTLNG(dpram + bcboffset));
        bcbptr->ob.offset = to16local(bcbOUTOFF(dpram + bcboffset));
        bcbptr->ob.page   = bcbOUTPAG(dpram + bcboffset);

#ifdef DARTIC_DEBUG2
        printf("          : ob.page   = %X\n",(ulong)bcbptr->ob.page);
        printf("          : ob.offset = %X\n",(ulong)bcbptr->ob.offset);
        printf("          : ob.length = %X\n",(ulong)bcbptr->ob.length);
#endif

        /*      Input Buffer    */

        bcbptr->ib.length = to16local(bcbINLNG(dpram + bcboffset));
        bcbptr->ib.offset = to16local(bcbINOFF(dpram + bcboffset));
        bcbptr->ib.page   = bcbINPAG(dpram + bcboffset);

#ifdef DARTIC_DEBUG2
        printf("          : ib.page   = %X\n",(ulong)bcbptr->ib.page);
        printf("          : ib.offset = %X\n",(ulong)bcbptr->ib.offset);
        printf("          : ib.length = %X\n",(ulong)bcbptr->ib.length);
#endif

        /*      Secondary Status Buffer */

        bcbptr->ssb.length = to16local(bcbSTATLNG(dpram + bcboffset));
        bcbptr->ssb.offset = to16local(bcbSTATOFF(dpram + bcboffset));
        bcbptr->ssb.page   = bcbSTATPAG(dpram + bcboffset);

#ifdef DARTIC_DEBUG2
        printf("          : ssb.page   = %X\n",(ulong)bcbptr->ssb.page);
        printf("          : ssb.offset = %X\n",(ulong)bcbptr->ssb.offset);
        printf("          : ssb.length = %X\n",(ulong)bcbptr->ssb.length);
#endif


        BUSMEM_DET(dpram);                      /* detach from MCA bus  */

        /*      Restore CPUPAGE register to last value                  */

        setCPUpage(coprocnum,lastvalue);

        /*      Clear jumpbuffer from exception stack                   */

        clrjmpx(&artic_jumpbuffers[coprocnum]);

        /*      Unlock to allow access to cpupage register              */

        unlockl(&cpupage_lock[coprocnum]);

        /*      return 0 to indicate success                                    */

#ifdef DARTIC_DEBUG
        printf("artic_get_bcb: exiting.\n");
#endif
        return(NO_ERROR);
}


/*
 *      to16local:
 *
 *      This subroutine accepts an intel byte-order ushort as a parameter,
 *      and returns the same value in local byte order format.
 */

ushort
to16local(thebit16)
ushort thebit16;
{
        SPEC16  tb16;

        tb16.b16 = thebit16;

        return  ((ushort)((tb16.sep.i_low         & 0x00FF) |
                                         ((tb16.sep.i_high << 8)  & 0xFF00)));
}


/*
 *      artic_get_psb
 *
 *      artic_get_psb is used to retrieve the task 0 Primary Status
 *      Byte from the Interface Block.  The PSB is returned
 *      in *psbptr.  The return value is either 0 for success,
 *      EINTR if lockl fails, and EIO if an exception occurs
 *      (setjmpx returns non-zero).
 */

artic_get_psb(coprocnum, psbptr)
int coprocnum;        /* coprocessor number           */
uchar   *psbptr;      /* char for return value        */
{
        int      retval;    /* return value from lockl      */
        char    *dpram;     /* pointer to adapter memory    */
        uchar    lastvalue; /* previous value in CPUPAGE    */

#ifdef DARTIC_DEBUG
        printf("artic_get_psb: Entered.\n");
#endif

        /*      Lock the cpupage_lock to serialize access       */


        if (retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET))
        {
#ifdef DARTIC_DEBUG
                printf("artic_get_psb: lockl failure - retval = %d.\n",
                                        retval);
#endif
                return(EINTR);          /* lockl failure due to signal */
        }

        /*
         *      Set up a jumpbuffer for exceptions.
         */

        if (retval = setjmpx(&artic_jumpbuffers[coprocnum]))
        {
#ifdef DARTIC_DEBUG
                printf("artic_get_psb: setjmpx returned %d\n",retval);
#endif

                unlockl(&cpupage_lock[coprocnum]);   /* release lock word    */

                return(EIO);                                                            /* return error code    */
        }

        /* position window over IB page 0 */

        lastvalue = setCPUpage(coprocnum,(uchar) 0);

        /*      Attach to MCA memory bus */

        dpram = BUSMEM_ATT(ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                           artic_adapter[coprocnum].basemem);

        /*      Retrieve the Primary Status Byte */

        *psbptr = ibSTATARRAY(dpram + IBADDR, 0);

#ifdef DARTIC_DEBUG
        printf("artic_get_psb: *psbptr = %X\n",*psbptr);
#endif

        BUSMEM_DET(dpram);                      /* detach from MCA bus  */

        /*      Restore CPUPAGE register to last value                  */

        setCPUpage(coprocnum,lastvalue);

        /*      Clear jumpbuffer from exception stack                   */

        clrjmpx(&artic_jumpbuffers[coprocnum]);

        /*      Unlock to allow access to cpupage register              */

        unlockl(&cpupage_lock[coprocnum]);

        /*      return 0 to indicate success                                    */

        return(NO_ERROR);
}


/*
 *      readdreg - writedreg
 *
 *      On the IBM Artic board, various registers are written to and read from
 *      indirectly by first programming the PTRREG register and then reading
 *      the desired register from DREG.  DREG is a "window" to other registers,
 *      based on the "pointer" value first programmed into PTRREG.  Values for the
 *      register "pointers" and such are in articdd.h and taken from the Realtime
 *      Interface Co-Processor Portmaster Adapter hardware tech ref, in the System
 *      Addressable Registers chapter.
 *
 */

uchar
readdreg(coprocnum,regp)
int coprocnum;       /* coprocessor number     */
int regp;            /* PTRREG "pointer"       */
{
        uchar    tbyte;         /* holds byte read from register        */
        char    *busio_addr;    /* address returned from BUSIO_ATT      */


        /*
         *      Attach to MCA I/O bus
         */

        busio_addr = BUSIO_ATT(ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                               artic_adapter[coprocnum].baseio);

        /*      Program "pointer" register */

        BUSIO_PUTC(busio_addr + PTRREG, (uchar) regp);          /* output to PTRREG */

        /*      Get the byte from the data register */

        tbyte = BUSIO_GETC(busio_addr + DREG);

        /*      Detach from the MCA I/O bus */

        BUSIO_DET(busio_addr);

        return(tbyte);                          /* return DREG value    */
}

writedreg(coprocnum,regp,rvalue)
int     coprocnum;                      /* coprocessor number */
int     regp;                           /* PTRREG "pointer"   */
int             rvalue;
{
        char    *busio_addr;

        /*
         *      Attach to MCA I/O bus
         */

        busio_addr = BUSIO_ATT(ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                               artic_adapter[coprocnum].baseio);

        BUSIO_PUTC(busio_addr + PTRREG, (uchar) regp);          /* "pointer" reg        */
        BUSIO_PUTC(busio_addr + DREG,   (uchar) rvalue);        /* data register        */

        /* detach */

        BUSIO_DET(busio_addr);

}


/*
 *      setCPUpage
 *
 *      This programs the CPUPAGE register for the specified adapter
 *      with the value passed in the page parameter.
 */

uchar
setCPUpage(coprocnum,page)
int     coprocnum;
uchar   page;
{
        uchar    lastpage;              /* last value for page                  */
        char    *busio_addr;    /* address of CPUPAGE register  */

#ifdef DARTIC_DEBUG
        printf("setCPUpage: Entered. page = %d\n",(int)page);
#endif

        busio_addr = BUSIO_ATT(ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                               artic_adapter[coprocnum].baseio);

        lastpage = BUSIO_GETC(busio_addr + CPUPAGE);

        BUSIO_PUTC(busio_addr + CPUPAGE, page);

        BUSIO_DET(busio_addr);

#ifdef DARTIC_DEBUG
        printf("setCPUpage: exiting. lastpage = %d\n",(int)lastpage);
#endif

        return(lastpage);
}


/*
 *      inteltolocal
 *
 *      This subroutine accepts as its parameter an address assumed to be
 *      in Intel byte order format.  It returns the same address in local byte
 *      order format, whatever that may be.  Returns a char pointer.
 */

char *
inteltolocal(intelptr)
SPEC32 intelptr;
{
        char    *retvalue;              /* for return value     */

        /* put address into local byte format */

        retvalue =      (char *)
    ((((ulong) intelptr.bytes.ll      )   & 0x000000FF)  +
         (((ulong) intelptr.bytes.lh << 8 )   & 0x0000FF00)  +
         (((ulong) intelptr.bytes.hl << 16)   & 0x00FF0000)  +
         (((ulong) intelptr.bytes.hh << 24)   & 0xFF000000));

        return(retvalue);
}

/*
 *      localtointel
 *
 *              This routine accepts a ulong in local format and returns
 *      the "same" value in Intel byte order format.
 */


localtointel(local)
ulong local;
{
        SPEC32  inteladd;

        /* put address into Intel format */

        inteladd.seper.offset.sep.i_low =
                        (uchar) ( local                 & 0xFF);
        inteladd.seper.offset.sep.i_high =
                        (uchar) ((local >> 8)   & 0xFF);
        inteladd.seper.segment.sep.i_low =
                        (uchar) ((local >> 16)  & 0xFF);
        inteladd.seper.segment.sep.i_high =
                        (uchar) ((local >> 24)  & 0xFF);

        /* send it back whole */

        return(inteladd.b32);
}

/*
 *      lookup_articproc
 *
 *      lookup_articproc uses the calling processes PID (obtained from the kernel
 *      service getpid() ) to see if an DARTIC_Proc structure is on
 *      any linked list pointed to by the artic_procptr array.
 *
 *      lookup_articproc returns a pointer to the DARTIC_Proc structure on the linked
 *      list, or returns (struct DARTIC_Proc *) NULL if one is not found.
 */

struct DARTIC_Proc *
lookup_articproc()
{
        struct DARTIC_Proc *artic_ptr;   /* pointer to an DARTIC_Proc structure */
        int              index;      /* used to index artic_procptr array  */
        int              retval;

        /*      The index to the pointer array is calculated by dividing the
         *      process ID by the array size (ARTICPA_SIZE) and taking the remainder:
         */

        index = getpid() & ARTICPA_MASK;   /* same as PID % ARTICPA_SIZE */

#ifdef DARTIC_DEBUG
        printf("lookup_articproc: index = %d\n",index);
#endif

        /*
         *      Lock access to artic_procptr array
         */

        retval = lockl(&proc_lock, LOCK_SIGRET);

        if (retval)
        {
                return(PANULL);
        }

        /*      The linked list pointed to by artic_procptr[index] is traversed, looking
         *      for a matching pid in the myPID field of the current DARTIC_Proc struct.
         *      IF we find a match, we break, and return the pointer to that structure,
         *      else we reach the end of the list, and return NULL.
         */

        artic_ptr = artic_procptr[index];    /* get pointer to first in list */

        while (artic_ptr != PANULL )              /* while not  NULL                              */
        {
                if (artic_ptr->myPID == getpid() ) /* Matching PIDs?       */
                {
                        break;
                }
                artic_ptr = artic_ptr->next;                /* next link    */
        }

#ifdef DARTIC_DEBUG
        printf("lookup_articproc: artic_ptr = %X\n",artic_ptr);
#endif

        unlockl(&proc_lock);            /* unlock access to proc_lock   */

        return(artic_ptr);                        /* return either NULL or pointer to struct */
}

/*
 *      remove_articproc
 *
 *      remove_articproc uses the calling processes PID (obtained from the kernel
 *      service getpid() ) to see if an DARTIC_Proc structure is on
 *      any linked list pointed to by the artic_procptr array.
 *
 *      remove_articproc returns a pointer to the DARTIC_Proc structure that was on
 *      on the linked list after removeing it from that list, or returns
 *      PANULL if one is not found.
 */

struct DARTIC_Proc *
remove_articproc()
{
        struct DARTIC_Proc *artic_ptr;    /* pointer to an DARTIC_Proc structure */
        struct DARTIC_Proc *last_ptr;   /* pointer to an DARTIC_Proc structure */
        int              index;       /* used to index artic_procptr array  */

        /*      The index to the pointer array is calculated by dividing the
         *      process ID by the array size (ARTICPA_SIZE) and taking the remainder:
         */

        index = getpid() & ARTICPA_MASK;   /* same as PID % ARTICPA_SIZE */

#ifdef DARTIC_DEBUG
        printf("remove_articproc: index = %d\n",index);
#endif

        /*      The linked list pointed to by artic_procptr[index] is traversed, looking
         *      for a matching pid in the myPID field of the current DARTIC_Proc struct.
         *      IF we find a match, we break, and return the pointer to that structure,
         *      else we reach the end of the list, and return NULL.
         */

        artic_ptr = artic_procptr[index];    /* get pointer to first in list */

        if (artic_ptr != PANULL)
        {
                if (artic_ptr->myPID == getpid() )         /* First on list        */
                {
                        artic_procptr[index] = artic_ptr->next;              /* next on list         */
                }
                else
                {
                        while (artic_ptr != PANULL )                      /* while not  NULL              */
                        {
                                if (artic_ptr->myPID == getpid() )
                                {
                                        last_ptr->next = artic_ptr->next;         /* close list   */
                                        break;
                                }
                                last_ptr = artic_ptr;                             /* save current pointer */
                                artic_ptr  = artic_ptr->next;               /* next link                    */
                        }
                }
        }

#ifdef DARTIC_DEBUG
        printf("remove_articproc: artic_ptr = %X\n",artic_ptr);
#endif

        return(artic_ptr);                        /* return either NULL or pointer to struct */
}


/*
 *      artic_icaissuecmd
 *
 *      This is the ioctl subfunction for Issue Command.   This function is used
 *      to send an RCM style command to an adapter task, with optional parameters.
 */

artic_icaissuecmd(device, arg)
dev_t device;             /* major/minor number             */
ICAISSUECMD_PARMS *arg;   /* pointer to the parameter block */
{
        ICAGETBUFADDRS_PARMS thebcb; /* BCB for specified task       */
        int     retval = 0;          /* return value                 */
        int     coprocnum;           /* from minor number            */
        int     numticks;            /* number of ticks for timeout  */
        uchar   psb;                 /* Primary Status Byte          */
        uchar   pcselect;            /* PC Select Byte               */

#ifdef DARTIC_DEBUG
        printf("artic_icaissuecmd: Entered.\n");
#endif

        arg->retcode = NO_ERROR;     /* preset to success            */

        /*
         *      Do Validity Checks on parameters.
         *
         *              coprocessor number must be < MAXADAPTERS and >= 0.
         *
         *              task number must be >=0 and <= artic_adapter[coprocnum].maxtask.
         */

        coprocnum = minor(device);   /* retrieve the coprocessor number  */

#ifdef DARTIC_DEBUG
        printf("artic_icaissuecmd: coprocnum = %d\n",coprocnum);
#endif

        /*
         *      Adapter must be in READY state to accept commands.
         */

        if (artic_adapter[coprocnum].state != DARTIC_READY)
        {
#ifdef DARTIC_DEBUG
                printf("artic_icaissuecmd: Adapter not ready, state = %d\n",
                          artic_adapter[coprocnum].state);
#endif

                arg->retcode = E_ICA_INVALID_COPROC;
                return(retval);
        }

        /*      Retrieve the PSB for the specified task */

        retval = artic_get_psb(coprocnum, &psb);

        if (retval)
        {
                return(retval);   /* Error occured during artic_get_psb      */
        }

#ifdef DARTIC_DEBUG
        printf("artic_icaissuecmd: artic_get_psb returned PSB = %X\n",psb);
#endif

        if (arg->length)      /* if there are parameters      */
        {

                /*       Retrieve the BCB       */

                retval = artic_get_bcb(coprocnum, &thebcb);

                if (retval)     /* failed with EINTR or EIO     */
                {
                        return(retval);
                }

                /*      if parameter length > bcb output buffer size,
                 *      return an error.
                 */

                if (arg->length > thebcb.ob.length)
                {
                        arg->retcode = E_ICA_OB_SIZE;
                        return(retval);
                }

                /*
                 *      Write parameters to output buffer
                 */

#ifdef DARTIC_DEBUG
                printf("artic_icaissuecmd: thebcb.ob.page    = %X\n",(ulong)thebcb.ob.page);
                printf("              : thebcb.ob.offset  = %X\n",(ulong)thebcb.ob.offset);
                printf("artic_icaissuecmd: thebcb.ob.length  = %X\n",(ulong)thebcb.ob.length);
#endif

                retval = artic_write_mem(coprocnum,
                                       arg->prms,
                                       thebcb.ob.page,
                                       thebcb.ob.offset,
                                       (int) arg->length);

                if (retval == E_ICA_INVALID_PAGE)
                {
                        arg->retcode=retval;
                        retval = 0 ;
                        return(retval);
                }

                if (retval)                     /* error EINTR or EIO   */
                {
                        return(retval);         /* return error         */
                }
        }

        /*      Put Command Code into the BCB   */

        retval = artic_put_cmd(coprocnum, arg->cmdcode);

        if (retval)                     /* error EINTR or EIO   */
        {
                return(retval);         /* return error         */
        }

        /*      Serialize access to the PC select byte  */

        if (retval = lockl(&pcselect_lock[coprocnum], LOCK_SIGRET))
        {
#ifdef DARTIC_DEBUG
                printf("artic_icaissuecmd: lockl failure - retval = %d.\n",
                                        retval);
#endif
                return(EINTR);          /* lockl failure due to signal */
        }

        /*      Retrieve the PC select bytes current value      */

        retval = artic_get_pcselect(coprocnum, &pcselect);

        if (retval)
        {
                unlockl(&pcselect_lock[coprocnum]);     /* unlock resource      */
                return(retval);
        }

        /*      PC Select byte must be value 0xFF or 0xFE (OK_TO_INT1 & 2)
         *      in order to allow access
         */

        if (pcselect != OK_TO_INT1 && pcselect != OK_TO_INT2)
        {
                arg->retcode    = E_ICA_BAD_PCSELECT;
                arg->reserved   = pcselect;

                unlockl(&pcselect_lock[coprocnum]);     /* unlock resource      */
                return(NO_ERROR);
        }

#ifdef DARTIC_DEBUG
        printf("artic_icaissuecmd: first check: PC Select O.K.\n");
#endif

        /*
         *      Lock access to CPUPAGE register
         */

        retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET);

        if (retval)
        {
#ifdef DARTIC_DEBUG
                printf("artic_get_pcselect: lockl failure - retval = %d.\n",
                                        retval);
#endif
                unlockl(&pcselect_lock[coprocnum]);     /* unlock resource      */
                return(EINTR);                          /* lockl failure */
        }

        /*
         *      Mask interrupts as per RCM manual instructions (Chap 4, p 67).
         */

#ifdef DARTIC_DEBUG
        printf("artic_icaissuecmd: Masking interrupts...\n");
#endif

        i_mask(&artic_intr[coprocnum]);

        /*      Store the task number 0 in the PC Select byte.  This
         *      will cause an interrupt to the adapter, and RCM will
         *      vector to the task's command handler.
         */

        retval = artic_put_pcselect(coprocnum, (uchar) 0);

        if (retval)     /* Error! EINTR or EIO during put       */
        {
                unlockl(&pcselect_lock[coprocnum]);     /* unlock resource      */
                return(retval);
        }

        /*      Interrupt The Adapter */

        intboard(coprocnum);

        /*      Unlock resources */

        unlockl(&cpupage_lock[coprocnum]);

        /*      Unmask interrupts */

#ifdef DARTIC_DEBUG
        printf("artic_icaissuecmd: Unmasking interrupts.\n");
#endif

        i_unmask(&artic_intr[coprocnum]);

        /*
         *      Check PC Select byte after every timer tick up
         *      to arg->timeout milliseconds.
         */

#ifdef DARTIC_DEBUG
        printf("artic_icaissuecmd: after intboard.\n");
#endif

        delay(1);

        retval = artic_get_pcselect(coprocnum, &pcselect);

        if (retval)
        {
                unlockl(&pcselect_lock[coprocnum]);     /* unlock resource      */
                return(retval);
        }

#ifdef DARTIC_DEBUG
        printf("artic_icaissuecmd: after artic_get_pcselect.\n");
#endif
        delay(1);

        if (pcselect != OK_TO_INT1 && pcselect != OK_TO_INT2)
        {
                if (arg->timeout == 0)
                {
                        arg->retcode = E_ICA_TIMEOUT;
                }
                else
                {
                        /* Number of ticks to wait is the timeout
                         *      value divided by the number of ms per
                         *      tick (NMS_PER_TICK).
                         */

                        numticks = arg->timeout / NMS_PER_TICK;

                        /*
                         *      Loop waiting for PC Select byte to indicate command
                         *      was accepted.
                         */

                        while (numticks > 0)
                        {
                                /* retrieve the PC Select byte */

                                retval = artic_get_pcselect(coprocnum, &pcselect);

                                if (retval)
                                {
                                        unlockl(&pcselect_lock[coprocnum]);     /* unlock resource      */
                                        return(retval);
                                }

                                /* check to see if command was accepted         */

                                if (pcselect == OK_TO_INT1 || pcselect == OK_TO_INT2)
                                {
                                        break;
                                }
                                else
                                {
                                        /* delay for 1 tick (NMS_PER_TICK milliseconds) */

                                        delay(1);
                                        numticks--;
                                }
                        }

                        if (numticks <= 0)      /* timed out! */
                        {
                                arg->retcode = E_ICA_TIMEOUT;
                        }
                        else
                        {
                                if (pcselect == OK_TO_INT1) /* cmd accepted     */
                                {
                                        arg->retcode = NO_ERROR;                /* indicate success */
                                }
                                else                                    /* command was rejected */
                                {
                                        arg->retcode = E_ICA_CMD_REJECTED;
                                }
                        }
                }
        }

        /*
         *      Unlock access to the pcselect byte
         */

        unlockl(&pcselect_lock[coprocnum]);     /* unlock resource      */

#ifdef DARTIC_DEBUG
        printf("artic_icaissuecmd: Exiting.\n");
#endif

        return (retval);
}



/*
 *      artic_icagetprimstat
 *
 *              This ioctl is used to retrieve the Primary Status
 *              Byte (PSB) of a specified task.
 */

artic_icagetprimstat(device, arg)
dev_t device;                   /* major/minor number              */
ICAGETPRIMSTAT_PARMS    *arg;   /* pointer to the parameter block  */
{
        int     retval = 0;     /* return value                 */
        int     coprocnum;      /* from minor number            */
        uchar   psb;            /* returned Primary Status Byte */

#ifdef DARTIC_DEBUG
        printf("artic_icagetprimstat: Entered.\n");
#endif

        arg->retcode = NO_ERROR;    /* preset to success */

        /*
         *      Do Validity Checks on parameters.
         *
         *              coprocessor number must be < MAXADAPTERS and >= 0.
         *              adapter state must not be DARTIC_EMPTY.
         *              task number must be >=0 and <= artic_adapter[coprocnum].maxtask.
         */

        coprocnum = minor(device);   /* retrieve the coprocessor number */

#ifdef DARTIC_DEBUG
        printf("artic_icagetprimstat: coprocnum = %d\n",coprocnum);
#endif

        /*      Verify adapter exists   */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;  /* Slot is Empty or not config */
                return(retval);
        }

        /*
         *      Call artic_get_psb to retrieve the PSB.  artic_get_psb will
         *      take care of locking access to the CPUPAGE register.
         */

        retval = artic_get_psb(coprocnum, &psb);

        arg->psb = psb;         /* put PSB into parameter block */

#ifdef DARTIC_DEBUG
        printf("artic_icagetprimstat: psb = %X Exiting.\n",psb);
#endif

        return (retval);
}




/*
 *      artic_icagetbufaddrs
 *
 *              This ioctl is used to retrieve the Buffer Control
 *              Block (BCB) of a specified task.
 */

artic_icagetbufaddrs(device, arg)
dev_t device;                   /* major/minor number              */
ICAGETBUFADDRS_PARMS    *arg;   /* pointer to the parameter block       */
{
        int     retval = 0;     /* return value         */
        int     coprocnum;      /* from minor number    */
        uchar   psb;            /* Primary Status Byte  */

#ifdef DARTIC_DEBUG
        printf("artic_icagetbufaddrs: Entered.\n");
#endif

        arg->retcode = NO_ERROR; /* preset to success */

        /*
         *      Do Validity Checks on parameters.
         *
         *              adapter must not be in DARTIC_EMPTY state.
         */

        coprocnum = minor(device);  /* retrieve the coprocessor number */


        /*      Check adapter state.    */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;
                return(retval);
        }

        /*
         *      Call artic_get_psb to retrieve the PSB.  artic_get_psb will
         *      take care of locking access to the CPUPAGE register.
         */

        retval = artic_get_psb(coprocnum, &psb);

        if (retval)
        {
                return(retval);         /* EINTR or EIO */
        }

#ifdef DARTIC_DEBUG
        printf("artic_icagetbufaddrs: psb = %X\n",psb);
#endif

        /*
         *      Get the BCB
         */

        retval = artic_get_bcb(coprocnum, arg);

#ifdef DARTIC_DEBUG
        printf("artic_icagetbufaddrs: exiting.\n");
#endif

        return (retval);
}


/*
 *      intboard
 *
 *      Sends an interrupt to the specified
 *      coprocessor adapter.
 */

void
intboard(coprocnum)
int coprocnum;
{
        int    retval;        /* return value         */
        char   *busio_addr;   /* adapter I/O address  */

#ifdef DARTIC_DEBUG
        printf("intboard: Entered. adapter %d\n",coprocnum);
#endif

        /* Retrieve a pointer to the I/O space of the adapter */

        busio_addr = BUSIO_ATT(ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                               artic_adapter[coprocnum].baseio);

        /*      Put INTBOARD value in the PTRREG to cause interrupt */

        BUSIO_PUTC(busio_addr + PTRREG, (uchar) INTBOARD);

        /*      Detach from MCA */

        BUSIO_DET(busio_addr);

#ifdef DARTIC_DEBUG
        printf("intboard: Exiting.\n");
#endif
}



/*
 *      artic_icaintreg
 *
 *      This is the ioctl subfunction for Interrupt Register.
 *      This function allocates the ProcReg structure and puts it
 *      on the linked list pointed to by the DARTIC_Proc structure for
 *      the process.
 */

artic_icaintreg(device, arg)
dev_t device;           /* major/minor number                   */
ICAINTREG_PARMS *arg;   /* pointer to the parameter block       */
{
        struct DARTIC_Proc  *procptr;    /* pointer to per-process structure   */
        struct ProcReg  *prptr;        /* ptr to Process Registration struct */
        struct ProcReg  *lastptr;      /* ptr to Process Registration struct */
        int     retval = 0;            /* return value                       */
        int     coprocnum;             /* from arg->coprocnum                */

#ifdef DARTIC_DEBUG
        printf("artic_icaintreg: Entered.\n");
#endif

        arg->retcode = NO_ERROR;                                /* preset to success                    */

        /*
         *      Do Validity Checks on parameters.
         *
         *              adapter must not be in DARTIC_EMPTY state.
         */

        coprocnum = minor(device);  /* retrieve the coprocessor number */

#ifdef DARTIC_DEBUG
        printf("artic_icaintreg: coprocnum = %d\n",coprocnum);
#endif

        /*      Verify adapter exists   */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;  /* Slot is Empty or not config */
                return(retval);
        }

        /*
         *      Retrieve pointer to this processes DARTIC_Proc struct.
         */

        procptr = lookup_articproc();

        /*      Retrieve pointer to linked list of ProcReg structures  */

        prptr = procptr->prptr[coprocnum];

#ifdef DARTIC_DEBUG
        printf("artic_icaintreg: prptr = %X\n",prptr);
#endif

        /*
         *      Check to see if we are already registered.  If not,
         *      allocate a ProcReg structure.
         */

        if (prptr == PRNULL)
        {
                /*      Allocate a ProcReg structure */

                procptr->prptr[coprocnum] = prptr =
                                        xmalloc(sizeof(struct ProcReg), 1, kernel_heap);

                if (prptr == PRNULL)
                {
                        arg->retcode = E_ICA_XMALLOC_FAIL;
                        return(NO_ERROR);
                }
        }
        else
        {
                /*  Return error if already registered */

                arg->retcode = E_ICA_ALREADY_REG;
                return(NO_ERROR);
        }

        /*      prptr points to our new ProcReg structure.
         *      Initialize it with task number, current interrupt
         *      count, and set link pointer to PRNULL.
         */

        prptr->eventcount = artic_intr_count[coprocnum];

#ifdef DARTIC_DEBUG
        printf("artic_icaintreg: artic_intr_count[%d] = %d\n",
                                                coprocnum, artic_intr_count[coprocnum]);
#endif

        prptr->next = PRNULL;

        /*      Increase the number of the kernel's callout table entries
         */

        if (! timeoutcf(1))
        {
                procptr->tocfcount++;
        }

#ifdef DARTIC_DEBUG
        printf("artic_icaintreg: exiting.\n");
#endif


        return(NO_ERROR);                               /* Success */

}


/*
 *      artic_icaintwait
 *
 *      This is the ioctl subfunction for "Interrupt Wait".  This function
 *      checks the current interrupt count versus the one stored in the
 *      processes ProcReg structure for the task.   If an interrupt has
 *      occured, then success is returned.
 */

artic_icaintwait(device, arg)
dev_t device;                   /* major/minor number              */
ICAINTWAIT_PARMS        *arg;   /* pointer to the parameter block  */
{
        struct DARTIC_Proc  *procptr;   /* pointer to per-process structure     */
        struct ProcReg  *prptr;       /* ptr to Process Registration struct   */
        int     retval = 0;           /* return value                         */
        int slpret;                   /* return value from e_sleep            */
        int     coprocnum;            /* from minor number                    */
        int     numticks;             /* number of timer ticks for timeout    */
        uchar   psb;                  /* for Primary Status Byte              */

#ifdef DARTIC_DEBUG
        printf("artic_icaintwait: Entered.\n");
#endif

        arg->retcode = NO_ERROR;      /* preset to success                    */

        /*
         *      Do Validity Checks on parameters.
         *
         *              adapter state must not be DARTIC_EMPTY
         */

        coprocnum = minor(device);             /* retrieve the coprocessor number      */

#ifdef DARTIC_DEBUG
        printf("artic_icaintwait: coprocnum = %d\n",coprocnum);
#endif

        /*      Verify adapter exists   */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;  /* Slot is Empty or not config */
                return(retval);
        }

        /*
         *      Retrieve pointer to this processes DARTIC_Proc struct.
         */

        procptr = lookup_articproc();

        /*      Retrieve pointer to linked list of ProcReg structures  */

        prptr = procptr->prptr[coprocnum];

        /*
         *      Check to see if we are already registered.  If not,
         *      allocate a ProcReg structure.
         */

        if (prptr == PRNULL)
        {
                arg->retcode = E_ICA_NOT_REG;
                return(NO_ERROR);
        }

        /*      prptr points to our ProcReg structure.
         *      If the eventcount in the ProcReg structure does not
         *      match the current interrupt count then an interrupt
         *      has occured.
         */

#ifdef DARTIC_DEBUG
        printf("artic_icaintwait: eventcount = %d artic_intr_count[%d] = %d\n",
                prptr->eventcount,coprocnum, artic_intr_count[coprocnum]);
#endif

        if (prptr->eventcount != artic_intr_count[coprocnum])
        {
                prptr->eventcount = artic_intr_count[coprocnum];  /* rearm   */

                return(NO_ERROR);                               /* Success */
        }

        /*
         *      If we made it here then we must go to sleep for the
         *      recommended timeout, waiting for either an interrupt
         *      or the timeout function to wake us up.
         */

        if (arg->timeout)
        {
                /* calculate ticks delay */

                numticks = arg->timeout / NMS_PER_TICK;

                /* have function artic_time_func called after numticks
                        clock ticks                                                             */

#ifdef DARTIC_DEBUG
                printf("artic_icaintwait: TO: coproc = %d numticks = %d\n",
                                 coprocnum, numticks);
#endif

                timeout(artic_time_func,&artic_wu_cookie[coprocnum],numticks);

                /*      Go To Sleep, wakeup from artic_time_func or
                        darticintr interrupt handler  */

                slpret = e_sleep(&artic_wu_cookie[coprocnum],EVENT_SIGRET);

#ifdef DARTIC_DEBUG
                printf("artic_icaintwait: after sleep: slpret = %d ecount = %d icount = %d\n",
                                slpret,prptr->eventcount, artic_intr_count[coprocnum]);
#endif

                if (slpret == EVENT_SUCC)
                {

                        /*      Now We are Awake.  Determine if interrupt occured */

                        if (prptr->eventcount != artic_intr_count[coprocnum])
                        {
                                prptr->eventcount =
                                                artic_intr_count[coprocnum];      /* rearm        */

                                arg->retcode = NO_ERROR;                                        /* Success      */

                                untimeout(artic_time_func,&artic_wu_cookie[coprocnum]);
                        }
                        else
                        {
                                arg->retcode = E_ICA_TIMEOUT;           /* timed out */
                        }
                }
                else    /* assume slpret == EVENT_SIG   */
                {
                        arg->retcode = E_ICA_INTR;      /* Interrupted by signal         */
                        untimeout(artic_time_func,&artic_wu_cookie[coprocnum]);
                }
        }
        else
        {
                arg->retcode = E_ICA_TIMEOUT;           /* timed out */
        }

#ifdef DARTIC_DEBUG
        printf("artic_icaintwait: Exiting, retcode = %X\n",
                  arg->retcode);
#endif
        return(NO_ERROR);
}

void
artic_time_func(wakupaddr)
int     *wakupaddr;
{
#ifdef DARTIC_DEBUG
        printf("artic_time_func: Called.\n");
#endif

        e_wakeup(wakupaddr);    /* Wake Up Sleepers */
}

/*
 *      artic_icaintdereg
 *
 *      This is the ioctl subfunction for "Interrupt Deregister".  This function
 *      deregisters the process for task interrupt notification.  This is done
 *      by removing the ProcReg structure from the linked list pointed to by
 *      the processes DARTIC_Proc structure.
 */

artic_icaintdereg(device, arg)
dev_t  device;                  /* major/minor number              */
ICAINTDEREG_PARMS       *arg;   /* pointer to the parameter block  */
{
        struct DARTIC_Proc  *procptr;   /* pointer to per-process structure     */
        struct ProcReg  *prptr;       /* ptr to Process Registration struct   */
        struct ProcReg  *lastptr;     /* ptr to Process Registration struct   */
        int     retval = 0;           /* return value                         */
        int     coprocnum;            /* minor(device)                        */
        int     numticks;             /* number of timer ticks for timeout    */
        uchar   psb;                  /* for Primary Status Byte              */

#ifdef DARTIC_DEBUG
        printf("artic_icaintdereg: Entered.\n");
#endif

        arg->retcode = NO_ERROR;      /* preset to success                    */

        /*
         *      Do Validity Checks on parameters.
         *
         *              adapter state must not be DARTIC_EMPTY.
         */

        coprocnum = minor(device);    /* retrieve the coprocessor number      */

#ifdef DARTIC_DEBUG
        printf("artic_icaintdereg: coprocnum = %d\n",coprocnum);
#endif

        /*      Verify adapter exists   */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;  /* Slot is Empty or not config */
                return(retval);
        }

        /*
         *      Retrieve pointer to this processes DARTIC_Proc struct.
         */

        procptr = lookup_articproc();

        /*      Retrieve pointer to linked list of ProcReg structures  */

        prptr = procptr->prptr[coprocnum];

        /*
         *      Check to see if we are already registered.  If not,
         *      return an error.
         */

        if (prptr == PRNULL)
        {
                arg->retcode = E_ICA_NOT_REG;
                return(NO_ERROR);
        }
        else
        {
                        /* Remove ProcReg structure from the linked list */

                procptr->prptr[coprocnum] = PRNULL;

#ifdef DARTIC_DEBUG
                printf("artic_icaintdereg: freeing prptr = %X\n",
                                        prptr);
#endif

                xmfree(prptr,kernel_heap);     /* free memory  */

                if (procptr->tocfcount)
                {
                    if (!timeoutcf(-1))     /* decrement callout table entries      */
                     {
                         procptr->tocfcount--;   /* decrement count      */
                     }
                }
        }

#ifdef DARTIC_DEBUG
        printf("artic_icaintdereg: Exiting.\n");
#endif

        return(NO_ERROR);
}

/*
 *      artic_icareadmem
 *
 *      This is the ioctl subfunction for "Read Memory".  This function
 *      reads adapter memory, and copies it to user-space.  The address
 *      format is specified by the user in the parameter block.
 */

artic_icareadmem(device, arg)
dev_t device;             /* major/minor number             */
ICAREADMEM_PARMS  *arg;   /* pointer to the parameter block */
{
        int     retval = 0;       /* return value              */
        int     coprocnum;        /* from major/minor  number  */
        ulong   b32address;       /* for address conversions   */
        ushort  rcmoffset;        /* for address conversions   */
        uchar   rcmpage;          /* for address conversions   */

#ifdef DARTIC_DEBUG
        printf("artic_icareadmem: Entered.\n");
#endif

        arg->retcode = NO_ERROR;   /* preset to success                    */

        /*
         *      Validity and range checking:
         *
         *      2.      an adapter must be present.
         */

        coprocnum = minor(device);     /* retrieve the coprocessor number      */

#ifdef DARTIC_DEBUG
        printf("artic_icareadmem: coprocnum = %d\n",coprocnum);
#endif

        /*      Verify adapter exists   */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;  /* Slot is Empty or not config */
                return(retval);
        }


        /*
         *      Create page/offset format for artic_read_mem out of whatever
         *      the user gave us.  If format not recognized, return
         *      E_ICA_INVALID_FORMAT.
         */

        switch (arg->addr_format)
        {
                /*
                 *      ADDRFMT_PAGE: address is passed in page/offset format,
                 *  therefore no conversion is necessary to use artic_read_mem.
                 */

                case ADDRFMT_PAGE:                              /* page/offset format   */
                {
#ifdef DARTIC_DEBUG
                        printf("artic_icareadmem: case ADDRFMT_PAGE\n");
                        printf("artic_icareadmem:  No Conversion necessary.\n");
#endif

                        /*
                         *      Offset must be less than window size.  Page value must
                         *      be less than maxpage.
                         */

                        if (arg->offset >= artic_adapter[coprocnum].windowsize)
                        {
                                arg->retcode = E_ICA_INVALID_OFFSET;
                        }
                        else if (arg->segpage > artic_adapter[coprocnum].maxpage)
                        {
                                arg->retcode = E_ICA_INVALID_PAGE;
                        }
                        else
                        {
                                /*       Read 'em       */

                                retval = artic_read_mem(coprocnum,         /* coprocessor number   */
                                                        arg->dest,                              /* user space buffer    */
                                                        (uchar)arg->segpage,    /* beginning page number*/
                                                        arg->offset,                    /* beginning offset     */
                                                        arg->length);                   /* amount to transfer   */

                                if (retval == E_ICA_INVALID_PAGE)
                                {
                                        arg->retcode=retval;
                                        retval = 0 ;
                                }
                        }

                        break;
                }

                /*
                 *      ADDRFMT_SEGMENT: address is passed in segment/offset format,
                 *  therefore we must convert to page/offset format.  This is done
                 *      in the following manner:
                 *
                 *              1.              Convert address to 32 bit unsigned long.
                 *              2.              Divide address by page size to get page register value.
                 *              3.              Mask out upper bits to retrieve offset size.
                 */

                case    ADDRFMT_SEGMENT:
                {
                        /* Intel segment:offset format.  Shift segment value left 4 bits
                         *      and add to offset to create a real 32bit address
                         */

                        b32address = (arg->segpage << 4) + arg->offset;

                        /*      Convert to page:offset format by integer dividing by the
                         *      window size.  Offset is created by masking off the lower
                         *      (windowsize -1) bits since windowsize is always a power of 2.
                         */

                        rcmpage         = b32address / artic_adapter[coprocnum].windowsize;
                        rcmoffset       = b32address & (artic_adapter[coprocnum].windowsize - 1);

#ifdef DARTIC_DEBUG
                        printf("artic_icareadmem: case ADDRFMT_SEGMENT\n");
                        printf("artic_icareadmem:  b32address = %X\n",
                                  b32address);
                        printf("artic_icareadmem:  rcmpage    = %X\n",
                                  (ulong)rcmpage);
                        printf("artic_icareadmem:  rcmoffset  = %X\n",
                                  (ulong)rcmoffset);
#endif
                        /*
                         *      Offset must be less than window size.  Page value must
                         *      be less than maxpage.
                         */

                        if (rcmoffset >= artic_adapter[coprocnum].windowsize)
                        {
                                arg->retcode = E_ICA_INVALID_OFFSET;
                        }
                        else if (rcmpage > artic_adapter[coprocnum].maxpage)
                        {
                                arg->retcode = E_ICA_INVALID_PAGE;
                        }
                        else
                        {
                                /*       Read 'em       */

                                retval = artic_read_mem(coprocnum, /* coprocessor number   */
                                                        arg->dest,                      /* user space buffer    */
                                                        rcmpage,                        /* beginning page number*/
                                                        rcmoffset,                      /* beginning offset     */
                                                        arg->length);           /* amount to transfer   */

                                if (retval == E_ICA_INVALID_PAGE)
                                {
                                        arg->retcode=retval;
                                        retval = 0 ;
                                }
                        }
                        break;
                }

                /*
                 *      ADDRFMT_32BIT: address is passed in 32 bit format, as two shorts.
                 *  therefore we must convert to page/offset format.  This is done
                 *      in the following manner:
                 *
                 *              1.              Convert address to 32 bit unsigned long.
                 *              2.              Divide address by page size to get page register value.
                 *              3.              Mask out upper bits to retrieve offset size.
                 */

                case    ADDRFMT_32BIT:
                {
                        /*      Create the 32bit address by shifting the segpage value left
                         *      16 bits and adding the offset.
                         */

                        b32address = (arg->segpage << 16) + arg->offset;

                        /*      Convert to page:offset format by integer dividing by the
                         *      window size.  Offset is created by masking off the lower
                         *      (windowsize -1) bits since windowsize is always a power of 2.
                         */

                        rcmpage         = b32address / artic_adapter[coprocnum].windowsize;
                        rcmoffset       = b32address & (artic_adapter[coprocnum].windowsize - 1);

#ifdef DARTIC_DEBUG
                        printf("artic_icareadmem: case ADDRFMT_32BIT\n");
                        printf("artic_icareadmem:  b32address = %X\n",
                                  b32address);
                        printf("artic_icareadmem:  rcmpage    = %X\n",
                                  (ulong)rcmpage);
                        printf("artic_icareadmem:  rcmoffset  = %X\n",
                                  (ulong)rcmoffset);
#endif

                        /*
                         *      Offset must be less than window size.  Page value must
                         *      be less than maxpage.
                         */

                        if (rcmoffset >= artic_adapter[coprocnum].windowsize)
                        {
                                arg->retcode = E_ICA_INVALID_OFFSET;
                        }
                        else if (rcmpage > artic_adapter[coprocnum].maxpage)
                        {
                                arg->retcode = E_ICA_INVALID_PAGE;
                        }
                        else
                        {
                                /*       Read 'em       */

                                retval = artic_read_mem(coprocnum, /* coprocessor number   */
                                                        arg->dest,                      /* user space buffer    */
                                                        rcmpage,                        /* beginning page number*/
                                                        rcmoffset,                      /* beginning offset     */
                                                        arg->length);           /* amount to transfer   */

                                if (retval == E_ICA_INVALID_PAGE)
                                {
                                        arg->retcode=retval;
                                        retval = 0 ;
                                }
                        }

                        break;
                }

                /*
                 * Invalid address format identifier (arg->addr_format)
                 */

                default:
                {
                        arg->retcode = E_ICA_INVALID_FORMAT;
                        return(retval);
                }
        }

#ifdef DARTIC_DEBUG
        printf("artic_icareadmem: Exiting.\n");
#endif

        return(retval);
}



/*
 *      artic_icawritemem
 *
 *      This is the ioctl subfunction for "Read Memory".  This function
 *      reads user memory space, and copies it to adapter memory.  The
 *      address format is specified by the user in the parameter block.
 */

artic_icawritemem(device, arg)
dev_t device;             /* major/minor number              */
ICAWRITEMEM_PARMS *arg;   /* pointer to the parameter block  */
{
        int     retval = 0;      /* return value             */
        int     coprocnum;       /* from major/minor(number) */
        ulong   b32address;      /* for address conversions  */
        ushort  rcmoffset;       /* for address conversions  */
        uchar   rcmpage;         /* for address conversions  */

#ifdef DARTIC_DEBUG
        printf("artic_icawritemem: Entered.\n");
#endif

        arg->retcode = NO_ERROR;                                /* preset to success                    */

        /*
         *      Validity and range checking:
         *
         *           an adapter must be present.
         */

        coprocnum = minor(device);             /* retrieve the coprocessor number      */

#ifdef DARTIC_DEBUG
        printf("artic_icawritemem: coprocnum = %d\n",coprocnum);
#endif

        /*      Verify adapter exists   */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;  /* Slot is Empty or not config */
                return(retval);
        }

        /*
         *      Create page/offset format for artic_write_mem out of whatever
         *      the user gave us.  If format not recognized, return
         *      E_ICA_INVALID_FORMAT.
         */

        switch (arg->addr_format)
        {
                /*
                 *      ADDRFMT_PAGE: address is passed in page/offset format,
                 *  therefore no conversion is necessary to use artic_write_mem.
                 */

                case    ADDRFMT_PAGE:                           /* page/offset format   */
                {
#ifdef DARTIC_DEBUG
                        printf("artic_icawritemem: case ADDRFMT_PAGE\n");
                        printf("artic_icawritemem:  No Conversion necessary.\n");
#endif

                        /*
                         *      Offset must be less than window size.  Page value must
                         *      be less than maxpage.
                         */

                        if (arg->offset >= artic_adapter[coprocnum].windowsize)
                        {
                                arg->retcode = E_ICA_INVALID_OFFSET;
                        }
                        else if (arg->segpage > artic_adapter[coprocnum].maxpage)
                        {
                                arg->retcode = E_ICA_INVALID_PAGE;
                        }
                        else
                        {
                                /*      Write 'em       */

                                retval = artic_write_mem(coprocnum,      /* coprocessor number   */
                                                       arg->source,    /* buffer in user-space */
                                                       (uchar)arg->segpage,/* beginning page number*/
                                                       arg->offset,    /* beginning offset             */
                                                       arg->length);   /* amount to transfer   */

                                if (retval == E_ICA_INVALID_PAGE)
                                {
                                        arg->retcode=retval;
                                        retval = 0 ;
                                }
                        }
                        break;
                }

                /*
                 *      ADDRFMT_SEGMENT: address is passed in segment/offset format,
                 *  therefore we must convert to page/offset format.  This is done
                 *      in the following manner:
                 *
                 *              1.              Convert address to 32 bit unsigned long.
                 *              2.              Divide address by page size to get page register value.
                 *              3.              Mask out upper bits to retrieve offset size.
                 */

                case    ADDRFMT_SEGMENT:
                {
                        /* Intel segment:offset format.  Shift segment value left 4 bits
                         *      and add to offset to create a real 32bit address
                         */

                        b32address = (arg->segpage << 4) + arg->offset;

                        /*      Convert to page:offset format by integer dividing by the
                         *      window size.  Offset is created by masking off the lower
                         *      (windowsize -1) bits since windowsize is always a power of 2.
                         */

                        rcmpage         = b32address / artic_adapter[coprocnum].windowsize;
                        rcmoffset       = b32address & (artic_adapter[coprocnum].windowsize - 1);

#ifdef DARTIC_DEBUG
                        printf("artic_icawritemem: case ADDRFMT_SEGMENT\n");
                        printf("artic_icawritemem:  b32address = %X\n",
                                  b32address);
                        printf("artic_icawritemem:  rcmpage    = %X\n",
                                  (ulong)rcmpage);
                        printf("artic_icawritemem:  rcmoffset  = %X\n",
                                  (ulong)rcmoffset);
                        printf("artic_icawritemem:  maxpage    = %X\n",
                                   artic_adapter[coprocnum].maxpage);
                        printf("artic_icawritemem:  windowsize = %X\n",
                                   artic_adapter[coprocnum].windowsize);
#endif

                        /*
                         *      offset must be less than window size, page must be less
                         *      than the maximum page value
                         */

                        if (rcmoffset >= artic_adapter[coprocnum].windowsize)
                        {
                                arg->retcode = E_ICA_INVALID_OFFSET;
                        }
                        else if (rcmpage > artic_adapter[coprocnum].maxpage)
                        {
                                arg->retcode = E_ICA_INVALID_PAGE;
                        }
                        else
                        {
                                /*      Write 'em       */

                                retval = artic_write_mem(coprocnum,    /* coprocessor number   */
                                                       arg->source,  /* buffer in user-space */
                                                       rcmpage,      /* beginning page number*/
                                                       rcmoffset,    /* beginning offset             */
                                                       arg->length); /* amount to transfer   */

                                if (retval == E_ICA_INVALID_PAGE)
                                {
                                        arg->retcode=retval;
                                        retval = 0 ;
                                }
                        }
                        break;
                }

                /*
                 *      ADDRFMT_32BIT: address is passed in 32 bit format, as two shorts.
                 *  therefore we must convert to page/offset format.  This is done
                 *      in the following manner:
                 *
                 *              1.              Convert address to 32 bit unsigned long.
                 *              2.              Divide address by page size to get page register value.
                 *              3.              Mask out upper bits to retrieve offset size.
                 */

                case    ADDRFMT_32BIT:
                {
                        /* Intel segment:offset format.  Shift segment value left 4 bits
                         *      and add to offset to create a real 32bit address
                         */

                        b32address = (arg->segpage << 16) + arg->offset;

                        /*      Convert to page:offset format by integer dividing by the
                         *      window size.  Offset is created by masking off the lower
                         *      (windowsize -1) bits since windowsize is always a power of 2.
                         */

                        rcmpage         = b32address / artic_adapter[coprocnum].windowsize;
                        rcmoffset       = b32address & (artic_adapter[coprocnum].windowsize - 1);

#ifdef DARTIC_DEBUG
                        printf("artic_icawritemem: case ADDRFMT_32BIT\n");
                        printf("artic_icawritemem:  b32address = %X\n",
                                  b32address);
                        printf("artic_icawritemem:  rcmpage    = %X\n",
                                  (ulong)rcmpage);
                        printf("artic_icawritemem:  rcmoffset  = %X\n",
                                  (ulong)rcmoffset);
#endif

                        /*
                         *      offset must be less than window size, page must be less
                         *      than the maximum page value
                         */

                        if (rcmoffset >= artic_adapter[coprocnum].windowsize)
                        {
                                arg->retcode = E_ICA_INVALID_OFFSET;
                        }
                        else if (rcmpage > artic_adapter[coprocnum].maxpage)
                        {
                                arg->retcode = E_ICA_INVALID_PAGE;
                        }
                        else
                        {
                                /*      Write 'em       */

                                retval = artic_write_mem(coprocnum,        /* coprocessor number   */
                                                                arg->source,            /* buffer in user-space */
                                                                rcmpage,                        /* beginning page number*/
                                                                rcmoffset,                      /* beginning offset             */
                                                                arg->length);           /* amount to transfer   */

                                if (retval == E_ICA_INVALID_PAGE)
                                {
                                        arg->retcode=retval;
                                        retval = 0 ;
                                }
                        }
                        break;
                }

                /*
                 * Invalid address format identifier (arg->addr_format)
                 */

                default:
                {
                        arg->retcode = E_ICA_INVALID_FORMAT;
                        return(retval);
                }
        }

#ifdef DARTIC_DEBUG
        printf("artic_icawritemem: Exiting.\n");
#endif

        return(retval);
}


/*
 *      boardreset
 *
 *      This routine does a hardware reset of the coprocessor
 *  board.  It is called by ioctl function artic_icareset.
 */


boardreset(coprocnum)
int     coprocnum;
{
        int      retval, i;
        char    *busio_addr;
        uchar   comregvalue;
        uchar   initreg1;
        uchar   cc_enable;

        /* Lock access to CPUPAGE register */

        retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET);

        if (retval)
        {
#ifdef DARTIC_DEBUG
                printf("boardreset: lockl failure - retval = %d.\n",
                                        retval);
#endif
                return(EINTR);                          /* lockl failure */
        }

        /*
         *      Set up a jumpbuffer for exceptions.
         */

        if (retval = setjmpx(&artic_jumpbuffers[coprocnum]))
        {
#ifdef DARTIC_DEBUG
                printf("boardreset: setjmpx returned %d\n",retval);
#endif
                unlockl(&cpupage_lock[coprocnum]);              /* release lock word    */

                return(EIO);                                                    /* return error code    */
        }

        /*      Attach to MicroChannel Bus      */

        busio_addr = BUSIO_ATT(ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                               artic_adapter[coprocnum].baseio);

        artic_adapter[coprocnum].state = DARTIC_NOTREADY;      /* mark adapter as not ready */

        /*
         *      Reprogram the POS registers in case the adapter is really
         *      messed up.
         */

        reprogramposreg(coprocnum);

        /*
         *      If the adapter is type MULTIPORT_2 or X25_ADAPTER, then
         *      degate the adapter RAM before issuing reset.  After POST
         *      finishes, re-enable the RAM.  This is necessary because of a
         *      bug in the POST code of these adapters.  The bug may allow
         *      the adapter to access memory outside its domain.  This code
         *      added at the request of David Carew, IBM Boca Raton.
         */

        /*
         *      Read initreg1 where the PROMREADY bit lives.  Then bitwise AND
         *      the value with the one's complement of PROMREADY to reset the
         *      bit and write it back out.  This is so we can recheck the bit
         *      after the hardware reset.
         */

        initreg1 = readdreg(coprocnum,INITREG1);                /* read INITREG1 */

#ifdef DARTIC_DEBUG
        printf("boardreset: readdreg(%X,%X) returns %X\n",
                                coprocnum,INITREG1,initreg1);
#endif

        initreg1 &= NOTPROMREADY;                               /* reset PROMREADY bit  */

        writedreg(coprocnum,INITREG1,initreg1); /* write INITREG1               */

        /*
         *      Do the hardware reset by setting COMREG to CR_RESETVAL (0x11),
         *      then 00, then to INTENABLE (0x10) to enable interrupts.
         */

        BUSIO_PUTC((busio_addr + COMREG),(uchar) CR_RESETVAL);
        delay(1);
        BUSIO_PUTC((busio_addr + COMREG),(uchar) 0);
        delay(1);
        BUSIO_PUTC((busio_addr + COMREG),(uchar) INTENABLE);
        delay(1);


        /*
         *      Now loop with sleep waiting for the board to reset.
         */

        retval = -1;

        for  (i = 0 ; i < 30 ; i++)
        {
                if ((artic_adapter[coprocnum].state = isbrdready(coprocnum)) == DARTIC_READY)
                {
                        retval = 0;
                        break;
                }
                else
                {
                        /*
                         *      sleep for 1 second       (delay in units of 1/HZ seconds)
                         */
                        delay(HZ);
#ifdef DARTIC_DEBUG
                        printf("boardreset: iteration %d....\n",i);
#endif
                }
        }

        /* Enable synchronous channel check for Portmasters and Sandpipers */
        if((artic_adapter[coprocnum].basetype == PORTMASTER) ||
           (artic_adapter[coprocnum].basetype == SP5_ADAPTER))
        {
           BUSIO_PUTC(busio_addr + PTRREG, PCPAR2);
           cc_enable = BUSIO_GETC(busio_addr + DREG);
           cc_enable |= 0x60;
           BUSIO_PUTC(busio_addr + DREG, cc_enable);
        }

        BUSIO_DET(busio_addr);                  /* detach from bus         */

        /* If we're a Sandpiper we need to reenable data parity            */
        /* (It's cleared on reset).                                        */
        if (artic_adapter[coprocnum].basetype == SP5_ADAPTER)
        {
           busio_addr = IOCC_ATT(ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                                 SLOTADDR(artic_adapter[coprocnum].slot));
           BUSIO_PUTC(busio_addr + 5,artic_adapter[coprocnum].pos5);
           IOCC_DET(busio_addr);                   /* Detach from IOCC        */
        }

        clrjmpx(&artic_jumpbuffers[coprocnum]); /* clear exception stack   */

        unlockl(&cpupage_lock[coprocnum]);      /* unlock cpupage lock     */

#ifdef DARTIC_DEBUG
        printf("boardreset: iteration %d: brdready(%d) returned %d\n",
                  i,coprocnum,artic_adapter[coprocnum].state);
#endif

        return(retval);
}


/*
 *      isbrdready
 *
 *              This routine determines if the board is ready by checking the
 *      PROM READY bit in the INITREG1 register (on the board).  If this
 *      bit is set, then the board PROM completed its initialization and POST.
 *
 *              We check this bit by first programming the PTRREG, and then reading
 *      INITREG1 from the DREG register.
 */

isbrdready(coprocnum)
int coprocnum;
{
        uchar    initreg1;

        /*
         *      Call readdreg to read INITREG1 from DREG after programming PTRREG.
         */

        initreg1 = readdreg(coprocnum,INITREG1);

#ifdef DARTIC_DEBUG
        printf("isbrdready: readdreg(%X,%X) returns %X\n",
                  coprocnum,INITREG1,initreg1);
#endif

        if (initreg1 & PROMREADY)
        {
                return(DARTIC_READY);               /* PROM completed initialization and POST */
        }
        else
        {
                return(DARTIC_NOTREADY);            /* PROMREADY bit not set!                       */
        }
}



/*
 *      artic_icareset
 *
 *      This is the ioctl subfunction for "Reset".  This function issues
 *      a hardware reset to the specified coprocessor adapter.
 */

artic_icareset(device, arg)
dev_t device;
ICARESET_PARMS  *arg;   /* pointer to the parameter block       */
{
        int     retval;    /* return value         */
        int     coprocnum; /* from arg->coprocnum  */

#ifdef DARTIC_DEBUG
        printf("artic_icareset: Entered.\n");
#endif

        arg->retcode = NO_ERROR;     /* preset to success */

        coprocnum = minor(device);   /* retrieve the coprocessor number */

#ifdef DARTIC_DEBUG
        printf("artic_icareset: coprocnum = %d\n",coprocnum);
#endif

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY )
        {
                arg->retcode = E_ICA_INVALID_COPROC;                    /* out of Range */
                return(retval);
        }

#ifdef DARTIC_DEBUG
        printf("artic_icareset: Calling boardreset(%d).\n",
                                        coprocnum);
#endif

        retval = boardreset(coprocnum);

#ifdef DARTIC_DEBUG
        printf("artic_icareset: Exiting.  retval = %d\n",retval);
#endif

        if (retval)
        {
                arg->retcode = E_ICA_TIMEOUT;                                   /* could not reset */
                artic_adapter[coprocnum].state = DARTIC_BROKEN;        /* say board is broken  */
        }

        return(NO_ERROR);
}




/*
 *      artic_icasendconfig
 *
 *      This is the ioctl subfunction for "Send Configuration Parameters".
 *      This function copies the RCM configuration parameters from the
 *      parameter block to the adapter table, and then to the Interface Block
 *      for RCM.  NOTE that this function uses the ICAGETPARMS_PARMS
 *      parameter block.
 */

artic_icasendconfig(device, arg)
dev_t device;             /* major/minor number              */
ICAGETPARMS_PARMS *arg;   /* pointer to the parameter block  */
{
        int   retval = 0;      /* for return values                */
        int   coprocnum;       /* from major/minor number          */
        char  *dpram;          /*  pointer to adapter memory       */
        uchar lastpage;        /* used to restore CPUPAGE register */

#ifdef DARTIC_DEBUG
        printf("artic_icasendconfig: Entered.\n");
#endif

        arg->retcode = NO_ERROR;   /* preset to success */

        /*
         *              coprocessor number must be < MAXADAPTERS and >= 0.
         */

        coprocnum = minor(device);    /* retrieve the coprocessor number */

#ifdef DARTIC_DEBUG
        printf("artic_icasendconfig: coprocnum = %d\n",coprocnum);
#endif

        if (artic_adapter[coprocnum].state !=  DARTIC_READY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;
                return(NO_ERROR);
        }

        /*
         *      Copy parameters from the parameter block to the
         *      adapter table.
         */

        artic_adapter[coprocnum].maxpri    = (int) arg->cfgparms.maxpri;
        artic_adapter[coprocnum].maxtimer  = (int) arg->cfgparms.maxtime;
        artic_adapter[coprocnum].maxqueue  = (int) arg->cfgparms.maxqueue;
        artic_adapter[coprocnum].maxtask   = (int) arg->cfgparms.maxtask;

        /* Lock access to CPUPAGE register */

        retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET);

        if (retval)
        {
#ifdef DARTIC_DEBUG
                printf("artic_icasendconfig: lockl failure - retval = %d.\n",
                                        retval);
#endif
                return(EINTR);                          /* lockl failure */
        }

        /*      Set the CPUPAGE so that the IB is revealed */

        lastpage = setCPUpage(coprocnum,(uchar)0);

        /*      Get a pointer to the Interface Block */

        dpram = BUSMEM_ATT(ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                           artic_adapter[coprocnum].basemem + IBADDR);

        /*      Copy to Interface Block */

#ifdef DARTIC_DEBUG
        printf("artic_icasendconfig: maxpri   = 0x%X\n",
                                                     arg->cfgparms.maxpri);
        printf("artic_icasendconfig: maxtask  = 0x%X\n",
                                                     arg->cfgparms.maxtask);
        printf("artic_icasendconfig: maxtime  = 0x%X\n",
                                                     arg->cfgparms.maxtime);
        printf("artic_icasendconfig: maxqueue = 0x%X\n",
                                                     arg->cfgparms.maxqueue);
#endif

        ibMAXPRI(dpram)   = arg->cfgparms.maxpri;
        ibMAXTASK(dpram)  = arg->cfgparms.maxtask;
        ibMAXTIME(dpram)  = arg->cfgparms.maxtime;
        ibMAXQUEUE(dpram) = arg->cfgparms.maxqueue;

        BUSMEM_DET(dpram);   /* detach from MC       */

        /*      Restore the CPUPAGE register */

        setCPUpage(coprocnum,lastpage);

        /*      Unlock the CPUPAGE register lock        */

        unlockl(&cpupage_lock[coprocnum]);

#ifdef DARTIC_DEBUG
        printf("artic_icasendconfig: exiting.\n");
#endif

        return(NO_ERROR);
}

/*
 *      artic_icagetparms
 *
 *      This is the ioctl subfunction for "Get Configuration Parameters".
 *      This function copies the RCM configuration parameters from the
 *      adapter table to the parameter block.
 */

artic_icagetparms(device, arg)
dev_t device;             /* major/minor number             */
ICAGETPARMS_PARMS *arg;   /* pointer to the parameter block */
{
        int retval = 0;   /* for return values               */
        int coprocnum;    /* from major/minor number         */
        char *dpram;      /*      pointer to adapter memory  */

#ifdef DARTIC_DEBUG
        printf("artic_icagetparms: Entered.\n");
#endif

        arg->retcode = NO_ERROR;   /* preset to success */

        /*
         *              coprocessor number must be < MAXADAPTERS and >= 0.
         */

        coprocnum = minor(device);  /* retrieve the coprocessor number      */

#ifdef DARTIC_DEBUG
        printf("artic_icagetparms: coprocnum = %d\n",coprocnum);
#endif

        /*      Check adapter state.    */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;
                return(NO_ERROR);
        }

        /*
         *      Copy parameters from the parameter block to the
         *      adapter table.
         */

        arg->cfgparms.maxpri    =  (uchar) artic_adapter[coprocnum].maxpri;
        arg->cfgparms.maxtime   =  (uchar) artic_adapter[coprocnum].maxtimer;
        arg->cfgparms.maxqueue  =  (uchar) artic_adapter[coprocnum].maxqueue;
        arg->cfgparms.maxtask   =  (uchar) artic_adapter[coprocnum].maxtask;
        arg->cfgparms.int_level =  (uchar) artic_adapter[coprocnum].intlevel;
        arg->cfgparms.io_addr   =  (ushort)artic_adapter[coprocnum].baseio;

        switch(artic_adapter[coprocnum].windowsize)
        {
                case 0x2000:
                {
                        arg->cfgparms.ssw_size  =       (uchar) 0;
                        break;
                }
                case 0x4000:
                {
                        arg->cfgparms.ssw_size  =       (uchar) 1;
                        break;
                }
                case 0x8000:
                {
                        arg->cfgparms.ssw_size  =       (uchar) 2;
                        break;
                }
                case 0x10000:
                {
                        arg->cfgparms.ssw_size  =       (uchar) 3;
                        break;
                }
                default:
                {
                        arg->cfgparms.ssw_size  =       -1;             /* Error! */
                        break;
                }
        }

#ifdef DARTIC_DEBUG
        printf("artic_icagetparms: exiting.\n");
#endif

        return(NO_ERROR);
}

/*
 *      artic_icagetadaptype
 *
 *      This is the ioctl subfunction for "Get Adapter type".
 *      This function copies the type received from the config method and
 *      stored in the adapter table to the parameter block.
 */

artic_icagetadaptype(device, arg)
dev_t device;                /* major/minor number             */
ICAGETADAPTYPE_PARMS *arg;   /* pointer to the parameter block */
{
        int retval = 0;   /* for return values               */
        int coprocnum;    /* from major/minor number         */

#ifdef DARTIC_DEBUG
        printf("artic_icagetadaptype: Entered.\n");
#endif

        arg->retcode = NO_ERROR;   /* preset to success */

        /*
         *              coprocessor number must be < MAXADAPTERS and >= 0.
         */

        coprocnum = minor(device);  /* retrieve the coprocessor number      */

#ifdef DARTIC_DEBUG
        printf("artic_icagetadaptype: coprocnum = %d\n",coprocnum);
#endif

        /*      Check adapter state.    */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;
                return(NO_ERROR);
        }

        /*
         *      Copy type to the parameter block from the
         *      adapter table.
         */

        arg->type =  artic_adapter[coprocnum].adaptertype;


#ifdef DARTIC_DEBUG
        printf("artic_icagetadaptype: exiting.\n");
#endif

        return(NO_ERROR);
}

/*
 *      artic_icaiowrite:
 *
 *      This function is used to write to I/O ports on the coprocessor adapter
 */

artic_icaiowrite(device, arg)
dev_t device;                 /* major/minor number              */
ICAIOWRITE_PARMS      *arg;   /* pointer to the parameter block  */
{
        char    *busio_addr;        /* used for BUSIO_ATT      */
        int      retval = 0;        /* return value            */
        int      coprocnum;         /* from major/minor number */

#ifdef DARTIC_DEBUG
        printf("oa_icaiowrite: Entered.\n");
#endif

        arg->retcode = NO_ERROR;   /* preset to success */

        /*
         *      Validity and range checking:
         *
         *      1.      an adapter must be present.
         */

        coprocnum = minor(device);  /* retrieve the coprocessor number  */

#ifdef DARTIC_DEBUG
        printf("artic_icaiowrite: coprocnum = %d\n",coprocnum);
        printf("              : portnum  = %d value = %d\n",
                 (int)arg->portnum,(int)arg->value);
#endif

        /*      Verify adapter exists   */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;
                return(retval);
        }

        /* Lock access to CPUPAGE register */

        retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET);

        if (retval)
        {
#ifdef DARTIC_DEBUG
                printf("artic_icaiowrite: lockl failure - retval = %d.\n",
                                        retval);
#endif
                return(EINTR);                          /* lockl failure */
        }

        /*
         *      Set up a jumpbuffer for exceptions.
         */

        if (retval = setjmpx(&artic_jumpbuffers[coprocnum]))
        {
#ifdef DARTIC_DEBUG
                printf("artic_icaiowrite: setjmpx returned %d\n",retval);
#endif
                unlockl(&cpupage_lock[coprocnum]);  /* release lock word    */

                return(EIO);                                                            /* return error code    */
        }

        /*      Attach to MicroChannel Bus      */

        busio_addr = BUSIO_ATT(ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                               artic_adapter[coprocnum].baseio);

        /*      Write value to port                     */

        BUSIO_PUTC(busio_addr + arg->portnum, arg->value);

        /*      Detach from MicroChannel Bus */

        BUSIO_DET(busio_addr);

        clrjmpx(&artic_jumpbuffers[coprocnum]);    /* clear exception stack */

        unlockl(&cpupage_lock[coprocnum]);       /* unlock cpupage lock  */

        return(retval);                          /* return no error      */
}

/*
 *      artic_icaioread:
 *
 *      This function is used to read from I/O ports on the coprocessor adapter
 */

artic_icaioread(device, arg)
dev_t device;                 /* major/minor number              */
ICAIOREAD_PARMS *arg;         /* pointer to the parameter block  */
{
        char    *busio_addr;    /* used for BUSIO_ATT  */
        int      retval = 0;    /* return value        */
        int      coprocnum;     /* from arg->coprocnum */

#ifdef DARTIC_DEBUG
        printf("artic_icaioread: Entered.\n");
#endif

        arg->retcode = NO_ERROR;        /* preset to success */

        /*
         *      Validity and range checking:
         *
         *      1.      an adapter must be present.
         */

        coprocnum = minor(device); /* retrieve the coprocessor number */

#ifdef DARTIC_DEBUG
        printf("artic_icaioread: coprocnum = %d\n",coprocnum);
        printf("             : portnum  = %d\n", arg->portnum);
#endif

        /*      Verify adapter exists   */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;
                return(retval);
        }

        /* Lock access to CPUPAGE register */

        retval = lockl(&cpupage_lock[coprocnum], LOCK_SIGRET);

        if (retval)
        {
#ifdef DARTIC_DEBUG
                printf("artic_icaioread: lockl failure - retval = %d.\n",
                                        retval);
#endif
                return(EINTR);                          /* lockl failure */
        }

        /*
         *      Set up a jumpbuffer for exceptions.
         */

        if (retval = setjmpx(&artic_jumpbuffers[coprocnum]))
        {
#ifdef DARTIC_DEBUG
                printf("artic_icaioread: setjmpx returned %d\n",retval);
#endif
                unlockl(&cpupage_lock[coprocnum]); /* release lock word    */

                return(EIO);                                                            /* return error code    */
        }

        /*      Attach to MicroChannel Bus      */

        busio_addr = BUSIO_ATT(ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                               artic_adapter[coprocnum].baseio);

        /*      Read value from port    */

        arg->value = BUSIO_GETC(busio_addr + arg->portnum);

        /*      Detach from MicroChannel Bus */

        BUSIO_DET(busio_addr);

        clrjmpx(&artic_jumpbuffers[coprocnum]);  /* clear exception stack */

        unlockl(&cpupage_lock[coprocnum]);    /* unlock cpupage lock   */

        return(retval);                       /* return no error       */
}

/*
 *      artic_icaposread:
 *
 *      This function is used to read from the adapter's POS registers
 */

artic_icaposread(device, arg)
dev_t device;                 /* major/minor number              */
ICAPOSREAD_PARMS *arg;        /* pointer to the parameter block  */
{
        char    *busio_addr;    /* used for BUSIO_ATT  */
        int      retval = 0;    /* return value        */
        int      coprocnum;     /* from arg->coprocnum */

#ifdef DARTIC_DEBUG
        printf("artic_icaposread: Entered.\n");
#endif

        arg->retcode = NO_ERROR;        /* preset to success */

        /*
         *      Validity and range checking:
         *
         *      1.      an adapter must be present.
         */

        coprocnum = minor(device); /* retrieve the coprocessor number */

#ifdef DARTIC_DEBUG
        printf("artic_icaposread: coprocnum = %d\n",coprocnum);
        printf("             : regnum = %d\n", arg->regnum);
#endif

        /*      Verify adapter exists   */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;
                return(retval);
        }

        /*      Verify POS register is valid   */

        if ((arg->regnum < 0) || (arg->regnum > 5))
        {
                arg->retcode = E_ICA_INVALID_POSREG;
                return(retval);
        }

        /*
         *      Set up a jumpbuffer for exceptions.
         */

        if (retval = setjmpx(&artic_jumpbuffers[coprocnum]))
        {
#ifdef DARTIC_DEBUG
                printf("artic_icaposread: setjmpx returned %d\n",retval);
#endif
                return(EIO);                                                            /* return error code    */
        }

        /*      Attach to IOCC      */

        busio_addr = IOCC_ATT(ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                              SLOTADDR(artic_adapter[coprocnum].slot));

        /*      Read value from POS register    */

        arg->value = BUSIO_GETC(busio_addr + arg->regnum);

        /*      Detach from IOCC */

        IOCC_DET(busio_addr);

        clrjmpx(&artic_jumpbuffers[coprocnum]);  /* clear exception stack */

        return(retval);                       /* return no error       */
}

/*
 *      artic_icaposwrite:
 *
 *      This function is used to write to the adapter's POS registers
 */

artic_icaposwrite(device, arg)
dev_t device;                 /* major/minor number              */
ICAPOSWRITE_PARMS *arg;       /* pointer to the parameter block  */
{
        char    *busio_addr;    /* used for BUSIO_ATT  */
        int      retval = 0;    /* return value        */
        int      coprocnum;     /* from arg->coprocnum */

#ifdef DARTIC_DEBUG
        printf("artic_icaposwrite: Entered.\n");
#endif

        arg->retcode = NO_ERROR;        /* preset to success */

        /*
         *      Validity and range checking:
         *
         *      1.      an adapter must be present.
         */

        coprocnum = minor(device); /* retrieve the coprocessor number */

#ifdef DARTIC_DEBUG
        printf("artic_icaposwrite: coprocnum = %d\n",coprocnum);
        printf("              : regnum  = %d value = %d\n",
               arg->regnum,(int)arg->value);
#endif

        /*      Verify adapter exists   */

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY)
        {
                arg->retcode = E_ICA_INVALID_COPROC;
                return(retval);
        }

        /*      Verify POS register is valid   */

        if ((arg->regnum < 2) || (arg->regnum > 5))
        {
                arg->retcode = E_ICA_INVALID_POSREG;
                return(retval);
        }

        /*
         *      Set up a jumpbuffer for exceptions.
         */

        if (retval = setjmpx(&artic_jumpbuffers[coprocnum]))
        {
#ifdef DARTIC_DEBUG
                printf("artic_icaposread: setjmpx returned %d\n",retval);
#endif
                return(EIO);                                                            /* return error code    */
        }

        /*      Attach to IOCC      */

        busio_addr = IOCC_ATT(ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                              SLOTADDR(artic_adapter[coprocnum].slot));

        /*      Write new value to POS register    */

        BUSIO_PUTC(busio_addr + arg->regnum, arg->value);

        /*      Detach from IOCC */

        IOCC_DET(busio_addr);

        clrjmpx(&artic_jumpbuffers[coprocnum]);  /* clear exception stack */

        return(retval);                       /* return no error       */
}


/*
 *      artic_icadmasetup
 *
 */

artic_icadmasetup(device, arg)
dev_t device;             /* major/minor number             */
ICADMASETUP_PARMS *arg;   /* pointer to the parameter block */
{
        int     retval = 0;      /* return value                     */
        int     rc;              /* return code from  function calls */
        int     coprocnum;       /* from arg->coprocnum              */
        struct xmem *xmemptr;    /* Ptr to cross mem descriptors     */
        int memory_pinned = 0;   /* Flag to indicate pinned memory   */
        int dma_initialized = 0; /* Flag to indicate d_init called   */

#ifdef DARTIC_DEBUG
        printf("artic_icadmasetup: Entered.\n");
#endif

        arg->retcode = NO_ERROR; /* preset to success        */

        /*
         *      Do Validity Checks on parameters.
         *
         *              coprocessor number must be < MAXADAPTERS and >= 0.
         */

        coprocnum = minor(device);    /* retrieve the coprocessor number  */

#ifdef DARTIC_DEBUG
        printf("artic_icadmasetup: coprocnum = %d\n",coprocnum);
#endif

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY )
        {
                arg->retcode = E_ICA_INVALID_COPROC;                    /* out of Range */
                return(retval);
        }

        /* Make sure DMA not already setup on this channel */
        if (artic_dma[coprocnum].channel_id != DMA_FAIL)
        {
                arg->retcode = E_ICA_DMASETUP;
                return(retval);
        }

        /*
         *      Set up a jumpbuffer for exceptions.
         */

        if (retval = setjmpx(&artic_jumpbuffers[coprocnum]))
        {
#ifdef DARTIC_DEBUG
                printf("artic_icadmasetup: setjmpx returned %d\n",retval);
#endif
                if (memory_pinned)
                   unpinu(arg->uaddr, arg->count, UIO_USERSPACE);
                if (dma_initialized)
                {
                  d_clear(artic_dma[coprocnum].channel_id);
                  artic_dma[coprocnum].channel_id = DMA_FAIL;
                }
                return(EIO);                                                            /* return error code    */
        }

        /*
         * Do a cross memory attach if DMA involves
         * system unit memory
         */

        artic_dma[coprocnum].dp.aspace_id = XMEM_INVAL;
        if (arg->type == ADAPTER_TO_SU || arg->type == SU_TO_ADAPTER)
        {
          /*
           * Pin the  memory if DMA involves
           * system unit memory
           */
#ifdef DARTIC_DEBUG
           printf("artic_icadmasetup: Calling pinu\n");
#endif
           if (rc = pinu(arg->uaddr, arg->count, UIO_USERSPACE))
           {
              if (rc == ENOMEM)
                 arg->retcode = E_ICA_NOMEM;
              else
                 arg->retcode = E_ICA_DMAPARMS;
#ifdef DARTIC_DEBUG
              printf("artic_icadmarel: pinu returned %d\n",rc);
#endif
              return(retval);
           }
           else
             ++memory_pinned;

          /*
           * Do a cross memory attach if DMA involves
           * system unit memory
           */
#ifdef DARTIC_DEBUG
           printf("artic_icadmasetup: Calling xmattach\n");
#endif
           if (xmattach(arg->uaddr, arg->count,
                        &artic_dma[coprocnum].dp,
                        USER_ADSPACE) == XMEM_FAIL)
           {
#ifdef DARTIC_DEBUG
              printf("artic_icadmasetup: xmattach failed\n");
#endif
              arg->retcode = E_ICA_DMAPARMS;
              unpinu(arg->uaddr, arg->count, UIO_USERSPACE);
              clrjmpx(&artic_jumpbuffers[coprocnum]);    /* clear exception stack */
              return(retval);
           }

        }

#ifdef DARTIC_DEBUG
        printf("artic_icadmasetup: Calling d_init\n");
#endif
        if ((artic_dma[coprocnum].channel_id = d_init(artic_adapter[coprocnum].dmalevel,
                                                   MICRO_CHANNEL_DMA,
                                                   ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id)) == DMA_FAIL)
        {
#ifdef DARTIC_DEBUG
           printf("artic_icadmasetup: d_init failed\n");
#endif
           arg->retcode = E_ICA_DMASETUP;
           unpinu(arg->uaddr, arg->count, UIO_USERSPACE);
           clrjmpx(&artic_jumpbuffers[coprocnum]);    /* clear exception stack */
           return(retval);
        }
        else
           ++dma_initialized;

        /*
         * Check DMA type parm sent by application
         */

        switch(arg->type)
        {
           case ADAPTER_TO_SU:
              artic_dma[coprocnum].flags = DMA_READ | DMA_NOHIDE;
              xmemptr = &artic_dma[coprocnum].dp;
              break;

           case SU_TO_ADAPTER:
              artic_dma[coprocnum].flags = DMA_WRITE_ONLY;
              xmemptr = &artic_dma[coprocnum].dp;
              break;

           case PEER_TO_ADAPTER:
              artic_dma[coprocnum].flags = BUS_DMA;
              xmemptr = NULL;
              break;

           case ADAPTER_TO_PEER:
              artic_dma[coprocnum].flags = DMA_READ | BUS_DMA;
              xmemptr = NULL;
              break;

           default:
#ifdef DARTIC_DEBUG
              printf("artic_icadmasetup: invalid dma type\n");
#endif
              arg->retcode = E_ICA_DMAPARMS;
              d_clear(artic_dma[coprocnum].channel_id);
              artic_dma[coprocnum].channel_id = DMA_FAIL;
              unpinu(arg->uaddr, arg->count, UIO_USERSPACE);
              clrjmpx(&artic_jumpbuffers[coprocnum]);    /* clear exception stack */
              return(retval);
        }

#ifdef DARTIC_DEBUG
        printf("artic_icadmasetup: Calling d_master\n");
#endif
        d_master(artic_dma[coprocnum].channel_id,
                 artic_dma[coprocnum].flags,
                 arg->uaddr,
                 arg->count,
                 xmemptr,
                 (caddr_t) artic_adapter[coprocnum].dmamem);

        if (arg->type == ADAPTER_TO_SU)
        {
           d_cflush(artic_dma[coprocnum].channel_id,
                    arg->uaddr,
                    arg->count,
                    (caddr_t) artic_adapter[coprocnum].dmamem);
        }

        arg->physaddr = artic_adapter[coprocnum].dmamem;
        artic_dma[coprocnum].uaddr = arg->uaddr;
        artic_dma[coprocnum].count = arg->count;
        artic_dma[coprocnum].type = arg->type;

#ifdef DARTIC_DEBUG
        printf("artic_icadmasetup: Calling d_unmask\n");
#endif
        d_unmask(artic_dma[coprocnum].channel_id);

        clrjmpx(&artic_jumpbuffers[coprocnum]);    /* clear exception stack */

#ifdef DARTIC_DEBUG
        printf("artic_icadmasetup: Exiting with rc = %X\n",arg->retcode);
#endif

        return(retval);
}

/*
 *      artic_icadmarel
 *
 */

artic_icadmarel(device, arg)
dev_t device;              /* major/minor number             */
ICADMAREL_PARMS *arg;      /* pointer to the parameter block */
{
        int     retval = 0;      /* return value                     */
        int     coprocnum;       /* from arg->coprocnum              */
        int     rc;              /* return code from  function calls */

#ifdef DARTIC_DEBUG
        printf("artic_icadmarel: Entered.\n");
#endif

        arg->retcode = NO_ERROR;                               /* preset to success                    */

        /*
         *      Do Validity Checks on parameters.
         *
         */

        coprocnum = minor(device);     /* retrieve the coprocessor number */

#ifdef DARTIC_DEBUG
        printf("artic_icadmarel: coprocnum = %d\n",coprocnum);
#endif

        if (artic_adapter[coprocnum].state == DARTIC_EMPTY )
        {
                arg->retcode = E_ICA_INVALID_COPROC;   /* out of Range */
                return(retval);
        }

        if (artic_dma[coprocnum].channel_id == DMA_FAIL)
        {
                arg->retcode = E_ICA_DMAREL;
                return(retval);
        }


        /*
         *      Set up a jumpbuffer for exceptions.
         */

        if (retval = setjmpx(&artic_jumpbuffers[coprocnum]))
        {
#ifdef DARTIC_DEBUG
                printf("artic_icadmarel: setjmpx returned %d\n",retval);
#endif
                return(EIO);                                                            /* return error code    */
        }

#ifdef DARTIC_DEBUG
        printf("artic_icadmarel: Calling d_complete\n");
#endif

        if ((rc = d_complete(artic_dma[coprocnum].channel_id,
                             artic_dma[coprocnum].flags,
                             artic_dma[coprocnum].uaddr,
                             artic_dma[coprocnum].count,
                             &artic_dma[coprocnum].dp,
                             (caddr_t) artic_adapter[coprocnum].dmamem)) != DMA_SUCC)
        {
           arg->retcode = E_ICA_DMAREL;
#ifdef DARTIC_DEBUG
           printf("artic_icadmarel: d_complete returned %d\n",rc);
#endif
        }

        if ((artic_dma[coprocnum].type == ADAPTER_TO_SU) ||
            (artic_dma[coprocnum].type == SU_TO_ADAPTER))
        {
#ifdef DARTIC_DEBUG
           printf("artic_icadmarel: Calling xmdetach\n");
#endif
           if (xmdetach(&artic_dma[coprocnum].dp) == XMEM_FAIL)
           {
              arg->retcode = E_ICA_DMAREL;
#ifdef DARTIC_DEBUG
              printf("artic_icadmarel: xmdetach failed\n");
#endif
           }

#ifdef DARTIC_DEBUG
           printf("artic_icadmarel: Calling unpinu \n");
#endif
           if (unpinu(artic_dma[coprocnum].uaddr,
                      artic_dma[coprocnum].count,
                      UIO_USERSPACE))
           {
              arg->retcode = E_ICA_DMAREL;
#ifdef DARTIC_DEBUG
              printf("artic_icadmarel: unpinu failed\n");
#endif
           }
        }

#ifdef DARTIC_DEBUG
        printf("artic_icadmarel: Calling d_clear\n");
#endif
        d_clear(artic_dma[coprocnum].channel_id);
        artic_dma[coprocnum].channel_id = DMA_FAIL;

        clrjmpx(&artic_jumpbuffers[coprocnum]);    /* clear exception stack */

#ifdef DARTIC_DEBUG
        printf("artic_icadmarel: Exiting with rc = %X\n",arg->retcode);
#endif

        return(retval);
}


/*
 *      This function reprograms the POS registers with values that
 *      were stored during configuration.
 */

reprogramposreg(coprocnum)
int coprocnum;
{
        char *busio_addr;               /* for attachment to IOCC bus   */

        /*
         *      Attach to IO bus so we can access POS registers
         */

        busio_addr = IOCC_ATT(ARTIC_BUS_ID | artic_adapter[coprocnum].bus_id,
                              SLOTADDR(artic_adapter[coprocnum].slot));


        BUSIO_PUTC(busio_addr + 2,artic_adapter[coprocnum].pos2);

        BUSIO_PUTC(busio_addr + 3,artic_adapter[coprocnum].pos3);

        BUSIO_PUTC(busio_addr + 4,artic_adapter[coprocnum].pos4);

        BUSIO_PUTC(busio_addr + 5,artic_adapter[coprocnum].pos5);

        delay(1);

        IOCC_DET(busio_addr);   /* Detach from MCA bus  */

}


