static char sccsid[] = "@(#)77	1.1  src/bos/usr/ccs/lib/libmi/cpp.c, cmdpse, bos411, 9428A410j 5/7/91 13:08:03";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990  Mentat Inc.
 ** cpp.c 2.1, last change 11/14/90
 **/


#include <pse/common.h>
#include <pse/clib.h>

/* The cpp routines operate on nil terminated arrays of char pointers.
** Those routines that overwrite elements of the array call free()
** on the contents before doing so. Those routines that need to resize
** the array pointed at by cpp use realloc().
*/

/* Free all elements in cpp then free cpp itself. */
void
cpp_blast (cpp)
	char	** cpp;
{
reg	char	** cpp1;

	if (cpp1 = cpp) {
		while (*cpp1)
			free(*cpp1++);
		free(cpp);
	}
}

/* Free cpp[pos] then pull elements at cpp[pos+1] and later up one.
** Returns cpp on success, nil on failure (cpp passed in nil or
** pos out of range).
*/
char **
cpp_del (cpp, position)
	char	** cpp;
	int	position;
{
reg	char	** cpp1;

	if (cpp  &&  position >= 0  &&  position < cpp_len(cpp)) {
		cpp1 = &cpp[position];
		free(*cpp1);
		while (cpp1[0] = cpp1[1])
			cpp1++;
		return cpp;
	}
	return nilp(char *);
}

/* Duplicate the string array pointed at by cpp.  The newly malloc()ed
** array pointing at the newly strdup()ed strings is returned on success,
** nil on failure (no memory).
*/
char **
cpp_dup (cpp)
	char	** cpp;
{
	char	** cpp_base, ** cpp_dst;

	if (cpp  &&  (cpp_base = newa(char *, cpp_len(cpp) + 1))) {
		cpp_dst = cpp_base;
		do {
			if (!*cpp)
				return cpp_base;
		} while (*cpp_dst++ = strdup(*cpp++));
		cpp_blast(cpp_base);
	}
	return nilp(char *);
}

/* Insert a strdup()ed copy of str at cpp[position] pushing all elements at
** cpp[position] and below down one.  Cpp is realloced as needed to grow
** the array.  Success returns the possibly realloced cpp, failure
** returns nil.
*/
char **
cpp_ins (orig_cpp, position, str)
	char	** orig_cpp;
	int	position;
	char	* str;
{
	char	** cpp;
reg	int	len, i1;
reg	char	** new_cpp;
	int	new_len;

	if (!str
	||  (!(cpp = orig_cpp)  &&  !(cpp = newa(char *, 2))))
		return nilp(char *);
	len = cpp_len(cpp);
	new_len = MAX(len, position) + 1;
	if (new_cpp = (char **)realloc(cpp, (new_len+1) * sizeof(char *))) {
		if (position <= len) {
			for (i1 = new_len-1; i1 > position; i1--) {
				new_cpp[i1] = new_cpp[i1-1];
			}
		} else {
			for (i1 = len; i1 < position; i1++)
				new_cpp[i1] = strdup("");
		}
		new_cpp[new_len] = nilp(char);
		if (new_cpp[position] = strdup(str))
			return new_cpp;
	}
	if (!orig_cpp)
		free(cpp);
	return nilp(char *);
}

/* Count and return the number of elements (non-nil char pointers) in cpp.
** Return -1 if cpp is nil.
*/
int
cpp_len (cpp)
	char	** cpp;
{
reg	char	** cpp1;

	if (!(cpp1 = cpp))
		return -1;
	while (*cpp1)
		cpp1++;
	return cpp1 - cpp;
}

/* Print the contents of a string list, along with a text identifier.  Used mostly for debugging. */
void
cpp_print (cpp, text)
	char	** cpp;
{
	printf("%s:\n", text);
	for ( ; cpp && *cpp; )
		printf("\t'%s'\n", *cpp++);
}
