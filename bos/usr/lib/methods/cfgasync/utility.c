#ifndef lint
static char sccsid[] = "@(#)13 1.4 src/bos/usr/lib/methods/cfgasync/utility.c, cfgtty, bos41J, 9520A_all 4/27/95 12:49:41";
#endif
/*
 * COMPONENT_NAME: (CFGTTY)  VPD functions for adapter config
 *
 * FUNCTIONS: check_pos2, define_children, chk_lion, query_vpd
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>          /* standard I/O */
#include <stdlib.h>         /* standard C library */
#include <sys/types.h>      /* standard typedef */
#include <cf.h>             /* error messages */
#include <errno.h>          /* standard error numbers */
#include <string.h>          

#include <sys/cfgdb.h>      /* config #define */
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/cfgodm.h>     /* config structures */
#include <sys/mdio.h>

#include "cfgdebug.h"
#include "ttycfg.h"

/*
 * ==============================================================================
 * defines and strutures
 * ==============================================================================
 */
/* defines for attributes search */
/* These defines MUST be in coherence with ODM database attribute names */
#define AUTOCONFIG_ATT              "autoconfig"

/*
 * =============================================================================
 *                       LOCAL UTILITY ROUTINES
 * =============================================================================
 */
/*
 * -----------------------------------------------------------------------------
 *                       CHECK_POS2
 * -----------------------------------------------------------------------------
 * Check pos reg 2 for 64 port problem
 *
 * This routine implement the check of pos reg 2 on a 64
 * port to determine if it has the bus memory problem.
 * This check is performed by writing 0 to POS 2 and
 * then reading it back.  
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * -----------------------------------------------------------------------------
 */
static int check_pos2(cusDev, value, busDev)
struct CuDv * cusDev;
uchar *       value;
struct CuDv * busDev;
{
    int        return_code;
    int     file_desc;          /* File descriptor for bus */
    uchar   pos_reg;            /* Pos register */
    char    sstring[50];

    /* ODM structures declarations */
    struct Class * cus_dev_class;     /* customized devices class ptr */

    MACH_DD_IO      mdd;        /* mach dd ioctl struct for POS access    */

    /* open customized devices object class (CuDv) */
    if ((int)(cus_dev_class = odm_open_class(CuDv_CLASS)) == -1) {
        DEBUG_0("check_pos2: CuDv open class error\n");
        return(E_ODMOPEN);
    };

    /* ================================================= */
    /* Must get the bus this card is attached to         */
    /* ================================================= */
    if (return_code = Get_Parent_Bus(cus_dev_class, cusDev->parent, busDev)) {
        DEBUG_1("check_pos2: Error getting parent bus, return code = %d\n",
                return_code);
        return(return_code);
    };

    /* If adapter is directly on the bus, use machdd to write */
    /* and read tp POS registers */
    if (!strcmp(cusDev->parent, busDev->name)) {
        DEBUG_1("check_pos2: Card is directly on %s\n", busDev->name);
        sprintf(sstring, "/dev/%s", busDev->name);
        if ((file_desc = open(sstring, O_RDWR)) < 0) {
            DEBUG_1("check_pos2: open of %s failed\n", sstring);
            return(E_BUSRESOURCE);
        }
        else {
            mdd.md_size = 1 ;
            mdd.md_incr = MV_BYTE ;
            mdd.md_data = &pos_reg;
            mdd.md_addr = (POSREG(2, atoi(cusDev->connwhere) - 1)) ;
            pos_reg = 0 ;
            if (ioctl(file_desc, MIOCCPUT, &mdd) < 0) {
                DEBUG_0("check_pos2: write to POS_2 failed\n");
                return(E_BUSRESOURCE);
            };
            if (ioctl(file_desc, MIOCCGET, &mdd) < 0) {
                DEBUG_0("check_pos2: read to POS_2 failed\n");
                return(E_BUSRESOURCE);
            };
            /* Get the POS data back */
            DEBUG_1("check_pos2: POS reg 2 = 0x%02x\n", pos_reg);
            *value = pos_reg;
        } /* End if ((file_desc = open(...)) < 0) */
    };

    if (close(file_desc) < 0) {
        DEBUG_1("check_pos2: close of %s failed\n", sstring);
    };

   /* That's OK */
    return(E_OK);
} /* End static int check_pos2(...) */

/*
 * =============================================================================
 *                       FUNCTIONS USED BY CONFIGURATION METHOD
 * =============================================================================
 * 
 * All these functions are used for adapters configuration
 *
 * =============================================================================
 */
/*
 * -----------------------------------------------------------------------------
 *                       DEFINE_CHILDREN
 * -----------------------------------------------------------------------------
 * This fucntion detects and manages children of the asynchronous
 * adapters.
 *
 * Children that need to be configured are detected by an
 * autoconfig attribute value of "available".
 * All children to be configured are returned via stdout to the
 * configuration manager which in turn will eventually invoke the
 * configure method for these child devices.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * -----------------------------------------------------------------------------
 */
int define_children(logical_name,phase)
char * logical_name;        /* logical name of parent device */
int    phase;                /* IPL or runtime flag */
{
    int return_code;
    int odm_search;                 /* flag for used in getting objects */

    char auto_config[ATTRVALSIZE];  /* auto config value */
    char sstring[256];                /* search criteria */

    /* ODM structures declarations */
    struct Class * cus_dev_class;     /* customized devices class ptr */
    struct Class * cus_att_class;     /* customized attribute class ptr */
    struct Class * pre_att_class;     /* predefined attribute class ptr */

    struct CuDv    cus_dev;           /* customized device object storage */

    /* open customized devices object class (CuDv) */
    if ((int)(cus_dev_class = odm_open_class(CuDv_CLASS)) == -1) {
        DEBUG_0("define_children: CuDv open class error\n");
        return(E_ODMOPEN);
    };

    /* open customized devices attribute class (CuAt) */
    if ((int)(cus_att_class = odm_open_class(CuAt_CLASS)) == -1) {
        DEBUG_0("define_children: CuAt open class error\n");
        return(E_ODMOPEN);
    };

    /* open predifined devices attribute class (PdAt) */
    if ((int)(pre_att_class = odm_open_class(PdAt_CLASS)) == -1) {
        DEBUG_0("define_children: PdAt open class error\n");
        return(E_ODMOPEN);
    };

    /* ===================================== */
    /* Loop until no more children are found */
    /* ===================================== */
    /* Search for customized object with parent = device */
    sprintf(sstring, "parent = '%s'", logical_name);
    odm_search = ODM_FIRST;
    while ((return_code =
            (int)odm_get_obj(cus_dev_class, sstring, &cus_dev, odm_search)) > 0) {
        DEBUG_1("define_children: found child %s\n", cus_dev.name);

        odm_search = ODM_NEXT;

        /* If child is to be 'autoconfigured' */
        auto_config[0] = '\0';
        if (return_code = getatt(auto_config, 's', cus_att_class, pre_att_class,
                             cus_dev.name, cus_dev.PdDvLn_Lvalue,
                             AUTOCONFIG_ATT, NULL)) {
            DEBUG_3("define_children: getatt '%s' for %s fails with error %x\n",
                    AUTOCONFIG_ATT, cus_dev.name, return_code);
            return(return_code);
        };
        if (!strcmp(auto_config, "available")) {
            fprintf(stdout, "%s\n", cus_dev.name);
        };
    } /* End while ((return_code = (int)odm_get_obj(...))) */
    /* Check for error */
    if (return_code) {
        DEBUG_1("define_children: odm_get_obj fails for %s\n", sstring);
    };

    /* close object classes */
    if (odm_close_class(cus_dev_class) < 0) {
        return(E_ODMCLOSE);
    };
    if (odm_close_class(pre_att_class) < 0) {
        return(E_ODMCLOSE);
    };
    if (odm_close_class(cus_att_class) < 0) {
        return(E_ODMCLOSE);
    };

   /* That's OK */
    return(E_OK);
} /* End int define_children(...) */

/*
 * -----------------------------------------------------------------------------
 *                       chk_lion
 * -----------------------------------------------------------------------------
 * This function checks for conflicts in 64 ports card.
 *
 * Nothing is done in ODM database.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * -----------------------------------------------------------------------------
 */
int chk_lion(cusObjPtr, kmid, devno)
     struct  CuDv * cusObjPtr;   /* Customized Device object pointer */
     mid_t   kmid;               /* Kernel module I.D. for Dev Driver */
     dev_t   devno;              /* Concatenated Major & Minor No.s */
{
    int   return_code;
    uchar   pos_reg;            /* Pos register */
    char    sstring[128];       /* string for searches */

    /* ODM structures declarations */
    struct CuDv bus_obj;            /* customized devices class ptr */
    struct CuDv *adp_list;          /* CuDv ptr for adapter list    */
    struct listinfo adp_lst_info;   /* ODM list info for adaptr list*/

    /* ========================================== */
    /* The following definition is to allocate */
    /* space for the 64 port table required */
    /* by routines in the cfgchk_64p.c unit. */
    /* The format of the area is irrelevant to */
    /* this funciton, but space must be provided */
    /* by this function.  This approach was taken */
    /* for expedency in resolving defect 43992. */
    /* I know it is not the best way to do it. */
    /* ========================================== */
    char p64_tbl[128];

    /* ============================== */
    /* set up initial variable values */
    /* ============================== */
    return_code = 0;

    /* ============================================================ */
    /* In order to handle a 64 port problem of ingoring the high */
    /* order 8 bits of the 32 bit bus memory address, the following */
    /* has been implemented to check for conflicts.  The checks */
    /* are only done if the card is a 64 port. */
    /* ============================================================ */
    if (!strcmp(cusObjPtr->PdDvLn_Lvalue, "adapter/mca/64p232")) {
        DEBUG_1("chk_lion: card (%s) is a 64 port adapter\n", cusObjPtr->name);
        if (return_code = check_pos2(cusObjPtr, &pos_reg, &bus_obj)) {
            DEBUG_1("chk_lion: error from check_pos2 = %d\n", return_code);
        }
        else {
            if ((pos_reg & 0x20) == 0) {
                DEBUG_0("chk_lion: Card is BAD\n");
                /* Card is BAD.  Must know get a list of */
                /* all adapters currently in this bus and */
                /* check for conflicts between this 64 */
                /* port and each of the other adapters. */
                sprintf(sstring, "parent=%s and chgstatus!=%d and chgstatus!=%d",
                        bus_obj.name, MISSING, DONT_CARE);
                /* Expected number of objects to be returned set to 8 */
                /* Number of level to recurse for objects with ODM_LINK */
                /* descriptors is set to 1 (only the top level) */
                adp_list = odm_get_list(CuDv_CLASS, sstring,
                                        &adp_lst_info, 8, 1);
                if ((int)adp_list != -1) {
                    DEBUG_1("chk_lion: got adp_list, count = %d\n",
                            adp_lst_info.num);
                    bld_64p_tbl(cusObjPtr, 1, &p64_tbl);
                    if (chk_adapters(adp_list, adp_lst_info.num,
                     &p64_tbl, 1, sstring)) {
                        DEBUG_0("chk_lion: CONFLICTS found!!\n");
                        return_code = E_BUSRESOURCE;
                    }
                    else {
                        DEBUG_0("chk_lion: No conflicts\n");
                    } /* End if (chk_adapters(...)) */
                    odm_free_list(adp_list, &adp_lst_info);
                }
                else {
                    DEBUG_0("chk_lion: ODM error getting adpater list\n");
                    return_code = E_ODMGET;
                } /* End if ((int)adp_list != -1) */
            }; /* End if ((pos_reg & 0x20) == 0) */
        } /* End if (return_code = check_pos2(...)) */
    }; /* End if (!strcmp(...)) */


    return(return_code);
} /* End int chk_lion(...) */
  


/*
 * -----------------------------------------------------------------------------
 *                       query_vpd
 * -----------------------------------------------------------------------------
 * 
 * This function extracts the adapters VPD data from the POS registers
 * via the bus.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * -----------------------------------------------------------------------------
 */



int query_vpd(cusDev,ret_vpd)
	struct  CuDv * cusDev;		/* Customized Device object pointer */
	char *ret_vpd;			/* return vpd buffer		*/
{

	MACH_DD_IO	read_record,	/* machdd ioctl access struct, for */
			write_record;   /* POS access */
        int     	rc;
        int     	fd;
        int		len;
	int		vpd_index;
	char		vpd_data[256];
	uchar		tmpbyte;
	char		devname[32];
   	 /* ODM structures declarations */
	struct Class * cus_dev_class;     /* customized devices class ptr */
	struct  CuDv  busDev;		/* Bus Device object  */


    	/* open customized devices object class (CuDv) */
    	if ((int)(cus_dev_class = odm_open_class(CuDv_CLASS)) == -1) {
        	DEBUG_0("check_pos2: CuDv open class error\n");
        	return(E_ODMOPEN);
    	};

	/* ================================================= */
    	/* Must get the bus this card is attached to         */
    	/* ================================================= */
        DEBUG_1("query_vpd: cusDev->parent= %s\n", cusDev->parent);
        DEBUG_1("query_vpd: cusDev->name= %s\n", cusDev->name);
        DEBUG_1("query_vpd: cusDev->name= %s\n", cusDev->connwhere);
    	if (rc = Get_Parent_Bus(cus_dev_class, cusDev->parent, &busDev)) {
        	DEBUG_1("query_vpd: Error getting parent bus, return code = %d\n", rc);
        	return(rc);
    	};

	/* build bus's dev name */
	sprintf(devname, "/dev/%s" ,busDev.name);
        if((fd = open( devname, O_RDWR )) < 0 ){
       	        return(E_OPEN);
        };


       	/* write 0x01 - 0xFF to pos reg 6 */
       	/* read related vpd byte back from pos reg 3  */

       	write_record.md_size = 1;       /* write 1 byte */
       	write_record.md_incr = MV_BYTE;
       	write_record.md_data = &tmpbyte;
 	write_record.md_addr = (POSREG(6, atoi(cusDev->connwhere) - 1)) ;

	read_record.md_size = 1;        /* Transfer 1 byte */
       	read_record.md_incr = MV_BYTE;
       	read_record.md_data = &tmpbyte;
       	read_record.md_addr = (POSREG(3, atoi(cusDev->connwhere) - 1)) ;

       	for (vpd_index = 0x01; vpd_index <= 0xFF; vpd_index++) {
               	tmpbyte = (uchar)vpd_index;

               	/* write to pos reg 6 */
               	if( ioctl( fd, MIOCCPUT, &write_record ) < 0 ) {
                       	close(fd);
                       	return (E_VPD);
               	}

               	/* read from pos reg 3 */
               	if( ioctl( fd, MIOCCGET, &read_record ) < 0 ) {
                       	close(fd);
                       	return (E_VPD);
               	}

               	vpd_data[vpd_index-1] = tmpbyte;
       	}

       	close(fd);

#if 0
#ifdef  CFGDEBUG
        dump_dds(vpd_data,VPDSIZE * 4);
#endif  CFGDEBUG
#endif 

	put_vpd (ret_vpd, vpd_data, VPDSIZE-1 );

	return(0);

}


