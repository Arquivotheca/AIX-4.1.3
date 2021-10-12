static char sccsid[] = "@(#)41	1.6  src/bos/usr/ccs/lib/libIN/XXauxent.c, libIN, bos411, 9428A410j 6/10/91 10:22:53";
/*
 * LIBIN: XXauxent
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
 * FUNCTION: Figure out format of auxilary entry from the struct syment
 *	     to which it belongs, and swap the fields accordingly.
 *
 * RETURN VALUE DESCRIPTION: void
 */

#include <gpoff.h>
extern short XXshort();
extern long XXlong();

#define IsFcn(t) ((((t)>>4)&3)==DT_FCN)

XXauxent(sym,aux,cpu)register struct syment *sym; register AUXENT *aux;
							unsigned char cpu;{
   unsigned char class=(sym->n_sclass)&N_CLASS;
   unsigned short type;
   register int i;
   switch(class){
	case C_FILE:	return;

	case C_STRTAG:
	case C_ENTAG:
	case C_UNTAG:	aux->x_sym.x_misc.x_lnsz.x_size=
			   XXshort(aux->x_sym.x_misc.x_lnsz.x_size,cpu);
			aux->x_sym.x_fcnary.x_fcn.x_endndx=
			   XXlong(aux->x_sym.x_fcnary.x_fcn.x_endndx,cpu);
			return;
	
	case C_FIELD:	/* tag field should be 0 */
			aux->x_sym.x_misc.x_lnsz.x_size=
			   XXshort(aux->x_sym.x_misc.x_lnsz.x_size,cpu);
			return;

	case C_EOS:	aux->x_sym.x_tagndx=
			   XXlong(aux->x_sym.x_tagndx,cpu);
			aux->x_sym.x_misc.x_lnsz.x_size=
			   XXshort(aux->x_sym.x_misc.x_lnsz.x_size,cpu);
			return;

	case C_FCN:
	case C_BLOCK:	aux->x_sym.x_misc.x_lnsz.x_lnno=
			   XXshort(aux->x_sym.x_misc.x_lnsz.x_lnno,cpu);
			return;}

	if(IsFcn(sym->n_type)){
	    aux->x_sym.x_tagndx=XXlong(aux->x_sym.x_tagndx,cpu);
   	    aux->x_sym.x_misc.x_fsize=XXlong(aux->x_sym.x_misc.x_fsize,cpu);
   	    aux->x_sym.x_fcnary.x_fcn.x_lnnoptr=
			     XXlong(aux->x_sym.x_fcnary.x_fcn.x_lnnoptr,cpu);
   	    aux->x_sym.x_fcnary.x_fcn.x_endndx=
			     XXlong(aux->x_sym.x_fcnary.x_fcn.x_endndx,cpu);
	    return;}
	/* must be an array, or struct.  Some unused fields may get swapped*/
	aux->x_sym.x_tagndx=XXlong(aux->x_sym.x_tagndx,cpu);
	aux->x_sym.x_misc.x_lnsz.x_lnno=
				XXshort(aux->x_sym.x_misc.x_lnsz.x_lnno,cpu);
   	aux->x_sym.x_misc.x_lnsz.x_size=
				XXshort(aux->x_sym.x_misc.x_lnsz.x_size,cpu);
   	for(i=0;i<DIMNUM;i++)
	   aux->x_sym.x_fcnary.x_ary.x_dimen[i]=
			  XXshort(aux->x_sym.x_fcnary.x_ary.x_dimen[i],cpu);}
