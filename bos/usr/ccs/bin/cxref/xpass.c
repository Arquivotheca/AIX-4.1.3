static char sccsid[] = "@(#)49  1.4  src/bos/usr/ccs/bin/cxref/xpass.c, cmdprog, bos411, 9428A410j 5/16/91 08:48:45";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: CXBeginBlk, CXDefFtn, CXDefName, CXEndBlk, CXRefName
 *
 * ORIGINS: 00 03 10 27 32
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

# include "mfile1.h"
# include "cxref_msg.h"

#define         MSGSTR(Num, Str) catgets(catd, MS_CXREF, Num, Str)
nl_catd catd;

int blocknos[BNEST];
int blockptr = 0;
int nextblock = 1;              /* block counter */

/*
** The following functions are embedded in the grammar of the 1st pass
** whenever a new block is created or destroyed.
*/
CXBeginBlk()
{
        /* Code for beginning a new block. */
        blocknos[blockptr] = nextblock++;
        printf("B%d\t%05d\n", blocknos[blockptr], lineno);
        blockptr++;
}

CXEndBlk()
{
        /* Code for ending a block. */
        if (--blockptr < 0)
                uerror(MSGSTR(M_MSG_1, "bad block nesting"));
        else
                printf("E%d\t%05d\n", blocknos[blockptr], lineno);
}

/*
** The following functions are embedded in the grammar of the 1st pass
** whenever a NAME is seen.
*/

CXRefName(i, line)
int i, line;
{
        /* Code for referencing a NAME. */
        printf("R%s\t%05d\n", stab[i].psname, line);
}

CXDefName(i, line)
int i, line;
{
        /* Code for defining a NAME. */
        if (stab[i].sclass == EXTERN)
                CXRefName(i, line);
        else
                printf("D%s\t%05d\n", stab[i].psname, line);
}

CXDefFtn(i, line)
int i, line;
{
        /* Code for defining a function NAME. */
        printf("F%s\t%05d\n", stab[i].psname, line);
}
