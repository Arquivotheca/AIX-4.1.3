static char sccsid[] = "@(#)67	1.1.3.5  src/bos/usr/lib/methods/cfgchk/cfgchk.c, cfgmethods, bos411, 9428A410j 5/9/94 15:56:45";
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: BB_test
 *		TR_test
 *		bstrncpy
 *		dev_ID
 *		free_vbuf
 *		generror
 *		genexit
 *		get_LL
 *		get_byte
 *		is_OEM_card
 *		is_desktop
 *		log_error
 *		main
 *		null_ptrs
 *		parse_vpd
 *		real_isprint
 *		ucode_chk
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/****************************************************************
****************************************************************/
#include <stdio.h>
#include <diag/diag.h>
#include <diag/diago.h>
#include <sys/cfgdb.h>
#include <diag/diagvpd.h>	/* vpd info and structures		*/
#include <sys/errids.h>		/* errlog() structure and #defines	*/
#include <sys/mdio.h>		/* NVRAM data structure			*/
#include <sys/audit.h>
#include "hlcmds.h"
#include <cf.h>			    /* cfg err codes and findmcode parms.   */
#include "cfgchk_msg.h"
#include "cfgchk.h"
#include "cfgdebug.h"

/* FUNCTION PROTOTYPES */
char *BB_test(void);			/* Tests BlueBonnet config. 	*/
int is_OEM_card(struct CuDv);		/* Determines card origin	*/
int dev_ID(char *);			/* Determines device ID		*/
char *TR_test(void);			/* Tests Token Ring config.	*/
int ucode_chk(char *)	;		/* chk for ucode download failures */
unsigned int get_LL(struct CuDv,char *);/* get adapter level of object  */
unsigned char is_desktop(void);		/* Determines desktop models	*/
void generror(int, char *);		/* generate error message       */
void genexit(int, char *, int);		/* gen err msg and exit.        */
void log_error(int, char *);		/* Logs errors			*/
int parse_vpd(char *, VPDBUF *, int);
void free_vbuf(VPDBUF *vbuf);
void null_ptrs(VPDBUF *vbuf);
void bstrncpy(char *, char *, int, int);
int real_isprint(char);
int get_byte(int, char *, char *);

extern char *strstr(char *, char *);

#define MAX_BUS   3			/* max bus number               */

#define NUM_TESTS 2			/* For each SRN that might be	*/
char *(*tests[NUM_TESTS])(void) =	/* generated, call a function	*/
	{				/* to perform the test.  If no	*/
	BB_test,			/* problem exists the function	*/
	TR_test				/* returns a 0 length string.	*/
	};				/* The list of these functions	*/
					/* is pointed to by tests[].	*/

/*  */
/****************************************************************
*			PSEUDO CODE
*
*  For each SRN that might be generated:
*	{
*	Call the subroutine that tests for a bad configuration
*		{
*		For each CuDv:
*			{
*			If a CuDv is the source of a bad
*			configuration, then log an error
*			with the device name
*			}
*		}
*
*	If the subroutine returns with a valid string, then
*	   add this string to the list of SRN's to be printed
*	}
*
*  If no SRN's were generated exit with 0
*  If 1 SRN was generated, then print message ERROR1
*  If > 1 SRN's were generated, then print message ERROR2
*
****************************************************************/

/*  */
/****************************************************************
* NAME: main
*
* FUNCTION: Controls flow of configuration testing
*	    See PSEUDO CODE above
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	All routines pointed to by tests[]()
*	generror()
*       genexit()
*       ucode_chk()
*
* RETURNS:
*	0	(=no SRN's generated)
*	ERROR1	(=1 SRN generated)
*	ERROR2	(=2 or more SRN's generated)
*	ERROR3	(=error retrieving ODM data)
*	ERROR4	(=error retrieving NVRAM data)
****************************************************************/
main()
{
  int test_cnt;
  int i1;
  int prev_error ;                       /* used to hold err_nmbr */

  char *service_no[NUM_TESTS];
  char out_str[128];
  char *fmt;

	setlocale(LC_ALL,"");

	/****************************************************************
	* Call each test.  If the test returns a string longer than 0,	*
	* then add this string (an SRN) to the overall string of SRN's.	*
	****************************************************************/
	odm_initialize();

        prev_error = 0 ;

        i1 = chk_64_port(&out_str) ;
        if (strlen(out_str) > 0)
	{
	    generror(ERROR1, "834_990") ;
	    prev_error = ERROR1 ;
	}
        if (i1 < 0)
	{
	    generror(ERROR3, NULL) ;
	}

        ucode_chk(&out_str) ;
        if (strlen(out_str) > 0)
        {
	    generror(ERROR5, out_str);
            prev_error = ERROR5 ;
        }

	test_cnt = 0;
	out_str[0] = '\0';

	for (i1=0; i1<NUM_TESTS; i1++)
	{
	  service_no[i1] = tests[i1]();
	  if (strlen(service_no[i1]) > 0)
	  {
	    if (++test_cnt > 1) 
                strcat(out_str, ", ");
	    strcat(out_str, service_no[i1]);
	  }
	}

	/****************************************************************
	* If one SRN was generated, exit with ERROR1.  If > 1 SRN was	*
	* generated, exit with ERROR2.  Else, all is fine.		*
	****************************************************************/
	if (test_cnt == 1)
        {
	    genexit(ERROR1, out_str, ERROR1);
        }
	else if (test_cnt > 1)
        {
	    genexit(ERROR2, out_str, ERROR2);
        }
        else
        {
            genexit(0, NULL, prev_error) ;
        }
} /* END main */


/*  */
/****************************************************************
* NAME: BB_test
*
* FUNCTION: Test for bad BlueBonnet configurations
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	is_OEM_card()
*	parse_vpd()
*	log_error()
*	genexit()
*
* RETURNS:
*	NULL string  (= no bad configurations)
*	SRN value    (= bad configuration with 1
*			or more BlueBonnett cards)
****************************************************************/
char *BB_test(void)
{
  int i1, j1, k1;
  int tot_BB_cnt;			/* Total # of BB cards  	*/
  int bad_BB_cnt;			/* # of BB cards w/bad part nos.*/
  int OEM_cnt;				/* Total # of OEM cards		*/
  int bus_cnt ;				/* cntr for bus loop.		*/

  char SRN_str[8];
  char criteria[80];
  char *PN_str;				/* String containing Part No.	*/
  char BB_name[MAXSLOTS][ERR_NAMESIZE]; /* Device names of bad BB cards	*/

  struct CuDv  *cudv;
  struct CuVPD *cuvpd;
  struct listinfo cudv_info;
  struct listinfo cuvpd_info;

  VPDBUF vpdbuf;			/* Buffer containing VPD info.	*/

    /****************************************************************
    * Initialization:						*
    ****************************************************************/
    SRN_str[0] = '\0';

    for (bus_cnt = 0; bus_cnt <= MAX_BUS; bus_cnt++)
    {
	tot_BB_cnt = 0;
	bad_BB_cnt = 0;
	OEM_cnt = 0;
	/****************************************************************
	* Retrieve all possible CuDv in bus.  Look for CuDv that have a	*
	* PdDvLn_Lvalue of BB_LVALUE.  Parse the related VPD data and	*
	* find the part number (PN).  If the PN matches the list of bad	*
	* PN's, then save the device name.  Also, count the number of	*
	* OEM cards.							*
	****************************************************************/

	sprintf(criteria, "chgstatus != %d and parent=bus%1d", 
		MISSING, bus_cnt);
	if ((int )(cudv = get_CuDv_list(CuDv_CLASS, criteria,
		  &cudv_info, 20, 2)) < 0) genexit(ERROR3, NULL, ERROR3);

	for (k1=0; k1<cudv_info.num; k1++)
	  {
	  if (!strcmp(cudv[k1].PdDvLn_Lvalue, BB_LVALUE))
	    {
	    tot_BB_cnt++;
	    sprintf(criteria, "name=%s", cudv[k1].name);
	    if ((int )(cuvpd = get_CuVPD_list(CuVPD_CLASS, criteria,
			&cuvpd_info, 1, 1)) < 0) genexit(ERROR3, NULL, ERROR3);
	    if (cuvpd_info.num != 1) genexit(ERROR3, NULL, ERROR3);

	    parse_vpd(cuvpd[0].vpd, &vpdbuf, 0);
	    for (j1=0; j1<vpdbuf.entries; j1++)
	      {
	      PN_str = vpdbuf.vdat[j1];
	      if (!strncmp(++PN_str, VPD_PN_STR, strlen(VPD_PN_STR)))
		{
		PN_str += 3;
		j1 = vpdbuf.entries;
		}
	      }
	    if (j1 == vpdbuf.entries) genexit(ERROR3, NULL, ERROR3);

	    for (j1=0; j1<NUM_BB_PARTS; j1++)
	      if (strstr(PN_str, BB_part_nos[j1]) != (char *)NULL)
		{
		strcpy(BB_name[bad_BB_cnt++], cudv[k1].name);
		break;
		}

	    free_vbuf(&vpdbuf);
	    odm_free_list(cuvpd, &cuvpd_info);
	    }

	  if (is_OEM_card(cudv[k1])) OEM_cnt++;
	  }

	odm_free_list(cudv, &cudv_info);

	/****************************************************************
	* If there is a total of 3 or more BB cards OR a total of 2 or	*
	* more OEM cards, then log an error for each bad BB card.  If	*
	* there are no bad BB cards, then no error will be logged and	*
	* no SRN will be generated.					*
	****************************************************************/
	if ((tot_BB_cnt > 2) || (OEM_cnt > 1))
	  for (k1=0; k1<bad_BB_cnt; k1++)
	    {
	    strcpy(SRN_str, BB_SRN);
	    log_error(ERRID_SCSI_ERR9, BB_name[k1]);
	    }
    } /* END bus_loop */
    return SRN_str;
}

/*  */
/****************************************************************
* NAME: is_OEM_card
*
* FUNCTION: Determine if a card is of IBM or OEM origin
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	dev_ID()
*
* RETURNS:
*	TRUE  = Card is an OEM built card
*	FALSE = Card is not an OEM card (is an IBM card)
*
****************************************************************/
int is_OEM_card(struct CuDv cudv)
{
  int i1;

	if (strcmp(cudv.PdDvLn->subclass, "mca"))
	  return FALSE;
	i1 = dev_ID(cudv.PdDvLn->devid);
	if (i1 == SIXTY_FOUR_PORT)
	  return FALSE;
	else
	  {
	  if ((i1 >= OEM_LOWER) && (i1 <= OEM_UPPER))
	    return TRUE;
	  else
	    return FALSE;
	  }
}

/*  */
/****************************************************************
* NAME: dev_ID
*
* FUNCTION: Determine the device ID of a CuDv
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	genexit()
*
* RETURNS:
*	OEM_LOWER - 1	(= device ID does not indicate OEM card)
*	n		(= device ID, either within OEM card
*			 range or not)
****************************************************************/
int dev_ID(char *hex_str)
{
char ID_str[7];
char temp[3];
int i1;

	/****************************************************************
	* The string containing the device ID is assumed to be a 6 	*
	* characters hex value beginning with "0x" of "0X".  It is also	*
	* byte swapped.  Calculate the correct ID value and return it.	*
	****************************************************************/
	i1 = OEM_LOWER - 1;
	if (strlen(hex_str) > 0)
	  {
	  strncpy(temp, hex_str, 2);
	  temp[2] = '\0';
	  if (!strcmp(temp, "0x") || !strcmp(temp, "0X"))
	    {
	    if (strlen(hex_str) != 6) genexit(ERROR3, NULL, ERROR3);
	    strcpy(ID_str, hex_str);
	    strncpy(temp, ID_str + 2, 2);
	    strncpy(ID_str + 2, ID_str + 4, 2);
	    strncpy(ID_str + 4, temp, 2);
	    i1 = (int )strtol(ID_str, (char **)NULL, 16);
	    }
	  else
	    genexit(ERROR3, NULL, ERROR3);
	  }
	return i1;
}


/*  */
/****************************************************************
* NAME: TR_test
*
* FUNCTION: Test for bad Token-Ring configurations:
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	is_desktop()
*	parse_vpd()
*	log_error()
*	genexit()
*
* RETURNS:
*	NULL string	(= no bad configurations)
*	SRN value	(= bad configuration with 1
*			 or more Token Ring cards)
*
****************************************************************/
char *TR_test(void)
{
  int i1, j1, k1;
  int TR_cnt, SK_cnt;
  int bus_cnt ;
  int TR_slot[MAXSLOTS];
  int SK_slot[MAXSLOTS];

  char SRN_str[8];
  char criteria[80];
  char *PN_str;
  char TR_name[MAXSLOTS][ERR_NAMESIZE];
  unsigned char failed;
  unsigned char is_7012;

  struct CuDv  *cudv;
  struct CuVPD *cuvpd;
  struct listinfo cudv_info;
  struct listinfo cuvpd_info;

  VPDBUF vpdbuf;

    /****************************************************************
    * Initialization:						*
    ****************************************************************/
    SRN_str[0] = '\0';
    failed = FALSE;
    is_7012 = is_desktop();

    for (bus_cnt = 0; bus_cnt <= 3; bus_cnt++)
    {
	TR_cnt = 0;
	SK_cnt = 0;
	/****************************************************************
	* Retrieve all possible CuDv.  For each non-missing CuDv,	*
	* determine if it is a token ring (TR) or a skyway (SK) card.	*
	* If it is a TR card, then retrieve the VPD.  Parse the VPD to	*
	* find the part number (PN).  If the PN matches the list of bad	*
	* TR PN's, then save the device name and the slot #.  If the	*
	* card is a SK, then save the slot #.				*
	****************************************************************/
	sprintf(criteria, "parent=bus%1d", bus_cnt) ;
	if ((int )(cudv = get_CuDv_list(CuDv_CLASS, criteria,
		  &cudv_info, 20, 1)) < 0) genexit(ERROR3, NULL, ERROR3);
	for (k1=0; k1<cudv_info.num; k1++)
	  if (cudv[k1].chgstatus != MISSING)
	    {
	    if (!strcmp(cudv[k1].PdDvLn_Lvalue, TR_LVALUE))
	      {
	      strcpy(criteria, "name=");
	      strcat(criteria, cudv[k1].name);
	      if ((int )(cuvpd = get_CuVPD_list(CuVPD_CLASS, criteria,
			&cuvpd_info, 1, 1)) < 0) genexit(ERROR3, NULL, ERROR3);
	      if (cuvpd_info.num != 1) genexit(ERROR3, NULL, ERROR3);

	      parse_vpd(cuvpd[0].vpd, &vpdbuf, 0);
	      for (j1=0; j1<vpdbuf.entries; j1++)
	        {
	        PN_str = vpdbuf.vdat[j1];
	        if (!strncmp(++PN_str, VPD_PN_STR, strlen(VPD_PN_STR)))
		  {
		  PN_str += 3;
		  j1 = vpdbuf.entries;
		  }
	        }
	      if (j1 == vpdbuf.entries) genexit(ERROR3, NULL, ERROR3);

	      for (j1=0; j1<NUM_TR_PARTS; j1++)
	        if (strstr(PN_str, TR_part_nos[j1]) != (char *)NULL)
		  {
		  strcpy(TR_name[TR_cnt], cudv[k1].name);
		  TR_slot[TR_cnt] = atoi(cudv[k1].connwhere);
		  TR_cnt++;
		  break;
		  }

	      free_vbuf(&vpdbuf);
	      odm_free_list(cuvpd, &cuvpd_info);
	      }

	    if (!strcmp(cudv[k1].PdDvLn_Lvalue, SK1_LVALUE) ||
	        !strcmp(cudv[k1].PdDvLn_Lvalue, SK2_LVALUE))
	      {
	      SK_slot[SK_cnt++] = atoi(cudv[k1].connwhere);
	      }
	    }
	odm_free_list(cudv, &cudv_info);

	/****************************************************************
	* Each combination of TR and SK card must be checked.  If the	*
	* pair of cards is > 2 slots apart, then an error exists.  Else,*
	* if the machine type is 7012 AND the TR slot is > (SK slot + 1)*
	* then an error exists.  For each bad TR card that causes an	*
	* error, log the TR device name in the error log.		*
	****************************************************************/
	for (i1=0; i1<TR_cnt; i1++)
	  {
	  for (j1=0; j1<SK_cnt; j1++)
	    {
	    k1 = TR_slot[i1] - SK_slot[j1];
	    if (abs(k1) > 2)
	      {
	      log_error(ERRID_TOK_ADAP_ERR, TR_name[i1]);
	      failed = TRUE;
	      }
	    else if (is_7012 && (k1 > 1))
	      {
	      log_error(ERRID_TOK_ADAP_ERR, TR_name[i1]);
	      failed = TRUE;
	      }
	    }
	  }
    } /* END bus loop */
    /****************************************************************
    * If an error was logged, then return the SRN value.		*
    ****************************************************************/
    if (failed) 
    {
        strcpy(SRN_str, TR_SRN);
    }
    return SRN_str;
}


/*
 *******************************************************************
 * NAME     : ucode_chk
 * FUNCTION : Checks that scsi adapters that have microcode to
 *            be downloaded have had it downloaded.  A message 
 *            is generated listing the adapters that have had
 *            microcode download fail (if any).
 * NOTES    :
 *     This routine is currently set up to handle scsi adapters
 *     only.  Its basic algorithm is general enough that it 
 *     could work for other adapters as well.  The one catch to
 *     making this routine totally generic is accessing the 
 *     attribute that identifies the name of the microcode file
 *     that has been downloaded.  There is no standard name for
 *     this attribute and there is no type identified for this
 *     type of attribute.  Therefore, there is no generic way
 *     to get this attribute for any given device.
 * RETURNS  :
 *            -1 : could not get list of devices to check.
 *             0 : successfully checked every adapter.
 *            >0 : number of adapters that could not be checked
 *                 for some reason (probably odm error getting
 *                 database information)
 *******************************************************************
 */

int
ucode_chk (name_list)
    char		*name_list ;	/* str of dvc names that failed  */
{
    int			rc    ;		/* rtn code from called routines */
    int			myrc  ;		/* rtn code this routine returns */
    int			i     ;         /* loop control			 */
    int			len   ;         /* lenth of mircode file	 */
    unsigned int   	level ;      	/* card LL field from VPD	 */
    struct CuDv		*cudv ;		/* ptr to list of cudv objects.  */
    struct listinfo	cudv_info ;	/* list info fr odm_get_list     */
    struct CuAt 	cuat  ;		/* microcode file name attribute */
    char		sstr[64] ;	/* string for ODM queries.	 */
    char		tstr[64] ;	/* temp string for micrcode cmp	 */
    char                mc_path[128] ;  /* path to ucode file fr findmcode*/
    char                rlevel;         /* card RL field from VPD         */

#define TOP_ONLY  1
#define EXPECT_HITS  2

/* BEGIN ucode_chk */

    myrc = 0 ;
    name_list[0] = '\0' ;

    sprintf(sstr, "PdDvLn like adapter/*scsi") ;
    cudv = (struct CuDv *) odm_get_list(CuDv_CLASS, sstr, &cudv_info, 
                                        EXPECT_HITS, TOP_ONLY) ;

    if ((int)cudv != -1)
    {
       /*
	*---------------------------------------------------------
	* Got a list of at least 1 adapter to check.  So, loop
	* through the list checking each for downloaded microcode.
	*---------------------------------------------------------
	*/
        for (i = 0; i < cudv_info.num; i++)
        {
            if (cudv[i].status == AVAILABLE)
            {
                sprintf(sstr, "name = %s and attribute = ucode",
                        cudv[i].name) ;
                if ((rc = (int)odm_get_first(CuAt_CLASS, sstr, &cuat)) == 0)
                {
                   /*
		    *-----------------------------------------------
		    * No ucode attribute found (means nothing was
		    * downloaded).  Now check to see if something
		    * should have been.  If a microcode file exists
		    * for the adapter, and it is of a higher level
                    * than what is on the eprom then there is something that
		    * should have been downloaded.
		    *-----------------------------------------------
		    */
                    if ((level = get_LL(cudv[i],&rlevel)) != -1)
                    {
                        sprintf(sstr, "8d77.%x", level) ;
                        if (findmcode(sstr, mc_path, VERSIONING, 
                                      (char *)NULL))
                        {
                        /*        
                         *-----------------------------------------
                         * Now check to see if mc level on the adapter
                         * eprom is greater than the level of the microcode
                         * on the filesystem
                         *------------------------------------------
                         */
                            len=strlen(mc_path);
                            sprintf(tstr, "%02x", rlevel);
                            DEBUG_1("tstr   =%s\n",tstr) 
                            DEBUG_1("mc_path=%s\n",mc_path) 
                            DEBUG_1("mc_path=%s\n",&mc_path[len-2]) 
                            if (strcmp(tstr, &mc_path[len-2]) < 0) {
                               /*
			        *---------------------------------------
			        * This adapter should have had a
			        * microcode download, but it didn't.
			        * Add its name to the list to output.
			        *---------------------------------------
			        */
                                if (strlen(name_list) == 0)
                                {
                                   sprintf(name_list, "%s", cudv[i].name) ;
                                }
                                else
                                {
                                    sprintf(name_list, "%s %s", name_list, 
                                            cudv[i].name) ;
                                }
                            } /* end if strncmp */
                        } /* end if findmcode */
                    }
                    else /* could not get level of adapter */
                    {
                        myrc++ ;
                    }
                }
                else if (rc == -1)       /* odm failure should be flagged */
                {
                    myrc++ ;
                }
            } /* END if available */
        } /* END for loop */
    }
    else /* odm_get failed */
    {
        myrc = -1 ;
    }

    return(myrc) ;
} /* END ucode_chk */

/*
 ************************************************************************
 * NAME     : get_LL
 * FUNCTION : get and return the LL field from the VPD of the given
 *            device.
 * NOTES    :
 *            This routine can handle the case of the device being a 
 *            child of sio and having its VPD be a part of the sio's VPD.
 * RETURNS  :
 *            -1   : could not get VPD.      
 *            >-1  : LL value 
 ************************************************************************
 */
unsigned int
get_LL(struct CuDv obj,char *rlevel)
{
    int			level    ;	/* LL field.			 */
    char        	sstr[32] ;	/* search string		 */
    struct CuVPD	vpd      ;	/* VPD object of adapter.	 */
    char                *p       ;      /* tmp ptr to find LL field.     */

/* BEGIN get_LL */
   /*
    *----------------------------------------------------
    * Get the object's VPD from CuVPD
    *----------------------------------------------------
    */
    sprintf(sstr, "name = %s", obj.name) ;
    level = (int)odm_get_first(CuVPD_CLASS, sstr, &vpd) ;

    if (level == 0 || level == -1)
    {
	/* odm failure */
        level = -1 ;
    }
    else
    {
        p =vpd.vpd ;
        while (*p != '*')
            p++ ;
        level = (int)strtoul(read_descriptor(p, "LL"), NULL, 16) ;
        DEBUG_1("level=%x\n", level)
        *rlevel = (char)strtoul(read_descriptor(p, "RL"), NULL, 16) ;
        DEBUG_1("rlevel=%x\n", *rlevel)
    }
    return (level) ;
} /* END get_LL */

/*  */
/****************************************************************
* NAME: is_desktop
*
* FUNCTION: Determine if system is a desktop machine,
*	    i.e., if machine type = 7012, e.g., Llano
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	genexit()
*
* RETURNS:
*	TRUE	(= machine is a desktop)
*	FALSE	(= machine is NOT a desktop)
*
****************************************************************/
#define MODEL_MASK 0x0003
#define TOWER	   0x0000
#define DESK	   0x0001
#define RACK	   0x0002

unsigned char is_desktop(void)
{
  unsigned char ret_val;
  char criteria[80];
  int modelcode;
  struct CuAt *cuat;
  struct listinfo cuat_info;

	/****************************************************************
	* One of the custom attributes of "sys0" is the model number.	*
	* In this modelcode, the last 2 bits represents the type of	*
	* system, i.e., tower, desktop, or rack mounted.  The value 	*
	* of this attribute is a 2 or 4 digit hex string, e.g. "0x01"	*
	****************************************************************/
	ret_val = FALSE;
	strcpy(criteria, "name=sys0 AND attribute=modelcode");
	if ((int )(cuat = get_CuAt_list(CuAt_CLASS, criteria,
		  &cuat_info, 1, 1)) < 0)
	  genexit(ERROR3, NULL, ERROR3);
	if (cuat_info.num != 1) genexit(ERROR3, NULL, ERROR3);

	modelcode = (int )strtol(cuat->value, (char **)NULL, 16);
	if ((modelcode & MODEL_MASK) == DESK) ret_val = TRUE;

	odm_free_list(cuat, &cuat_info);
	return ret_val;
}


/****************************************************************
* NAME: generror
*
* FUNCTION: Generate error messages
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  No other local routines called.
*
* RETURNS:  Does not return.
*
****************************************************************/
void 
generror(int errorcode, char *str)
{
    FILE    *fd_msg;
    nl_catd fd_cat;
    char scanf_chr;
    char *fmt;
    char *def_msg ;
    char *err_out;		/* pointer to dummy err buffer 	*/
    static int  err_cnt = 0 ;           /* nmbr err msgs generated      */
    static char *msg_ptr[] =		/* Default messages in case	*/
    {					/* of catalog errors.		*/
	NULL,				/* Place holder. msg_ptr[0] not used */
	DEFAULT_ERR_MSG1,
	DEFAULT_ERR_MSG2,
	DEFAULT_ERR_MSG3,
	DEFAULT_ERR_MSG4,
        DEFAULT_ERR_MSG5 
    };

    static char *command1 =
    {
	"lsgroup -c -a users system | grep system: | sed \"s/system://\" | \
	awk -F, '{cnt=1; {while (cnt<=NF) {print \"mail -s cfgchk \",$cnt, \
	\" < /tmp/cfgchk.msg\"; cnt++;}}}' > /tmp/cfgchk.mail"
    } ;

    static char *command2 = { "chmod +x /tmp/cfgchk.mail" };
    static char *command3 = { "/tmp/cfgchk.mail" };
    static char *command4 = { "rm /tmp/cfgchk.mail; rm /tmp/cfgchk.msg" };

/* BEGIN generror */

    if (!err_cnt)
        CLEAR_SCREEN();
    err_cnt++ ;

   /* 
    *--------------------------------------
    * Delay if there are 2 messages on
    * the screen since the last delay.
    *--------------------------------------
    */
    if ((err_cnt > 2) &&
	(err_cnt & 0x01))
    {
	sleep(SCREEN_DELAY) ;
    }

    if (errorcode > 0)
    {
        def_msg = msg_ptr[errorcode] ;
        fd_cat = catopen(MF_CFGCHK, NL_CAT_LOCALE);
        fmt = catgets(fd_cat, ERROR_SET, errorcode, def_msg);

        if ((errorcode == ERROR1) || (errorcode == ERROR2) || 
            (errorcode == ERROR5))
        {
            fd_msg = fopen("/tmp/cfgchk.msg", "w");
            fprintf(fd_msg, fmt, str);
            fclose(fd_msg);

            odm_run_method(command1, NULL, NULL, &err_out);
            odm_run_method(command2, NULL, NULL, &err_out);
            odm_run_method(command3, NULL, NULL, &err_out);
            odm_run_method(command4, NULL, NULL, &err_out);
        }
        printf(fmt, str);
        catclose(fd_cat);
    }
    return ;
} /* END generror */


/****************************************************************
* NAME: genexit
*
* FUNCTION: 1) Generate error messages if any
*	    2) Terminate any initializations
*	    3) Exit program
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  No other local routines called.
*
* RETURNS:  Does not return.
*
****************************************************************/
void 
genexit(int errorcode, char *str, int exitcode)
{
/* BEGIN exitcode */

    if (errorcode > 0)
    {
        generror(errorcode, str) ;
    }

    if (exitcode > 0)
    {
        sleep(SCREEN_DELAY);
    }
    odm_terminate();
    exit(exitcode);
} /* END genexit */


/*  */
/****************************************************************
* NAME: log_error
*
* FUNCTION: Log an error in the errlog file.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  No other local routines called.
*
* RETURNS:  No return value.
*
****************************************************************/
void log_error(int id, char *name)
{
  int i1;
  static int log_cnt = {0};
  static char dev_name[MAXSLOTS][ERR_NAMESIZE];
  struct err_rec0 errbuf;

	/****************************************************************
	* If the device name has not already been logged, then log it	*
	* with the error ID in the error log.				*
	****************************************************************/
	for (i1=0; i1<log_cnt; i1++)
	  if (!strcmp(name, dev_name[i1])) i1 = log_cnt;

	if (i1 == log_cnt)
	  {
	  errbuf.error_id = id;
	  strcpy(errbuf.resource_name, name);
	  errlog((char *)&errbuf, ERR_REC_SIZE);
	  strcpy(dev_name[log_cnt++], name);
	  }
	return;
}


/*  */
/****************************************************************
* The following routines were taken from file parsevpd.c	*
* These routines reside in libdiag.a.  They exist here in	*
* order that cfgchk() be independent of libdiag.a		*
****************************************************************/
/*
 * NAME: parse_vpd
 *
 * FUNCTION: build null terminated ascii strings from vpd data
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0 = OK
 *	   -1 = Something wrong
 */
#define CAST int (*)(int, char *, char *)

int parse_vpd(
	char *vpd,
	VPDBUF *vbuf,
	int cflag)
{
int 	(*ptr)(int, char *, char *);
int	length;
int 	blength=512;
int 	i;
int	tlen;
char 	key[2];
char 	*dat;
int 	valid_keywd;
int 	cnt=0;

	vbuf->entries = 0;
	null_ptrs(vbuf);

	while (vpd = (char *)memchr(vpd, '*' ,blength)) {
		valid_keywd = FALSE;
		key[0] = toupper( *(vpd+1) );
		key[1] = toupper( *(vpd+2) );
		tlen = *(vpd+3);
		tlen *= 2;

		for (i=0; i < NVPDKEYS; i++) {
			if (!strncmp(key, vpd_data[i].keyword, 2))  {
				valid_keywd = TRUE;
				ptr = (CAST )vpd_data[i].func;
				break;
			}
		}

		if (!valid_keywd) {
			if (*(vpd+tlen) == '*' || *(vpd+tlen) == '\0') {
				valid_keywd = TRUE;
				ptr = (CAST )NULL;
			}
		}

		if (valid_keywd) {
			dat = (char *)malloc(tlen*2);
			if (dat == (char *)NULL) return(-1);
			else vbuf->vdat[vbuf->entries++] = dat;

			*(dat++) = *vpd;
			*(dat++) = *(vpd+1);
			*(dat++) = *(vpd+2);
			*(dat++) = ' ';
			if (ptr != (CAST )NULL)
				(*ptr)(tlen-4, (vpd+4), dat);
			else
				bstrncpy(dat, (vpd+4), tlen-4, cflag);
			blength -= tlen;
		}

		else {
			tlen = 1;
			--blength;
		}

		if(*(vpd+=tlen) == '\0')
			break;
	}
	return(0);
}

/*  */
/*
 * NAME: free_vbuf
 *
 * FUNCTION: free previously allocated VPDBUF pointers
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void free_vbuf(VPDBUF *vbuf)
{
int	cnt;
	for (cnt=0; cnt < vbuf->entries; cnt++)
		free(vbuf->vdat[cnt]);
}

/*  */
/*
 * NAME: null_ptrs
 *
 * FUNCTION: set array of VPDBUF pointers to nulls
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void null_ptrs(VPDBUF *vbuf)
{
int	cnt;

	for (cnt=0; cnt < MAX_ENTRIES; cnt++)
		vbuf->vdat[cnt] = NULL;
}

/*  */
/*
 * NAME: bstrncpy
 *
 * FUNCTION: replace nonprintable characters with spaces if convert flag true
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void bstrncpy(
	char *dest,
	char *src,
	int len,
	int convert)
{
	while(len-- > 0) {
		*dest++ =(convert) ? ((real_isprint(*src)) ? *src : ' ') : *src;
		++src;
	}
	*dest = '\0';
}


/*  */
/*
 * NAME: real_isprint
 *
 * FUNCTION: test if value of character is printable
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: TRUE if c > 0x1F and < 0x7F
 *	    FALSE otherwise
 */

int real_isprint(char c)
{
	return(((c > 0x1F) && (c < 0x7F)) ? TRUE : FALSE);
}

/**/
/*
 * NAME: get_byte
 *
 * FUNCTION: Convert binary data from buffer into displayable characters
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:  This function is called indirectly via a pointer.
 *	   It is referenced in the vpd_data structure.
 *
 * RETURNS: 0
 *
 */
int get_byte(int len, char *inptr, char *outptr)
{
	int i;

	for (i=0; i < len; i++) {
		sprintf(outptr, "%02X", *inptr++);
		outptr += 2;
	}
	*outptr = '\0';
	return 0;
}
