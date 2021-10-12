#ifndef lint
static char sccsid[] = "@(#)21 1.5 src/bos/usr/lib/methods/cfgsys_p/cfgcpu.c, cfgmethods, bos41J, 9519B_all 5/9/95 10:26:10";
#endif
/*
 * COMPONENT_NAME: (CFGMETHODS) CPU Configuration Functions
 *
 * FUNCTIONS: cfgcpu, update_customized, get_cpucard_vpd
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

#define SYSPLANAR_NAME   "sysplanar0"
#define CPUCARD_UTYPE    "cpucard/sys/cpucard1"
#define CPUCARD_NAME     "cpucard"
#define L2CACHE_UTYPE    "memory/sys/L2cache"
#define L2CACHE_NAME     "L2cache"
#define PROC_UTYPE       "processor/sys/proc1"
#define PROC_NAME        "proc"
#define PowerPC_MP_MODEL 0x080000A0
#define POWER_RS1        0x0001          /* RS1 class CPU */
#define POWER_RSC        0x0002          /* RSC class CPU */
#define POWER_RS2        0x0004          /* RS2 class CPU */
#define POWER_601        0x0008          /* 601 class CPU */
#define POWER_604        0x0010          /* 604 class CPU */
#define POWER_620        0x0040          /* 620 class CPU */

struct config_table config_tb;    /* structure for configuration table built by the BUMP */
char *cpu_connection[]={"P","Q","R","S"};

/*
 * NAME: cfgcpu
 *
 * FUNCTION: determines that processors and CPU daughter board are present and configures them into the system           
 *
 * RETURNS: error code. 0 means no error.
 *
 */
int
cfgcpu(iplcb_dir, pname, model)
IPL_DIRECTORY *iplcb_dir;   /* Pointer to IPL Control Block Dir */ 
char   *pname;              /* Parent device logical name */
int    model;               /* machine model */
{
	struct CuDv  cudv;              /* structure for CuDv object */

	char sstring[256];              /* search criteria pointer */
	char location[16];
	char l2cachename[NAMESIZE];
	char proc_name[NAMESIZE];
	char cpucard_name[NAMESIZE];
	char proc_num[3];
	char connection[3];             /* use connection parameter of get_dev_objs_by_type function */
	int i, j;                          /* loop variable */
	int num_processors;             /* number of processors read in processor_info structure of the ILPCB */
	int type;                       /* type of the processor */
	int rc;                         /* return code go here */

	char *iplcb_per_proc_info_ptr;  /* use to realize the malloc of the processor_info structure */

	PROCESSOR_DATA_PTR iplcb_per_proc_info_ptr_1, iplcb_per_proc_info_ptr_2;
	PROCESSOR_DATA_PTR iplcb_per_proc_info_1, iplcb_per_proc_info_2;

	/* read in the processor_info section of IPL control block */
	iplcb_per_proc_info_ptr = malloc(iplcb_dir->processor_info_size);
	
	iplcb_per_proc_info_ptr_1 = (PROCESSOR_DATA_PTR) iplcb_per_proc_info_ptr;

	DEBUG_0("Get the processor info section of the ILPCB\n")
	rc = mdd_get(iplcb_per_proc_info_ptr, iplcb_dir->processor_info_offset, iplcb_dir->processor_info_size, MIOIPLCB);
	if (rc){
		DEBUG_0("Error in MDD for ILPCB\n")
		err_exit(rc);
	}

	num_processors = iplcb_per_proc_info_ptr_1->num_of_structs;
	DEBUG_1("Number max of CPU: %d\n", num_processors)

	/* get the configuration table in novram */
	DEBUG_0("Get the configuration table built by the BUMP\n")
	rc = mdd_get( &config_tb, iplcb_dir->system_vpd_offset, sizeof(struct config_table), MIONVGET);
	if (rc)
	{
		err_exit(rc);
	}

	/* configure each CPU daughter board as child of sysplanar0 */
	/* define cpuplanarX if it doesn't already exist */
        /* slot P is always cpucard0 */
        /* slot Q is always cpucard1 */
        /* slot R is always cpucard2 */
        /* slot S is always cpucard3 */
	DEBUG_0("Starting CPU daughter board configuration\n")
	for (i = 0;i<(num_processors/2);i++) {
		iplcb_per_proc_info_1 = iplcb_per_proc_info_ptr_1;
		iplcb_per_proc_info_2 = (PROCESSOR_DATA_PTR) ((uint)iplcb_per_proc_info_ptr_1 + iplcb_per_proc_info_ptr_1->struct_size);
		if ((iplcb_per_proc_info_1->processor_present != 0) || (iplcb_per_proc_info_2->processor_present != 0)) {
		  sprintf( cpucard_name, "%s%d", CPUCARD_NAME, i);
		  sprintf(location, "00-0%s", cpu_connection[i]);
		  DEBUG_0("Call to get_dev_objs\n");
		  rc = get_dev_objs_by_name(cpucard_name, CPUCARD_UTYPE, pname, cpu_connection[i], &cudv);
		  if (rc) {
			err_exit(rc);
		  }

		  /* Save cpucard's VPD in database */
		  rc = get_cpucard_vpd(cpucard_name, config_tb.cpu[i].board_vpd);
		  if (rc)
			{
			  DEBUG_0("Error in get_cpucard_vpd\n")
			  err_exit(rc);
			}
		  /* cpuplanar is configured, make sure it is available */
		  rc = update_cudv(&cudv, location, pname, CPUCARD_UTYPE, cpu_connection[i], TRUE);
		  if (rc) {
			DEBUG_0("Error in update_cudv\n")		
			err_exit(rc);
		  }

		  DEBUG_0("End of CPU planar configuration\n");
		  
		  /*** PROCESS L2CACHE DEVICE ***/
		  /* get any old L2 device object that is in database */
		  sprintf(l2cachename, "%s%d", L2CACHE_NAME, i);
		  sprintf(sstring, "name='%s'", l2cachename);
		  rc = odm_get_first(cusdev, sstring, &cudv);
		  if (rc == -1)
			err_exit(E_ODMGET);
		  /* if there was no previous L2 cache in database, create it */
		  else if (rc == 0) {
			DEBUG_0("No L2 cache found in database - create new ones.\n");
			if (iplcb_per_proc_info_1->L2_cache_size != 0)
			  /* create cudv and cuat as needed */
			  setup_L2cache(l2cachename, cpucard_name, location,
							iplcb_per_proc_info_1->L2_cache_size, 
							iplcb_per_proc_info_1->processor_present, 
							iplcb_per_proc_info_2->L2_cache_size, 
							iplcb_per_proc_info_2->processor_present);
		  }
		  else {
			/* there was L2 cache previously */
			DEBUG_0("L2 cache WAS found in database.\n");
			DEBUG_1("new parent = %s\n", cpucard_name);
			DEBUG_1("old parent = %s\n", cudv.parent);

			if (iplcb_per_proc_info_1->L2_cache_size !=0)
			  setup_L2cache(l2cachename, cpucard_name, location,
							iplcb_per_proc_info_1->L2_cache_size, 
							iplcb_per_proc_info_1->processor_present, 
							iplcb_per_proc_info_2->L2_cache_size, 
							iplcb_per_proc_info_2->processor_present);
		  
			else if (strcmp(cudv.parent, cpucard_name) != 0) {
			  DEBUG_0("L2 cache WAS found in database and new size == 0 so, deleteing old stuff.\n");
			  /* delete L2 cache CuDv and CuAt */
			  odm_rm_obj(CuAt_CLASS, sstring);
			  odm_rm_obj(CuDv_CLASS, sstring);
			}
		  }

		  DEBUG_0("End of L2 cache  configuration\n");

		  /* Configure each CPU as child of cpuplanar */
		  iplcb_per_proc_info_ptr_2 = iplcb_per_proc_info_ptr_1;
		  DEBUG_0("Real starting CPU configuration step 2\n")
			for (j = (i*2); j <((i*2)+2); j++) {
			  if (iplcb_per_proc_info_ptr_2->processor_present != 0) {
				sprintf(connection, "%d", j%2);
				sprintf(proc_name, "%s%d", PROC_NAME, j);
				rc = get_dev_objs_by_name( proc_name, PROC_UTYPE, cpucard_name, connection, &cudv);
				
				switch(iplcb_per_proc_info_ptr_2->processor_present) {
				case -2:
				  type = iplcb_per_proc_info_ptr_2->implementation;
				  rc = update_customized( &cudv, "disable", connection, type);
				  if (rc) {
					DEBUG_0("Error in update_customized\n")
					err_exit(rc);
				  }
				  break;
				case -1:
				case 2:
				case 3:
				  type = iplcb_per_proc_info_ptr_2->implementation;
				  rc = update_customized( &cudv, "faulty", connection, type);
				  if (rc) {
					DEBUG_0("Error in update_customized\n")
					err_exit(rc);
				  }
				  break;
				case 1:
				  type = iplcb_per_proc_info_ptr_2->implementation;
				  rc = update_customized( &cudv, "enable", connection, type);
				  if (rc) {
				    DEBUG_0("Error in update_customized\n")
					err_exit(rc);
				  }
				  break;
				default:
				  DEBUG_0("Case not plan\n")
				  break;
				}
			  }
			  iplcb_per_proc_info_ptr_2 = (PROCESSOR_DATA_PTR) ((uint) iplcb_per_proc_info_ptr_2 +
iplcb_per_proc_info_ptr_2->struct_size);

			}
		  
		}
		iplcb_per_proc_info_ptr_1 = (PROCESSOR_DATA_PTR) ((uint) iplcb_per_proc_info_2 + iplcb_per_proc_info_2->struct_size);
	}

	free(iplcb_per_proc_info_ptr);

	DEBUG_0("End of CPU configuration\n")
	return(0);
}


/*
 * NAME: update_customized
 *
 * FUNCTION: Update CuDv and CuAt classes
 *
 * RETURNS: error code. 0 means no error.
 *
 */

int 
update_customized(cudv,state,cpu_connection, cpu_type)
struct CuDv  *cudv;                             /* pointer to the CuDv object */
char         state[16];                       /* processor state */
char         *cpu_connection;   /* processor connection on cpucard */
int          cpu_type;          /* processor type */
{
	int     rc;                     /* return codes go here */
	char    type[16];               /* processor type */
	char    proc_name[NAMESIZE];    /* processor device name */
	char    pname[NAMESIZE];               /* parent device name */
	char    loc[16];                /* processor location */
	char    tempo[16];
	char    *slot;                  /* CPU daughter board */

	DEBUG_0("Starting the updating CPU configuration\n")
	/* Save the proc's logical_name */
	strcpy(proc_name, cudv->name);
	strcpy(pname, cudv->parent);

	/* Save the proc's location */
	sprintf(loc,"00-%c%c-00-0%s", cudv->location[3],
			cudv->location[4], cpu_connection);

	rc = strcmp(state, "enable");
	if ( (rc == 0) && (cudv->status != AVAILABLE)){
		
		/* procX is now configured, make sure it is available */
		rc = update_cudv( cudv, loc, pname, PROC_UTYPE, cpu_connection, TRUE);
		if (rc) {
			DEBUG_0("Error in updating CuDv\n")
			err_exit(rc);
		}
	} else {
		/* update CuDv but don't make the device available */
		rc = update_cudv( cudv, loc, pname, PROC_UTYPE, cpu_connection, FALSE);
	}

	/* set up procX's system attributes in database */
	rc = setattr(proc_name, "state", state);
	if (rc) {
		DEBUG_0("Error in updating CuAt\n")
		err_exit(rc);
	}

	if (cpu_type == POWER_RS1)
	  sprintf(type, "%s", "POWER");
	if (cpu_type == POWER_RSC)
	  sprintf(type, "%s", "POWER");
	if (cpu_type == POWER_RS2)
	  sprintf(type, "%s", "POWER2");
	if (cpu_type == POWER_601)
	  sprintf(type, "%s", "PowerPC_601");
	if (cpu_type == POWER_604)
	  sprintf(type, "%s", "PowerPC_604");
	if (cpu_type == POWER_620)
	  sprintf(type, "%s", "PowerPC_620");
	
	rc = setattr(proc_name, "type", type);
	if (rc) {
		DEBUG_0("Error in updating CuAt\n")
		err_exit(rc);
	}

	return(0);
}


int
get_cpucard_vpd( lname, config_table_vpd)
char *lname;                      /* device logical name */
char  config_table_vpd[];         /* string containing the VPD of the device */
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
				if ( !strncmp("PC", ident, 2)) {
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

	DEBUG_0("End of get VPD of cpucard\n")
	return(0);
}

/*
 * NAME: setup_L2cache
 *
 * FUNCTION: Set attribute L2cache for cpu card in the database
 *
 * RETURNS:  error code.  0 means no error.
 */
int
setup_L2cache(lname, pname, cpucard_loc, L2cache_cpu1, cpu1, L2cache_cpu2, cpu2)
char *lname;         /* logical name */
char *pname;         /* parent name */
char *cpucard_loc;   /* cpu card location */
int L2cache_cpu1;    /* value in byte of L2 cache for the first cpu */
int cpu1;            /* processor present or not */
int L2cache_cpu2;    /* value in byte of L2 cache for the second cpu */
int cpu2;            /* processor present or not */  
{
  struct CuDv cudv;
  int rc;
  int L2_cache_size;
  char L2cache[16];  /* for setting the L2 cache attribute */
  char loc[16];      /* for setting the location code of L2 cache object */

  if (cpu1 != 0 && cpu2!= 0) {
	if (L2cache_cpu1 == L2cache_cpu2)
	  L2_cache_size = L2cache_cpu1 / 1024;
	else
	  L2_cache_size = 0;
  }
  if (cpu1 !=0 && cpu2 == 0)
	L2_cache_size = L2cache_cpu1 / 1024;

  DEBUG_1("L2cache=%d\n",L2_cache_size);
  sprintf( L2cache, "%d", L2_cache_size);

  DEBUG_0("entering setup_L2cache.\n");
  /* configure the L2 cache device */
  rc = get_dev_objs_by_name(lname, L2CACHE_UTYPE, 
							pname, "L", &cudv);
  DEBUG_1("Returned from get by name with rc = %d\n", rc);

  if (rc)
	{
	  /* rc could be the following:
		 E_NOPdDv
		 E_NOCuDv
		 E_ODMGET
		 E_ODMRUNMETHOD
	   */
	  err_exit(rc);
	}

  /* found device, now make sure location is correct */
  sprintf(loc,"%s-00-0L", cpucard_loc);

  rc = update_cudv(&cudv, loc, pname,
				   L2CACHE_UTYPE, "L", TRUE);

  DEBUG_1("Returned from update_cudv with rc = %d\n", rc);
  if (rc)
	{
	  /* returns E_ODMUPDATE or 0 */
	  err_exit(rc);
	}

  setattr(lname, "size", L2cache);
}

