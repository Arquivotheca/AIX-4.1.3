#ifndef lint
static char sccsid[] = "@(#)11 1.5 src/bos/usr/lib/methods/cfgasync/rsadapdds.c, cfgtty, bos41J, 9520A_all 4/27/95 12:49:37";
#endif
/*
 * COMPONENT_NAME: (CFGTTY) Build RS adapters DDS
 *
 * FUNCTIONS: rsadapdds
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

/* Includes for srs */
#include <sys/cblock.h>
#include <sys/intr.h>
#include <sys/str_tty.h>

#include "srs.h"            /* Useful for RS adapters */

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

#define BUS_IO_ADDR_ATT     "bus_io_addr"
#define FREQUENCY_ATT       "frequency"
#define ARB_REG_ATT         "arb_reg"
#define BUS_INTR_LVL_ATT    "bus_intr_lvl"
#define INTR_PRIORITY_ATT   "intr_priority"
#define BUS_TYPE_ATT        "bus_type"
#define BUS_ID_ATT          "bus_id"
#define NSEG_REG_ATT        "nseg_reg"
#define ID_SEG_REG_ATT      "id_seg_reg"

#define SHARE_INTR_FLAG     0

/* Existing native I/O line adapter */
/* It is used to retrieve which native I/O line adapter */
/* is concerned by this DDS build function */
#define EXISTING_NIO        "123"
#define NATIVEC_GAP         0x50

#define IBASE	0x400000	/* Base address for POS register */
extern int	   trueslot;		    /* real slot of STD I/O */

/*
 * =============================================================================
 *                       RSADAPDDS
 * =============================================================================
 * 
 * This function builds the DDS for the streams based SRS driver which is in
 * charge of aynchronous adapters (8,16 port and SIO serial).
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

int rsadapdds(cusDevPtr, ddsPtr, ddsSize)
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
    long   busid_base;              /* bus id base address */
    long   devid;                   /* device id */
    long   width;                   /* address width for sio port */

    /* temporary bus_io_addr variables */
    long   start_io;
    long   end_io;
    long   increment_io;
    long   next_io;
    long   current_io;

    struct rs_adap_dds * adap_dds;   /* pointer to dds structure */
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
    if ((adap_dds = (struct rs_adap_dds *) malloc (sizeof(struct rs_adap_dds)))
        == NULL) {
        DEBUG_0("rsadapdds: Malloc of dds failed\n");
        return(E_MALLOC);
    }
    else {
        DEBUG_0("rsadapdds: Clear of new allocated DDS\n");
        bzero((char *)adap_dds, sizeof(struct rs_adap_dds));
    }

    /* start up odm is done by the caller function */
    /* open customized devices object class (CuDv) */
    if ((int)(cus_dev_class = odm_open_class(CuDv_CLASS)) == -1) {
        DEBUG_0("rsadapdds: CuDv open class error\n");
        return(E_ODMOPEN);
    };

    /* ============================= */
    /* Get predefined devices object */
    /* ============================= */

    /* Open predefined object class */
    if ((int)(pre_dev_class = odm_open_class(PdDv_CLASS)) == -1) {
        DEBUG_0("rsadapdds: PdDv open class error\n");
        return(E_ODMOPEN);
    };

    /* Get predefined object */
    sprintf (sstring,"uniquetype = '%s'",cusDevPtr->PdDvLn_Lvalue);
    return_code = (int) odm_get_obj(pre_dev_class, sstring, &pre_dev, ODM_FIRST);
    switch (return_code) {
      case 0: /* odm objects not found */
        DEBUG_1 ("rsadapdds: PdDv %s found no object.\n", sstring);
        return(E_NOPdDv);
        break;  /* To avoid bug if return is suppressed */
        
      case -1: /* odm error occurred */
        DEBUG_1 ("rsadapdds: get_obj failed, %s.\n", sstring);
        return(E_ODMGET);
        break;  /* To avoid bug if return is suppressed */
        
      default: /* odm object found ==> That's OK */
        DEBUG_1 ("rsadapdds: PdDv %s found objects.\n",sstring);
    } /* End switch (return_code) */

    /* Open Customized Attribute Object Class */
    if ((int)(cus_att_class = odm_open_class(CuAt_CLASS)) == -1) {
        DEBUG_0("rsadapdds: CuAt open class error\n");
        return(E_ODMOPEN);
    };

    /* Open Predefined Attribute Object Class */
    if ((int)(pre_att_class = odm_open_class(PdAt_CLASS)) == -1) {
        DEBUG_0("rsadapdds: PdAt open class error\n");
        return(E_ODMOPEN);
    };

    /* ============== */
    /* Set DDS fields */
    /* ============== */

    /* ======== */
    /* DDS TYPE */
    /* ======== */
    adap_dds->which_dds = RS_ADAP_DDS;

    /* =========== */
    /* DEVICE NAME */
    /* =========== */
    strncpy(adap_dds->rc_name, cusDevPtr->name, DEV_NAME_LN);
    adap_dds->rc_name[DEV_NAME_LN] = '\0';

    /* ============ */
    /* ADAPTER TYPE */
    /* ============ */
    /* If we are directly connected on the mca bus */
    /* it means it isn't on the native I/O card */
    if (!strcmp(pre_dev.subclass, "mca")) {
        devid = strtoul (pre_dev.devid, (char **) NULL, 0);
        switch (devid) {
          case 0xd0ed:        /* 8 port 232 */
            DEBUG_1("rsadapdds: 8p232 found rc_type = 0x%x\n", Eight_232);
            adap_dds->rc_type = Eight_232;
            break;
          case 0xd1ed:        /* 8 port 422 */
            DEBUG_1("rsadapdds: 8p422 found rc_type = 0x%x\n", Eight_422);
            adap_dds->rc_type = Eight_422;
            break;
          case 0xd2ed:        /* 8 port 188 */
            DEBUG_1("rsadapdds: 8p188 found rc_type = 0x%x\n", Eight_Mil);
            adap_dds->rc_type = Eight_Mil;
            break;
          case 0xd3ed:        /* 16 port 422 */
            DEBUG_1("rsadapdds: 16p422 found rc_type = 0x%x\n", Sixteen_422);
            adap_dds->rc_type = Sixteen_422;
            break;
          case 0xd6ed:        /* 16 port 232 */
            DEBUG_1("rsadapdds: 16p232 found rc_type = 0x%x\n", Sixteen_232);
            adap_dds->rc_type = Sixteen_232;
            break;
          default:
            DEBUG_1("rsadapdds: Invalid device: 0x%x\n", devid);
            return(E_WRONGDEVICE);
        } /* End switch (devid) */
    }
    else { /* Not on mca bus ==> Must be on native I/O card */
        /* Native I/O adapters are connected on native I/O card */
        /* at connwhere position. This field contains the number */
        /* corresponding with which native I/O is concerned */
        port = cusDevPtr->connwhere[strcspn(cusDevPtr->connwhere, EXISTING_NIO)];
        /* SIO Async */
        switch (port) {
          case '1':
            DEBUG_1("rsadapdds: NativeA_SIO found rc_type = 0x%x\n", NativeA_SIO);
            adap_dds->rc_type = NativeA_SIO;
            break;
          case '2':
            DEBUG_1("rsadapdds: NativeB_SIO found rc_type = 0x%x\n", NativeB_SIO);
            adap_dds->rc_type = NativeB_SIO;
            break;
          case '3':
            DEBUG_1("rsadapdds: NativeC_SIO3 found rc_type = 0x%x\n", NativeC_SIO3);
            adap_dds->rc_type = NativeC_SIO3;
            break;
          default:
            DEBUG_1("rsadapdds: Unknown native for port = %c\n", port);
            return(E_WRONGDEVICE);
        } /* End switch (port) */
    } /* End if (!strcmp(pre_dev.subclass, "mca")) */
    
    /* ======================== */
    /* DMA LEVEL */
    /* dma is no more supported */
    /* ======================== */
    adap_dds->rc_dma = (uchar)0;
    DEBUG_1("rsadapdds: DMA no more supported: rc_dma = %d\n", adap_dds->rc_dma);

    /* =================================================== */
    /* BASE ADDRESS */
    /* Be careful: rc_base is modified for NativeB_SIO and */
    /* ==========  NativeC_SIO3 hereafter */
    /* =================================================== */
    if (return_code = getatt(&adap_dds->rc_base, 'l', cus_att_class,
                              pre_att_class, cusDevPtr->name,
                              cusDevPtr->PdDvLn_Lvalue, BUS_IO_ADDR_ATT,
                              NULL)) {
        DEBUG_3("rsadapdds: getatt '%s' for %s fails with error %x\n",
                BUS_IO_ADDR_ATT, cusDevPtr->name, return_code);
        return(return_code);
    };

    /* ========= */
    /* FREQUENCY */
    /* ========= */
    if (return_code = getatt(&adap_dds->rc_xtal, 'l', cus_att_class,
                              pre_att_class, cusDevPtr->name,
                              cusDevPtr->PdDvLn_Lvalue, FREQUENCY_ATT,
                              NULL)) {
        DEBUG_3("rsadapdds: getatt '%s' for %s fails with error %x\n",
                FREQUENCY_ATT, cusDevPtr->name, return_code);
        return(return_code);
  };

    /* ============================== */ 
    /* INTERRUPT ARBITRATION REGISTER */ 
    /* ============================== */ 

    if (return_code = getatt(&adap_dds->rc_arb, 'l', cus_att_class,
                             pre_att_class, cusDevPtr->name,
                             cusDevPtr->PdDvLn_Lvalue, ARB_REG_ATT,
                             NULL)) {
    		DEBUG_3("rsadapdds: getatt '%s' for %s fails with error %x\n",
             		ARB_REG_ATT, cusDevPtr->name, return_code);
         	return(return_code);
     };

    /* ============== */
    /* ADAPTER NUMBER */
    /* ============== */

    /* Get bus I/O address predefined attribute object for device */
    sprintf (sstring,"uniquetype = '%s' AND attribute = '%s'",
                cusDevPtr->PdDvLn_Lvalue, BUS_IO_ADDR_ATT);

    return_code = (int) odm_get_obj(pre_att_class, sstring, &pre_att, ODM_FIRST);
    switch (return_code) {
      case 0: /* odm objects not found */
        DEBUG_1("rsadapdds: PdAt %s found no objects.\n", sstring);
        return(E_NOATTR);
        break;  /* To avoid bug if return is suppressed */
        
      case -1: /* odm error occurred */
        DEBUG_1("rsadapdds: get_obj failed, %s.\n", sstring);
        return(E_ODMGET);
        break;  /* To avoid bug if return is suppressed */
        
      default: /* odm object found ==> That's OK */
        DEBUG_1("rsadapdds: PdAt %s found objects.\n", sstring);
    } /* End switch (return_code) */

    /* Set current bus i/o address */
    current_io = adap_dds->rc_base;

    /* Get possible values for bus I/O address */
    strcpy(work_ptr, pre_att.values);

    /* If Representation of bus i/o address is range */
    if (!strcmp(pre_att.rep, RANGE)) {
        
        /* Scan the possible values of the i/o bus address attribute */
        /* and Convert the starting, ending, and increment hexadecimal */
        /* addresses into their decimal values. */
        start_io = strtoul (strtok(work_ptr, "-"), (char **)NULL, 0);
        end_io = strtoul (strtok(NULL, ","), (char **)NULL, 0);
        increment_io = strtoul (strtok(NULL, ""), (char **)NULL, 0);
        
        /* Calculate adapter number by incrementing through the possible */
        /*  address range and comparing each possible address with the */
        /*  customized address value. The adapter number corresponds */
        /*  sequentially to the increments in the range. i.e. 1st addr = */
        /*  adpater number 1, 2nd addr = adapter number 2, etc. */
        for (adapter_no = 0,next_io = start_io; (current_io != next_io) 
             && (next_io <= end_io); next_io+= increment_io,
             adapter_no++);
        
    }
    else {
        /* If Representation of bus i/o address is list */
        if (!strcmp(pre_att.rep,LIST)) {
            if (strchr(work_ptr,',') == NULL) {
                /* Only one address in list - Set adapter num to 0 */
                adapter_no = 0;
            }
            else {
                /* More than one address in the list */
                next_io = strtoul(strtok(work_ptr,","),(char **)NULL,0);
                for (adapter_no=1; current_io != next_io; adapter_no++,
                     next_io = strtoul(strtok(NULL,","), (char **)NULL, 0));
            } /* End if (strchr(...) == NULL) */
        }
        else {
            return(E_BADATTR);
        } /* End if (!strcmp(pre_att.rep,LIST)) */
    } /* End if (!strcmp(pre_att.rep,RANGE)) */

    /* Put adapter number in dds for device */
    adap_dds->rc_anum = (uchar) adapter_no;
    DEBUG_1("rsadapdds: adapter_no = %d.\n",adap_dds->rc_anum);

    /* Subtract width from address of SIO async port B - */
    /* Driver needs base address of the adapter (ports A&B) */
    /* The third line has a NATIVEC_GAP gap */
    /* from the NativeA_SIO BUS_IO_ADDR_ATT attribute */
    switch (adap_dds->rc_type) {
      case NativeB_SIO:
        width = strtoul (pre_att.width, (char **)NULL, 0);
        adap_dds->rc_base -= width;
        break;
      case NativeC_SIO3:
        adap_dds->rc_base -= NATIVEC_GAP;
        break;
      default:
        DEBUG_2("rsadapdds: No substract needed for %s (adapter type = 0x%x)\n",
                BUS_IO_ADDR_ATT, adap_dds->rc_type);
    } /* End switch (adap_dds->rc_type) */
    
    /* =========== */
    /* SLOT NUMBER */
    /* =========== */
    if (return_code = get_slot(cusDevPtr->connwhere, &slot)) {
        return(return_code);
    };
    slot = (slot-1) & 0x0f;
    adap_dds->rc_slot = (uchar) slot;

    /* ===================================================================== */
    /* CALCULATE ADAPTER POS REGISTER ADDRESS FROM POS BASE ADDRESS AND SLOT */
    /* ===================================================================== */
	/* ================================================================= */
        /* if Native I/O get the true slot of SIO parent                     */
	/* ================================================================= */
    if (slot == 0xf) 
       adap_dds->rc_ibase = IBASE + (trueslot<<16);
    else 
       adap_dds->rc_ibase = IBASE + (slot<<16);

    /* =============== */
    /* INTERRUPT LEVEL */
    /* =============== */
    if (return_code = getatt(&adap_dds->rc_level, 'i', cus_att_class,
                              pre_att_class, cusDevPtr->name,
                              cusDevPtr->PdDvLn_Lvalue, BUS_INTR_LVL_ATT,
                              NULL)) {
        DEBUG_3("rsadapdds: getatt '%s' for %s fails with error %x\n",
                BUS_INTR_LVL_ATT, cusDevPtr->name, return_code);
        return(return_code);
    };

    /* ======================== */
    /* INTERRUPT PRIORITY CLASS */
    /* ======================== */
    if (return_code = getatt(&adap_dds->rc_priority, 'i', cus_att_class,
                              pre_att_class, cusDevPtr->name,
                              cusDevPtr->PdDvLn_Lvalue, INTR_PRIORITY_ATT,
                              NULL)) {
        DEBUG_3("rsadapdds: getatt '%s' for %s fails with error %x\n",
                INTR_PRIORITY_ATT, cusDevPtr->name, return_code);
        return(return_code);
    };

    /* ================================== */
    /* INTERRUPT SHARE FLAGS - SET TO OFF */
    /* ================================== */
    adap_dds->rc_flags = (ushort)SHARE_INTR_FLAG;

    /* ==================================== */
    /* BUS ID and SEGMENT FOR NORMAL ACCESS */
    /* ==================================== */
    /* Gets parent bus */
    return_code = Get_Parent_Bus(cus_dev_class, cusDevPtr->parent, &par_cus_dev);
    switch (return_code) {
      case 0: /* Bus device found */
        DEBUG_1("rsadapdds: Get_Parent_Bus found %s as parent bus.\n",
                 par_cus_dev.name);
        break;  /* To avoid bug if return is suppressed */
        
      case E_ODMGET: /* odm error occurred */
        DEBUG_0("rsadapdds: Get_Parent_Bus failed to access ODM database.\n");
        return(E_ODMGET);
        break;  /* To avoid bug if return is suppressed */
        
      case E_PARENT: /* Bus device not found */
        DEBUG_0("rsadapdds: Get_Parent_Bus found no parent bus.\n");
        return(E_NOCuDvPARENT);
        break;  /* To avoid bug if return is suppressed */
        
      default: /* Unknown return code */
        DEBUG_0("rsadapdds: Unknown return code from Get_Parent_Bus.\n");
        return(E_BADATTR);
    } /* End switch (return_code) */
    
    /* Build  DDS fields from Parent attributes */
    if (return_code = getatt(&adap_dds->rc_bus, 'h', cus_att_class,
                             pre_att_class, par_cus_dev.name,
                             par_cus_dev.PdDvLn_Lvalue, BUS_TYPE_ATT,
                             NULL)) {
        DEBUG_3("rsadapdds: getatt '%s' for %s fails with error %x\n",
                BUS_TYPE_ATT, par_cus_dev.name, return_code);
        return(return_code);
    };
    if (return_code = getatt(&busid_base, 'l', cus_att_class,
                             pre_att_class, par_cus_dev.name,
                             par_cus_dev.PdDvLn_Lvalue, BUS_ID_ATT,
                             NULL)) {
        DEBUG_3("rsadapdds: getatt '%s' for %s fails with error %x\n",
                BUS_ID_ATT, par_cus_dev.name, return_code);
        return(return_code);
    }
    else {
        if (return_code = getatt(&adap_dds->rc_nseg, 'l', cus_att_class,
                                 pre_att_class, cusDevPtr->name,
                                 cusDevPtr->PdDvLn_Lvalue, NSEG_REG_ATT,
                                 NULL)) {
            DEBUG_3("rsadapdds: getatt '%s' for %s fails with error %x\n",
                    NSEG_REG_ATT, cusDevPtr->name, return_code);
            return(return_code);
        };
        adap_dds->rc_nseg |= busid_base;
    }

    /* =================== */
    /* ID SEGMENT REGISTER */
    /* =================== */
    if (return_code = getatt(&adap_dds->rc_iseg, 'l', cus_att_class,
                             pre_att_class, cusDevPtr->name,
                             cusDevPtr->PdDvLn_Lvalue, ID_SEG_REG_ATT,
                             NULL)) {
        DEBUG_3("rsadapdds: getatt '%s' for %s fails with error %x\n",
                ID_SEG_REG_ATT, cusDevPtr->name, return_code);
        return(return_code);
    };
    adap_dds->rc_iseg |= busid_base;


    DEBUG_0("\tadap_dds:\n");
    DEBUG_1("\t\twhich_dds = %d\n",adap_dds->which_dds);
    DEBUG_1("\t\trc_name = %s\n",adap_dds->rc_name);
    DEBUG_1("\t\trc_type = %d\n",adap_dds->rc_type);
    DEBUG_1("\t\trc_anum = %d\n",adap_dds->rc_anum);
    DEBUG_1("\t\trc_slot = %d\n",adap_dds->rc_slot);
    DEBUG_1("\t\trc_parent = 0x%x\n",adap_dds->rc_parent);
    DEBUG_1("\t\trc_level = %d\n",adap_dds->rc_level);
    DEBUG_1("\t\trc_priority = %d\n",adap_dds->rc_priority);
    DEBUG_1("\t\trc_dma = %d\n",adap_dds->rc_dma);
    DEBUG_1("\t\trc_xtal = %lu\n",adap_dds->rc_xtal);
    DEBUG_1("\t\trc_bus = %d\n",adap_dds->rc_bus);
    DEBUG_1("\t\trc_flags = %d\n",adap_dds->rc_flags);
    DEBUG_1("\t\trc_arb = 0x%x\n",adap_dds->rc_arb);
    DEBUG_1("\t\trc_nseg = 0x%x\n",adap_dds->rc_nseg);
    DEBUG_1("\t\trc_base = 0x%x\n",adap_dds->rc_base);
    DEBUG_1("\t\trc_ibase = 0x%x\n",adap_dds->rc_ibase);
    DEBUG_1("\t\trc_iseg = 0x%x\n",adap_dds->rc_iseg);

    /* ========================== */
    /* Set argument output values */
    /* ========================== */
    *ddsPtr = (uchar *)adap_dds;
    *ddsSize = (int)sizeof(struct rs_adap_dds);

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

    DEBUG_0("Returning from rsadapdds\n");

    return(0); /* return to generic config with success */
} /* End int rsadapdds(...) */
