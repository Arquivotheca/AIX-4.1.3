static char sccsid[] = "@(#)00	1.2  src/bos/usr/lib/methods/common/cfgchk_64p.c, cfgmethods, bos411, 9428A410j 1/8/92 07:34:11";
/*
 * COMPONENT NAME:  (CFGMETH) Configuration Method
 *
 * FUNCTIONS:	chk_64_port
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 ***********************************************************************
 * 
 * MODULE PURPOSE
 *     This module implements the checking for conflicts between
 *     bus memory attributes of 64 port adapters and any/all other
 *     adapters in the system.  This check is required due to the
 *     problem described below.
 *     
 * PROBLEM DESCRIPTION
 *     The 64-port async adapter has a hardware bug such that it does
 *     not look at the high order 8 bits of the bus memory address 
 *     lines.  What this situation means is that the 64-port will 
 *     take writes to an address that is not his and will respond to
 *     reads from an address that is not his.
 *
 * OVERVIEW of CHECKING 64 PORT BUS MEMORY CONFLICT
 *     This code will check for any existing conflicts caused by the
 *     64-port looking only at the low 24 bits of the bus memory address
 *     lines.  It will then report the names of the 64-port adapters
 *     that are conflicting.  No corrective action is taken by this pgm.
 *
 * GENERAL ALGORITHM
 *     Malloc space for 64 port table
 *     LOOP through each bus in system
 *         Build 64 port table for this bus (including expansion units (EU))
 *         LOOP through adapters in this bus (including bus extenders).
 *             Determine lower 24 bit space usage for this adapter
 *             Check for conflicts with each 64 port adapter in this bus.
 *         END_LOOP
 *     END_LOOP
 *     return results to caller.
 *
 * The following functions implement the algorithm above.  chk_64_port 
 *     is the controlling function.
 *
 ****************************************************************************
 */
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include "cfgdebug.h"

#define TOP_ONLY 1
#define L24_MAX  0xFFFFFF

/* 
 *--------------------------------------------------------------
 * The following structure is used to define the usage of
 * a 64 port adapters bus_mem_addr attribute.  A list (array)
 * of these structures is generated so that every other MCA
 * adapters type M and type B attributes can be checked against
 * each 64 port.
 *--------------------------------------------------------------
 */
typedef struct
{
    char	name[8] ;	/* name of the adapter			*/
    ulong	start	;	/* starting bus mem addr		*/
    ulong	size	;	/* size of the bus memory allocated	*/
    char	reported;	/* bool TRUE if adapt caused conflict.  */
} p64_tbl_entry_t, *p64_tbl_entry_p ;

/* 
 *--------------------------------------------------------------
 * The following structure is used to describe the low 24 bit
 * address space usage of an individual type M or type B 
 * attribute for a single adapter.  No table is made, each
 * type M or B attribute is checked individually for conflicts
 * with the 64 port adapters.
 *--------------------------------------------------------------
 */
typedef struct
{
    ulong	start	;	/* low 24 starting bus mem addr		*/
    ulong	size    ;	/* size (fr start to 24 bit MAX_ADDR    */
    ulong	wrap_size ;     /* amount of wrap (start fr 0).		*/
    int		full_24 ;	/* flag; TRUE if total size uses ALL of
				   low 24 bit address space.		*/ 
} L24_info_t, *L24_info_p ;

/* 
 *--------------------------------------------------------------
 * The following are variables used in this module only.
 * They define the uniquetype values used to check for
 * various types of devices (adapters).
 *--------------------------------------------------------------
 */
static char UT_64P[] = "adapter/mca/64p232" ;
static char UT_EU[] = "adapter/mca/eu"   ;
static char UT_SIO[] = "adapter/mca/sio" ;


int
chk_64_port(char  *conf_list)
/* 
 *********************************************************************
 * NAME     : chk_64_port
 * FUNCTION : controls the process of checking for conflicts in the
 *            low 24 bit range of bus memory between all MCA adapters
 *            and 64 port adapters.
 * INPUTS   : none
 * OUTPUTS  :
 *     conf_list - List of names of 64 port adapters with conflicts
 *     return    - -2 => unable to malloc space for tables
 *                 -1 => ODM error accessing database
 *              FALSE => No conflicts detected
 *               TRUE => Conflicts detected
 * NOTES    :
 *     - If the return value from this routine is negative, conf_list
 *       may still contain names of conflicts detected BEFORE the
 *       error was detected causes the negative return code.
 ********************************************************************* 
 */
{
    struct CuDv *adp_list ;		/* CuDv ptr for adapter list	*/
    struct CuDv *bus_list ;		/* CuDv ptr for bus list	*/
    struct listinfo adp_lst_info ;	/* ODM list info for adaptr list*/
    struct listinfo bus_lst_info ;	/* ODM list info for bus list	*/

    char	sstr[128]       ;	/* search string for ODM queries*/
    p64_tbl_entry_p p64_list   ;	/* ptr to list of 64 ports info */
    int		nmbr_p64s      ;	/* nmbr of 64 ports in list	*/
    int		conf_detected  ;	/* flag to gen msg only once	*/
    int		i              ;

/* BEGIN chk_64_port */
    DEBUG_0("in 'chk_64_port'...\n") ;

    conf_detected = FALSE ;
    conf_list[0] = '\0' ;
   /* 
    *-----------------------------------------------------
    * Malloc space for 64 port table.  Malloc enough 
    * for all cards in the system.
    * NOTE : the ODM search is done for 64 ports in an
    *        DEFINED state because the config method
    *        leaves them unconfigured if it detects a 
    *        conflict.
    *-----------------------------------------------------
    */
    sprintf(sstr, 
	   "PdDvLn=%s and status=%d and chgstatus!=%d and chgstatus!=%d", 
	    UT_64P, DEFINED, DONT_CARE, MISSING) ;
    adp_list = odm_get_list(CuDv_CLASS, sstr, &adp_lst_info,
			    6, TOP_ONLY) ;
    nmbr_p64s = adp_lst_info.num ;
    odm_free_list(adp_list, &adp_lst_info) ;

    if (nmbr_p64s > 0)
    {
	DEBUG_1("chk_64: mallocing space -> nmbr_p64s = %d\n", nmbr_p64s) ;

	p64_list = malloc(nmbr_p64s * sizeof(p64_tbl_entry_t)) ;
	if (p64_list == NULL)
	{
	    DEBUG_0("chk_64: malloc failed!\n") ;
	    conf_detected = -2 ;
	}
	else
	{
	   /* 
	    *--------------------------------------------------
	    * Got at least 1 64 port in system.  Loop thru
	    * each bus checking for conflicts on a per bus
	    * basis.
	    * NOTE: the search criteria on the odm_get_list
	    *       MUST include DEFINED adapters because the
	    *       the 64 port config method leaves the 
	    *       adapter unconfigured if a bus memory 
	    *       conflict is detected.  The bld_64p_tbl
	    *       uses the adapter list to get the 64 ports
	    *       from.
	    *--------------------------------------------------
	    */
	    sprintf(sstr, "name like bus* and status=%d", AVAILABLE) ;
	    bus_list = odm_get_list(CuDv_CLASS, sstr, 
				    &bus_lst_info, 2, TOP_ONLY) ;
	    if ((bus_list != NULL) &&
		((int)bus_list != -1))
	    {
		for (i = 0; i < bus_lst_info.num; i++)
		{
		    DEBUG_1("chk_64: processing %s\n", bus_list[i].name) ;
    		    sprintf(sstr, 
			   "parent=%s and chgstatus!=%d and chgstatus!=%d", 
	    		    bus_list[i].name, DONT_CARE, MISSING) ;
		    adp_list = odm_get_list(CuDv_CLASS, sstr, 
					    &adp_lst_info,
					    8, TOP_ONLY) ;
		    if ((int)adp_list != -1)
		    {
			DEBUG_1("chk_64: got adp_list, cnt = %d\n", 
				adp_lst_info.num) ;
			nmbr_p64s = bld_64p_tbl(adp_list, 
						adp_lst_info.num, 
						p64_list) ;
			DEBUG_1("chk_64: back from bld_64p; nmbr=%d\n",
				nmbr_p64s) ;
			if (nmbr_p64s > 0)		
			{
			    conf_detected |= chk_adapters(adp_list, 
							  adp_lst_info.num,
							  p64_list, 
							  nmbr_p64s,
							  conf_list) ;
			} 
			odm_free_list(adp_list, &adp_lst_info) ;
		    }
		    else /* an ODM error occured getting the adapter list */
		    {
			DEBUG_0("chk_64: ODM_ERR getting adp_list!\n") ;
			conf_detected = -1 ;
		    }
		} /* end loop thru bus_list */
		odm_free_list(bus_list, &bus_lst_info) ;
		free(p64_list) ;
	    }
	    else /* an ODM error occurred getting the list of busses */
	    {
		conf_detected = -1 ;
	    }
	}
    }
    DEBUG_1("chk_64: exiting; rtn code = %d\n", conf_detected) ;
    return(conf_detected) ;
} /* END chk_64_port */

int
bld_64p_tbl(struct CuDv  *adp_list, 
	    int	         nmbr_adps,
	    p64_tbl_entry_p p64_list)
/* 
 *********************************************************************
 * NAME     : bld_64p_tbl
 * FUNCTION : builds a table containing bus memory information for
 *            each 64 port adapter in the given list of adapters
 * INPUTS   :
 *     adp_list - Ptr to list of adpaters.
 *     nmbr_adps- Nmbr of adapters in list
 * OUTPUTS  :
 *     p64_list-Contents of array updated with info about each 64
 *              port adapter.
 *     return - Count of 64 port adapters in the adapter list.
 * NOTES    :
 *        -   This routine must handle bus extenders.
 *        -   The check to determine if a 64 port adapter is bad
 *            is made by checking to see if the config method
 *            configured it or not.  If the method did NOT configure
 *            the adapter (status == DEFINED), then the adapter is
 *            bad (and also has a conflict).  Otherwise, the adapter
 *            is assumed to be good (it might be bad, but it does not
 *            conflict with anybody).
 ********************************************************************* 
 */
{
    struct CuDv *eu_list ;		/* CuDv ptr for adp's in eu list*/
    struct listinfo eu_lst_info ;	/* ODM list info for above list	*/

    struct PdAt pdat  ;			/* For access to width field.	*/
    struct CuAt *attr ;			/* Ptr to bus_mem_addr attribute*/
    int		attr_nmbr ;

    char	sstr[64] ;	/* search string for ODM queries	*/
    int		tmp_nmbr  ;	/* holder for count to return to caller	*/
    int		i ;		/* for going through adapter list	*/

/* BEGIN bld_64p_tbl */
    DEBUG_0("bld_64p: Entering 'bld_64p_tbl'...\n") ;

    tmp_nmbr = 0 ;
    for (i = 0; i < nmbr_adps; i++)
    {
	if (!strcmp(UT_64P, adp_list[i].PdDvLn_Lvalue))
	{
	    DEBUG_1("bld_64p: Got 64 port => %s\n", adp_list[i].name) ;
	    if (adp_list[i].status == DEFINED)
	    {
	       /* 
		*----------------------------------------------
		* The adapter is a 64 port adapter,
		* so fill its table with bus_mem_addr 
		* info. 
		*----------------------------------------------
		*/
		strcpy(p64_list[tmp_nmbr].name, adp_list[i].name) ;
		attr = getattr(adp_list[i].name, "bus_mem_addr", FALSE,
			       &attr_nmbr) ;
		if (attr != NULL)
		{
		    sprintf(sstr, "uniquetype=%s and attribute=%s", 
			    adp_list[i].PdDvLn_Lvalue, attr->attribute) ;
		    attr_nmbr = (int)odm_get_obj(PdAt_CLASS, sstr, &pdat, 
						 ODM_FIRST) ;
		    if (attr_nmbr == 0 || attr_nmbr == -1)
			strcpy(pdat.width, "0x40000") ;

		    p64_list[tmp_nmbr].start = strtoul(attr->value,
						       NULL, 0) ;
		    p64_list[tmp_nmbr].size = strtoul(pdat.width,
						      NULL, 0) ;
		    p64_list[tmp_nmbr].reported = FALSE ;
		    DEBUG_2("bld_64p: start = 0x%x\tsize = 0x%x\n",
			    p64_list[tmp_nmbr].start, 
			    p64_list[tmp_nmbr].size) ;
		    tmp_nmbr++ ;
		}
	    }
	}
	else if (!strcmp(UT_EU, adp_list[i].PdDvLn_Lvalue))
	{
	   /* 
	    *---------------------------------------------
	    * Adapter is an expansion unit, 
	    * so process its children.
	    *---------------------------------------------
	    */
	    DEBUG_1("bld_64p: expansion unit detected => %s\n",
		    adp_list[i].name) ;

    	    sprintf(sstr, "parent=%s and chgstatus!=%d and chgstatus!=%d", 
	   	    adp_list[i].name, DONT_CARE, MISSING) ;
	    eu_list = odm_get_list(CuDv_CLASS, sstr, &eu_lst_info,
				   8, TOP_ONLY) ;
	    if ((int)eu_list != -1)
	    {
		DEBUG_1("bld_64p: Got adp list for eu; cnt=%d\n",
			eu_lst_info.num) ;
		if (eu_lst_info.num > 0)
		{
		    tmp_nmbr += bld_64p_tbl(eu_list, eu_lst_info.num,
					    p64_list + tmp_nmbr) ;
		    odm_free_list(eu_list, &eu_lst_info) ;
		}
	    }
	}
    }
    DEBUG_1("bld_64p: returning; rtn code=%d\n", tmp_nmbr) ;
    return(tmp_nmbr) ;
} /* END bld_64p_tbl */


int
chk_adapters(struct CuDv     *adp_list,
	     int	     nmbr_adps,
	     p64_tbl_entry_p p64_list,
	     int	     nmbr_p64s,
	     char	     *conf_list)
/* 
 *********************************************************************
 * NAME     : check adapters
 * FUNCTION : Loops through the adapters checking type M attributes
 *            against those in the 64 port list.
 * INPUTS   :
 *    adp_list - Ptr to list (array) of MCA adapters to check
 *    nmbr_adps- nmbr of adapters in above list
 *    p64_list - Ptr to list (array) of 64 port table entries
 *    nmbr_p64s- nmbr of entries in above list
 * OUTPUTS  :
 *    return   - TRUE if a conflict is found
 *               FALSE if no conflict found
 *    conf_list- String containing names of 64 ports with conflicts.
 * NOTES    :
 *            This routine must handle bus extenders.  It is 
 *            accomplished via recurrsion.
 ********************************************************************* 
 */
{
    struct CuDv *eu_list ;		/* CuDv ptr for adp's in eu list*/
    struct listinfo eu_lst_info ;	/* ODM list info for above list	*/
    struct CuAt *attr ;			/* Ptr to attrs rtned by getattr*/

    char	sstr[128] ;		/* search str for ODM queries	*/
    L24_info_t	L24_info  ;		/* low 24 space usage info	*/
    int		nmbr_attrs;		/* nmbr attrs rtned fr getattr. */
    int		i         ;		/* counter.			*/
    int		j         ;		/* counter.			*/
    int		conflict = FALSE  ;	/* flag TRUE if conf, else FALSE*/

/* BEGIN chk_adapters */
    DEBUG_0("chk_adp: Entering 'chk_adapters'...\n") ;

    for (i = 0; i < nmbr_adps; i++)
    {
	if (adp_list[i].status == AVAILABLE)
	{
	    if (strcmp(UT_64P, adp_list[i].PdDvLn_Lvalue))
	    {
	       /* This adapter is NOT a 64 port. */

	        if ((!strcmp(UT_EU, adp_list[i].PdDvLn_Lvalue)) ||
		    (!strncmp(UT_SIO, adp_list[i].PdDvLn_Lvalue,
			      sizeof(UT_SIO))))
	        {
	           /* 
		    *----------------------------------------
		    * Adapter is a bus extender, get and
		    * check its children.  We only care 
		    * about AVAILABLE status this time 
		    * because we are checking adapters only.
		    *----------------------------------------
		    */
		    DEBUG_1("chk_adp: Got bus extender => %s\n",
			    adp_list[i].name) ;

		    sprintf(sstr, "parent=%s and status=%d", 
			    adp_list[i].name, AVAILABLE) ;
		    eu_list = odm_get_list(CuDv_CLASS, sstr, &eu_lst_info,
				           8, TOP_ONLY) ;
		    if ((int)eu_list != -1)
		    {
		        DEBUG_1("chk_adp: got adps in extender; cnt=%d\n",
			        eu_lst_info.num) ;
		        if (eu_lst_info.num > 0)
		        {
			    conflict |= chk_adapters(eu_list, eu_lst_info.num,
						     p64_list, nmbr_p64s,
						     conf_list) ;
			    odm_free_list(eu_list, &eu_lst_info) ;
		        }
		    }
	        }
	        else
	        {
	           /* 
		    *-----------------------------------------
		    * This adapter is a "normal" adapter that
		    * must be checked for conflicts.
		    *-----------------------------------------
		    */
		    attr = getattr(adp_list[i].name, NULL, TRUE, &nmbr_attrs) ;
		    DEBUG_2("chk_adp: got %d attrs for %s\n", nmbr_attrs,
			    adp_list[i].name) ;
		    if (attr != NULL)
		    {
		        for (j = 0; j < nmbr_attrs; j++)
		        {
			    DEBUG_2("chk_adp: working w/ attr %s; type = %s\n",
				    attr[j].attribute, attr[j].type) ;
			    if (attr[j].type[0] == 'M' ||
			        attr[j].type[0] == 'B')
			    {
			        DEBUG_1("chk_adp: bus mem attr=>%s\n",
				        attr[j].attribute) ;
			        conflict |= calc_low_24_usage(&adp_list[i], 
							      &attr[j], 
							      &L24_info) ;
			        conflict |= chk_L24_conflicts(&L24_info, 
							      p64_list, 
							      nmbr_p64s, 
							      conf_list) ;
			    }
		        }   /* end loop thru attributes */
		        free(attr) ;
		    }
	        }   /* end "normal" adapter processing */
	    }   /* end non_64 port processing */ 
	}   /* end AVAILABLE processing */
    }   /* end loop thru adapters */

    DEBUG_1("chk_adp: returning; rtn code = %d\n", conflict) ;
    return(conflict) ;
} /* END chk_adapters */


static int
calc_low_24_usage(struct CuDv *cudv,
		  struct CuAt *attr,
		  L24_info_t  *L24_info)
/* 
 *********************************************************************
 * NAME     : Calculate Lower 24 Bit Usage
 * FUNCTION : Calculate the range of lower 24 bit address space used
 *            by the input attribute.
 * INPUTS   :
 *     attr    - Ptr to adpater's type M attribute.
 * OUTPTUS  :
 *     L24_info- Ptr to structure into which the lower 24 bit usage
 *               information is to be placed.
 * NOTES    :
 *            This routine expects to get type M or B attributes ONLY!
 ********************************************************************* 
 */
{
    char	sstr[128] ;	/* search string for getting PdAt	*/
    struct PdAt	pdat      ;	/* Predef Attr for CuAt being processed	*/
    struct CuAt	*width    ;	/* ptr to CuAt struct for attr width	*/
    ulong	top	  ;

/* BEGIN calc_low_24_usage */
    DEBUG_0("calc: Entering 'calc_low_24_usage'...\n") ;

    L24_info->start = 0 ;
    L24_info->size  = 1 ;
    L24_info->wrap_size = 0 ;
    L24_info->full_24   = FALSE ;

    sprintf(sstr, "uniquetype=%s and attribute=%s", cudv->PdDvLn_Lvalue, 
	    attr->attribute) ;
    DEBUG_1("calc: getting PdAt for %s\n", sstr) ;
    if ((int)odm_get_obj(PdAt_CLASS, sstr, &pdat, ODM_FIRST) == -1)
    {
	DEBUG_0("calc: ODM_ERR getting PdAt\n") ;
	return -1 ;
    }

    if (strlen(pdat.width) == 0)
    {
        DEBUG_0("calc: attr's width is customizable\n") ;
       /* 
	*---------------------------------------------------
	* Attribute's width is customizable.  Must get the
	* attribute that specifies that width.
	*---------------------------------------------------
	*/
	sprintf(sstr, "uniquetype=%s and width=%s",
		pdat.uniquetype, pdat.attribute) ;
	top = (ulong)odm_get_obj(PdAt_CLASS, sstr, &pdat, ODM_FIRST) ;
	if (((int)top != -1) && (top != 0))
	{
	    width = getattr(cudv->name, pdat.attribute, FALSE, &top) ;
	    if (width == NULL)
	    {
		DEBUG_1("calc: getattr failed on %s\n", pdat.attribute) ;
		L24_info->size = strtoul(pdat.deflt, NULL, 0) ;
	    }
	    else
	    {
		L24_info->size = strtoul(width->value, NULL, 0) ;
		free(width) ;
	    }
	}
	else
	{
	    DEBUG_0("calc: ODM_ERR getting width specifier\n") ;
	    return(-1) ;
	}
    }
    else
    {
	L24_info->size  = strtoul(pdat.width, NULL, 0) ;
    }
    L24_info->start = strtoul(attr->value, NULL, 0) ;
    DEBUG_2("calc: orig start = 0x%x;\torig size = 0x%x\n", 
	    L24_info->start, L24_info->size) ;

    L24_info->start &= L24_MAX ;		/* get low 24 bit start */

    if (!(L24_info->full_24 = (L24_info->size >= L24_MAX)))
    {
       /* 
	*-----------------------------------------------
	* Attribute does NOT use the full range of the
	* lower 24 bit address space.
	* So check for any wrap and calc its size.
	*-----------------------------------------------
	*/
	top = L24_info->start + L24_info->size - 1;
	if (top > L24_MAX)
	{
	    L24_info->size = (L24_MAX - L24_info->start) + 1 ;
	    L24_info->wrap_size = top - L24_MAX ;
	}
    }

    DEBUG_4("calc: exiting, L24_info :\n\tstart\t0x%x\n\tsize\t0x%x\n\twrap_size 0x%x\n\tfull_24\t0x%x\n",
	    L24_info->start, L24_info->size, L24_info->wrap_size, 
	    L24_info->full_24) ;

    return(0) ;
} /* END calc_low_24_usage */


static int
chk_L24_conflicts(L24_info_t      *L24_info ,
		  p64_tbl_entry_p p64_list ,
		  int		  nmbr_p64s,
		  char		  *conf_list)
/* 
 *********************************************************************
 * NAME     : Check for lower 24 conflicts
 * FUNCTION : Check the L24 info against each 64 port in the input
 *            list of 64 port adapters.  If conflicts are found
 *            add the offending 64 port adapters name to the end
 *            of the string.
 * INPUTS   :
 *     L24_info  - Ptr to low 24 bit usage iformation
 *     64p_list  - Ptr to list of 64 port usage information
 *     nmbr_p64s - Number of 64 ports in above list      
 *     conf_list - Ptr to string containing list of conflicting 64 ports
 * OUTPUTS  :
 *     conf_list - Updated string containing any newly detected conflicts.
 *     return    - TRUE if conflicts detected;
 *                 FALSE if no conflicts detected.
 * NOTES    :
 *            This routine modifies fields in the table of 64 port 
 *            adapters if a conflict is detected.  The "reported"
 *            flag is set to TRUE.
 ********************************************************************* 
 */
{
    static int	first_conflict = TRUE ;	/* for putting commas name list*/

    int		i ;			/* loop counter	      		*/
    ulong	adp_top ;		/* last addr in range for adapt.*/
    ulong	p64_top ;		/* last addr in range for 64 pt */
    int		conf_detected ;		/* tmp flag			*/

/* BEGIN chk_L24_conflicts */
    DEBUG_0("chk_L24: Entering 'chk_L24_conflicts'...\n") ;

    for (i = 0; i < nmbr_p64s; i++)
    {
	DEBUG_1("chk_L24: chking 64 port %s\n", p64_list[i].name) ;
	if (!(conf_detected  = L24_info->full_24))
	{
	    DEBUG_0("chk_L24: adp does NOT use all low 24\n") ;
	   /* 
	    *------------------------------------------------
	    * Current adapter does NOT use all of low 24
	    * bit range.  So check for overlaps.
	    * NOTE : these checks were taken from busresolve.
	    * 	     They take some thought to verify, but
	    *        they do work.
	    *------------------------------------------------
	    */
	    p64_top = (p64_list[i].start + p64_list[i].size) - 1 ;
	    adp_top = (L24_info->start + L24_info->size) - 1 ;
	    if ((L24_info->start > p64_top) ||
		(adp_top < p64_list[i].start))
	    {
		DEBUG_0("chk_L24: no conflict in main area; checking wrap\n") ;
	       /* 
		*--------------------------------------------
		* No conflict yet.  Check to see if the
		* adapter wrapped its low 24 usage and
		* check conflicts again if it did.
		*--------------------------------------------
		*/
		if (L24_info->wrap_size > 0)
		{
		    adp_top = L24_info->wrap_size - 1 ; /* wrap starts fr 0 */

		    conf_detected = !((L24_info->start > p64_top) ||
				      (adp_top < p64_list[i].start)) ;
		}
	    }
	    else
	    {
		DEBUG_0("chk_L24: CONFLICT DETECTED IN MAIN AREA\n") ;
		conf_detected = TRUE ;
	    }    
	}

	if (conf_detected && !p64_list[i].reported)
	{
	    DEBUG_0("chk_L24: CONFLICT DETECTED!!; marking 64port\n") ;
	    p64_list[i].reported = TRUE ;
	    if (first_conflict)
	    {
		strcpy(conf_list, p64_list[i].name) ;
		first_conflict = FALSE ;
	    }
	    else
	    {
		sprintf(conf_list, "%s, %s", conf_list, 
			p64_list[i].name) ;
	    }
	}
    }   /* end loop thru 64 port table */

    DEBUG_1("chk_L24: leaving; rc = %d\n", conf_detected) ;
    return(conf_detected) ;
} /* END chk_L24_conflicts */
