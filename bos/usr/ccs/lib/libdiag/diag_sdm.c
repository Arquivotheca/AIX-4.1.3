static char sccsid[] = "@(#)35	1.2  src/bos/usr/ccs/lib/libdiag/diag_sdm.c, libdiag, bos41B, bai4 1/9/95 16:43:59";
/*
 * COMPONENT_NAME: (LIBDIAG) Diagnostic Library
 *
 * FUNCTIONS: 	init_dgodm
 *	      	term_dgodm
 *		diag_get_list
 *		diag_free_list
 *		diag_open_class
 *		diag_close_class
 *		diag_add_obj
 *		diag_rm_obj
 *		diag_change_obj
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/* FUNCTION PROTOTYPES */

void *diag_get_list (void *, char *, struct listinfo *, int, int);
int diag_free_list (void *, struct listinfo *);
void *diag_open_class(void * );
int  diag_close_class(void * );
int  diag_add_obj(void * , void *);
int  diag_rm_obj(void * , char *);
int  diag_change_obj(void * , void *);
int  init_dgodm(void);
int  term_dgodm(void);

/* NAME: init_dgodm  
 *
 * FUNCTION: Initialize the ODMI interface and set up repository path
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:
 *	0 
 *
 */

int
init_dgodm()
{
	odm_initialize();
	return(0);
}

/* NAME: term_dgodm  
 *
 * FUNCTION: Terminate the ODMI interface
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:
 *	0 
 *
 */

int
term_dgodm()
{
	odm_terminate();
	return(0);
}

/*  */
/*
 * NAME: diag_get_list
 *                                                                    
 * FUNCTION: Obtain a list of objects from the ODM
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *                                                                   
 * RETURNS: Pointer to object class
 *
 */

void *
diag_get_list(
	void	*classp,
	char	*criteria,
	struct	listinfo *info,
	int 	max_expect,
	int	depth)

{
	return((void *)odm_get_list(classp, criteria, info, max_expect, depth));
}

/*  */
/*
 * NAME: diag_free_list
 *                                                                    
 * FUNCTION: Frees the list of objects
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *
 * RETURNS: 0 or -1.
 *
 */
int diag_free_list(
	void	*p_obj,
	struct	listinfo *info)
{
	return(odm_free_list(p_obj, info));
}

/*  */
/*
 * NAME: diag_open_class
 *                                                                    
 * FUNCTION: Open an object class
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *
 * RETURNS: NONE.
 *
 */

void *
diag_open_class(void *classp)
{

	return((void *)odm_open_class(classp));
}
/*  */
/*
 * NAME: diag_close_class
 *                                                                    
 * FUNCTION: Close an object class
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *
 * RETURNS: NONE.
 *
 */

int diag_close_class(void *classp)
{

	return(odm_close_class(classp));
}
/*  */
/*
 * NAME: diag_add_obj
 *                                                                    
 * FUNCTION: Add an object into the ODM data base
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *
 * RETURNS: 0 or -1.
 *
 */

int diag_add_obj(
	void	*classp,
	void	*p_obj)
{
	return(odm_add_obj(classp,p_obj));
}

/*  */
/*
 * NAME: diag_rm_obj
 *                                                                    
 * FUNCTION: Removes an object from the ODM data base
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *
 * RETURNS: 0 or -1.
 *
 */

int diag_rm_obj(
	void	*classp,
	char	*criteria)
{
	return(odm_rm_obj(classp,criteria));
}
/*  */
/*
 * NAME: diag_change_obj
 *                                                                    
 * FUNCTION: Changes an object in the ODM data base
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *
 * RETURNS: 0 or -1.
 *
 */

int diag_change_obj(
	void	*classp,
	void	*p_obj)
{
	return(odm_change_obj(classp,p_obj));
}

