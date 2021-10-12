static char sccsid[] = "@(#)53	1.6  src/bos/usr/ccs/lib/libIN/LS1killall.c, libIN, bos411, 9428A410j 6/10/91 10:20:03";
/*
 * LIBIN: LS1killall
 *
 * ORIGIN: 9
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * FUNCTION: 
 *	Calls free() on each element of L, leaving L empty.
 *
 *	Services for singly linked lists with link fields in the first
 *	position in the record.  End of list NULL.  Both ends accessible.
 *
 * RETURN VALUE DESCRIPTION: void
 */

#include <IN/LSdefs.h>

LS1killall(L) register ListSingle *L; {
    while (!LS1isempty(L)) LS1killfirst(L);}
