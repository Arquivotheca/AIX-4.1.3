static char sccsid[] = "@(#)72	1.3  src/bos/diag/tu/iop/tod_funcs.c, tu_iop, bos411, 9431A411a 7/8/94 09:22:12";
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: set_iocc, get_iocc, set_nvram, get_nvram, clear_tod_flags,
 *            save_tod_regs, restore_tod_regs, check_tod_flags, debug_tod.
 *            get_tod, set_tod, get_key_position, get_pwr_stat, is_pegasus
 *
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 *
 */

#include <signal.h>
#include "diag/modid.h"
#include "tu_type.h"
#include "address.h"

void save_tod_regs();
void restore_tod_regs();
void clear_tod_flags();
int  check_tod_flags();
int	get_key_position();

/* Flag for each TOD register */
extern char tod_reg_flags[];
/* array of saved TOD registers */
extern uchar  tod_reg_array[];


extern FILE *dbg_fd;

/*--------------------------------------------------------------------*/
/* This function is added not to have to issue a get_cpu_model call
 * each time an machine device driver ioctl call is needed
 * It is called from this file, from ioptu_06.c and from some of todtest*.c
 */
int
is_pegasus()
{
	static int pegasus_flag, flag_is_valid = 0;

	if (!flag_is_valid) {
		pegasus_flag = IsPowerPC_SMP(get_cpu_model(&flag_is_valid));
		flag_is_valid = 1;
	}
	return (pegasus_flag);
}

/*--------------------------------------------------------------------*/
/* This function is needed for the TOD test called from this file */
int
get_tod(fdesc,addr,valptr,flag, tucb_ptr)
int fdesc;
u_long addr;
char *valptr;
int flag;
TUTYPE *tucb_ptr;
{
        int err;
        MACH_DD_IO mddrec;      /* made this a temporary, local var */

        mddrec.md_size = 1;                     /* one byte or word transfer  */
        mddrec.md_incr = flag;                  /* BYTE or WORD */
        mddrec.md_addr = addr;                  /* IOCC address */
        mddrec.md_data = valptr;                /* returns value here */

	if (is_pegasus())
        	err = ioctl(fdesc, MIOTODGET, &mddrec);	/* PEGASUS	*/
	else
		err = ioctl(fdesc, MIOCCGET, &mddrec);	/* RS/6000	*/

        if (err)
                tucb_ptr->tu_stats.bad_others++;
        else
                tucb_ptr->tu_stats.good_others++;

        return(err);
}
/*--------------------------------------------------------------------*/
/* This function is needed for the TOD test called from this file */
set_tod(fdesc,addr,valptr, flag, tucb_ptr)
int fdesc;
u_long addr;
char *valptr;
int flag;
TUTYPE *tucb_ptr;
{
        int err;
        MACH_DD_IO mddrec;      /* made this a temporary, local var */


        mddrec.md_size = 1;                /* one byte or word transfer  */
        mddrec.md_incr = flag;                  /* BYTE or WORD */
        mddrec.md_addr = addr;                  /* IOCC address  */
        mddrec.md_data = valptr;                /* writes value here */
        mddrec.md_sla  = 0;

	if (is_pegasus())
        	err = ioctl(fdesc, MIOTODPUT, &mddrec);	/* PEGASUS	*/
	else
		err = ioctl(fdesc, MIOCCPUT, &mddrec);	/* RS/6000	*/

        if (err)
                tucb_ptr->tu_stats.bad_others++;
        else
                tucb_ptr->tu_stats.good_others++;

        return(err);
}

/*----------------------------------------------------------------------*/

get_iocc(fdesc,addr,valptr,flag, tucb_ptr)
int fdesc;
u_long addr;
char *valptr;
int flag;
TUTYPE *tucb_ptr;
{
        int err;
        MACH_DD_IO mddrec;      /* made this a temporary, local var */

        mddrec.md_size = 1;                     /* one byte or word transfer  */
        mddrec.md_incr = flag;                  /* BYTE or WORD */
        mddrec.md_addr = addr;                  /* IOCC address */
        mddrec.md_data = valptr;                /* returns value here */
        err = ioctl(fdesc, MIOCCGET, &mddrec);  /* read from IOCC address */

        if (err)
                tucb_ptr->tu_stats.bad_others++;
        else
                tucb_ptr->tu_stats.good_others++;

        return(err);
}
/*--------------------------------------------------------------------*/
/* This function is needed for the TOD test called from this file */
set_iocc(fdesc,addr,valptr, flag, tucb_ptr)
int fdesc;
u_long addr;
char *valptr;
int flag;
TUTYPE *tucb_ptr;
{
        int err;
        MACH_DD_IO mddrec;      /* made this a temporary, local var */


        mddrec.md_size = 1;                /* one byte or word transfer  */
        mddrec.md_incr = flag;                  /* BYTE or WORD */
        mddrec.md_addr = addr;                  /* IOCC address  */
        mddrec.md_data = valptr;                /* writes value here */
        mddrec.md_sla  = 0;
        err = ioctl(fdesc, MIOCCPUT, &mddrec);  /* write to IOCC addr */

        if (err)
                tucb_ptr->tu_stats.bad_others++;
        else
                tucb_ptr->tu_stats.good_others++;

        return(err);
}

/*----------------------------------------------------------------------*/
/* This function is needed for nvram reads and writes                   */

get_nvram(fdesc,addr,valptr,flag1,flag2,tucb_ptr)
int fdesc;
u_long addr;
char *valptr;
int flag1;
int flag2;
TUTYPE *tucb_ptr;
{
        int err;
        MACH_DD_IO mddrec;      /* made this a tmeportary, local var */

        mddrec.md_size = flag2;          /* How many bytes or words to trans */
        mddrec.md_incr = flag1;          /* BYTE or WORD */
        mddrec.md_addr= addr;           /* NVRAM address */
        mddrec.md_data = valptr;        /* write value here */
        err=ioctl(fdesc, MIONVGET, &mddrec); /* read NVRAM address */

        if(err)
                tucb_ptr->tu_stats.bad_others++;
        else
                tucb_ptr->tu_stats.good_others++;
        return(err);
}

/*----------------------------------------------------------------------*/
/* This function is needed for nvram reads and writes                   */

set_nvram(fdesc,addr,valptr,flag1,flag2,tucb_ptr)
int fdesc;
u_long addr;
char *valptr;
int flag1;
int flag2;
TUTYPE *tucb_ptr;
{
        int err;
        MACH_DD_IO mddrec;      /* made this a temporary, local var */

        mddrec.md_size = flag2;          /* How many bytes or words to trans */
        mddrec.md_incr = flag1;          /* BYTE or WORD */
        mddrec.md_addr = addr;          /* NVRAM address */
        mddrec.md_data = valptr;        /* write value here */
        err=ioctl(fdesc, MIONVPUT, &mddrec); /* write NVRAM address */

        if(err)
                tucb_ptr->tu_stats.bad_others++;
        else
                tucb_ptr->tu_stats.good_others++;
        return(err);
}



/***********************************************************************
*  Check if flags are set
***********************************************************************/
int check_tod_flags()
{
        /* This flag is set to 1 if TOD registers have been saved */

        return (tod_reg_flags[F_SAVE_COMPLETE]);
}

/***********************************************************************
*  Clear all flags.
***********************************************************************/
void clear_tod_flags()
{
        int i;

        bzero(&tod_reg_flags, sizeof(char [NUM_TOD_FLAGS]));
        bzero(&tod_reg_array, sizeof(uchar [NUM_TOD_REGS]));
}


/***********************************************************************
*  Save & Restore the Control and Storage Registers of the TOD chip   **
***********************************************************************/

void save_tod_regs(fdesc,tucb_ptr)
int fdesc;
TUTYPE *tucb_ptr;
{
        int                     i;
        uchar                   *ucp;
        uchar                   byte_val, xval;
        int                     tod_offset;

/*
        printf("save_tod_regs(): tu#=%d\n", tucb_ptr->header.tu);
*/

        /* If second count flag is checked, save off time */
        /* To save time, one has to set the TIME SAVE bit, bit #7,
           and then clear it. */
        if (tod_reg_flags[F_SECCOUNT])  {
                byte_val = PG0REG0;  /* write to  control reg */
                set_tod(fdesc,MAIN,&byte_val,MV_BYTE,tucb_ptr);
                get_tod(fdesc,IRQROUTE,&xval,MV_BYTE,tucb_ptr);
                byte_val = xval | 0x80;    /* enable time save */
                set_tod(fdesc,IRQROUTE,&byte_val,MV_BYTE,tucb_ptr);
                byte_val = xval & 0x7f;    /* turn off time save bit */
                set_tod(fdesc,IRQROUTE,&byte_val,MV_BYTE,tucb_ptr);
        }


        for (i = 0; i <=35; i++) {
                if (tod_reg_flags[i])  {
                        ucp=&tod_reg_array[i];
                        byte_val = PG0REG0;
                        tod_offset = i;

                        /* IF array index is > 4, then subtract 4  */
                        /* bytes for proper TOD reg offset.  Also */
                        /* set byte_val to PG0REG1. */

                        if (i > 4)  {
                                tod_offset -= 4;
                                byte_val = PG0REG1;
                        }  /* if from reg select = 1 */

                        /*  If (6 <= tod_offset <= 10), then the flag
                            is a request to save off the time values.
                            If this is the case, the F_SECSAVE flag
                            already caused the read of time values at the
                            beginning of this routine.  Now those
                            values are actually stored in the SAVE
                            registers, 0xD9-0xDD, (25-29).  */

                        if (tod_offset >= 6 && tod_offset <= 10) {
                                tod_offset += 19;
                        }
                        set_tod(fdesc,MAIN,&byte_val,MV_BYTE,
                                tucb_ptr);
                        get_tod(fdesc,MAIN + tod_offset,ucp,
                                MV_BYTE,tucb_ptr);
/*
       printf("Save array[%d], tod_offset=%d, bval(pagereg)=%x\n",
                                i, tod_offset, byte_val);
*/
                }
        }

        if (tod_reg_flags[F_PG1RAM]) {
/*      printf("save the PAGE 1 RAM\n"); */
                byte_val = PG1REG0;
                set_tod(fdesc,MAIN,&byte_val,MV_BYTE,tucb_ptr);
                ucp=&tod_reg_array[F_PG1RAM];
                /* Save in tod_reg_array[33-64] */
                for (i = 1; i <=31; i++, ucp++) {
                        get_tod(fdesc,MAIN + i,ucp,MV_BYTE,tucb_ptr);
                }
        }

        /* Set flag denoting that all necessary registers  */
        /* have been saved.                                */

        tod_reg_flags[F_SAVE_COMPLETE] = 1;

return;

}

/*--------------------------------------------------------------------*/
void restore_tod_regs(fdesc,tucb_ptr)
int fdesc;
TUTYPE *tucb_ptr;
{
        int                     i;
        uchar                   *ucp;
        uchar                   byte_val;
        uchar                   real_val;
        int                     tod_offset;
        int                     restart = 0;

/*
        printf("restore_tod_regs(): tu#=%d\n", tucb_ptr->header.tu);
*/

        /* If second count flag is checked, reset time. */
        /* To reset time, one has to stop the clock, by setting
           the Clock Start/Stop bit (D3) to zero in the REALTIME reg. */

        if (tod_reg_flags[F_SECCOUNT])  {
                byte_val = PG0REG1;  /* write to  control reg */
                set_tod(fdesc,MAIN,&byte_val,MV_BYTE,tucb_ptr);
                get_tod(fdesc,REALTIME,&real_val,MV_BYTE,tucb_ptr);
                byte_val = real_val & 0xF7;    /* stop clock, mask D3 */
                set_tod(fdesc,REALTIME,&byte_val,MV_BYTE,tucb_ptr);
                restart = 1;
        }
        /* Start at 1, reset the Main Status register last. */
        for (i = 1; i <=35; i++) {
                if (tod_reg_flags[i])  {
                        ucp=&tod_reg_array[i];
                        byte_val = PG0REG0;
                        tod_offset = i;

                        /* IF array index is > 4, then subtract 4  */
                        /* bytes for proper TOD reg address.  Also */
                        /* set byte_val to PG0REG1. */

                        if (i > 4)  {
                                tod_offset -= 4;
                                byte_val = PG0REG1;
                        }  /* if from reg select = 1 */

                        set_tod(fdesc,MAIN,&byte_val,MV_BYTE,
                                tucb_ptr);
                        set_tod(fdesc, MAIN + tod_offset, ucp,
                                 MV_BYTE,tucb_ptr);
/*
       printf("Restore array[%d], tod_offset=%d, bval(pagereg)=%x\n",
                                i, tod_offset, byte_val);
*/
                }
        }

        if (restart) {
                byte_val = PG0REG1;  /* write to  control reg */
                set_tod(fdesc,MAIN,&byte_val,MV_BYTE,tucb_ptr);

                /* use restored REALTIME value - if saved */
                get_tod(fdesc,REALTIME,&real_val,MV_BYTE,tucb_ptr);
                byte_val = real_val | 0x08;    /* start clock, set D3 */
                set_tod(fdesc,REALTIME,&byte_val,MV_BYTE,tucb_ptr);
        }

        if (tod_reg_flags[F_PG1RAM]) {
/*              printf("Restore the PAGE 1 RAM\n"); */
                byte_val = PG1REG0;
                set_tod(fdesc,MAIN,&byte_val,MV_BYTE,tucb_ptr);
                ucp=&tod_reg_array[F_PG1RAM];
                /* Reset Page 1 RAM with tod_reg_array[36-66] */
                for (i = 1; i <=31; i++, ucp++) {
                        set_tod(fdesc,MAIN + i,ucp,MV_BYTE,
                                tucb_ptr);
                }
        }

        /* Restore the status register last, if it was saved. */
        if (tod_reg_flags[F_MAIN]) {
                        ucp=&tod_reg_array[0];
                        set_tod(fdesc, MAIN, ucp, MV_BYTE,
                                tucb_ptr);
/*
  printf("Restore array[%d], tod_offset=%d, bval(pagereg)=%x\n",
                                F_MAIN, 0, byte_val);
*/
        }

        /* Denote that registers have been restored */
        tod_reg_flags[F_SAVE_COMPLETE] = 0;
        clear_tod_flags();
return;

}


/*--------------------------------------------------------------------*/
void debug_tod(fdesc,tucb_ptr)
int fdesc;
TUTYPE *tucb_ptr;

{
        u_char main_save;
        u_char bval, rval;

        get_tod(fdesc,MAIN,&main_save,MV_BYTE,tucb_ptr);
        printf("        MAIN=%x\n", main_save);

        rval = PG0REG0;         /* page 0 reg 0 */
        set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr);

        get_tod(fdesc,TCR0,&bval,MV_BYTE,tucb_ptr);
        printf("        TCR0=%x\n", bval);
        get_tod(fdesc,TCR0,&bval,MV_BYTE,tucb_ptr);
        printf("        TCR1=%x\n", bval);
        get_tod(fdesc,PERFLAG,&bval,MV_BYTE,tucb_ptr);
        printf("        PERFLAG=%x\n", bval);
        get_tod(fdesc,IRQROUTE,&bval,MV_BYTE,tucb_ptr);
        printf("        IRQROUTE=%x\n", bval);

        bval=PG0REG1;
        set_tod(fdesc,MAIN,&bval,MV_BYTE,tucb_ptr);
        get_tod(fdesc,REALTIME,&bval,MV_BYTE,tucb_ptr);
        printf("        REALTIME=%x\n", bval);
        get_tod(fdesc,OUTPUTMODE,&bval,MV_BYTE,tucb_ptr);
        printf("        OUTPUTMODE=%x\n", bval);
        get_tod(fdesc,IRQ0,&bval,MV_BYTE,tucb_ptr);
        printf("        IRQ0=%x\n", bval);
        get_tod(fdesc,IRQ1,&bval,MV_BYTE,tucb_ptr);
        printf("        IRQ1=%x\n", bval);

        set_tod(fdesc,MAIN,&main_save,MV_BYTE,tucb_ptr);

}


/*--------------------------------------------------------------------*/
void fdebug_tod(fdesc,tucb_ptr)
int fdesc;
TUTYPE *tucb_ptr;

{
	u_char main_save;
	u_char bval, rval;

        time_t lt;
        struct tm *ptr;

        lt = time(NULL);
        ptr = localtime(&lt);
        fprintf(dbg_fd, "TOD #%d    %s",tucb_ptr->header.tu, asctime(ptr));

	get_tod(fdesc,MAIN,&main_save,MV_BYTE,tucb_ptr);
	fprintf(dbg_fd,"	MAIN=%x\n", main_save);

	rval = PG0REG0;         /* page 0 reg 0 */
	set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr);

	get_tod(fdesc,TCR0,&bval,MV_BYTE,tucb_ptr);
	fprintf(dbg_fd,"	TCR0=%x\n", bval);
	get_tod(fdesc,TCR0,&bval,MV_BYTE,tucb_ptr);
	fprintf(dbg_fd,"	TCR1=%x\n", bval);
	get_tod(fdesc,PERFLAG,&bval,MV_BYTE,tucb_ptr);
	fprintf(dbg_fd,"	PERFLAG=%x\n", bval);
	get_tod(fdesc,IRQROUTE,&bval,MV_BYTE,tucb_ptr);
	fprintf(dbg_fd,"	IRQROUTE=%x\n", bval);
	
	bval=PG0REG1;
	set_tod(fdesc,MAIN,&bval,MV_BYTE,tucb_ptr);
	get_tod(fdesc,REALTIME,&bval,MV_BYTE,tucb_ptr);
	fprintf(dbg_fd,"	REALTIME=%x\n", bval);
	get_tod(fdesc,OUTPUTMODE,&bval,MV_BYTE,tucb_ptr);
	fprintf(dbg_fd,"	OUTPUTMODE=%x\n", bval);
	get_tod(fdesc,IRQ0,&bval,MV_BYTE,tucb_ptr);
	fprintf(dbg_fd,"	IRQ0=%x\n", bval);
	get_tod(fdesc,IRQ1,&bval,MV_BYTE,tucb_ptr);
	fprintf(dbg_fd,"	IRQ1=%x\n\n", bval);
	
	set_tod(fdesc,MAIN,&main_save,MV_BYTE,tucb_ptr);
	
}

/*--------------------------------------------------------------------*/
/* This function is needed for the KEYLOCK test 		      */
get_key_position(fdesc,valptr,tucb_ptr)
int fdesc;
char *valptr;
TUTYPE *tucb_ptr;
{
        int err;
        MACH_DD_IO mddrec;      /* made this a temporary, local var */

        mddrec.md_size = 1;                     /* one byte or word transfer  */
        mddrec.md_incr = MV_WORD;
        mddrec.md_addr = 0xff620064 ;
        mddrec.md_data = valptr;                /* returns value here */
       /* err = ioctl( fdesc, MIONVGET, &mddrec ) ;*/
        err = ioctl(fdesc, MIOGETKEY, &mddrec);  /* read from IOCC address */

        if (err)
                tucb_ptr->tu_stats.bad_others++;
        else
                tucb_ptr->tu_stats.good_others++;

        return(err);
}

/*--------------------------------------------------------------------*/
/* This function is needed for the POWER STATUS test 		      */
get_pwr_stat(fdesc, addr, valptr, tucb_ptr)
int fdesc;
u_long addr;
char *valptr;
TUTYPE *tucb_ptr;
{
        int err;
        MACH_DD_IO mddrec;      /* made this a temporary, local var */
	char data[4];

	if (!is_pegasus())
		return(get_iocc(fdesc,addr,valptr,MV_BYTE,tucb_ptr));

        mddrec.md_size = 1;                     /* one word transfer  */
        mddrec.md_incr = MV_WORD;
        mddrec.md_data = data;                  /* returns value here */

        err = ioctl(fdesc, MIOGETPS, &mddrec);

        if (err)
                tucb_ptr->tu_stats.bad_others++;
        else
                tucb_ptr->tu_stats.good_others++;

	*valptr = data[(addr - POWERSTAT) & 3];

        return(err);
}
