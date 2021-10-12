static char sccsid[] = "@(#)33    1.5.1.15  src/bos/usr/ccs/lib/libdbx/dpi_memory.c, libdbx, bos411, 9428A410j 3/28/94 10:11:57";
/*
 * COMPONENT_NAME: (LIBDBX) - dbx symbolic debugger library
 *
 *   FUNCTIONS: dpi_calc_effective_addr
 *		dpi_check_heap_and_stacks
 *		dpi_get_address_of_line
 *		dpi_get_file_length
 *		dpi_get_func_size
 *		dpi_get_mapinfo
 *		dpi_get_memory
 *		dpi_get_registers
 *		dpi_invalid_regs
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/

#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
#include <setjmp.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/reg.h>
#include <stdarg.h>
/*#include <string.h>*/
#ifdef KDBX
#include <stdlib.h>
#endif /* KDBX */

#include "defs.h"
#include "process.h"
#include "decode.h"
#include "source.h"
#include "mappings.h"
#include "envdefs.h"
#include "ops.h"
#include <sys/proc.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <stddef.h>
#include <errno.h>
#include <procinfo.h>

#ifdef K_THREADS
#include "k_thread.h"
#endif

#define U_OFF(x)        ((int)&(((struct user *)NULL)->x))

typedef struct dpi_ldinfo {
	char	filename[255];
	uint 	textorg;
	uint 	dataorg;
	uint 	textsize;
	uint	datasize;
} MAP;

extern int loadcnt;
extern Process process;
extern Boolean notstdout;

public int *envptr;

/*
 * NAME: dpi_get_registers
 *
 * FUNCTION: 
 *	Reads the general purpose registers, and floating point registers.
 *	The values of all the registers are returned through two input
 *	parameters "gpr" and "fpr".  Space is allocated for both the
 *	parameters that must be freed by the caller.
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES:
 *	There is no need for the standard longjmp setup since setreg(), reg(),
 *	and fpregval() simply make calls to ptrace and do not have any error
 *	conditions.
 *	If -1 is returned then the output variables are not filled in.
 *
 * PARAMETERS:
 *	gpr - output parameter which will contain a vector of values of
 *	      the general purpose (NGREG) and system control registers (NSYS).
 *	      Currently this is 41 registers.
 *	fpr - output parameter which will contain a vector of values of
 *	      the floating point registers (fpregs in number).
 *	      Currently this is 32 registers.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES:
 *	fpregs - number of floating point registers.
 *	process must be set
 *
 * RETURNS:
 *	Success: 0
 *	Failure: -1
 */
int dpi_get_registers(gpr, fpr)
uint 	**gpr;
double 	**fpr;
{
	int i;

	/*
	 * If there's no process then there are no registers to get.
	 */
	if ( ! process )
		return -1;

	/*
	 * Allocate space for the registers.
	 */
	*gpr = (uint  *) malloc( (NGREG+NSYS) * sizeof(uint) );
	*fpr = (double *) malloc ( fpregs * sizeof (double ) );

	/*
	 * force the registers in the ptrace process area
	 * to be up to date
	 */
	setregs(process);

	/*
	 * copy in the gprs and system control regs.
	 */
	for (i=0; i < NGREG+NSYS; i++) {
		(*gpr)[i] = reg(i);
	}	
	
	/*
	 * copy in the fprs
	 */
	for (i=0; i < fpregs; i++) {
		(*fpr)[i] = fpregval(i);
	}

	return 0;
}


/*
 * NAME: dpi_invalid_regs
 *
 * FUNCTION:
 *      Returns a count and an array of invalid registers based
 *      on the current operating hardware.
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES:  This function would need to be modified in the
 *         future if there is a new hardware for which certain registers
 *         are invalid.
 *         It also assumed by its caller that only special purpose
 *         registers are invalid on some architectures.  That is,
 *         all floating point registers are valid on all hardware
 *         architectures.
 *
 *
 * PARAMETERS:
 *      invalid_count - output integer parameter indicating the number
 *                      of invalid registers in the array invalid_regs
 *      invalid_regs  - output parameter to be used to return
 *                      an array containing the numbers of the invalid
 *                      registers for the current operating hardware.
 *                      Space is allocated which must be freed by the caller.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES:
 *      current_hardware must be set
 *
 * RETURNS: NONE
 */

void dpi_invalid_regs(invalid_count,invalid_regs)
int     *invalid_count;
int    **invalid_regs;
{

#define MAX_INVALID_REGS 2
    /* Allocate space for the invalid register array */
    *invalid_regs = (int  *) malloc(MAX_INVALID_REGS * sizeof(int) );

    /* initialize count of number of invalid registers */
    *invalid_count = 0;

    /*
       if current hardware is POWERPC (and not 601 POWERPC),
       then set mq register to be invalid.
    */
    if (current_hardware == PPC)
    {
       (*invalid_regs)[*invalid_count] = SYSREGNO(MQ);
       (*invalid_count)++;
    }

    /*
       if current hardware is POWERPC or 601 POWERPC,
       then set tid register to be invalid.
    */
    if ((current_hardware == PPC) || (current_hardware == SET_601))
    {
       (*invalid_regs)[*invalid_count] = SYSREGNO(TID);
       (*invalid_count)++;
    }
    return;
}

/*
 * NAME: dpi_get_memory
 *
 * FUNCTION: 
 *	Reads the memory from the current process's address space and
 *	upon successful completion returns the memory's value in "memory"
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES:
 *	Space is allocated to hold the memory which must be freed by
 *	the calling function on successful completion.
 *	If an error occurs then -1 is returned and the memory allocated
 *	is freed.
 *
 * PARAMETERS:
 *	address		base address from which to retrieve memory
 *	num_items	number of bytes to read
 *	memory		output, pointer to buffer holding read memory
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES:
 *	process must be set
 *
 * RETURNS:
 *	Success: 0
 *	Failure: -1
 */
int dpi_get_memory(address, num_items, memory)
unsigned int	address;	/* Address to read from */
int		num_items;	/* Number of bytes to read */
char		**memory;	/* A pointer to the read memory */
{
	int  i;
	jmp_buf env;
	int  *svenv;

	/*
	 * standard longjmp error recovery setup
	 */
	if (! process )
		return -1;
	svenv = envptr;
	envptr = env;
	switch ( setjmp(env) ) {
	/* longjmp error cases */
	case ENVCONT:
	case ENVEXIT:
	case ENVQUIT:
	case ENVFATAL:
		envptr = svenv;
		dispose( *memory );
		return -1;
	/* no error occurred */
	default:
		break;
	}

	/*
	 * Allocate space for the memory.
	 */
	*memory = malloc ( (uint) num_items );

	/*
	 * get the memory and check for normal errors
	 */
	if ( dread(*memory, address, num_items) ) {
		dispose( *memory );
		envptr = svenv;
		return -1;
	}
	else	{
		envptr = svenv;
		return 0;
	}
}


/*
 * NAME: dpi_get_address_of_line
 *
 * FUNCTION: 
 *	Fill in "address" with the machine address of the given file
 *	line number.  If there was an error or if the address couldn't
 *	be found then "address" is set to nil.
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * PARAMETERS:
 *	line		source line number
 *	filename	name of file containing the line
 *	address		output - the address of the specified source line
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * RETURNS:
 *	Success: 0
 *	Failure: -1
 */
int dpi_get_address_of_line(line, filename, address)
int	line;
char 	*filename;
uint	*address;
{
	jmp_buf env;
	int  *svenv;

	/*
	 * standard longjmp error recovery setup
	 */
	svenv = envptr;
	envptr = env;
	switch ( setjmp(env) ) {
	case ENVCONT:
	case ENVEXIT:
	case ENVQUIT:
	case ENVFATAL:
		envptr = svenv;
		*address = nil;
		return -1;
	default:
		break;
	}

	*address = objaddr (line, filename);
	if ( *address == NOADDR ) {
		*address = nil;
		envptr = svenv;
		return -1;
	}
	else	{
		envptr = svenv;
		return 0;
	}
}


/*
 * NAME: dpi_calc_effective_addr
 *
 * FUNCTION: 
 *	Calculate the effective address from the expression of the
 *	form:
 *
 *		offset(register_number)
 *
 *	where offset could be any valid expression, and register_number
 *	must be one of the register numbers as defined in the sys/reg.h.
 *
 *	Result of the expression = offset + contents of register_number.
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES:
 *	If the input expression is not correctly formed then -1 will
 *	be returned.
 *
 * PARAMETERS:
 *	expression	input expression to be evaluated.
 *	address		output - effective address of the input expression.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * RETURNS:
 *	The value of the expression is returned through the pointer
 *	variable result if successful.
 *
 *	Success: 0
 *	Failure: -1
 */
int dpi_calc_effective_addr(expression, result)
char 	*expression;
uint	*result;
{
	char *p;
	char *q;
	char *r;
	char *res;
	char cmd[80];

	/*
	 * In order to calculate the effective address, we add a '+'
	 * character before the '(', and a '$' before the register
	 * number, then build the dbx command 
	 * "print offset+($register_number)\n"
	 * and then use the dpi function get_dbx_result to evaluate the
	 * command.
	 */
	
	q = expression;
	p = strchr( expression, '(' );  /* Look for the '(' */
	if ( p == NULL )	/* if no '(' then input was bad      */
		return ( -1 );
	strcpy( cmd, "print " );
	r = cmd + 6;		/* this is the length of "print "    */
	
	/* Copy the offset expression */
	while ( q < p )	
		*r++ = *q++;

	*r++ = '+';		/* Add '+' after the offset */
	*r++ = '(';
	*r++ = '$';		/* Add $ to the register name , there should
				   not be a single blank between $ and regno.*/
	q = p+1;
	while ( *q == ' ' )	/* skip the blanks */
		q++;
	p = strchr( q, ')' );
	if ( p == NULL )	/* if no ')' then input was bad */
		return ( -1 );
	while ( q <= p )
		*r++ = *q++;
	*r++ = '\n';
	*r = '\0';
	if ( get_dbx_result(cmd, NOFORMAT, &res) == -1 )
		return -1;
	/*
	 * convert the ascii result to int
	 */
	*result = strtoul(res, NULL, 0);
	free( res );
	return 0;
}

/*
 * NAME : dpi_get_mapinfo
 *
 * FUNCTION: This function is used to get the information about the loaded
 *	objects, i.e address of beginning of text section, size of
 *	text section etc. 
 *	
 *	It basically reorganizes the information present in the
 *	dbx data structure loader_info, for xde/adc.
 *	loader_info is defined in process.h.
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES:
 *	Storage is allocated for mapinfo that must be freed by the caller
 *	on successful invocations.
 *
 * PARAMETERS:  
 *	mapinfo : output - is used to return the array containing the map info.
 *	count   : output - tells xde/adc, about the number of loaded objects.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES:
 *	process must be set
 *
 * RETURNS:
 *	   SUCCESS : returns 0;
 *	   FAILURE : returns -1;
 */
int dpi_get_mapinfo(mapinfo, count)
MAP 	**mapinfo;
int	*count;
{
	MAP	*loadmap;
	int	i;

	*count = loadcnt;

	/* If there is no process, then the loadcnt == 0 */
	if (! process )
		return -1;

	/*
	 * Allocate space for the mapinfo and fill it in for all
	 * the loaded segments.
	 */
	*mapinfo = (MAP *)malloc(loadcnt * sizeof(MAP));
	loadmap = *mapinfo;
	for(i=0; i< loadcnt; i++) {
		loadmap[i].textorg = loader_info[i].textorg;
		loadmap[i].textsize = loader_info[i].textsize;
		loadmap[i].dataorg = loader_info[i].dataorg;
		loadmap[i].datasize = loader_info[i].datasize;
		strcpy(loadmap[i].filename, fd_info[i].pathname);
	}
	return 0;
}

/*
 * NAME : dpi_get_file_length
 *
 * FUNCTION: This function returns the number of lines in the current file.
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * PARAMETERS: NONE
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES:
 *	process must be set
 *
 * RETURNS:
 *	SUCCESS : linecount;
 *	FAILURE : -1;
 */
int dpi_get_file_length()
{
	extern	Lineno	lastlinenum;

	if (!process)
		return -1;
	if( canReadSource() == false ) {
		return -1;
	}
	return(lastlinenum);
}

/*
 * NAME: dpi_get_func_size
 *
 * FUNCTION: returns the start and end addresses of the function that the
 *	input address is in.  If those addresses cannot be determined,
 *	then the start and end addresses of the load segment that the
 *	input address is in is returned.
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * PARAMETERS:
 *	address :	address for which to find the function information
 *	start   :	output - starting address of the function containing
 *				 the input address.
 *	end     :	output - ending address of the function containing
 *				 the input address.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES:
 *	process must be set
 *
 * RETURNS: 
 *	SUCCESS : 0;
 *	FAILURE : -1;
 */
int dpi_get_func_size(address, start, end)
unsigned	address;
unsigned	*start;
unsigned	*end;
{
	if (!process || !getfuncinfo(address, start, end))
		return -1;
	
	return(0);
}


/*
 * NAME: dpi_qtoa
 *
 * FUNCTION: 
 *	Convert the input quad precision floating point number to
 *	ascii.
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES: space is allocated to hold the returned ascii representation
 *	of the quad float number.  This space must be freed by the
 *	calling routine.
 *
 * PARAMETERS:
 *	q	- input quad number.
 *	result	- output ascii representation of q.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * RETURNS:
 *	The ascii representation of q is returned via result.
 *
 *	Success: 0
 *	Failure: -1
 */
int dpi_qtoa(q, result)
quadf 	q;
char	**result;
{
	msgbegin;
	printquad (rpt_output, stdout, q);
	msgend( *result );
	return 0;
}

/*
 * NAME: dpi_check_heap_and_stacks
 *
 * FUNCTION: Determines whether a passed address is in the heap or stacks.
 *           If it is, then returns the start and end of the heap or stack.
 *
 * RECOVERY OPERATION: none
 *
 * PARAMETERS:
 *      address  - address to be checked (input)
 *      maxdval  - end of last data segment (input)
 *      stacktop - top of stack (input)
 *      start    - if the passed address is in the heap or a stack, then the
 *                 start of the heap or stack is returned in this
 *                 parameter.
 *                 Otherwise, 0 is returned in this parameter.
 *                 (output)
 *      end      - if the passed address is in the heap or a stack, then the
 *                 end of the heap or stack is returned in this
 *                 parameter.
 *                 Otherwise, 0 is returned in this parameter.
 *                 (output)
 *
 * DATA STRUCTURES: none
 *
 * RETURNS: void
 *      start and end parameters are returned as defined above.
 *
 */
void    dpi_check_heap_and_stacks(uint address,
                                  uint maxdvalue,
                                  uint stacktop,
                                  uint *start,
                                  uint *end)
{

    int i;
    uint                 stackend;
    uint                 heap_end;
    uint                 dvalue;
    pid_t                pid;
    *start = *end = 0;
    /*
     * force the registers in the ptrace process area
     * to be up to date
     */
    setregs(process);

    /* get the value of the stack pointer register */
    stackend = reg(GPR1);

    /* See if it is on the stack */
    if (address <= stacktop && address >= stackend)
    {
        *start = stackend;
        *end = stacktop;
    }
    else
    {
        /*
            Determine if address is in the heap
        */
        /* find size of heap */
        pid = dpi_report_newProcessId();
        if (pid != -1)
        {
#ifdef  PT_READ_U
            heap_end = ptrace(PT_READ_U, pid,
                              (int *)U_OFF(u_dsize), NULL, NULL);
            if (heap_end == -1)  heap_end = 0;
#else
            heap_end = getheapsize(pid);
#endif
            if (heap_end != 0)
            {
                heap_end += BASE(DATASEG) - 1;
                if (address >= maxdvalue && address <= heap_end)
                {
                    *start = maxdvalue;
                    *end = heap_end;
                }
#ifdef K_THREADS
                else
                {
                    if (lib_type == KERNEL_THREAD)
                    {
                        check_thread_stacks(address,start,end);
                    }
                }
#endif
            }
        }
    }
}
