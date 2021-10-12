# @(#)39    1.1  src/bos/usr/ccs/lib/libcurses/keyname.sh, libcurses, bos411, 9428A410j 9/3/93 15:09:40
#
#   COMPONENT_NAME: LIBCURSES
#
#   FUNCTIONS:
#
#   ORIGINS: 27, 4
#
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved
#
#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.
#

# ident	"@(#)curses:screen/keyname.sh	1.3"
${ODE_TOOLS}/usr/bin/rm -f keyname.c
${ODE_TOOLS}/usr/bin/grep -v 'copyright.h' copyright.h > keyname.c
echo "#include	\"curses_inc.h\"\n" >> keyname.c
echo "static	char	*keystrings[] =\n\t\t{" >> keyname.c
{
    ${ODE_TOOLS}/usr/bin/grep -v 'KEY_F(' keycaps | ${ODE_TOOLS}/usr/bin/awk '{ print $5, $4 }' | ${ODE_TOOLS}/usr/bin/sed -e 's/,//g' -e 's/KEY_//'
    # These three aren't in keycaps
    echo '03501 BREAK\n03630 SRESET\n03631 RESET'
} |  ${ODE_TOOLS}/usr/bin/sort -n | ${ODE_TOOLS}/usr/bin/awk '
    {
	print "\t\t    \"" $2 "\",	/* " $1 " */"
    }
' >> keyname.c

LAST=`/usr/bin/tail -1 keyname.c | ${ODE_TOOLS}/usr/bin/awk -F'"' '{print $2}'`
${ODE_TOOLS}/usr/bin/cat << ! >> keyname.c
		};

char	*keyname(key)
int	key;
{
    static	char	buf[16];

		/* KEY_MIN  = 03501 	KEY_MAX   = 03777
		   KEY_F(0) = 03510 	KEY_F(63) = 03607  */

    if (key >= (KEY_MIN - 1))    /* If greater than or equal KEY_MIN-1 */
    {
	register	int	i;

	if ((key == (KEY_MIN - 1)) || (key > KEY_${LAST})) 
	/* if less than min or greater than the last key which is KEY_SUNDO 03735 */

	    return ("UNKNOWN KEY");     /* return unknown */

	if (key > KEY_F(63))           /* if between KEY_F(63) and the last key */

	    i = key - (KEY_MIN + ((KEY_F(63) - KEY_F0) + 1)); 
		/* adjust the key to be in the range of KEY_MIN and KEY_F(63) */

	else
	    if (key >= KEY_F0) /* else if in between KEY_F(0) and KEY_F(63) */
	    {
	    	(void) sprintf(buf, "KEY_F(%d)", key - KEY_F0);
	    	return (buf);                   /* return the buf */
	    }
	    else

		i = key - KEY_MIN;      /* if between KEY_MIN and KEY_F(0) */
	(void) sprintf(buf, "KEY_%s", keystrings[i]);
	return (buf);                   /* return the buf */
    }

    if (key >= 0200)  /* if between 0200 (128 in decimal) and KEY_MIN */
    {
	if (SHELLTTY.c_cflag & CS8)
	    (void) sprintf(buf, "%c", key);
	else
	    (void) sprintf(buf, "M-%s", unctrl(key & 0177));
	return (buf);                   /* return the buf */
    }

    if (key < 0) /* If it is negative value */
    {
	(void) sprintf(buf, "%d", key); /* Then print decimal equivalent */
	return (buf);                   /* return the buf */
    }

    return (unctrl(key)); 
/* else if in the range of 0-177 (i.e., 0-127 dec) then return the control key from the _unctrl[] array */
}
!
exit 0
