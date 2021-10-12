static char sccsid[] = "@(#)48	1.2.1.3  src/bos/usr/ccs/lib/libdiag/menugoal.c, libdiag, bos41B, bai4 1/9/95 13:54:34";
/*
 * COMPONENT_NAME: (LIBDIAG) DIAGNOSTIC LIBRARY
 *
 * FUNCTIONS: 	menugoal
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include	<stdio.h>
#include 	<diag/class_def.h>
#include 	<diag/tmdefs.h>

/*
 * FUNCTION:	menugoal()
 * 
 * DESCRIPTION:
 * 
 * This function associates a menu goal with the device currently
 * being tested; this device is identified by the tm_input object class.
 * 
 * msg -	pointer to text string that states a repair action
 * 		intended for the customer.
 * 
 * RETURNS:
 * 	 0	on success,
 * 	-1	on failure.
 * 
 */

 int
menugoal(msg)
char	*msg;

{
	int		rc = -1;
	long    	menu_number;
	struct TMInput	*T_TMInput;
	struct MenuGoal *T_MenuGoal;
	struct listinfo c_info;

	if (strlen(msg) > 0)  {
		/* get the device currently being tested */
		T_TMInput = (struct TMInput *)diag_get_list( TMInput_CLASS,
				"",&c_info,1,1);
		if (T_TMInput == (struct TMInput *) -1)
			return(-1);

		/* if duplicate message number - do not add */
                sscanf( msg, "%X", &menu_number);
		rc = chk_duplicate( T_TMInput->dname, menu_number );
		if ( rc == -1 )
			return ( -1 );
		else if ( rc != 0 ) {
			diag_free_list ( T_TMInput, &c_info );
			return (0);
		}

		/* allocate some space for the text */
		T_MenuGoal = (struct MenuGoal *)
				calloc(1,sizeof(struct MenuGoal));
		if (T_MenuGoal == (struct MenuGoal *) -1)
			return(-1);

		if ((c_info.num > 0) && (T_TMInput->exenv != EXENV_SYSX))
			strcpy(T_MenuGoal->dname,T_TMInput->dname);

		if(strlen(msg) < 1000)
			strcpy(T_MenuGoal->tbuffer1,msg);
		else  {
			strncpy(T_MenuGoal->tbuffer1,msg,1000);
			T_MenuGoal->tbuffer1[999] = '\0';
			strncpy(T_MenuGoal->tbuffer2,msg+999,1000);
			T_MenuGoal->tbuffer2[999] = '\0';
		}
		rc = diag_add_obj(MenuGoal_CLASS, T_MenuGoal);
		free ( T_MenuGoal );
	}
	return((rc>=0) ? 0 : -1);
}

chk_duplicate ( dname, menunumber )
char	*dname;
long	menunumber;
{
	int count;
	long	old_number;
	char	buffer[256];
	struct MenuGoal *T_MenuGoal;
        struct listinfo f_info;
	

	/* read all entries from MenuGoal Object Class */
	sprintf( buffer, "dname = %s", dname );
        T_MenuGoal = (struct MenuGoal *)diag_get_list(MenuGoal_CLASS,
			buffer,&f_info,1,1);
        if ( ( T_MenuGoal != (struct MenuGoal *) -1) && (f_info.num != 0) ) {
                for(count=0; count < f_info.num; count++)  {
                        /* extract menu number from text */
                        sscanf( T_MenuGoal[count].tbuffer1, "%X", &old_number);
			if ( old_number == menunumber )
				return (1);
		}
		diag_free_list ( T_MenuGoal, &f_info );
		return(0);		
	}
        if ( T_MenuGoal == (struct MenuGoal *) -1) 
		return (-1);
	if ( f_info.num == 0 )
		return (0);

}
