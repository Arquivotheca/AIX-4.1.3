#ifndef lint
static char sccsid[] = "@(#)20 1.4 src/bos/usr/lib/methods/cfgsys_p/cfgcabinet_p.c, cfgmethods, bos412, 9448A 11/24/94 07:37:23";
#endif
/*
 * COMPONENT_NAME: (CFGMETHODS) Cabinet Configuration Functions 
 *
 * FUNCTIONS: cfgcabinet, cfgchild, build_vpd
 *
 * ORIGINS: 83
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 */
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/devinfo.h>
#include <sys/mdio.h>
#include <sys/iplcb.h>
#include "cfgdebug.h"
#include <sys/errids.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/cfgdb.h>
#include <pgs_novram.h>

extern struct Class  *cusdev;   /* customized devices class ptr */
extern struct Class  *cusatt;   /* customized attributes class ptr */
extern struct Class  *preatt;   /* predefined attributes class ptr */
extern struct Class  *predev;   /* predefined devices class ptr */
extern struct Class  *cusvpd;   /* customized vpd class ptr */

#define CABINET_UTYPE    "container/sys/cabinet1"
#define OP_PANEL_NAME    "op_panel0"
#define OP_PANEL_UTYPE   "container/cabinet/op_panel1"
#define MCAPLANAR_UTYPE  "ioplanar/planar/mcaplanar1"
#define POWERSUP_UTYPE   "container/cabinet/power_supply1"
#define SIF_UTYPE        "container/cabinet/sif1"

struct config_table config_tb;    /* structure for configuration table built by the BUMP */
extern int	ipl_phase;		/* ipl phase: 0=run,1=phase1,2=phase2	*/

/*
 * NAME: cfgcabinet
 *
 * FUNCTION: determines that cabinet are present
 * and configures them and their children into the system           
 *
 * RETURNS: error code. 0 means no error.
 *
 */
int
cfgcabinet( iplcb_dir, pname)
IPL_DIRECTORY *iplcb_dir;   /* Pointer to IPL Control Block Dir */
char          *pname;       /* Parent device logical name */
{
	struct CuDv  cudv;                /* structure for CuDv object */
	struct  vpd_head *s_p;		  /* structure for VPD object */
	char cabinet_name[NAMESIZE];      /* cabinet device name */
	int rc, i, num_cab=0;             /* return code go here */
	char connection[2];

	/* get the configuration table in novram */
	DEBUG_0("Get the configuration table built by the BUMP\n")
	rc = mdd_get( &config_tb, iplcb_dir->system_vpd_offset, sizeof(struct config_table), MIONVGET);
	if (rc)
	{
		err_exit(rc);
	}

	/* determine the number of cabinets present */
	if ( config_tb.sys.config.cab_list1 & 0x1)
	{
		DEBUG_0( "Bit 7 set\n" )
		num_cab++;
        }
	if ( config_tb.sys.config.cab_list1 & 0x2 )
	{
		DEBUG_0( "Bit 6 set\n" )
		num_cab++;
        }
	if ( config_tb.sys.config.cab_list1 & 0x4 )
	{
		DEBUG_0( "Bit 5 set\n" )
		num_cab++;
	}
	if ( config_tb.sys.config.cab_list1 & 0x8 )
	{
		DEBUG_0( "Bit 4 set\n" )
		num_cab++;
	}
	if ( config_tb.sys.config.cab_list1 & 0x10 )
	{
		DEBUG_0( "Bit 3 set\n" )
		num_cab++;
	}
	if ( config_tb.sys.config.cab_list1 & 0x20 )
	{
		DEBUG_0( "Bit 2 set\n" )
		num_cab++;
	}
	if ( config_tb.sys.config.cab_list1 & 0x40 )
	{
		DEBUG_0( "Bit 1 set\n" )
		num_cab++;
	}
	if ( config_tb.sys.config.cab_list1 & 0x80 )
	{
		DEBUG_0( "Bit 0 set\n" )
		num_cab++;
	}

	DEBUG_1("Number of cabinets: %d\n", num_cab);
	/*** PROCESS BASIC CABINET DEVICE ***/
	/* configure the basic cabinet as child of sys0 */
	/* define cabinet0 if it doesn't already exit */
	rc = get_dev_objs_by_type( CABINET_UTYPE, NULL, pname, "0", TRUE, &cudv);
	if (rc) 
	{
		err_exit(rc);
	}

	strcpy( cabinet_name, cudv.name);

	if (cudv.status != AVAILABLE)
	{
		/* cabinet0 is configured, make sure it is available */
		rc = update_cudv( &cudv, "00-00", pname, CABINET_UTYPE, "0", TRUE);
		if (rc) 
		  {
			  err_exit(rc);
		  }
	}

	if (ipl_phase != 1) {
	    /* define op_panel0, if necessary */
		rc = get_dev_objs_by_name( OP_PANEL_NAME, OP_PANEL_UTYPE,
								  cabinet_name, "0", &cudv);
		if (rc)
		{	
			err_exit(rc);
		}

		if (cudv.status != AVAILABLE)
		{
			/* save op_panel0's VPD in the database */
			build_vpd( OP_PANEL_NAME, config_tb.op.op);

			/* op_panel0 is configured, make sure it is available */
			rc = update_cudv( &cudv, "00-00", cabinet_name, OP_PANEL_UTYPE,
							 "0", TRUE);
			if (rc) 
			  {
				  err_exit(rc);
			  }
		}

		/* define mcaplanar0, if necessary */
		rc = get_dev_objs_by_type( MCAPLANAR_UTYPE, NULL, cabinet_name, "p", TRUE, &cudv);
		if (rc)
		{
			err_exit(rc);
		}

		if (cudv.status != AVAILABLE)
		{
			/* save mcaplanar0's VPD in the database */
			build_vpd( cudv.name, config_tb.bas_pm_vpd.pm);
			/* previous VPD no avaiable for the moment in config_table */

			/* mcaplanar0 is configured, make sure it is available */
			rc = update_cudv( &cudv, "00-00", cabinet_name, MCAPLANAR_UTYPE, "p", TRUE);
			if (rc) 
			  {
				  err_exit(rc);
			  }
		}

		/* configures the SIF1 */
		rc = cfgchild( cabinet_name, SIF_UTYPE, "1", -1);
		if (rc)
		{
			err_exit(rc);
		}

		/* configures the power supply */
		rc = cfgchild( cabinet_name, POWERSUP_UTYPE, "2", -1);
		if (rc)
		{
			err_exit(rc);
		}
	}

	for (i=1; i<num_cab; i++) {

		/*** PROCESS EXPANSION CABINET DEVICES ***/
		/* configure the cabinet as child of sys0 */
		/* define cabinetX if it doesn't already exit */
	    sprintf(connection, "%d", i);
		rc = get_dev_objs_by_type( CABINET_UTYPE, NULL, pname,
								  connection, TRUE, &cudv);
		if (rc) 
		  {
			  err_exit(rc);
		  }

		strcpy( cabinet_name, cudv.name);
		
		if (cudv.status != AVAILABLE)
		  {
			  /* cabinetX is configured, make sure it is available */
			  rc = update_cudv( &cudv, "00-00", pname, CABINET_UTYPE,
							   connection, TRUE);
			  if (rc) 
				{
					err_exit(rc);
				}
		  }

		if (ipl_phase != 1) {		
			/* test if the second mcaplanar is present,
			 * if this expansion cabinet is the first 
			 */
			if (config_tb.expans_cab[i - 1].p_pme_vpd != 0)
			{
			  /* define mcaplanar1, if necessary */
			  rc = get_dev_objs_by_type( MCAPLANAR_UTYPE, NULL, cabinet_name,
										"p", TRUE, &cudv);
			  if (rc)
				{
					err_exit(rc);
				}
			  
			  if (cudv.status != AVAILABLE)
				{
					/* save mcaplanar1's VPD in the database */
					build_vpd( cudv.name, config_tb.ext_pm_vpd.pm);
					
					/* mcaplanar1 is configured, make sure it is available */
					rc = update_cudv( &cudv, "00-10", cabinet_name, MCAPLANAR_UTYPE, "p", TRUE);
					if (rc) 
					  {
						  err_exit(rc);
					  }
				}
		 	 }
		
			/* configures the SIF1 */
			rc = cfgchild( cabinet_name, SIF_UTYPE, "0", i);
			if (rc)
			  {
				  err_exit(rc);
			  }
		
			s_p = (struct vpd_head *)config_tb.expans_cab[i-1].sif2;
			if ( !strncmp("VPD", s_p->ident, 3) ) {
				/* configures the SIF2 */
				rc = cfgchild( cabinet_name, SIF_UTYPE, "1", i);
				if (rc)
			  	{
				  	err_exit(rc);
			  	}
			}

			/* configures the power supply */
			rc = cfgchild( cabinet_name, POWERSUP_UTYPE, "2", i);
			if (rc)
			  {
				  err_exit(rc);
			  }
		}
	}

	return(0);
}

/*
 * NAME: cfgchild
 *
 * FUNCTION: configures the SIF and power supply device into the system
 *
 * RETURNS: error code. 0 means no error.
 *
 */
int
cfgchild( pname, utype, connection, cab_num)
char   *pname;              /* Parent device logical name */
char   *utype;              /* uniquetype corresponding to the device */
char   *connection;         /* connwhere corresponding to the device */
int    cab_num;             /* cabinet number */
{
	struct CuDv  cudv;      /* structure for CuDv object */

	int rc;                 /* return code go here */

	/* configure a device  as child of cabinetX */
	/* define the device if it doesn't already exit */
	rc = get_dev_objs_by_type( utype, NULL, pname, connection, TRUE, &cudv);
	if (rc) 
	{
		err_exit(rc);
	}

	if (cudv.status != AVAILABLE)
	{
		/* save child's VPD in the database */
		if (cab_num == -1)
		{
		  /* here children of the basic cabinet are configured */
		  if (!strcmp(utype, SIF_UTYPE)) {
			build_vpd( cudv.name, config_tb.basic_cab.sif1);
		  }
		  if (!strcmp(utype, POWERSUP_UTYPE)) {
			build_vpd( cudv.name, config_tb.basic_cab.mvr);
		  }
		}
		else
		{
			/* here children of the expansion cabinets are configured */
			if (!strcmp(utype, SIF_UTYPE)) {
				if (!strcmp(connection, "0")) {
					/* here SIF1 is configured */
					build_vpd( cudv.name, config_tb.expans_cab[cab_num - 1].sif1);
				}
				if (!strcmp(connection, "1")) {
					/* here SIF2 is configured */
					build_vpd( cudv.name, config_tb.expans_cab[cab_num - 1].sif2);
				}
			}
			if (!strcmp(utype, POWERSUP_UTYPE)) {
				build_vpd( cudv.name, config_tb.expans_cab[cab_num - 1].mvr);
			}
		}

		/*  the child is configured, make sure it is available */
		rc = update_cudv( &cudv, "00-00", pname, utype, connection, TRUE);
		if (rc) 
		{
			err_exit(rc);
		}
	}

	return(0);
}

/*
 * NAME: build_vpd
 *
 * FUNCTION: Obtains the VPD for some children of cabinet device,
 *               and formats it
 *
 * RETURNS: error code. 0 means no error.
 *
 */
int
build_vpd( lname, config_table_vpd)
char  *lname;                  /* Device logical name */
char  config_table_vpd[];      /* string containing the VPD of the device */
{
	struct  vpd_head *v_h;
	struct  vpd_field_head *v_f;
	int l, rc, h, k;
	char    *v_p;
	char    value[256] = "";
	char    value_ok[256]= "";
	char    ident[3] = "";

	char    vpd[VPDSIZE];                   /* storage for holding VPD */

	/* Save VPD of the object corresponding to lname */
	memset( vpd, 0, VPDSIZE);
	*vpd = '\0';
	memset(value, 0, 256);
	memset(value_ok, 0, 256);
	memset(ident, 0, 3);

	v_h = (struct vpd_head *)config_table_vpd;
	v_f = (struct vpd_field_head *)((int) v_h + sizeof(*v_h));

	if ( v_h->length != 0)
	{
		l= 0;
		while( (2 * v_h->length) > l) {
			memcpy(ident, v_f->ident, 2);
			ident[3] = '\0';
			DEBUG_3("field VPD  %c%s, field length %d\n", v_f->star, ident, v_f->length);
			if (v_f->length != 0) {
				v_p = (char *)v_f + 4;
				memcpy(value, v_p, ((2 * v_f->length) - 4));
				value[((2 * v_f->length) - 4) + 1] = '\0';
				DEBUG_1("field VPD value: %s\n", value);
				if ( !strncmp("RL", ident, 2)) {
					sprintf(&value_ok[0], "%02x", value[0]);
					for( h=1, k=2; h < ((2 * v_f->length) - 4); h++, k++)
					  sprintf(&value_ok[k], "%c", value[h]);
				}
				else if ( !strncmp("PC", ident, 2)) {
					for( h=0, k=0; h < ((2 * v_f->length) - 4); h++, k = k+2)
					  sprintf(&value_ok[k], "%02x", value[h]);
				}
				else if ( !strncmp("Y0", ident, 2) || !strncmp("Y1", ident, 2)) {
					sprintf(&value_ok[0], "%02x%02x", value[0], value[1]);
					for( h=2, k=4; h < ((2 * v_f->length) - 4); h++, k++)
					  sprintf(&value_ok[k], "%c", value[h]);
				}
				else {
					strcpy(value_ok, value);
				}
				DEBUG_1("field VPD decoded: %s\n", value_ok);
				l = l + (2 * v_f->length);
				v_f = (struct vpd_field_head *) (v_p + ((2 * v_f->length) - 4));
			
				add_descriptor(vpd, ident, value_ok);
			
				memset(value, 0, 256);
				memset(value_ok, 0, 256);
				memset(ident, 0, 3);
			}
			else {
				break;
			}
		}
		
		rc = update_vpd( lname, vpd);
		if (rc) {
		  return(rc);
		}

	}

	return(0);
}

