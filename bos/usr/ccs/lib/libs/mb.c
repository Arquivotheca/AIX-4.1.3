static char sccsid[] = "@(#)15	1.1  src/bos/usr/ccs/lib/libs/mb.c, libs, bos411, 9428A410j 8/24/93 13:32:56";
/*
 *   COMPONENT_NAME: LIBS
 *
 *   FUNCTIONS: _MBInitialize
 *		_MBDiscard
 *		_MBAppend
 *		_MBReplace
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <varargs.h>
#include "libs.h"

/*
 * Global routines
 */
void	_MBInitialize(MsgBuf *, char **);
void	_MBDiscard(MsgBuf *);
int	_MBAppend();
int	_MBReplace();

/*
 * NAME: _MBInitialize
 *
 * FUNCTION: Initializes the MsgBuf structure.  This structure is used
 *	     when creating messages to return to the user.  The routines
 *	     allow for easy appending or replacement of a current message.
 *
 *  	     Example use:
 *
 *	  	#include "libs.h"	
 *		...
 *		char * 	msg;
 *		MsgBuf 	mb;
 *
 *		_MBInitialize(&mb, &msg);
 *		_MBAppend(&mb, "Now is the %s\n", "time");
 *		  
 *	     The "msg" variable has been allocated and contains the
 *	     string "Now is the time\n".
 *
 * RETURNS:  void
 */
void
_MBInitialize(MsgBuf *mb, char **msg)
{
        mb->buf  = msg;
	*mb->buf = (char *)NULL;
        mb->used = 0;
        mb->size = 0;
        mb->size_increment = 512;
}

/*
 * NAME: _MBDiscard
 *
 * Function: Discards the storage allocated for the message string, and
 *	     reinitializes the MsgBuf.
 * 	     
 *	     Callers must initialize the MsgBuf with _MBInitialize() before
 *	     calling this routine.
 *
 * RETURNS: void
 */
void
_MBDiscard(MsgBuf *mb)
{
        mb->used = 0;
        mb->size = 0;
        if (*mb->buf)
                free(*mb->buf);
	*mb->buf = (char *)NULL;
}


/*
 * NAME: _MBAppend
 *
 * FUNCTION: Appends a message to the MsgBuf buffer.  The first argument
 *	     must be the MsgBuf pointer, but subsequent arguments may
 *	     be in the form of a sprintf() format and argument group.
 *	     Callers must initialize the MsgBuf with _MBInitialize()
 *	     before calling this routine.
 *	
 *	     Errno is set on failure
 *
 *	     Example:   
 *		_MBAppend(&mb, "Now is the time ");
 *		_MBAppend(&mb, "for %d good men\n", 2);
 *
 * RETURNS:  0 : success
 *	    -1 : failure
 */
int
_MBAppend(va_alist)
	va_dcl
{
	int	terrno;
        char    *new_buf;
        int     new_size;
        int     s_size;
	va_list	ap;
	char 	*format;
	MsgBuf	*mb;
	char	buffer[BUFSIZ];

	va_start(ap);
	mb	= va_arg(ap, MsgBuf *);
	format  = va_arg(ap, char *);
	vsprintf(buffer, format, ap);
	va_end(ap);
		
        s_size = strlen(buffer);

        /*
         * Determine if buffer will fit in 'mb'.
         */
        if (s_size >= (mb->size - mb->used))
        {
                new_size = ((mb->used + (s_size + 1) + (mb->size_increment - 1))
                           / mb->size_increment) * mb->size_increment;

                if (*mb->buf)
                {
                        /*
                         * If the buffer is already malloc'ed, then realloc.
			 * If the realloc fails then this buffer is still
			 * valid and should therefore be left unmodified.
                         */
                        if (!(new_buf = (char *)realloc(*mb->buf, new_size)))
				return(-1);
                }
                else
                {
                        /*
                         * If no buffer exists, then malloc a buffer.
                         */
                        if (!(new_buf = (char *)malloc(new_size)))
                                return(-1);
                }
                *mb->buf  = new_buf;
                mb->size = new_size;
        }

        strcpy(&((*mb->buf)[mb->used]), buffer);
        mb->used += s_size;

        return(0);
}

/*
 * NAME: _MBReplace
 *
 * FUNCTION: Replaces the MsgBuf buffer with the specified new message.  
 *	     The first argument must be the MsgBuf pointer, but subsequent 
 *	     arguments may be in the form of a sprintf() format and argument 
 *	     group.  Callers must initialize the MsgBuf with _MBInitialize()
 *	     before calling this routine.
 *
 *	     Errno is set on failure
 *
 *	     Example:   
 *		_MBReplace(&mb, "Make that all good women\n");
 *
 * RETURNS:  0 : success
 *	    -1 : failure
 */
int
_MBReplace(MsgBuf *mb, va_list args)
{
        mb->used = 0;
	return(_MBAppend(mb, args));
}
