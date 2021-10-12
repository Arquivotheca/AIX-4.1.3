static char sccsid[] = "@(#)99	1.1  src/bos/usr/lbin/fddi_dd/main.c, sysxfddi, bos411, 9428A410j 3/30/94 15:35:16";
/*
 *   COMPONENT_NAME: sysxfddi
 *
 *   FUNCTIONS: da_lookup
 *		dsp_entry
 *		dsp_entrylist
 *		dsp_help
 *		hex_dmp
 *		main
 *		numchk
 *		prompt
 *		scan
 *		shell
 *		streq
 *		streq_cn
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/dump.h>
#include <sys/wait.h>


typedef void (*sigtype)();
static struct cdt0 head;
static struct cdt_entry cdt_entry[100];
static ulong bmoffset[100];       /* Bitmap offsets of data areas  */
static int num_entries;
static int kmem = -1;             /* fd for /dev/kmem or dump file */
static int offset;                /* offset in the dump file       */
static int BMoffset;              /* BITMAP offset                 */
static int lflag = FALSE;         /* true if in interactive mode   */
extern int errno;


/*****************************************************************************/
/*
 * NAME:     main
 *
 * FUNCTION: The main routine of the dumpfmt dump formatter.
 *	     The following flags may be passed into the main routine:
 *
 *      -f fd   - The file descriptor of the dump file. The file pointer is 
 *		  positioned at the beginning of the Component Dump Table
 *		  for the selected component.
 *
 *      -l      - Interactive mode. Currently, this formatter formats all the 
 *		  data of the component and exits, there is no extra work for
 *		  running the interactive mode or not, so this flag is valid 
 *		  but meaningless.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      Bid up by crash command
 *
 * INPUT:
 *      argc            - number of parameters
 *      argv            - array of parameters
 *
 * RETURNS:
 *      0 - OK
 *      non-0 - error exit
 */
/*****************************************************************************/
main(argc, argv)
int argc;
char *argv[];

{
    extern char *optarg;
    int ch;
    int i;
    int doffset;
    struct cdt_entry	tmp_entry;
    
    
    while ((ch = getopt(argc, argv, "f:l")) != EOF) {
	switch((char)ch) {
	case 'f':			
	    /* file descriptor of the dump file is followed */
	    kmem = atoi(optarg);
	    break;
	    
	case 'l':
	    lflag = TRUE;
	    break;
	    
	default:
	    /* illegal option */
	    exit(1);
	    break;
	    
	}
	
    } 
    
    if (kmem == -1) {
	printf("\ndumpfmt: The file descriptor of the dump is required\n");
	exit(1);
    }
    
    /*
     *  Set the base offset of our component entries
     */
    offset = lseek(kmem, 0, 1);   /* start of this component dump table */

    /*
     *  Read the cdt_head structure
     */
    if (read(kmem, &head, sizeof(head)) != sizeof(head)) {
	printf("\ndumpfmt: Can't read the dump file, offset=0x%x, errno=%d\n",
	       offset, errno);
	exit(1);
    }
    if (head.cdt_magic != DMP_MAGIC) {
	printf("\ndumpfmt: bad magic: 0x%x offset 0x%x\n", head.cdt_magic, 
	       offset);
	exit(1);
    }

    /*
     *  See how many entries are in our component
     */
    num_entries = NUM_ENTRIES(&head);

    BMoffset = offset + sizeof(head) + num_entries * sizeof(struct cdt_entry);

    /*
     *  Read all of the entries into the cdt_entry array
     */
    bzero(&tmp_entry, sizeof(tmp_entry));
    for (i = 0; i < num_entries; i++) {
	if (read(kmem, &tmp_entry, sizeof(struct cdt_entry)) != 
	    sizeof(struct cdt_entry))
	{
	    printf("\ndumpfmt: Can't read the dump file");
	    printf(", offset=0x%x, errno=%d, doffset=0x%x, i=%d\n", 
		   offset, errno, doffset, i);
	    exit(1);
	}
	cdt_entry[i] = tmp_entry;
	bmoffset[i] = BMoffset + BITMAPSIZE(tmp_entry.d_ptr,tmp_entry.d_len);
	BMoffset += BITMAPSIZE(tmp_entry.d_ptr,tmp_entry.d_len);
	BMoffset += cdt_entry[i].d_len;

    }

    /*
     *  If the list flag is set then do interactive mode, otherwise just
     *  dump the entire area.
     */
    if (lflag) {
	/*
	 *  Go into interactive mode
	 */
	scan();
    } else {
	/*
	 *  Dump all of the CDT entries
	 */
	for (i = 0; i < num_entries; i++) {
	    dsp_entry(i);
	}
    } /* End of else !lflag */

} /* End of main */

/*
 * NAME:     scan
 * FUNCTION: subcommand routine
 * INPUTS:   none (input from Cmdfp)
 * RETURNS:  none
 *
 *    Get subcommand
 *    If the subcommand starts with '\',
 *      treat it as the name of a data area. Look it up through da_lookup()
 *      and if it exists, display it with dsp_entry.
 *    If the subcommand is a number, it is the number of a data area within
 *      the current component. Display with dsp_entry.
 *    If the subcommand is not a keyword (such as list, quit, etc)
 *      treat as the name of a data area. Look it up through da_lookup()
 *      and if it exists, display it with dsp_entry. Otherwise, display a help
 *      message.
 *    Valid keywords are:
 *      list             List data areas in current component.
 *      all              Display all data areas in current component.
 *      ?, help          Display help screen.
 *      ! [shell cmd]    Escape to shell command.
 *      quit             Quit.
 *
 * Note: keyword matching is case-independent. Also, a match occurs if
 *       the scanned keyword matches the first n characters of a keyword,
 *       so that 'qui' matches 'quit'.
 */
scan()
{
    int i;
    int n;
    char *cp;
    char *line;
    char linebuf[128];
    
    dsp_entrylist();
    for(;;) {
	prompt();
	line = linebuf;
	fgets(line,128,stdin);
	
	/*
	 *  If it is a search pattern then search for that name in the
	 *  list of entries.  If found then display that entry.
	 */
	if(*line == '\\') {	/* leading '\' overrides keywords */
	    line++;
	    if(*line != '\\') {
		if((cp = strtok(line," \t\n")) && (n = da_lookup(cp)) >= 0) {
		    dsp_entry(n);
		    continue;
		}
	    }
	}

	/*
	 *  If a blank line then just continue (or if just did a search).
	 */
	if((cp = strtok(line," \t\n")) == 0)
	    continue;

	/*
	 *  If a number was specified and it was a valid number then
	 *  display that entry.
	 */
	if(numchk(cp)) {
	    n = atoi(cp);
	    if ((n == 0 ? 0 : n-1) < num_entries) {
		dsp_entry(n == 0 ? 0 : n-1);
	    }
	    continue;
	}

	/*
	 *  If list was specified then redisplay the list of entries for this
	 *  component.
	 */
	if(streq_cn(cp,"list")) {
	    dsp_entrylist();
	    continue;
	}

	/*
	 *  If all was specified then display all of the entries.
	 */
	if(streq_cn(cp,"all")) {
	    for(i = 0; i < num_entries; i++) {
		dsp_entry(i);
	    }
	    continue;
	}
	
	/*
	 *  display the help screen.
	 */
	if(streq_cn(cp,"?") || streq_cn(cp,"help")) {
	    dsp_help();
	    continue;
	}
	
	/*
	 *  Execute the shell command
	 */
	if(streq_cn(cp,"!")) {
	    cp = cp + strlen(cp) + 1;
	    while(*cp && isspace(*cp))
		cp++;
	    shell(*cp ? cp : 0);
	    continue;
	}

	/*
	 *  Quit
	 */
	if(streq_cn(cp,"quit")) {
	    break;
	}

	/*
	 *  Default is to search for this entry and display it if found.
	 */
	if((n = da_lookup(cp)) >= 0) {
	    dsp_entry(n);
	    continue;
	}
	dsp_help();
    }
    return(0);
} /* End of scan() */

/*
 * NAME:     dsp_entry
 * FUNCTION: Call dump routine for cdt_entry[i]
 * INPUTS:   none
 * RETURNS:  none
 */
dsp_entry(i)
    int i;
{
    int doffset;

    lseek(kmem, bmoffset[i], 0);
    
    /*
     *  Call media specific dump routine to format the entry and display
     *  it.
     */
    if (dmp_entry(kmem, &cdt_entry[i]) < 0) 
    {
	printf("\ndumpfmt1: Can't read the dump file");
	printf(", offset=0x%x, errno=%d\n", doffset, errno);
	exit(1);
    }

    /*
     *  Return back to the beginning of our component entry
     */
    lseek(kmem, offset, 0);

} /* End of dsp_entry() */

/*
 * NAME:     dsp_help
 * FUNCTION: display help screen
 * INPUTS:   none
 * RETURNS:  none
 */
static dsp_help()
{
    printf("\
<number>         display selected data area (number is 1-based)\n\
list             list all data areas\n\
all              display all data areast\n\
quit             exit\n\
?                show this help screen\n\
!                escape to shell\n");

} /* End of dsp_help() */

/*
 * NAME:     dsp_entry_list
 * FUNCTION: display list of entries for this component
 * INPUTS:   none
 * RETURNS:  none
 */
static dsp_entrylist()
{
    int	i;

    for (i=0; i < num_entries; i++) {
	printf("%d) %s\n", i+1, cdt_entry[i].d_name);
    }
}

/*
 * NAME:     prompt
 * FUNCTION: Output the subcommand prompt to stdout
 * INPUTS:   none
 * RETURNS:  none
 *
 */
prompt()
{
    
    printf("-> ");
    fflush(stdout);	/* fflush is necessary because there is no '\n' */
}

/*
 * NAME:     da_lookup
 * FUNCTION: Find the slot in the cdt_entry whose data area name matches
 *           the supplied name.
 * INPUTS:   name           name of data area
 * RETURNS:  index into cdt_entry[] of data area 'name'
 *           -1 if not found
 */
static
da_lookup(name)
char *name;
{
    int i;
    
    for(i = 0; i < num_entries; i++)
	if(streq(cdt_entry[i].d_name,name))
	    return(i);
    return(-1);
} /* End of da_lookup() */

/*
 * NAME:     numchk
 * FUNCTION: return true if the string is a valid decimal number
 * INPUTS:   str     character strings to check
 * RETURNS:  0 if not a number. non-zero otherwise.
 */
numchk(str)
char *str;
{
    char *cp;
    int c;
    
    cp = str;
    if(*cp == '-') {		/* leading -  as in   -35 */
	cp++;
	if(*cp == '\0')		/* filter out a '-' by itself */
	    return(0);
    }
    while(c = *cp++)
	if(!isdigit(c))
	    return(0);
    return(1);
} /* End of numchk() */

/*
 * NAME:      streq
 * FUNCTION:  determine if two strings have the same value
 * INPUTS:    s1,s2
 * RETURNS:   1 if equal (non-zero)
 *            0 if not equal
 *
 * Note: this routine is written for expressions like:
 *    if(streq(optarg,"stdin"))
 *           ...
 * which is closer to the way it is thought of than:
 *    if(strcmp(optarg,"stdin") == 0)
 *           ...
 */
streq(s1,s2)
register char *s1,*s2;
{
	register char c;

	while(c = *s1++)
		if(c != *s2++)
			return(0);
	return(*s2 == '\0' ? 1 : 0);
} /* End of streq() */

/*
 * NAME:     streq_cn
 * FUNCTION: return true if strings match.
 *           case insensitive.
 *           If s1 is shorter than s2 (and s1 matches s2), this is a match.
 * INPUTS:   character strings s1 and s2
 * RETURNS:  0 if no match. non-zero if match.
 */
streq_cn(s1,s2)
char *s1,*s2;
{
	char c1;
	char c2;

	while(c1 = *s1++) {
		c2 = *s2++;
		if(c1 == c2)
			continue;
		if(isupper(c1))
			c1 = _tolower(c1);
		if(isupper(c2))
			c2 = _tolower(c2);
		if(c1 == c2)
			continue;
		return(0);
	}
	return(1);
} /* End of streq_cn() */

/*
 * NAME:     shell
 * FUNCTION: Shell escape routine
 * INPUTS:   cmd  command line string to exec via sh -c
 * RETURNS:  return value from wait
 *           -1   cannot fork
 *
 * If 'cmd' is 0 or "", the shell is exec-ed directly with no -c option.
 */

/*
 * Execute the command string in 'cmd' as input to a shell.
 * For example, if 'cmd' = "echo hello world",
 *  what will be exec-ed here after the fork is
 * sh -c echo hello world
 * The shell will take care of parsing the command line.
 * If there is no command string in 'cmd', just exec the shell.
 * Signals are set to SIG_DFL in the child process.
 */

shell(cmd)
char *cmd;
{
    int i;
    int pid;
    int rv;
    int status;
    sigtype sigintsv;
    sigtype sigquitsv;
    
    if((pid = fork()) < 0) {		/* fork */
	perror("fork");	/* this error is very rare */
	return(-1);
    }
    if(pid) {						/* parent */
	sigintsv  = (sigtype) signal(SIGINT,SIG_IGN);
	sigquitsv = (sigtype) signal(SIGQUIT,SIG_IGN);
	while((rv = waitpid(pid,&status,WUNTRACED)) != pid) {
	    if(rv == 0) {
		break;
	    }

	    if(errno == EINTR)
		continue;
	}
	signal(SIGINT,(void(*)(int)) sigintsv);
	signal(SIGQUIT,(void(*)(int)) sigquitsv);
	return(status);
    }
    if(cmd == 0 || cmd[0] == '\0')
	cmd = "ksh";
    for(i = 3;i < 20;i++)   /* close files except for stdin,out,err */
	close(i);
    signal(SIGINT,SIG_DFL);
    signal(SIGQUIT,SIG_DFL);
    execl("/bin/ksh","ksh","-c",cmd,0);
    _exit(127);
} /* End of shell() */

/*****************************************************************************/
/*
 * NAME:     hex_dmp
 *
 * FUNCTION: Format a block of data in a hex with ascii format.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      fmt_dd_ctl 
 *	fmt_dev_ctl
 *	fmt_ndd
 *	fmt_dds
 *	fmt_wrk
 *
 * INPUT:
 *	pad	- a string for output indentation 
 *	adr	- starting address of the data area
 *	len	- length of the data area to be formatted
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
hex_dmp(pad, adr, len)
    char	*pad;
    char	*adr;
    int		len;

{
    
    int		i, j, offset;
    char	*dp;
    ulong	*lp;
    char	buf[20];
    
    
    
    printf("\n");
    
    offset = 0;
    dp = adr;
    buf[0] = '|';
    
    /*
     * Format 16 bytes of data per line with its hex offset in the front.
     * The buf is used as the ascii string buffer for the ascii display 
     * on the right.
     */
    while (len >= 16) {
	j = 1;		/* index for buf */
  	for (i = 1; i <= 16; i++) {
	    if (*dp >= 0x20 && *dp <= 0x7e) 
		buf[j++] = *dp;
	    else
		buf[j++] = '.';
	    dp++;
	}
	buf[j++] = '|';
	buf[j] = '\0';
	
	lp = (ulong *)adr;
	printf("%s%08x: %08x %08x %08x %08x    %s\n", pad, offset, *lp, 
	       *(lp + 1), *(lp + 2), *(lp + 3), buf);
	
	len -= 16;
	offset += 16;
	adr += 16;
    }
    
    /*
     * Format the last line with less than 16 bytes of data.
     */
    if (len) {
  	j = 1;		/* index for buf */
  	lp = (ulong *)adr;
  	printf("%s%08x: ", pad, offset);
  	for (i = 1; i <= len; i++) {
	    if (*dp >= 0x20 && *dp <= 0x7e)  
		buf[j++] = *dp;
	    else
		buf[j++] = '.';
	    printf("%02x", *dp);
	    dp++;
	    if (i % 4 == 0)
		printf(" ");
  	}
	
  	for (; i < 16; i++) {
	    printf("  ");
	    buf[j++] = ' ';
	    if (i % 4 == 0)
		printf(" ");
	}
  	buf[j++] = ' ';
  	buf[j++] = '|';
  	buf[j] = '\0';
	
  	printf("      ");
  	printf("%s\n", buf);
	
    }
} /* End of hex_dump() */
