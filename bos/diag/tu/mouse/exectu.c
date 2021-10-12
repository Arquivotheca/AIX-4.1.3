static char sccsid[] = "@(#)76  1.6  src/bos/diag/tu/mouse/exectu.c, tu_mouse, bos41J, 9515A_all 4/5/95 16:02:44";
/*
 * COMPONENT_NAME: tu_mouse
 *
 * FUNCTIONS: exectu
 *
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/***************************************************************************/
/* NOTE: This function is called by Hardware exerciser (HTX),Manufacturing */
/*	  application and Diagnostic application to invoke a test unit (TU) */
/*														   */
/*	  If the mfg mode in the tu control block (tucb) is set to be	  */
/*	  invoked by HTX then TU program will look at variables in tu	  */
/*	  control block for values from the rule file. Else, TU program	*/
/*	  uses the predefined values.							    */
/*														   */
/***************************************************************************/

#include <stdio.h>
#include "tu_type.h"		/* This also includes hxihtx.h */

int	pos_slot;
int	ipl_mod;
int	cuat_mod;

int exectu (fdes,tucb_ptr)
int	fdes;
TUTYPE	*tucb_ptr;
{
	register	i, loop, tu;  /* Loop Index */
	int		rc = SUCCESS;	 /* return code */
	TUTYPE		tmp_tucb;
	extern int	tu10 ();
	extern int	tu20 ();
	extern int	tu30 ();
	extern int	tu40 ();
	extern int	tu50 ();
#ifdef nodiag
	extern int	tu60 ();
	extern int	tu70 ();
	struct htx_data *htx_sp;
#endif

/* Make copy of tucb_ptr and set up with tu number to perform a HALT.
   We do this in case a TU fails. */
	tmp_tucb = *tucb_ptr;

#ifdef nodiag
	tmp_tucb.mouse_s.htx_sp = NULL;
#endif
	tu = tucb_ptr->header.tu;
	loop = tucb_ptr->header.loop;

#ifdef nodiag
	if (tucb_ptr->header.mfg != INVOKED_BY_HTX)
		tucb_ptr->mouse_s.htx_sp = NULL;
	htx_sp = tucb_ptr->mouse_s.htx_sp;
#endif

/* 7012-G30 keyboard/mouse adapter is at slot 7 instead of slot 15 */
	ipl_mod = get_cpu_model (&cuat_mod);
	if (IsPowerPC_SMP (ipl_mod))
		pos_slot = POS_SLOT7;
	else
		pos_slot = POS_SLOT0;

	for (i=0; i < loop; i++)
	{
		switch (tu)
		{
		case 10:
			rc = tu10 (fdes,tucb_ptr);
#ifdef nodiag
			if (rc)
			{
				htx_sp->bad_others++;
			}
			else
			{
				htx_sp->good_others+=7;
			}
#endif
			break;
		case 20:
			rc = tu20 (fdes,tucb_ptr);
#ifdef nodiag
			if (rc)
			{
				htx_sp->bad_others++;
			}
			else
			{
				htx_sp->good_others+=4;
			}
#endif
			break;
		case 30:
			rc = tu30 (fdes,tucb_ptr);
#ifdef nodiag
			if (rc)
		 	{
				htx_sp->bad_others++;
			}
			else
			{
				htx_sp->good_others+=4;
			}
#endif
			break;
		case 40:
			rc = tu40 (fdes,tucb_ptr);
#ifdef nodiag
			if (rc)
			{
				htx_sp->bad_others++;
			}
			else
			{
				htx_sp->good_others+=1;
			}
#endif
			break;
		case 50:
			rc = tu50 (fdes,tucb_ptr);
#ifdef nodiag
			if (rc)
			{
				htx_sp->bad_others++;
			}
			else
			{
				htx_sp->good_others+=2;
			}
#endif
			break;
#ifdef nodiag
/* test unit 6 is interactive therefore not to be executed under HTX */
		case 60:
			rc = tu60 (fdes,tucb_ptr);
			break;
/* test unit 7 is interactive therefore not to be executed under HTX */
		case 70:
			rc = tu70 (fdes,tucb_ptr);
			break;
#endif
		default :
			return (FAILURE);
		};  /* end case */
#ifdef nodiag
		if (rc)
		{
			/* if running mfg. diagnostic and a tu returns
			 * an error, then break out and return.  */
			if (tucb_ptr->header.mfg != INVOKED_BY_HTX)
				break;

			/* check on retries keyword.  If set from rule file
			   stanza, then continue loop to retry tu, else
			   break out and return. */
			if (!(*tucb_ptr->mouse_s.retries))
				break;
		}
		if (tucb_ptr->header.mfg == INVOKED_BY_HTX)
			hxfupdate (UPDATE, tucb_ptr->mouse_s.htx_sp);
#endif
	}
	return (rc);
} /* End function */
