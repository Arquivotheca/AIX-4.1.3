static char sccsid[] = "@(#)12	1.8  src/bos/usr/bin/ate/lookup.c, cmdate, bos411, 9428A410j 6/27/91 11:06:15";
/* 
 * COMPONENT_NAME: BOS lookup.c
 * 
 * FUNCTIONS: MSGSTR, Mlookup, alloc_list, item_cmp, lookup 
 *
 * ORIGINS: 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include "modem.h"
#include <stdlib.h>

#include "ate_msg.h" 
#include <nl_types.h>
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_ATE,n,s) 

struct list {
    int len;
    char **items;
};

struct list *alloc_list(len)
int len;
{
    struct list *list = (struct list *)malloc (sizeof (struct list));
    list->items = (char **)malloc (len * sizeof (char *));
    list->len = len;

    return (list);
}


/* lookup: return index of item in list */
lookup(list, item)
struct list *list;
char *item;
{
    int i;
    int len;

    len = list->len;

    for (i = 0; i < len; i++)
	if (item_cmp (list->items[i], item))
	    return (i);
    return (ERROR);
}

item_cmp (s1, s2)
char *s1, *s2;
{
      int i;	
      extern nl_catd catd;
	  
    if (mb_cur_max > 1) {   		/* multibyte code */
	wchar_t *wc_s1, *wc_s2;
	int n, mbcount1, mbcount2;
	/* convert multibyte to widecahr for processing */
	n = (strlen(s1) + 1 ) * sizeof(wchar_t);
	wc_s1 = (wchar_t *)malloc(n);
	mbcount1 = mbstowcs(wc_s1, s1, n);
	
	n = (strlen(s2) + 1) * sizeof(wchar_t);
	wc_s2 = (wchar_t *)malloc(n);
	mbcount2 = mbstowcs(wc_s2, s2, n);
	
	if (mbcount1 < 0 || mbcount2 < 0) {
		printf(MSGSTR(LK_MB, "item_cmp: Error converting to widechar\n"));
		return(FALSE);
	}
		for (i = 0; i < mbcount2; i++)
			if (towupper(wc_s1[i]) != towupper(wc_s2[i]))
				return(FALSE);
    }
    else {				/* single byte code */
	int	len;
	
	len = strlen(s2);
	for (i = 0; i < len; i++)
		if (toupper(s1[i]) != toupper(s2[i]))
			return(FALSE);
    }
    return (TRUE);
}


#ifdef TEST

main() /* to test out alloc_list */
{
    int i;
    struct list *list = alloc_list(7);

    for (i = 0; i < 7; i++) 
	list->items[i] = "foo"; 

    for (i = 0; i < 7; i++)
	printf ("%s\n", list->items[i]);
}
#endif
