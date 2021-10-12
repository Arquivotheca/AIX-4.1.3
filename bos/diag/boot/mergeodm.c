static char sccsid[] = "@(#)97	1.3  src/bos/diag/boot/mergeodm.c, diagboot, bos411, 9430C411a 7/22/94 11:29:45";
/*
 * COMPONENT_NAME: DIAGBOOT
 *
 * FUNCTIONS: 		mergeodm
 *			int_handler
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

#include <stdio.h>
#include <math.h>
#include <nl_types.h>
#include <sys/cfgdb.h>                  /* cfg db literals */
#include "diag/class_def.h"             /* object class data structures */
#include "diag/tmdefs.h"
#include "diag/diag.h"
#include <signal.h>
#include <locale.h>

#define DEPTH 1

/* GLOBAL VARIABLES */
struct PDiagDev *T_PDiagDev, *new_T_PDiagDev;
struct PDiagAtt *T_PDiagAtt, *new_T_PDiagAtt;
struct PdDv	*T_PdDv, *new_T_PdDv;
struct PdAt	*T_PdAt;
struct PdCn	*T_PdCn;
struct CuDv	*cudv_list_p;

int num_PDiagDev, new_num_PDiagDev;
int num_PDiagAtt, new_num_PDiagAtt;
int num_PdDv, new_num_PdDv;
int num_PdAt, new_num_PdAt;
int num_PdCn, new_num_PdCn;
int num_CuDv;
struct listinfo	pdiagdev_info, npdiagdev_info;
struct listinfo	pdiagatt_info, npdiagatt_info;
struct listinfo	pddv_info, npddv_info;
struct listinfo	pdat_info;
struct listinfo	pdcn_info;
struct listinfo cudv_info;

char	fromODMDIR[128];
char	toODMDIR[128];

/* Function prototypes */

void	int_handler(int);
void	cleanup();
int	mergeodm(char *, char *);

/*  */
int
main(argc, argv, envp)
int	argc;
char	**argv;
char	**envp;
{
	struct	sigaction	act;
	int	rc;

	setlocale(LC_ALL, "");
	act.sa_handler = int_handler;
	sigaction(SIGINT, &act, (struct sigaction *)NULL);
	init_dgodm();
	strcpy(fromODMDIR, argv[1]);
	strcpy(toODMDIR, argv[2]);
	rc=mergeodm(fromODMDIR, toODMDIR);
	cleanup();
	term_dgodm();
	exit(rc != 0 ? 1 : 0);

}
/*  */
/*
 * NAME:  int_handler
 *
 * FUNCTION: Perform clean up on receipt of an interrupt
 *
 * NOTES:
 *
 * RETURNS: None
 *
 */

void int_handler(int sig)
{
	term_dgodm();
	cleanup();
	exit(0);

}
/*  */
/*
 * NAME:  cleanup
 *
 * FUNCTION: Perform clean up
 *
 * NOTES:
 *
 * RETURNS: None
 *
 */
void cleanup()
{
	if(pdiagdev_info.num > 0)
		odm_free_list(T_PDiagDev, &pdiagdev_info);
	if(pdiagatt_info.num > 0)
		odm_free_list(T_PDiagAtt, &pdiagatt_info);
	if(npdiagdev_info.num > 0)
		odm_free_list(new_T_PDiagDev, &npdiagdev_info);
	if(npdiagatt_info.num > 0)
		odm_free_list(new_T_PDiagAtt, &npdiagatt_info);
	if(npddv_info.num > 0)
		odm_free_list(new_T_PdDv, &npddv_info);
	if(pddv_info.num > 0)
		odm_free_list(T_PdDv, &pddv_info);
	if(pdat_info.num > 0)
		odm_free_list(T_PdAt, &pdat_info);
	if(pdcn_info.num > 0)
		odm_free_list(T_PdCn, &pdcn_info);

}

/*  */
/* NAME: mergeodm 
 *
 * FUNCTION: This function merge various ODM object class on the system
 * 	into a local ODMDIR.
 * 
 * NOTES: 
 *
 * RETURNS:
 *	0 successful
 *	<0 fail
 *
 */
int mergeodm(char *fromODMDIR, char *toODMDIR)
{
	char	criteria[255];
	int	i,j;
	short	not_found;
	char	*ptype, *pclass, *psclass;
	int	rc;

	odm_set_path(fromODMDIR);

	/* get all object class PDiagDev PDiagAtt PdDv PdAt and PdCn */
	/* that exist on the system.				     */

	T_PDiagDev = get_PDiagDev_list(PDiagDev_CLASS, "", &pdiagdev_info,
			MAX_EXPECT, DEPTH);
	num_PDiagDev = pdiagdev_info.num;

	T_PDiagAtt = get_PDiagAtt_list( PDiagAtt_CLASS, "", &pdiagatt_info, 
			MAX_EXPECT, DEPTH);
	num_PDiagAtt = pdiagatt_info.num;

	cudv_list_p = get_CuDv_list( CuDv_CLASS, "", &cudv_info, MAX_EXPECT, 2);
	num_CuDv = cudv_info.num;

	odm_set_path(toODMDIR);

	/* Now obtain all the object class added from the CDROM file system */

	new_T_PDiagDev = get_PDiagDev_list(PDiagDev_CLASS, "", &npdiagdev_info,
			MAX_EXPECT, DEPTH);
	new_num_PDiagDev = npdiagdev_info.num;

	new_T_PDiagAtt = get_PDiagAtt_list( PDiagAtt_CLASS, "", &npdiagatt_info,
			MAX_EXPECT, DEPTH);
	new_num_PDiagAtt = npdiagatt_info.num;

	/* now search the new list of object class. If the object class */
	/* that is  on the system does not exist , then add it.		*/
	if(num_PDiagDev > 0)
		for (i=0; i < num_PDiagDev; i++){
			not_found=1;
			for (j = 0; j < new_num_PDiagDev; j++)
			 	if ( !strcmp(T_PDiagDev[i].DType,
					new_T_PDiagDev[j].DType) &&
				     !strcmp(T_PDiagDev[i].DClass,
					new_T_PDiagDev[j].DClass) &&
				     !strcmp(T_PDiagDev[i].DSClass,
					new_T_PDiagDev[j].DSClass) ) {

			/* Object class already added from the CDRFS */
			/* go to the next one.			     */

					not_found=0;
					break;
				}
			if (not_found){
				rc=odm_add_obj(PDiagDev_CLASS, &T_PDiagDev[i]);
				if(rc == -1)
					return(rc);
			}
		}
	if(num_PDiagAtt > 0)
		for (i=0; i < num_PDiagAtt; i++){
			not_found=1;
			for (j = 0; j < new_num_PDiagAtt; j++)
			 	if ( !strcmp(T_PDiagAtt[i].DType, 
					new_T_PDiagAtt[j].DType) &&
				     !strcmp(T_PDiagAtt[i].DClass, 
					new_T_PDiagAtt[j].DClass) &&
				     !strcmp(T_PDiagAtt[i].DSClass, 
					new_T_PDiagAtt[j].DSClass) &&
				     !strcmp(T_PDiagAtt[i].attribute,
					new_T_PDiagAtt[j].attribute) ){

			/* Object class already added from the CDRFS */
			/* go to the next one.			     */

					not_found=0;
					break;
				}
			if(not_found){
				rc=odm_add_obj(PDiagAtt_CLASS, &T_PDiagAtt[i]);
				if(rc == -1)
					return(rc);
			}
		}

	/* Now go through CuDv and only add the PdDv, PdAt and PdCn */
	/* for devices in the CuDv list.			    */

	for (j=0; j < num_CuDv; j++){
		ptype = (char *)substrg(PTYPE, cudv_list_p[j].PdDvLn_Lvalue);
		pclass = (char *)substrg(PCLASS, cudv_list_p[j].PdDvLn_Lvalue);
		psclass = (char *)substrg(PSCLASS, cudv_list_p[j].PdDvLn_Lvalue);
		sprintf(criteria, "uniquetype = %s",
				cudv_list_p[j].PdDvLn_Lvalue);
		not_found=1;

		/* Before adding, make sure the PdDv is not already in */
		/* the new data base. If new data base does not have   */
		/* any entry, then just add the pre-defined in.	       */

		odm_set_path(toODMDIR);
		new_T_PdDv = get_PdDv_list( PdDv_CLASS, "", &npddv_info,
			MAX_EXPECT, DEPTH);
		new_num_PdDv = npddv_info.num;
		if(new_num_PdDv > 0)
		{
			for (i=0; i < new_num_PdDv; i++){
			 	if ( !strcmp(ptype, new_T_PdDv[i].type) &&
				     !strcmp(pclass, new_T_PdDv[i].class) &&
				     !strcmp(psclass, new_T_PdDv[i].subclass)){

				/* Object class already added from the CDRFS */
				/* go to the next one.			     */

					not_found=0;
					break;
				}
			}
			odm_free_list(new_T_PdDv, &npddv_info);
			npddv_info.num = 0;
			
		}
		/* Now add all PdDv, PdAt and PdCn belonging to the current */
		/* CuDv entry, into the new data base. If any of the odmadd */
		/* operation fails, skip those entries and go to the next   */
		/* one.							    */

		if(not_found){
			odm_set_path(fromODMDIR);
	  		T_PdDv = get_PdDv_list(PdDv_CLASS, criteria, &pddv_info,
					MAX_EXPECT, DEPTH);
			T_PdAt = get_PdAt_list(PdAt_CLASS, criteria, &pdat_info,
					MAX_EXPECT, DEPTH);
			T_PdCn = get_PdCn_list(PdCn_CLASS, criteria, &pdcn_info,
					MAX_EXPECT, DEPTH);
			odm_set_path(toODMDIR);
			if(pddv_info.num > 0){
				for (i=0; i < pddv_info.num; i++){
					rc=odm_add_obj(PdDv_CLASS, &T_PdDv[i]);
					if(rc == -1)
					     break;
				}
 				odm_free_list(T_PdDv, &pddv_info);
			}
			if(pdat_info.num > 0){
		  		for (i=0; i < pdat_info.num; i++){
			  		rc=odm_add_obj(PdAt_CLASS, &T_PdAt[i]);
					if(rc == -1)
					     break;
				}
				odm_free_list(T_PdAt, &pdat_info);
			}
			if(pdcn_info.num > 0){
	 			for (i=0; i < pdcn_info.num; i++){
		 			rc=odm_add_obj(PdCn_CLASS, &T_PdCn[i]);
					if(rc == -1)
					     break;
			 	}
		                odm_free_list(T_PdCn, &pdcn_info);
			}
		}
	}
	return(rc);
}
