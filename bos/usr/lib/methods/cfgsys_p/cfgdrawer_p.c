#ifndef lint
static char sccsid[] = "@(#)30 1.1 src/bos/usr/lib/methods/cfgsys_p/cfgdrawer_p.c, cfgmethods, bos41J, 9511A_all 3/14/95 03:34:32";
#endif
/*
 * COMPONENT_NAME: (CFGMETHODS) Drawer Configuration Functions 
 *
 * FUNCTIONS: cfgdrawer, build_vpd_drw
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

#define DRAWER_UTYPE    "container/sys/smpdrawer1"
#define OP_PANEL_NAME    "op_panel0"
#define OP_PANEL_UTYPE   "container/cabinet/op_panel1"
#define MCAPLANAR_UTYPE  "ioplanar/planar/mcaplanar1"
#define POWERSUP_UTYPE   "container/cabinet/power_supply1"
#define SIF_UTYPE        "container/cabinet/sif1"

struct config_table config_tb;    /* structure for configuration table built by the BUMP */
extern int	ipl_phase;		/* ipl phase: 0=run,1=phase1,2=phase2	*/

/* Table giving the objects to be configured for a CPU drawer
 *
 * object unique type , connection , VPD address
 *
 */
struct	drw_child {
	char	*utype;
	char	*conn;
	char	*vpd;
};
	
struct	drw_child	bas_drw_child[] = 
		{ SIF_UTYPE, "1", &config_tb.basic_cab.sif1[0],
		  POWERSUP_UTYPE, "2", &config_tb.basic_cab.mvr[0],
		  POWERSUP_UTYPE, "3", &config_tb.expans_cab[0].mvr[0], };

int bas_drw_child_cnt = sizeof bas_drw_child / sizeof bas_drw_child[0];

/*
 * NAME: cfgdrawer
 *
 * FUNCTION: Configures CPU drawer and their children into the system :
 * i.e : 2 MCA planars (PME)
 *       operator panel
 *	 SIB
 *	 2 Power Supplies
 *
 * RETURNS: error code. 0 means no error.
 *
 */
int
cfgdrawer( iplcb_dir, pname)
IPL_DIRECTORY *iplcb_dir;   /* Pointer to IPL Control Block Dir */
char          *pname;       /* Parent device logical name */
{
	struct CuDv  cudv;                /* structure for CuDv object */
	struct  vpd_head *s_p;		  /* structure for VPD object */
	char drawer_name[NAMESIZE];      /* drawer device name */
	int rc, i;             /* return code go here */
	char connection[2];
	char sstring[50];

	/* get the configuration table in novram */
	DEBUG_0("Get the configuration table built by the BUMP\n")
	rc = mdd_get( &config_tb, iplcb_dir->system_vpd_offset, sizeof(struct config_table), MIONVGET); 
	if (rc)
	{
		err_exit(rc);
	}

	/*** PROCESS BASIC DRAWER DEVICE ***/
	/* configure the basic drawer as child of sys0 */
	/* define smpdrawer0 if it doesn't already exit */
	rc = get_dev_objs_by_type( DRAWER_UTYPE, NULL, pname, "0", TRUE, &cudv);
	if (rc) 
	{
		err_exit(rc);
	}

	strcpy( drawer_name, cudv.name);
	DEBUG_1("drawer_name : %s defined\n", drawer_name);

	if (cudv.status != AVAILABLE)
	{
		/* smpdrawer0 is configured, make sure it is available */
		rc = update_cudv( &cudv, "00-00", pname, DRAWER_UTYPE, "0", TRUE);
		if (rc) 
		  {
			  err_exit(rc);
		  }
	}
	DEBUG_1("%s available\n", drawer_name );

	if (ipl_phase != 1) {
	    /* define op_panel0, if necessary */
		rc = get_dev_objs_by_name( OP_PANEL_NAME, OP_PANEL_UTYPE,
								  drawer_name, "0", &cudv);
		if (rc)
		{	
			err_exit(rc);
		}
		DEBUG_0("Operator panel defined\n");
		if (cudv.status != AVAILABLE)
		{
			/* save op_panel0's VPD in the database */
			build_vpd_drw( OP_PANEL_NAME, config_tb.op.op);

			/* op_panel0 is configured, make sure it is available */
			rc = update_cudv( &cudv, "00-00", drawer_name, OP_PANEL_UTYPE,
							 "0", TRUE);
			if (rc) 
			  {
				  err_exit(rc);
			  }
		DEBUG_0("Operator panel available\n");
		}
		

		/* define mcaplanar0, if necessary */
		rc = get_dev_objs_by_type( MCAPLANAR_UTYPE, NULL, drawer_name, "p1", TRUE, &cudv);
		if (rc)
		{
			err_exit(rc);
		}
		DEBUG_3("1st PME : name=%s, connwhere=%s, parent=%s\n",
			cudv.name, cudv.connwhere, cudv.parent);

		if (cudv.status != AVAILABLE)
		{
			/* save mcaplanar0's VPD in the database */
			build_vpd_drw( cudv.name, config_tb.bas_pm_vpd.pm);
			/* previous VPD no available for the moment in config_table */

			/* mcaplanar0 is configured, make sure it is available */
			rc = update_cudv( &cudv, "00-00", drawer_name, MCAPLANAR_UTYPE, "p1", TRUE);
			if (rc) 
			  {
				  err_exit(rc);
			  }
		}
		DEBUG_0("1st PME available\n");

		/* define mcaplanar1, if necessary */
		rc = get_dev_objs_by_type( MCAPLANAR_UTYPE, NULL, drawer_name, "p2", TRUE, &cudv);
		if (rc)
		{
			err_exit(rc);
		}
		DEBUG_3("2nd PME : name=%s, connwhere=%s, parent=%s\n",
			cudv.name, cudv.connwhere, cudv.parent);

		if (cudv.status != AVAILABLE)
		{
			/* save mcaplanar1's VPD in the database */
			build_vpd_drw( cudv.name, config_tb.ext_pm_vpd.pm);
			/* previous VPD no available for the moment in config_table */

			/* mcaplanar1 is configured, make sure it is available */
			rc = update_cudv( &cudv, "00-00", drawer_name, MCAPLANAR_UTYPE, "p2", TRUE);
			if (rc) 
			  {
				  err_exit(rc);
			  }
		}
		DEBUG_0("2nd PME available\n");

		/* configures the children of CPU drawer */
		for (i=0; i< bas_drw_child_cnt; i++) {
			DEBUG_3("Configuring : %s, conn=%s, VPD add=%x\n",
				bas_drw_child[i].utype, 
				bas_drw_child[i].conn,
				bas_drw_child[i].vpd);
			/* configure a device  as child of drawerX */
			s_p = (struct vpd_head *)bas_drw_child[i].vpd;
			DEBUG_1("VPD : %s\n", bas_drw_child[i].vpd);
			
			/* define the device if it doesn't already exit */
			rc = get_dev_objs_by_type( bas_drw_child[i].utype, NULL, drawer_name, bas_drw_child[i].conn, TRUE, &cudv);
			if (rc) 
			{
				err_exit(rc);
			}

		DEBUG_3("Defined : name=%s, connwhere=%s, parent=%s\n",
			cudv.name, cudv.connwhere, cudv.parent);
			/* save child's VPD in the database */
			build_vpd_drw( cudv.name, bas_drw_child[i].vpd);

		if ( !strncmp("VPD", s_p->ident, 3) ) {
			/*  the child is configured, make sure it is available */
			rc = update_cudv( &cudv, "00-00", drawer_name, bas_drw_child[i].utype, bas_drw_child[i].conn, TRUE);
			
			if (rc) 
			{
				err_exit(rc);
			}
			DEBUG_3("Available : name=%s, connwhere=%s, parent=%s\n",
				cudv.name, cudv.connwhere, cudv.parent);
		}
		else 
			{
			/* No VPD ==> Object absent, we remove it */
			sprintf( sstring, "name = %s", cudv.name);
			odm_rm_obj( cusdev, sstring );
			}
		}
	}

	return(0);
}

/*
 * NAME: build_vpd_drw
 *
 * FUNCTION: Obtains the VPD for some children of drawer device,
 *               and formats it
 *
 * RETURNS: error code. 0 means no error.
 *
 */
int
build_vpd_drw( lname, config_table_vpd)
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

