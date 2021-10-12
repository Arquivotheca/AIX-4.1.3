static char sccsid[] = "@(#)98  1.22  src/bos/usr/ccs/lib/libodm/invoke.c, libodm, bos41B, 9504A 1/17/95 13:28:48";
/*
 * COMPONENT_NAME: (LIBODM) Object Data Manager library
 *
 * FUNCTIONS: odm_run_method
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************************************************************************
**********************          INVOKE          *******************************
*******************************************************************************
FUNCTION :
        This procedure executes the specified program by forking a child
                process.  If specified, this procedure will trap the child's
                output (either stdout, stderr, or both), and return a pointer
                to a buffer containing the output.  Upon completion, this
                procedure will return the child's exit status - note that
                because of the "wait" system call, only the lower 8 bits
                of the child's exit status is returned, so negative exit
                codes are lost; therefore, all returned codes are 0 - 255

PARAMETERS :
        progname        - string containing name of program to execute
        parameters      - string containing arguments for the "progname"
        out_ptr         - address of a char pointer in the calling routine
                                which will be initialized to the "progname"s
                                stdout output
                          if NULL, the child's stdout will not be piped
        err_ptr         - address of a char pointer in the calling routine
                                which will be initialized to the "progname"s
                                stderr output
                          if NULL, the child's stderr will not be piped

RETURNED VALUES :
        -1 if internal errors encountered, otherwise, the child's exit status
        the child's standard output in the buffer pointed to by out_ptr
        the child's standard error in the buffer pointed to by err_ptr

Notes :
        The process of trapping the child's output works in the following way:
                a pipe is setup between the child & the parent
                the child is forked
                the parent uses the SELECT system call to determine whether
                        there is any output to read
                the next reads after the child's death will return EOF, which
                        causes the read loop to terminate
		the "wait" system call is executed to get the child's exit
                the child's exit status is returned

Pseudo Code
-----------
check for a NULL program to execute
initialize any internal variables
generate one string which has the program to execute & its parameters
if (stdout is to be trapped)
   setup a pipe for stdout
if (stderr is to be trapped)
   setup a pipe for stderr
ignore SIGINT & SIGQUIT
fork the child
for the child
{       if (stdout is piped)
        {  close( stdout )
           duplicate that file descriptor on the pipe's descriptor
        }
        if (stderr is piped)
        {  close( stderr )
           duplicate that file descriptor on the pipe's descriptor
        }
        execlp the program
}
for the parent
{       setup a SIGCLD catcher to process the child's termination
        restore SIGINT & SIGQUIT
        if (output is to be trapped)
           initialize buffer pointers & variables
        while (trap_out or trap_err)
        {  use SELECT to determine whats ready to be read
           if (stdout ready)
           {  read from child's stdout
              if (EOF)
                 trap_out = FALSE
           }
           if (stderr ready)
           {  read from child's stderr
              if (EOF)
                 trap_err = FALSE
           }
        }
        return the child's exit status
}
******************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/access.h>
#include <values.h>
#include <errno.h>
#include <odmi.h>
#include "odmlib.h"
#include "odmtrace.h"
#include "odmhkids.h"

extern char *malloc();          /* memory allocation */
extern char *realloc();         /* memory reallocation */



#define BUFFER_SIZE     1000
#define FALSE           0
#define TRUE            1

static int status;


/**************************     START OF CODE ********************************/
odm_run_method ( progname, parameters, out_ptr, err_ptr )
char *progname;                         /* progname pathname */
char *parameters;                       /* progname parameter string */
char **out_ptr;                         /* ptr to output buffer ptr */
char **err_ptr;                         /* ptr to error buffer ptr */
{
    int m_stdout[2], m_stderr[2];       /* progname file descriptors*/
    char *command;                      /* command string to execute */
#if 0
    char *real_progname;                /* real program name */
    char * progname_ptr;		/* pointer to the progname */
#endif
    int c_pid;                          /* child's pid */
    int w;                              /* temp vars */
    int out_num,err_num;                /* # of chars read */
    char *out_tmp, *err_tmp;            /* temp buffer ptrs */
    char *out_end,*err_end;             /* ptr to end of buffer */
    int outb_size,errb_size;            /* buffer size */
    int out_count, err_count;           /* # of buffers allocated */
    int trap_out,trap_err;              /* flags; if >0, trap output */
    int rdmask[1 + _NFILE/32];          /* for select system call */
    int wrmask[1 + _NFILE/32];          /* for select system call */
    int exmask[1 + _NFILE/32];          /* for select system call */
    int i;                              /* loop indexing */

#if R5A
    int (*istat)(),(*qstat)(),          /* temp vars for signal status */
        (*cstat)();
#define ulong unsigned long
#else

    struct sigaction action,            /* parameters of sigaction */
           oldsigint_act,
           oldsigquit_act,
           oldsigcld_act;

#endif
    START_ROUTINE(ODMHKWD_INVOKE);
    TRC("odm_run_method","Running program %s",progname,"params %s",parameters);
    TRC("odm_run_method","out_ptr %x",out_ptr,"err_ptr %x",err_ptr);

    status = 0;

    /* trap output??? */
    if ( out_ptr != NULL)
      {
	trap_out = 1;
        *out_ptr = NULL;
      }
    else trap_out = 0;

    /* trap err??? */
    if (err_ptr != NULL)
      {
        trap_err = 1;
        *err_ptr = NULL;
      }
    else trap_err = 0;

    /* there is something to start (isn't there???) */
    if (!progname || progname[0] == '\0')
      {
        TRC("odm_run_method","Progname is NULL!","","","");
        odmerrno = ODMI_PARAMS;
        STOP_ROUTINE;
        return( -1 );
      }
#if 0
    /* Check the accessibility of the progname */
    real_progname = malloc( strlen(progname) + 1);
    if (real_progname == NULL)
          {
            TRC("odm_run_method","Could not malloc progname! err %d",errno,"","");
            odmerrno = ODMI_MALLOC_ERR;
            return(-1);
          } /* endif */

    strcpy(real_progname,progname);
    progname_ptr=strtok(real_progname," \t");
    if (access(progname_ptr,R_OK|X_OK) < 0)
     	 {
            TRC("odm_run_method","Unable to access the Progname!","","","");
            odmerrno = ODMI_PARAMS;
            STOP_ROUTINE;
	    free(real_progname);  /* free up the space for real_progname */
            return( -1 );
     	 }
     free(real_progname);	/* free up the space for real_progname */
#endif

    /* initialize internal variables */
    out_num = err_num = 0;

    /* generate command line */
    command = malloc( strlen(progname) + strlen(parameters) + 2 );
    if (command == NULL)
      {
        TRC("odm_run_method","Could not malloc command! err %d",errno,"","");
        odmerrno = ODMI_MALLOC_ERR;
        return(-1);
      } /* endif */

    sprintf( command, "%s %s", progname, parameters );

    TRC("odm_run_method","Command is %s",command,"","");

    /* setup pipes if specified */
    if (trap_out)
      {
        TRC("odm_run_method","Trapping stdout","","","");

        if ( pipe(m_stdout) < 0 )
          {
            TRC("odm_run_method","Could not open stdout pipe! err %d",errno,
                "","");
            odmerrno = ODMI_OPEN_PIPE;
	    free(command); /* free up the space for command */
            STOP_ROUTINE;
            return( -1 );
          }
      }

    if (trap_err)
      {

        if ( pipe(m_stderr) < 0 )
          {
            TRC("odm_run_method","Could not open stderr pipe! err %d",errno,
                "","");
            if (trap_out)
                close(m_stdout[0]);
            odmerrno = ODMI_OPEN_PIPE;
            STOP_ROUTINE;
	    free(command); /* free up the space for command */
            return( -1 );
          }
      }


#ifdef R5A

    /* ignore these signals */
    istat = signal( SIGINT, SIG_IGN );
    qstat = signal( SIGQUIT, SIG_IGN );
#else
    /* ignore these signals */
    memset (&action, '\0', sizeof (struct sigaction));
    memset (&oldsigquit_act, '\0', sizeof (struct sigaction));
    memset (&oldsigint_act, '\0', sizeof (struct sigaction));

    action.sa_handler = SIG_IGN;
    sigaction (SIGINT, &action, &oldsigint_act);

    action.sa_handler = SIG_IGN;
    sigaction (SIGQUIT, &action, &oldsigquit_act);
#endif

    TRC("odm_run_method","Forking method","","","");
    /* fork progname */
    if ( (c_pid = kfork()) == 0 )
      { /*------------------------- child process --------------------------*/

        if (trap_out)
          {  /* duplicate stdout */
	    if (dup2( m_stdout[1], 1 ) != 1)
		{
		odmerrno = ODMI_INTERNAL_ERR;	
            	close( m_stdout[0] );
		STOP_ROUTINE;
		return( -1 );
		}
	    close( m_stdout[0] );	
            close( m_stdout[1] );
          }

        if (trap_err)
          {  /* duplicate stderr */

	    if (dup2( m_stderr[1], 2) != 2)
		{
		odmerrno = ODMI_INTERNAL_ERR;
		close( m_stderr[0] );
		STOP_ROUTINE;
		return( -1 );		
		}	
	     close( m_stderr[0] );
	     close( m_stderr[1] );		
          }

        /* start the program */
        execlp( "/bin/sh", "sh", "-c", command, (char *) 0 );

        _exit( -1 );
      }
    else if (c_pid == -1)
      {
        TRC("odm_run_method","Unable to fork routine! err %d",errno,"","");
        if (trap_err)
            close(m_stderr[0]);

        if (trap_out)
            close(m_stdout[0]);

        odmerrno = ODMI_FORK;
        STOP_ROUTINE;
	free(command); /* free up the space for command */
        return( -1 );
      }

    /*--------------------------- parent process --------------------------*/


#ifdef R5A

    /* restore these signals */
    signal( SIGINT, istat );
    signal( SIGQUIT, qstat );

#else

    memset (&action, '\0', sizeof (struct sigaction));
    memset (&oldsigcld_act, '\0', sizeof (struct sigaction));

    /* restore signals */
    sigaction (SIGINT,&oldsigint_act, NULL);
    sigaction (SIGQUIT, &oldsigquit_act, NULL);

#endif


    free(command); /* free up the space for command */

    /* close the write side of the pipe for the parent */
    if (trap_out)
      {
        TRC("odm_run_method","Closing write side of stdout pipe","","","");
        close( m_stdout[1] );
        fcntl( m_stdout[0], F_SETFL, O_NDELAY );
      }

    if (trap_err)
      {
        TRC("odm_run_method","Closing write side of stderr pipe","","","");
        close( m_stderr[1] );
        fcntl( m_stderr[0], F_SETFL, O_NDELAY );
      }

    if ( (!trap_out) && (!trap_err) )
      {  /* no piped output */
        /* wait for the child to die */
        TRC("odm_run_method","Waiting for child to die","","","");
        while ( (w = wait(&status)) != c_pid && w != -1 );
        status = (status >> 8) & 0xFF;
        TRC("odm_run_method","status is %d",status,"","");
        STOP_ROUTINE;
        return( status );
      }

    /* initialize buffer pointers */
    if (trap_out)
      {
        TRC("odm_run_method","Setting up stdout buffer %x",out_ptr,"","");

        *out_ptr = malloc( BUFFER_SIZE );
        if (*out_ptr == NULL)
          {
            TRC("odm_run_method","Could not malloc! err %d",errno,"","");
            close(m_stdout[0]);
            if (trap_err)
                close(m_stderr[0]);
            odmerrno = ODMI_MALLOC_ERR;
            STOP_ROUTINE;
            return(-1);
          } /* endif */

        out_tmp = *out_ptr;
        out_end = *out_ptr + BUFFER_SIZE - 1;
        out_count = 1;
        outb_size = BUFFER_SIZE;
      }

    if (trap_err)
      {
        TRC("odm_run_method","Setting up stderr buffer %x",err_ptr,"","");
        *err_ptr = malloc( BUFFER_SIZE );
        if (*err_ptr == NULL)
          {
            TRC("odm_run_method","Could not malloc! err %d",errno,"","");
            close(m_stderr[0]);
            if (trap_out)
                close(m_stdout[0]);

            odmerrno = ODMI_MALLOC_ERR;
	    if ( *out_ptr) { 	/* free space for *out_ptr if any */
	      free(*out_ptr);
	      *out_ptr = NULL;
	    }
            STOP_ROUTINE;
            return(-1);
          } /* endif */

        err_tmp = *err_ptr;
        err_end = *err_ptr + BUFFER_SIZE - 1;
        err_count = 1;
        errb_size = BUFFER_SIZE;
      }

    while ( (trap_out) || (trap_err) )
      {
        /* reset the file descriptor masks */
        for(i=0;i<=_NFILE/32;i++) rdmask[i]= wrmask[i]= exmask[i]= 0;

        /* set the bit masks for the descriptors to be checked */
        if (trap_out)
            rdmask[m_stdout[0]/BITS(int)] |= (1 << (m_stdout[0]%32));
        if (trap_err)
            rdmask[m_stderr[0]/BITS(int)] |= (1 << (m_stderr[0]%32));

        /* check the status */
        if (select( (ulong)_NFILE,(void *)rdmask,(void *)wrmask,
            (void *)exmask,(struct timeval *)NULL) == -1)
          {
            if (errno == EINTR)
                continue;
            else
              {
                TRC("odm_run_method","could not select! err  %d",errno,"","");

                if (trap_out)
                    close(m_stdout[0]);
                if (trap_err)
                    close(m_stderr[0]);

                odmerrno = ODMI_READ_PIPE;
		/* free the *out_ptr and *err_ptr if any */
		if ( *out_ptr )
		   free(*out_ptr);
		if ( *err_ptr )
		   free(*err_ptr);
		*out_ptr = NULL;
		*err_ptr = NULL;
                STOP_ROUTINE;
                return( -1 );
              }
          }

        if ( (trap_out) &&
            (rdmask[m_stdout[0]/BITS(int)] & (1 << (m_stdout[0]%32))))
          {
            /* read the child's stdout*/
            if ( (out_num = read( m_stdout[0], out_tmp, outb_size )) < 0 )
              {
                TRC("odm_run_method","Could not read stdout! err %d",errno,
                    "","");

                close(m_stdout[0]);
                if (trap_err)
                    close(m_stderr[0]);

                odmerrno = ODMI_READ_PIPE;
		if ( *out_ptr ) free(*out_ptr);
		if ( *err_ptr ) free(*err_ptr);
		*out_ptr = NULL;
		*err_ptr = NULL;
                STOP_ROUTINE;
                return( -1 );
              }

            if (out_num == 0)
              {
                TRC("odm_run_method","No more stdout","","","");
                /* no more to read */
                trap_out = FALSE;
                close( m_stdout[0] );
                *out_tmp = '\0';
              }
            else if (out_num == outb_size)
              {
                /* filled up a buffer; allocate another one */
                out_count++;
                *out_ptr = realloc( *out_ptr, (out_count * BUFFER_SIZE) );
                if (*out_ptr == NULL)
                  {
                    TRC("odm_run_method","Could not realloc! %d",out_count,
                        "","");
                    close(m_stdout[0]);
                    if (trap_err)
                        close(m_stderr[0]);
                    odmerrno = ODMI_MALLOC_ERR;
		    if ( *err_ptr ) free(*err_ptr);
		    *err_ptr = NULL;
                    STOP_ROUTINE;
                    return(-1);
                  } /* endif */

                out_tmp = *out_ptr + ( (out_count - 1) * BUFFER_SIZE );
                out_end = out_tmp + BUFFER_SIZE - 1;
                outb_size = BUFFER_SIZE;
                TRC("odm_run_method","New buffer size is %d",outb_size,"","");

              }
            else if (out_num > 0)
              {
                /* read less than a full buffer; reset amount to read next */
                TRC("odm_run_method","Did not read full buffer","","","");
                out_tmp += out_num;
                outb_size = out_end - out_tmp + 1;
                outb_size = (outb_size > 0) ? outb_size : 0;
              }
          }/*if trap_out*/

        if ( (trap_err) &&
            (rdmask[m_stderr[0]/BITS(int)] & (1 << (m_stderr[0]%32))))
          {
            /* read the child's stderr */
            if ( (err_num = read( m_stderr[0], err_tmp, errb_size )) == -1 )
              {
                TRC("odm_run_method","Could not read stderr! err %d",errno,
                    "","");

                if (trap_out)
                    close(m_stdout[0]);

                close(m_stderr[0]);

                odmerrno = ODMI_READ_PIPE;
		if ( *out_ptr ) free(*out_ptr);
		if ( *err_ptr ) free(*err_ptr);
		*out_ptr = NULL;
		*err_ptr = NULL;
                STOP_ROUTINE;

                return( -1 );
              }

            if (err_num == 0)
              {
                TRC("odm_run_method","No more stderr to read","","","");
                /* no more to read */
                trap_err = FALSE;
                close( m_stderr[0] );
                *err_tmp = '\0';
              }
            else if (err_num == errb_size)
              {

                /* filled up a buffer; allocate another one */
                err_count++;
                *err_ptr = realloc( *err_ptr, (err_count * BUFFER_SIZE) );
                if (*err_ptr == NULL)
                  {
                    TRC("odm_run_method","Could not realloc! num %d",err_count,
                        "","");
                    close(m_stderr[0]);
                    if (trap_out)
                        close(m_stdout[0]);
                    odmerrno = ODMI_MALLOC_ERR;
		    if ( *out_ptr ) free(*out_ptr);
		    *out_ptr = NULL;
                    STOP_ROUTINE;
                    return(-1);
                  } /* endif */

                err_tmp = *err_ptr + ( (err_count - 1) * BUFFER_SIZE );
                err_end = err_tmp + BUFFER_SIZE - 1;
                errb_size = BUFFER_SIZE;

                TRC("odm_run_method","New stderr size is %d",errb_size,"","");

              }
            else if (err_num > 0)
              {
                TRC("odm_run_method","Did not read full buffer","","","");
                /* read less than a full buffer; reset amount to read next */
                err_tmp += err_num;
                errb_size = err_end - err_tmp + 1;
                errb_size = (errb_size > 0) ? errb_size : 0;
              }
          }/*if trap_err */
      }/* while trap_out or trap_err */


    while ( (w = wait(&status)) != c_pid && w != -1 );
    status = (status >> 8) & 0xFF;

    TRC("odm_run_method","Done with child. status %d",status,"","");

    /* restore the childs signals */

    STOP_ROUTINE;
    return( status );
}
