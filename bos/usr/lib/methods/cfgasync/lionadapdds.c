#ifndef lint
static char sccsid[] = "@(#)12 1.5 src/bos/usr/lib/methods/cfgasync/lionadapdds.c, cfgtty, bos41J, 9520A_all 4/27/95 12:49:39";
#endif
/*
 * COMPONENT_NAME: (CFGTTY) Build LION adapters DDS
 *
 * FUNCTIONS: lionadapdds
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>          /* standard I/O */
#include <string.h>         /* string manipulation */
#include <malloc.h>         /* Memory allocation */
#include <errno.h>          /* standard error numbers */
#include <ctype.h>
#include <math.h>
#include <cf.h>             /* error messages */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/cfgdb.h>      /* config #define */
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/cfgodm.h>     /* config structures */

/* Includes for slion */
#include <sys/str_tty.h>

#include "slion.h"            /* Useful for LION adapters */

#include "cfgdebug.h"

/*
 * ==============================================================================
 * defines and strutures
 * ==============================================================================
 */
/* defines for attributes search */
/* These defines MUST be in coherence with ODM database attribute names */
#define RANGE               "nr"
#define LIST                "nl"

#define BUS_MEM_ADDR        "bus_mem_addr"
#define BUS_INTR_LVL_ATT    "bus_intr_lvl"
#define INTR_PRIORITY_ATT   "intr_priority"
#define BUS_TYPE_ATT        "bus_type"
#define BUS_ID_ATT          "bus_id"
#define NSEG_REG_ATT        "nseg_reg"
#define ID_SEG_REG_ATT      "id_seg_reg"

#define SHARE_INTR_FLAG     0

#define IBASE	0x400000	/* Base address for POS register */
/*
 * =============================================================================
 *                       LIONADAPDDS
 * =============================================================================
 * 
 * This function builds the DDS for the streams based LION driver which is in
 * charge of the 64 portd aynchronous adapters.
 * 
 * This function operates as a device dependent subroutine called 
 * by the adapter configure method. It is used to build the dds which 
 * describes the characteristics of the adapter to the device driver.
 *
 * A pointer to the DDS built and its size are returned to the
 * configure method in order to configure the asynchronous device driver.
 * All attributes for the adapter and the bus to which it is attached are
 * obtained from the database.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * =============================================================================
 */

int lionadapdds(cusDevPtr, ddsPtr, ddsSize)
struct CuDv * cusDevPtr;/* Customized object pointer */
uchar ** ddsPtr;        /* ptr to ptr to dds structure */
int *    ddsSize;       /* ptr to size fo dds */
{
    extern int get_slot();          /* returns slot location number */
    extern int Get_Parent_Bus();    /* returns parent bus */

    int    return_code;             /* return codes go here */
    char   port;                    /* port location */
    char   work_ptr[ATTRVALSIZE];   /* temp. char array */
    char   sstring[256];            /* search criteria */

    int    slot, adapter_no;        /* attributes for device */
    ulong  busid_base;              /* bus id base address */
    long   devid;                   /* device id */
    long   width;                   /* address width for sio port */

    /* temporary bus_io_addr variables */
    long   start_io;
    long   end_io;
    long   increment_io;
    long   next_io;
    long   current_io;

    struct lion_adap_dds * adap_dds;/* pointer to dds structure */
    struct stat *   statbuf;        /* pointer to stat structure */

    /* ODM structures declarations */
    struct Class * cus_att_class;   /* customized attributes class ptr */
    struct Class * pre_att_class;   /* predefined attributes class ptr */
    struct Class * cus_dev_class;   /* customized devices class ptr */
    struct Class * pre_dev_class;   /* predefined devices class ptr */

    struct PdAt    pre_att;         /* predefined attributes object storage */
    struct CuDv    par_cus_dev;     /* parent customized device object storage */
    struct PdDv    pre_dev;         /* predefined device object storage */

    /* ============================== */
    /* set up initial variable values */
    /* ============================== */
    if ((adap_dds = (struct lion_adap_dds *) malloc (sizeof(struct lion_adap_dds)))
        == NULL) {
        DEBUG_0("lionadapdds: Malloc of dds failed\n");
        return(E_MALLOC);
    }
    else {
        DEBUG_0("lionadapdds: Clear of new allocated DDS\n");
        bzero((char *)adap_dds, sizeof(struct lion_adap_dds));
    }

    /* start up odm is done by the caller function */
    /* open customized devices object class (CuDv) */
    if ((int)(cus_dev_class = odm_open_class(CuDv_CLASS)) == -1) {
        DEBUG_0("lionadapdds: CuDv open class error\n");
        return(E_ODMOPEN);
    };

    /* ============================= */
    /* Get predefined devices object */
    /* ============================= */

    /* Open predefined object class */
    if ((int)(pre_dev_class = odm_open_class(PdDv_CLASS)) == -1) {
        DEBUG_0("lionadapdds: PdDv open class error\n");
        return(E_ODMOPEN);
    };

    /* Get predefined object */
    sprintf (sstring,"uniquetype = '%s'",cusDevPtr->PdDvLn_Lvalue);
    return_code = (int) odm_get_obj(pre_dev_class, sstring, &pre_dev, ODM_FIRST);
    switch (return_code) {
      case 0: /* odm objects not found */
        DEBUG_1 ("lionadapdds: PdDv %s found no object.\n", sstring);
        return(E_NOPdDv);
        break;  /* To avoid bug if return is suppressed */
        
      case -1: /* odm error occurred */
        DEBUG_1 ("lionadapdds: get_obj failed, %s.\n", sstring);
        return(E_ODMGET);
        break;  /* To avoid bug if return is suppressed */
        
      default: /* odm object found ==> That's OK */
        DEBUG_1 ("lionadapdds: PdDv %s found objects.\n",sstring);
    } /* End switch (return_code) */

    /* Open Customized Attribute Object Class */
    if ((int)(cus_att_class = odm_open_class(CuAt_CLASS)) == -1) {
        DEBUG_0("lionadapdds: CuAt open class error\n");
        return(E_ODMOPEN);
    };

    /* Open Predefined Attribute Object Class */
    if ((int)(pre_att_class = odm_open_class(PdAt_CLASS)) == -1) {
        DEBUG_0("lionadapdds: PdAt open class error\n");
        return(E_ODMOPEN);
    };

    /* ============== */
    /* Set DDS fields */
    /* ============== */

    /* ======== */
    /* DDS TYPE */
    /* ======== */
    adap_dds->which_dds = LION_ADAP_DDS;

    /* =========== */
    /* DEVICE NAME */
    /* =========== */
    strncpy(adap_dds->lc_name, cusDevPtr->name, DEV_NAME_LN);
    adap_dds->lc_name[DEV_NAME_LN] = '\0';

    /* ============ */
    /* ADAPTER TYPE */
    /* ============ */
    /* There is only one (for the moment) available devid */
    devid = strtoul (pre_dev.devid, (char **) NULL, 0);
    switch (devid) {
      case 0xfd61:        /* 64 port 232 */
        DEBUG_1("lionadapdds: 64p232 found lc_type = 0x%x\n", SixtyFourPort);
        adap_dds->lc_type = SixtyFourPort;
        break;
      default:
        DEBUG_1("lionadapdds: Invalid device: 0x%x\n", devid);
        return(E_WRONGDEVICE);
    } /* End switch (devid) */

    /* ================================================= */
    /* BASE ADDRESS */
    /* ================================================= */
    if (return_code = getatt(&adap_dds->lc_base, 'l', cus_att_class,
                              pre_att_class, cusDevPtr->name,
                              cusDevPtr->PdDvLn_Lvalue, BUS_MEM_ADDR,
                              NULL)) {
        DEBUG_3("lionadapdds: getatt '%s' for %s fails with error %x\n",
                BUS_MEM_ADDR, cusDevPtr->name, return_code);
        return(return_code);
    };

    /* ============================== */ 
    /* INTERRUPT ARBITRATION REGISTER */ 
    /* ============================== */ 

    /* ============== */
    /* ADAPTER NUMBER */
    /* ============== */

    /* Put adapter number in dds for device */
    /* Always 0 for 64 port */
    adap_dds->lc_anum = (uchar) 0;
    DEBUG_1("lionadapdds: adapter_no = %d.\n", adap_dds->lc_anum);

    /* =========== */
    /* SLOT NUMBER */
    /* =========== */
    if (return_code = get_slot(cusDevPtr->connwhere, &slot)) {
        return(return_code);
    };
    slot = (slot-1) & 0x0f;
    adap_dds->lc_slot = (uchar) slot;

    /* ===================================================================== */
    /* CALCULATE ADAPTER POS REGISTER ADDRESS FROM POS BASE ADDRESS AND SLOT */
    /* ===================================================================== */
    adap_dds->lc_ibase = IBASE + (slot<<16);

    /* =============== */
    /* INTERRUPT LEVEL */
    /* =============== */
    if (return_code = getatt(&adap_dds->lc_level, 'i', cus_att_class,
                              pre_att_class, cusDevPtr->name,
                              cusDevPtr->PdDvLn_Lvalue, BUS_INTR_LVL_ATT,
                              NULL)) {
        DEBUG_3("lionadapdds: getatt '%s' for %s fails with error %x\n",
                BUS_INTR_LVL_ATT, cusDevPtr->name, return_code);
        return(return_code);
    };

    /* ======================== */
    /* INTERRUPT PRIORITY CLASS */
    /* ======================== */
    if (return_code = getatt(&adap_dds->lc_priority, 'i', cus_att_class,
                              pre_att_class, cusDevPtr->name,
                              cusDevPtr->PdDvLn_Lvalue, INTR_PRIORITY_ATT,
                              NULL)) {
        DEBUG_3("lionadapdds: getatt '%s' for %s fails with error %x\n",
                INTR_PRIORITY_ATT, cusDevPtr->name, return_code);
        return(return_code);
    };

    /* ================================== */
    /* INTERRUPT SHARE FLAGS - SET TO OFF */
    /* ================================== */
    adap_dds->lc_flags = (ushort)SHARE_INTR_FLAG;

    /* ==================================== */
    /* BUS ID and SEGMENT FOR NORMAL ACCESS */
    /* ==================================== */
    /* Gets parent bus */
    return_code = Get_Parent_Bus(cus_dev_class, cusDevPtr->parent, &par_cus_dev);
    switch (return_code) {
      case 0: /* Bus device found */
        DEBUG_1("lionadapdds: Get_Parent_Bus found %s as parent bus.\n",
                 par_cus_dev.name);
        break;  /* To avoid bug if return is suppressed */
        
      case E_ODMGET: /* odm error occurred */
        DEBUG_0("lionadapdds: Get_Parent_Bus failed to access ODM database.\n");
        return(E_ODMGET);
        break;  /* To avoid bug if return is suppressed */
        
      case E_PARENT: /* Bus device not found */
        DEBUG_0("lionadapdds: Get_Parent_Bus found no parent bus.\n");
        return(E_NOCuDvPARENT);
        break;  /* To avoid bug if return is suppressed */
        
      default: /* Unknown return code */
        DEBUG_0("lionadapdds: Unknown return code from Get_Parent_Bus.\n");
        return(E_BADATTR);
    } /* End switch (return_code) */
    
    /* Build  DDS fields from Parent attributes */
    if (return_code = getatt(&adap_dds->lc_bus, 'h', cus_att_class,
                             pre_att_class, par_cus_dev.name,
                             par_cus_dev.PdDvLn_Lvalue, BUS_TYPE_ATT,
                             NULL)) {
        DEBUG_3("lionadapdds: getatt '%s' for %s fails with error %x\n",
                BUS_TYPE_ATT, par_cus_dev.name, return_code);
        return(return_code);
    };
    if (return_code = getatt(&busid_base, 'l', cus_att_class,
                             pre_att_class, par_cus_dev.name,
                             par_cus_dev.PdDvLn_Lvalue, BUS_ID_ATT,
                             NULL)) {
        DEBUG_3("lionadapdds: getatt '%s' for %s fails with error %x\n",
                BUS_ID_ATT, par_cus_dev.name, return_code);
        return(return_code);
    }
    else {
        if (return_code = getatt(&adap_dds->lc_nseg, 'l', cus_att_class,
                                 pre_att_class, cusDevPtr->name,
                                 cusDevPtr->PdDvLn_Lvalue, NSEG_REG_ATT,
                                 NULL)) {
            DEBUG_3("lionadapdds: getatt '%s' for %s fails with error %x\n",
                    NSEG_REG_ATT, cusDevPtr->name, return_code);
            return(return_code);
        };
        adap_dds->lc_nseg |= busid_base;
    }

    /* =================== */
    /* ID SEGMENT REGISTER */
    /* =================== */
    if (return_code = getatt(&adap_dds->lc_iseg, 'l', cus_att_class,
                             pre_att_class, cusDevPtr->name,
                             cusDevPtr->PdDvLn_Lvalue, ID_SEG_REG_ATT,
                             NULL)) {
        DEBUG_3("lionadapdds: getatt '%s' for %s fails with error %x\n",
                ID_SEG_REG_ATT, cusDevPtr->name, return_code);
        return(return_code);
    };
    adap_dds->lc_iseg |= busid_base;


    DEBUG_0("\tadap_dds:\n");
    DEBUG_1("\t\twhich_dds = %d\n",adap_dds->which_dds);
    DEBUG_1("\t\tlc_name = %s\n",adap_dds->lc_name);
    DEBUG_1("\t\tlc_type = %d\n",adap_dds->lc_type);
    DEBUG_1("\t\tlc_anum = %d\n",adap_dds->lc_anum);
    DEBUG_1("\t\tlc_slot = %d\n",adap_dds->lc_slot);
    DEBUG_1("\t\tlc_parent = 0x%x\n",adap_dds->lc_parent);
    DEBUG_1("\t\tlc_xbox = %d\n",adap_dds->lc_xbox);
    DEBUG_1("\t\tlc_level = %d\n",adap_dds->lc_level);
    DEBUG_1("\t\tlc_priority = %d\n",adap_dds->lc_priority);
    DEBUG_1("\t\tlc_bus = %d\n",adap_dds->lc_bus);
    DEBUG_1("\t\tlc_flags = %d\n",adap_dds->lc_flags);
    DEBUG_1("\t\tlc_nseg = 0x%x\n",adap_dds->lc_nseg);
    DEBUG_1("\t\tlc_base = 0x%x\n",adap_dds->lc_base);
    DEBUG_1("\t\tlc_iseg = 0x%x\n",adap_dds->lc_iseg);
    DEBUG_1("\t\tlc_ibase = 0x%x\n",adap_dds->lc_ibase);

    /* ========================== */
    /* Set argument output values */
    /* ========================== */
    *ddsPtr = (uchar *)adap_dds;
    *ddsSize = (int)sizeof(struct lion_adap_dds);

    /* close object classes */
    if (odm_close_class(cus_att_class) < 0) {
        return(E_ODMCLOSE);
    };
    if (odm_close_class(pre_att_class) < 0) {
        return(E_ODMCLOSE);
    };
    if (odm_close_class(cus_dev_class) < 0) {
        return(E_ODMCLOSE);
    };
    if (odm_close_class(pre_dev_class) < 0) {
        return(E_ODMCLOSE);
    };

    DEBUG_0("Returning from lionadapdds\n");

    return(0); /* return to generic config with success */
} /* End int lionadapdds(...) */
