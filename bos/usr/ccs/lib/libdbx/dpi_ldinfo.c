static char sccsid[] = "@(#)84    1.9.1.10  src/bos/usr/ccs/lib/libdbx/dpi_ldinfo.c, libdbx, bos411, 9434B411a 8/20/94 15:58:02";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: dpi_load_info, dpi_isredirected, dpi_get_configuration
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
#include <sys/signal.h>
#include "defs.h"
#include "envdefs.h"
#include "object.h"
#include "mappings.h"
#include "process.h"
#include "symbols.h"
#include "execute.h"
#include "examine.h"
#include "ops.h"
#include "tree.h"

extern char *findsource();
extern Symbol fake_main;
extern Symbol program;
extern integer loadcnt;
extern boolean lazy;
extern cases   casemode;
extern fork_type multproc;
extern List    sourcepath;

int dpi_load_info( dpi_info )
LoadStruct dpi_info;
{
    Symbol s;
    integer i;

    s = (fake_main != nil) ? fake_main : lookup(identname("main",true));
    if ((s != nil) && (isroutine(s) && (!nosource(s)))) {
         dpi_info->file = findsource(srcfilename(codeloc(s)), NULL);
    }
    else if (nfiles_total) {
	for (i = 0; i < loadcnt; i++) {
            if (filetab[i]) {
	        dpi_info->file = findsource(filetab[i][0].filename, NULL);
		break;
	    }
	}
    }
    else
        dpi_info->file = nil;
    dpi_info->address = loader_info[0].textorg;
    dpi_info->memory = loader_info[0].dataorg;
    for ( i = 1; i < loadcnt; i++ )
        if ( loader_info[i].dataorg < dpi_info->memory )
            dpi_info->memory = loader_info[i].dataorg;
    return 0;
}


boolean dpi_isredirected()
{
    return isredirected();
}


/* 
 * Defines utilized in configuration retrieval
 */
#define  DEC_INT_FORMAT    0
#define  OCT_INT_FORMAT    1
#define  HEX_INT_FORMAT    2
/* The above defines should be removed once the dpi_get_more_configuration
   is merged back into dpi_get_configuration.
*/
/* The following values need to match those defined in dbx_configd.c for
   dex/softdb
*/
typedef  enum {
   display_ints_decimal,
   display_ints_octal,
   display_ints_hex
} display_ints_type;
typedef  enum {
   case_default,
   case_lower,
   case_upper,
   case_mixed
} case_mode_type;
typedef  enum {
   step_skip_func,
   step_skip_mod,
   step_skip_none
} single_step_mode_type;
typedef  enum {
   ins_set_default,
   ins_set_common,
   ins_set_rs1power,
   ins_set_rs2power,
   ins_set_powerpc,
   ins_set_601powerpc,
   ins_set_any
} instrs_set_mode_type;
typedef  enum {
   mnemonics_default,
   mnemonics_power,
   mnemonics_powerpc
} instrs_mnemonics_mode_type;
#define  bitshift(n)       (1 << ((n)-1))
#define  wordsize          (8*sizeof(Word))

/* this has been added in case it is not possible to break
   old versions of softdb.
   case_mode and integer_fmt are returned on this function and on
   dpi_get_configuration so that the new enum values can be returned
   in case the interface to dpi_get_configuration can not be changed.
*/
void  dpi_get_more_configuration( case_mode_type	*case_mode,
				  display_ints_type	*integer_fmt,
				  instrs_set_mode_type	*instrs_set_mode,
				  instrs_mnemonics_mode_type	*mnemonics_mode,
				  single_step_mode_type	*single_step_mode)
{
    /*
     * Retrieve the format for displaying integers
     */
    if (varIsSet( "$octints"))
    {
      *integer_fmt = display_ints_octal;
    }
    else
    {
      if (varIsSet( "$hexints" ))
      {
        *integer_fmt = display_ints_hex;
      }
      else
      {
        *integer_fmt = display_ints_decimal;
      }
    }

    /* retrieve the case setting */
    if (casemode == mixed)
    {
      *case_mode = case_mixed;
    }
    else
    {
      if (casemode == lower)
      {
        *case_mode = case_lower;
      }
      else
      {
        if (casemode == upper)
        {
          *case_mode = case_upper;
        }
        else
        {
          *case_mode = case_default;
        }
      }
    }

    /* retrieve the step function mode setting */
    if (stepignore == skipmodule)
    {
      *single_step_mode = step_skip_mod;
    }
    else
    {
      if (stepignore == skipnone)
      {
        *single_step_mode = step_skip_none;
      }
      else
      {
        *single_step_mode = step_skip_func;
      }
    }

    /* retrieve the instruction set setting */
    switch(instruction_set) {
        case PWR:
             *instrs_set_mode = ins_set_rs1power;
             break;
        case PWRX:
             *instrs_set_mode = ins_set_rs2power;
             break;
        case PPC:
             *instrs_set_mode = ins_set_powerpc;
             break;
        case SET_601:
             *instrs_set_mode = ins_set_601powerpc;
             break;
        case COM:
             *instrs_set_mode = ins_set_common;
             break;
        case ANY:
             *instrs_set_mode = ins_set_any;
             break;
        case DEFAULT:
        default:
             *instrs_set_mode = ins_set_default;
             break;
    }

    /* retrieve the instruction mnemonics setting */
    switch(mnemonic_set) {
        case PWR:
             *mnemonics_mode = mnemonics_power;
             break;
        case PPC:
             *mnemonics_mode = mnemonics_powerpc;
             break;
        case DEFAULT:
        default:
             *mnemonics_mode = mnemonics_default;
             break;
    }
}
/*
 * Retrieve the following configuration data from dbx:
 *    settings of debugger variables, use path,
 *    format for displaying integers, case mode,
 *    variable dimensions (unknown array bounds),
 *    and signals that are caught/ignored.
 */
void  dpi_get_configuration( variables_mask, case_mode, integer_fmt,
                             use_path, vardim_value, signals_mask  )
unsigned int  *variables_mask;
int           *case_mode;
int           *integer_fmt;
char          **use_path;
int           *vardim_value;
unsigned int  signals_mask[2];
{
  
    Node          p;
    String        dir;
    int           i, length;
    int           s, sigset;
    unsigned int  *sigret;

    /* 
     * Retrieve the state of various dbx debugger variables
     * (The order they are checked in matches the expected return order,
     *  ie: the order they are displayed within the configuration dialog).
     */
    *variables_mask = i = 0;
    if ( lazy )
        *variables_mask |= ( 1 << i ); i++;
    if ( multproc != off)
	*variables_mask |= ( 1 << i ); i++;
    if ( varIsSet( "$catchbp" ))
	*variables_mask |= ( 1 << i ); i++;
    if ( varIsSet( "$expandunions" ))
	*variables_mask |= ( 1 << i ); i++;
    if ( varIsSet( "$hexchars" ))
	*variables_mask |= ( 1 << i ); i++;
    if ( varIsSet( "$hexstrings" ))
	*variables_mask |= ( 1 << i ); i++;
    if ( varIsSet( "$ignoreload" ))
	*variables_mask |= ( 1 << i ); i++;
    if ( varIsSet( "$noargs" ))
	*variables_mask |= ( 1 << i ); i++;
    if ( varIsSet( "$sigblock" ))
	*variables_mask |= ( 1 << i ); i++;
    if ( varIsSet( "$unsafeassign" ))
	*variables_mask |= ( 1 << i ); i++;
    if ( varIsSet( "$unsafebounds" ))
	*variables_mask |= ( 1 << i ); i++;
    if ( varIsSet( "$unsafegoto" ))
	*variables_mask |= ( 1 << i ); i++;
    if ( varIsSet( "$showbases" ))
	*variables_mask |= ( 1 << i ); i++;
    if ( varIsSet( "$showunlinked" ))
	*variables_mask |= ( 1 << i ); i++;
    if ( varIsSet( "$hold_next" ))
        *variables_mask |= ( 1 << i ); i++;

    /*
     * Retrieve the case mode 
     */
    *case_mode = casemode;

    /*
     * Retrieve the format for displaying integers 
     */
    *integer_fmt = DEC_INT_FORMAT;
    if ( varIsSet( "$octints" ))
	*integer_fmt = OCT_INT_FORMAT;
    else if ( varIsSet( "$hexints" ))
        *integer_fmt = HEX_INT_FORMAT;

    /*
     * Retrieve the use path
     */ 
    length = 0;
    foreach (String, dir, sourcepath)
       length += strlen( dir ) + 1;
    endfor
    *use_path = malloc( length + 1 );
    (*use_path)[0] = '\0';
    foreach (String, dir, sourcepath)
       strcat( *use_path, dir );
       strcat( *use_path, " " );
    endfor      

    /*
     * Retrieve the value of the debugger variable: $vardim
     */
    if ( varIsSet( "$vardim" )) {
        p = ( Node )findvar(identname("$vardim", false));
        *vardim_value = p->value.lcon;
    }
    else
        *vardim_value = 0;

    /*
     * Retrieve the signals that are currently being caught by dbx.
     * This signal state is returned in two unsigned ints: signals_mask[2].
     * The signal order is defined within <sys/signals.h>. They are
     * contiguious in order up to SIGMSG, where there is a break of 1,
     * and SIGGRANT where there is a break of 23. Return a mask of
     * contiguious bits, one per signal, taking into account these breaks,
     * such that the mask returned represents the order of display
     * within the configuration dialog.
     */
    signals_mask[0] = signals_mask[1] = 0;
    sigset = process->sigset[0];
    sigret = &(signals_mask[0]);
    for (s = 1; s < sizeof(signames) / sizeof(signames[0]); s++) {
	if ( s <= wordsize ) {   /* process the first word of signal bits */
   	    if (( sigset & bitshift(s)) && ( signames[s] != nil )) {
	        if (( s >= SIGHUP ) && ( s <= SIGXFSZ ))
                    *sigret |= bitshift(s);
	        else if (( s >= SIGMSG ) && ( s <= SIGPRE ))
                    *sigret |=  bitshift(s - 1);  /* Take SIGMSG break into account */
	    }
	}
	else {   /* s > wordsize : process the second word of signal bits. */
	         /* Take SIGMSG break into account for bit setting. */
            sigset = process->sigset[1];
	    sigret = &(signals_mask[1]);	  
   	    if (( sigset & bitshift(s - wordsize)) && ( signames[s] != nil )) {
	        if (( s >= SIGMSG ) && ( s <= SIGPRE )) {
		    if ( s == SIGDANGER ) {  /* Set the high order bit of signals_mask[0] */
                        sigret = &(signals_mask[0]);
                        *sigret |=  bitshift(s - 1);
		    }
		    else 
                        *sigret |= bitshift(s - wordsize - 1); 
		}
		else if (( s >= SIGGRANT ) && ( s <= SIGSAK ))
                    *sigret |=  bitshift(s - wordsize - 24 );  /* Take SIGGRANT break into account */
	    }
	}
    }
}

