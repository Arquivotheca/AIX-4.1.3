static char sccsid[] = "@(#)69	1.1  src/bos/diag/da/resid/dresid.c, daresid, bos41J, 9520A_all 5/15/95 17:27:52";
/*
 * COMPONENT NAME : DARESID
 *
 * FUNCTIONS      : battery test
 *                  memory, L2 cache residual data analysis 
 *                  8 digit error code display for rspc machines
 *
 * ORIGINS        : 27
 *
 * IBM CONFIDENTIAL --
 *
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
*/

#include <stdio.h>
#include <locale.h>
#include <signal.h>

#include <sys/mdio.h>
#include <sys/types.h>

#include <diag/atu.h>
#include "misc.h"
#include "dresid.h"

#include "diag/da.h"            /* The following header files has information */
#include "diag/diag_exit.h"     /* particular to this program. */
#include "diag/tmdefs.h"
#include "diag/tm_input.h"
#include "diag/diago.h"
/*#include "diag/dcda_msg.h"*/
#include "diag/class_def.h"
#include "diag/modid.h"
#include "dresid_msg.h"

struct fru_bucket planar[] = {
    { "", FRUB1, 0x969, 0x900, RR_PLANAR, {
			{ 100, "", "", 0, PARENT_NAME, NONEXEMPT },
			      },
    }
};


struct fru_bucket vict_simm[] = {
    { "", FRUB1, 0x969, 0x0, R_SIMM_ERROR, {
			 { 100, "SIMM", "", 0, DA_NAME, EXEMPT },
			      },
    }
};




#define GOOD_BATTERY 0x80
#define IO_ERROR (-1)

#define TYPE_MASK 0x01
#define SPEED_MASK 0x60
#define SIZE_MASK 0x0F

extern  getdainput();
static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

struct  tm_input        tm_input;       /* info. form dc to da */
struct  fru_bucket      temp_frub;      /* used for substitution */
extern  nl_catd diag_catopen(char *, int);
nl_catd fdes;

/*#define DEBUG 0*/

#if DEBUG
FILE *membug;
#endif

#if DEBUG
char *tbuf;
#endif

int fd;

char    vict_slot_table[] = { 'A','B','C','D','E','F','G','H' };
int s_slot;

main()
{
int rc;
int i;
int count;
int srn;

int simmtype;
int simmspeed;
int simmsize;

int   num_entries;
uint  *pdbits;
ulong *good_bad_simm;

    setlocale(LC_ALL,"");

    /* Initialize default return status */
    DA_SETRC_STATUS(DA_STATUS_GOOD);
    DA_SETRC_USER(DA_USER_NOKEY);
    DA_SETRC_ERROR(DA_ERROR_NONE);
    DA_SETRC_TESTS(DA_TEST_SHR);
    DA_SETRC_MORE(DA_MORE_NOCONT);


    if (init_dgodm() != 0) {
	term_dgodm();
	DA_SETRC_ERROR(DA_ERROR_OTHER);
	DA_EXIT();
    }

    rc = getdainput(&tm_input);
    if (rc != 0) {
	    term_dgodm();
	    DA_SETRC_ERROR(DA_ERROR_OTHER);
	    DA_EXIT();
    }

    if (tm_input.console == CONSOLE_TRUE)
	    diag_asl_init(ASL_INIT_DEFAULT);

    fdes = diag_catopen(MF_DRESID,0);


    if ((fd = open("/dev/nvram", 0)) < 0) {
       	DA_SETRC_ERROR(DA_ERROR_OTHER);
	DA_EXIT();
    }

    /* Display "stand by" screen and do analysis */
    if (tm_input.console == CONSOLE_TRUE) {
	switch (tm_input.advanced) {
        case ADVANCED_TRUE:
	    if (tm_input.loopmode != LOOPMODE_NOTLM)
	       rc = diag_msg_nw(0x812104,fdes,ALOOP,ALTITLE,tm_input.lcount,
	                        tm_input.lerrors);
            else
	       rc = diag_msg_nw(0x812103, fdes, ADVANCED, ATITLE);
	    break;
	case ADVANCED_FALSE:
	    if (tm_input.loopmode != LOOPMODE_NOTLM)
	       rc = diag_msg_nw(0x812102,fdes,LOOP,LTITLE,tm_input.lcount,
			        tm_input.lerrors);
	    else
	       rc = diag_msg_nw(0x812101, fdes, REGULAR, RTITLE);
	    break;
	default:
	    break;
        }
        sleep(4);
        rc = diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, FALSE, NULL);
        check_rc(rc);
   }

clean_up();

/* Memory Analysis - residual data */

    count = 0;
    diag_get_pdbits (&num_entries, &pdbits, &good_bad_simm);
    for (i=0; i<num_entries; ++i) {
	if ((good_bad_simm[i]==0) && (pdbits[i]!=0xFF)) {
	    count += 1;
	}
    }
    if (count > 2) {
	/* Call up system planar 100% bad */
	insert_frub(&tm_input, &planar[0]);
	strncpy(planar[0].dname,tm_input.dname, sizeof(planar[0].dname));
	addfrub(&planar[0]);
	DA_SETRC_STATUS(DA_STATUS_BAD);
    }
    else {
        for (i=0; i<num_entries; ++i) {
    	    srn = 0;
    	    if ((good_bad_simm[i]==0) && (pdbits[i]!=0xFF)) {
    	        simmtype = (pdbits[i] & TYPE_MASK) >> 7;
    	        srn += simmtype;
    	        simmsize = (pdbits[i] & SIZE_MASK);
    	        switch (simmsize) {
    	    	    case 0x04 : srn += 0x10;
    	    		        break;
    	    	    case 0x09 : srn += 0x20;
    	    		        break;
    	    	    case 0x0B : srn += 0x30;
    	    		        break;
    	    	    case 0x0D : srn += 0x40;
    	    		        break;
    	    	    case 0x0F : srn += 0x50;
    	    		        break;
     	    	    default   : /* software error */
				DA_SETRC_ERROR ( DA_ERROR_OTHER );
				clean_up();
    	        }
    	        simmspeed = (pdbits[i] & SPEED_MASK);
    	        switch (simmspeed) {
    	    	    case 0x00 : srn += 0x100;
    	    		        break;
    	    	    case 0x60 : srn += 0x200;
    	    		        break;
    	    	    case 0x40 : srn += 0x300;
    	    		        break;
    	    	    case 0x20 : srn += 0x400;
    	    		        break;
     	    	    default   : /* software error */
				DA_SETRC_ERROR ( DA_ERROR_OTHER );
				clean_up();
    	        }
    	    }
	    s_slot = vict_slot_table[i];
	    temp_frub = vict_simm[0];
	    temp_frub.rcode += srn;
	    sprintf(temp_frub.frus[0].floc,"00-0%c",s_slot);
	    addfrub(&temp_frub);
	    DA_SETRC_STATUS(DA_STATUS_BAD);
        }
    }

/* L2cache Analysis - residual data */


/* Battery test */
/* register D is read only. VRT (Valid RAM and Time) should */
/* always be 1. This is tested by BATTERY_TU.               */

    if (rc == SUCCESS) {
        if ((rc = battery_tu(fd)) == BATTERY_ERROR)
        {
            rc = TOD_BATT_ERROR;
        }
    }
} /* end main */

/* This function checks if the battery is good */
int
battery_tu(int fd)
{
  int rc;
  uchar_t reg_d;

  rc = SUCCESS;
  rc = get_tod_data(fd, (ulong_t)REG_D, &reg_d);

  if(rc == SUCCESS)
    if((reg_d & GOOD_BATTERY) != GOOD_BATTERY)
      rc = BATTERY_ERROR;

  return(rc);
}


/* Read a byte from TOD */

int
get_tod_data(int fd, ulong_t tod_addr, uchar_t *data)
{
  int rc;

  rc = SUCCESS;
  /*usleep(ACCESS_DELAY);*/

  rc = put_byte(fd, (uchar_t)tod_addr, (ulong_t)tod_idx_reg, MIOTODPUT);

  if(rc == SUCCESS) {
      rc = get_byte(fd, data, (ulong_t)tod_data_reg, MIOTODGET);
  }

  return(rc);
}



/* The following functions perform read and write operations to specified
   addresses */

int
get_byte(int fd, uchar_t *byte, ulong_t addr, int op)
{
   MACH_DD_IO iob;
   int rc;

   rc = SUCCESS;
   iob.md_incr = MV_BYTE;
   iob.md_size = sizeof(uchar_t);
   iob.md_data = byte;
   iob.md_addr = addr;

   if(ioctl(fd, op, &iob) == -1)
   {
       /*PRINTSYS("ioctl(GET)");*/
       rc = IO_ERROR;
   }

   return(rc);
}



int
put_byte(int fd, uchar_t byte, ulong_t addr, int op)
{
   MACH_DD_IO iob;
   int rc;

   rc = SUCCESS;
   iob.md_incr = MV_BYTE;
   iob.md_size = sizeof(uchar_t);
   iob.md_data = &byte;
   iob.md_addr = addr;

   if(ioctl(fd, op, &iob) == -1)
   {
      /*PRINTSYS("ioctl(PUT)");*/
      rc = IO_ERROR;
   }

   return(rc);
}


/*
* NAME: check_rc
*                                                                    
* FUNCTION: Checks if user has entered the Esc or Cancel key.
*                                                                    
* EXECUTION ENVIRONMENT:
*	This routine is called by the main program.
*
* RETURNS: Returns the input parameter to this routine - the user's input
*/

int 
check_rc(rc)
    int		rc;			/* User's input to the screen */
{
    if (rc == ASL_CANCEL) {
        if (tm_input.console == CONSOLE_TRUE)
            diag_asl_quit();
        DA_SETRC_USER(DA_USER_QUIT);
        DA_SETRC_TESTS(DA_TEST_SHR);
        DA_EXIT();
    }
    if (rc == ASL_EXIT) {
        if (tm_input.console == CONSOLE_TRUE)
            diag_asl_quit();
        DA_SETRC_USER(DA_USER_EXIT);
        DA_SETRC_TESTS(DA_TEST_SHR);
        DA_EXIT();
    }
    return (rc);
}


/*
* NAME: clean_up
*                                                                    
* FUNCTION: Calls check_missing and get ready to quit the application.
*                                                                    
* EXECUTION ENVIRONMENT:
*	This routine is called by the main program.
*
* RETURNS: NONE
*/

clean_up()

{
    catclose(fdes);
    if (fd > 0) 
        close(fd);
    term_dgodm();
    if (tm_input.console == CONSOLE_TRUE)
        diag_asl_quit();
#if DEBUG
fflush(residbug);
fclose(residbug);
#endif
    DA_EXIT();

}

