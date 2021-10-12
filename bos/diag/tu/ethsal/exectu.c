static char sccsid[] = "@(#)49  1.1.1.5  src/bos/diag/tu/ethsal/exectu.c, tu_ethsal, bos411, 9428A410j 3/24/94 17:45:19";
/*
 *   COMPONENT_NAME: tu_ethsal
 *
 *
 *   FUNCTIONS:     exectu, get_md, get_tu_name, set_tu_errno, 
 *		    setup_control_variables, get_machine_model
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <sys/mdio.h>

#include <sys/iplcb.h>
#include <sys/rosinfo.h>

#include "exectu.h"
#include "tu_type.h"


typedef struct
{
  int number;
  int (*pfcn)(void);
  char *desc;
} TUFCN;


static TUFCN tu_fcn[] =
{
  POS_TU,         pos_tu,         "POS registers",
  VPD_TU,         vpd_tu,         "VPD",
  IO_TU,          io_tu,          "I/O registers",
  SELFTEST_TU,    selftest_tu,    "Selftest",
  LB_82596_TU,    lb_82596_tu,    "Loopback at the 82596",
  LB_825XX_TU,    lb_82501_tu,    "Loopback at the 82501",
  LB_EXT_TU,      lb_ext_tu,      "Loopback external",
  LB_EXT_EMC_TU,  lb_ext_emc_tu,  "Loopback external (EMC)",

#ifdef DEBUG_ETHER
  DUMP_VPD_TU,    dump_vpd_tu,    "VPD dump",
  NADDR_TU,       naddr_tu,       "Network address",
#endif

};


static int fd[2];   /* special files' file descriptors */
static int tu_errno = 0;

static int run_tu(int);
static int setup_control_variables(TUCB *); 

/*
 * NAME : exectu
 *
 * DESCRIPTION :
 *
 *   Entry point for all Test Units.
 *
 * INPUT :
 *
 *   1. File descriptor of Ethernet device driver opened
 *      in diagnostics mode.
 *   2. Pointer to TUCB structure.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

int exectu(int eths_fdes, TUCB *tucb_ptr)
{
  int rc, loops;

  fd[ETHS_FD] = eths_fdes;
  fd[MDD_FD] = tucb_ptr->mdd_fdes;

  tu_errno = 0;

  rc = setup_control_variables(tucb_ptr);

  /* Now we can run the TU */
  for (loops = 0; rc == SUCCESS && loops < tucb_ptr->header.loop; loops++)
  {
    if((rc = run_tu(tucb_ptr->header.tu)) != SUCCESS)
      break;
  }

  tucb_ptr->errno = tu_errno;

  return(rc);
}



/*
 * NAME : get_fd
 *
 * DESCRIPTION :
 *
 *   Gets device driver file descriptor.
 *
 * INPUT :
 *
 *   Device driver file descriptor type
 *   (MDD_FD or ETHS_FD).
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Device driver file descriptor.
 *
*/

int get_fd(int type)
{
  return(fd[type]);
}



/*
 * NAME : run_tu
 *
 * DESCRIPTION :
 *
 *   Executes TU.
 *
 * INPUT :
 *
 *   TU number.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

static int run_tu(int tu)
{
  int i, rc;

  rc = TU_NUMBER_ERR;

  for(i = 0; i < (sizeof(tu_fcn) / sizeof(TUFCN)); i++)
  {
    if(tu_fcn[i].number == tu)
    {
      rc = SUCCESS;           /* TU found    */
      rc = tu_fcn[i].pfcn();  /* execute TU  */
      break;
    }
  }

  return(rc);
}



/*
 * NAME : get_tu_name
 *
 * DESCRIPTION :
 *
 *   Gets Test Unit description
 *
 * INPUT :
 *
 *   Test Unit number
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Returns Test Unit description.
 *
*/

char *get_tu_name(ulong_t tu)
{
  int i;

  for(i = 0; i < (sizeof(tu_fcn) / sizeof(TUFCN)); i++)
    if(tu_fcn[i].number == tu)
      return(tu_fcn[i].desc);

    return("NO SUCH TU ...");
}



/*
 * NAME : set_tu_errno
 *
 * DESCRIPTION :
 *
 *   Updates tu_errno with global variable errno
 *   when a system call fails.
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   tu_errno variable updated.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void set_tu_errno(void)
{
  tu_errno = errno;
}



#define SAL1_MODEL                   0x41   /* SAL1_MODEL (rosinfo.h)*/
#define SAL2_MODEL                   0x45   /* SAL2_MODEL (rosinfo.h)*/
#define CAB_MODEL                   0x43   /* CAB_MODEL (rosinfo.h)*/
#define CHAP_MODEL                  0x47   /* CHAP_MODEL (rosinfo.h)*/
#define RBW_MODEL                   0x46   /* RBW_MODEL (rosinfo.h)*/

int get_machine_model(int mdd_fdes)
{
  MACH_DD_IO    mdd;
  IPL_DIRECTORY *iplcb_dir;                                       /* iplcb.h */
  IPL_INFO      *iplcb_info;
  int  rc;
  int  machine_model;

  machine_model = INVALID_MACHINE_MODEL;       /* default, invalid model   */

  rc = SUCCESS;

    iplcb_dir = (IPL_DIRECTORY *) malloc (sizeof(IPL_DIRECTORY));
    iplcb_info = (IPL_INFO *) malloc (sizeof(IPL_INFO));
    if ( (iplcb_dir == ((IPL_DIRECTORY *) NULL))||
         (iplcb_info == ((IPL_INFO *) NULL)) ) {
        machine_model = INVALID_MACHINE_MODEL;
    } else {
        mdd.md_incr = MV_BYTE;
        mdd.md_addr = 128;
        mdd.md_data = (char *) iplcb_dir;
        mdd.md_size = sizeof(*iplcb_dir);
        if ( ioctl(mdd_fdes, MIOIPLCB, &mdd) ) {
          machine_model = INVALID_MACHINE_MODEL;
        } else {
            mdd.md_incr = MV_BYTE;
            mdd.md_addr = iplcb_dir -> ipl_info_offset;
            mdd.md_data = (char *) iplcb_info;
            mdd.md_size = sizeof(*iplcb_info);
            if ( ioctl(mdd_fdes, MIOIPLCB, &mdd) ) {
              machine_model = INVALID_MACHINE_MODEL;
            } else {

/* ***************************************************************************
 *  The model field contains information which allows software to determine  *
 *  hardware type, data cache size, and instruction cache size.              *
 *                                                                           *
 *  The model field is decoded as follows:                                   *
 *        0xWWXXYYZZ                                                         *
 *                                                                           *
 *  case 2: WW is nonzero.                                                   *
 *          WW = 0x01 This means that the hardware is SGR ss32 or SGR ss64   *
 *                    (ss is speed in MH).                                   *
 *          WW = 0x02 means the hardware is RSC.                             *
 *          XX has the following bit definitions:                            *
 *                  bits 0 & 1 (low order bits) - indicate package type      *
 *                        00 = Tower     01 = Desktop                        *
 *                        10 = Rack      11 = Reserved                       *
 *                  bits 2 through 7 are reserved.                           *
 *          YY = reserved.                                                   *
 *          ZZ = the model code:                                             *
 *                  0x45  : 
 *                  0x41  : 
 *                                                                           *
 *          The instruction cache K byte size is obtained from entry icache. *
 *          The data cache K byte size is obtained from entry dcache.        *
 *                                                                           *
 *  refer to '/usr/include/sys/rosinfo.h' for more information.              *
 *************************************************************************** */

               machine_model = iplcb_info -> model & 0xff;  /* retain ZZ fld*/
               if ( (machine_model == SAL1_MODEL) || (machine_model == SAL2_MODEL) || (machine_model == CAB_MODEL) || (machine_model == CHAP_MODEL) )
               {
                  machine_model = RSC_MACHINE_MODEL;
               }
               else {
                  machine_model = POWER_MACHINE_MODEL;
               }

                free (iplcb_dir);
                free (iplcb_info);
            } /* endif */
        }  /* end if */
    }  /* end if */
    rc = machine_model;

  return  (rc);

}

static int setup_control_variables (TUCB *tucb_ptr)
{
  int rc;
  ulong_t status;
  static BOOL done = FALSE;

 /* The following code returns the integrated ethernet thick/twisted pair 
    status inside the TU_PARMS structure.  Also, the machine model is checked
    and stored in TU_PARMS structure.  This section of code also initializes
    a couple of global variables because of register location changes between
    PowerPC and RSC.  More info. on these variables and the defines associated
    with them can be found in tu_type.h.

   1. Ethernet status register is read and bit 26 is checked.
   2. If bit 26 is 1, then we are using twisted pair.  If it is 0, then we are
      using thick. ( Checking bit 26 applies only for DVT systems )
   3. Store bit 26 info. (0 or 1) inside TU_PARMS structure.
 */

    rc = SUCCESS;

  /* setup control variables */

   /* Only run the following code once, setting defaults and init'ing values */
   if (!done) {

      power_flag = FALSE;
      port_reg_addr = ETH_IO_BASE_ADDR_RSC;
      creset_reg_addr = CRESET_REG_ADDR_RSC;

      tucb_ptr->tu_parms.eth_mode_setting = INVALID_MODE_POSITION;  /* Thick/Twisted Pair status */

      tucb_ptr->tu_parms.machine_model = get_machine_model(tucb_ptr->mdd_fdes);
      done = TRUE;
   }


   if (tucb_ptr->tu_parms.machine_model == POWER_MACHINE_MODEL) {  
	 /* If Power PC */

      power_flag = TRUE;     /* Turn global power_flag on */

      /* assign base I/O addr to global variable port_reg_addr - see tu_type.h*/
      PORT_REG_ADDR = ETH_IO_BASE_ADDR_POWER;  

      /* assign comp. reset reg. addr to global variable creset_reg_addr - see tu_type.h*/
      creset_reg_addr = CRESET_REG_ADDR_POWER;

      start_ether();  /* Make sure eth. card is enabled */

      if ((rc = get_word(tucb_ptr->mdd_fdes, &status, IO_STATUS_REG_ADDR , MIOBUSGET)) == SUCCESS) {

        if (status & ETH_MODE_SETTING_MASK)
	   tucb_ptr->tu_parms.eth_mode_setting = TWISTED_PAIR_POSITION;
        else
	   tucb_ptr->tu_parms.eth_mode_setting = NON_TWISTED_PAIR_POSITION;
      }
      else
	rc = READ_ERR;

      halt_ether();
	 
    }
     
  return(rc);
}
