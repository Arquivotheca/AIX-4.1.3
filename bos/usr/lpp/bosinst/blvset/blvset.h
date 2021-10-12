/*
 * @(#) 65 1.14.1.1 src/bos/usr/lpp/bosinst/blvset/blvset.h, bosinst, bos411, 9428A410j 93/06/30 17:35:45
 * COMPONENT_NAME: (BOSINST) Base Operating System Installation
 *
 * FUNCTIONS: blvset.h
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
 * NAME:	blvset.h
 *
 * FUNCTION: 	Template of the information stored in sector one
 *		of the boot logical volume. This information
 *		consists of system settings used as defaults
 *		during the BOS installation and the /u logical
 *		volume id. This structure is used by the BOS
 *		install low level command blvset.c.
 *
 */

#define RECSIZE 30		/* field size in blv record	*/
#define BIGRECSIZE 40		/* field size in blv record	*/
#define IPADDRSIZE 17		/* field size in blv record     */
#define GATEWAYQSIZE 5		/* field size in blv record     */

/*
 *
 *      3.1 menu struct: for 3.2 the keyboard and the language fields 
 *                       in this struct were replaced by a single
 *                       locale field. TZvar was also removed.
 *
 *	struct menu {
 *		char	keyboard[RECSIZE];
 *		char	language[RECSIZE];
 *		char	console[RECSIZE];	
 *		char	inst_dev[RECSIZE];	
 *		char	targ_dev[RECSIZE];	
 *		char	ipl_dev[BIGRECSIZE];	
 *		char	TZvar[RECSIZE];		
 *	} menu;
 *
 */


struct blvset
{
    /* -------------- 3.2 menu group ------------------ */
    struct menu
    {
	char	locale [RECSIZE];	/* Locale setting	*/
	char	console[RECSIZE];	/* Primary console	*/
	char	inst_dev[RECSIZE];	/* Install media device	*/
	char	targ_dev[RECSIZE];	/* Target install disk	*/
	char	ipl_dev[BIGRECSIZE];	/* Primary IPL device	*/
	char	pad1[RECSIZE];	        /* keep 3.2 menu same   */
	char	pad2[RECSIZE];	        /* size as 3.1 menu     */
    } menu;                           

    /* ------------------ network group --------------- */
    struct network
    {
	char	cIPaddr[IPADDRSIZE];	 /* client IP address    */
	char	gIPaddr[IPADDRSIZE];	 /* gateway IP address   */
	char	sIPaddr[IPADDRSIZE];	 /* server IP address    */
	char	subnetmask[IPADDRSIZE];	 /* subnetmask           */
	char	gateway[GATEWAYQSIZE];	 /* install through gateway?     */
	char	entconnect[IPADDRSIZE];	 /* type of ethernet connector   */
	                                 /* either bnc or 15 pin d-type  */
	char	ringspeed[GATEWAYQSIZE]; /* 4 mbit or 16 mbit ring speed */
    } network;

    /* ------------------ netboot group --------------- */    
    /* netboot is new in 3.2                            */

    struct netboot
    {
	char	cIPaddr[IPADDRSIZE];	 /* client IP address    */
	char	sIPaddr[IPADDRSIZE];	 /* server IP address    */
	char	gIPaddr[IPADDRSIZE];	 /* gateway IP address   */
	char	netdevtyp[GATEWAYQSIZE]; /* network device type  */
	char	hardslot[GATEWAYQSIZE];  /* hardware slot        */
	char    e802[GATEWAYQSIZE];      /* ethernet 802 indicator */
	char	subnetmask[IPADDRSIZE];	 /* subnetmask             */
	char	ringspeed[GATEWAYQSIZE]; /* 4 mbit or 16 mbit ring speed */
	char    netdevice[IPADDRSIZE];   /* network device name  */
	char    conntype[GATEWAYQSIZE];  /* ethernet connection type */
    } netboot;
    
    /* ------------------ unallocated --------------- */
    char reserved[512 - (6 * RECSIZE) - (1 * BIGRECSIZE) - (10*IPADDRSIZE) - (6 * GATEWAYQSIZE)];
};











