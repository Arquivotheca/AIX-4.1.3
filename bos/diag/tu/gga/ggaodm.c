static char sccsid[] = "@(#)98	1.1  src/bos/diag/tu/gga/ggaodm.c, tu_gla, bos41J, 9515A_all 4/6/95 09:27:44";
/*
 *
 * COMPONENT_NAME: (pcigga) Weitek PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS: build_dds, generate_minor, make_special_files,
 *            define_children, get_ODM_info, device_specific
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

#include <fcntl.h>
#include <stdio.h>
#include <sys/adspace.h>
#include <sys/device.h>
#include <sys/sysconfig.h>
#include <sys/types.h>
#include <sys/ioacc.h>
/*
 * odmi interface include files
 */
#include <cf.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>

#include <gga_reg.h>
#include <gga_funct.h>

#define DEVID_MAX_LEN          16
#define CONN_MAX_LEN           16
#define BUSN_MAX_LEN           16
#define FAIL -1

/* These defines are from tu_type.h */
#ifdef LOGMSGS
extern void logmsg (char *);
extern void logerror (char *);
extern void log_syserr (char *);
extern void update_msg (void);
#define LOG_SYSERR(msg)      log_syserr (msg)
#define LOG_MSG(msg)         logmsg (msg)
#define LOG_ERROR(msg)       logerror (msg)
#define ALIVE_MSG            update_msg ()
#else
#define LOG_SYSERR(msg)
#define LOG_MSG(msg)
#define LOG_ERROR(msg)
#define ALIVE_MSG
#endif

/*
 * NAME: get_slot 
 *
 * FUNCTION:
 *   Use the Customized Attribute Class to get the attribute
 *   objects needed (slot number).
 *
 * RETURNS:
 *   The return vales are
 *     0 - for success
 *     non-zero return code for failure
 */

int get_slot(char *lname, uint *slot_number, uint *bus_id)
{
int             rc;             /* return status */
int             how_many; 
int             i;              /* loop control variable*/
int             lock_id;        /* odm_lock id returned by odm_lock */
char            crit[80];       /* search criteria string */
ulong           value;          /* temp variable to get value into */
ulong           devid;
char            temp[256];      /* temporary string variables */
struct CuAt     *cusatt;        /* customized attribute ptr and object */
char            dev_id[DEVID_MAX_LEN];
char            where[CONN_MAX_LEN];
char            bus_name[BUSN_MAX_LEN];


        /* Initialize the ODM */
        if (odm_initialize () == FAIL)
        {
            /* initialization failed */
            LOG_SYSERR ("cfg_gga: odm_initialize() failed\n");
            return (E_ODMINIT);
        }

        if (((lock_id = odm_lock ("/etc/objrepos/config_lock", 0)) == FAIL))
        {
            LOG_SYSERR ("cfg_gga: odm_lock() failed\n");
            return (E_ODMLOCK);
        }

        /*-----------------------------------------------------------------
        | ODM is initialized and locked.
        |-----------------------------------------------------------------*/

        /* Read the needed info from the ODM to find the dev_id */

        rc = get_ODM_info (lname, dev_id, where, bus_name);
        if(rc == 0)
        {
            devid = strtoul(dev_id,NULL,16);
            *slot_number = strtoul(where,NULL,10);

            cusatt = getattr(bus_name, "bus_id", (char *)NULL, &how_many);
            if (cusatt != NULL)
              *bus_id = strtoul(cusatt->value,NULL,16);
            else 
	      {       
                LOG_SYSERR ("ggaodm: get_attrval() failed.\n");
                rc = -1;
	      }
        }
        else
	{
            LOG_SYSERR ("ggaodm: get_ODM_info() failed.\n");
            rc = -1;
	}

        odm_unlock(lock_id);

        odm_terminate( );

        return(rc);
}


get_ODM_info (logical_name, dev_id, where, bus_name)
        char    *logical_name;
        char    *dev_id;
        char    *where;
        char    *bus_name;
{
        struct Class    *p_Class_CuDv;
        struct Class    *p_Class_PdDv;
        struct PdDv     s_PdDv_object;
        struct CuDv     cusobj ;        /* Customized object for device */
        struct CuDv     parobj ;        /* Customized object for parent */
        int             rc;
        extern int      odmerrno;
        char            search_string[256];

        /*-----------------------------------------------------------------
        | Now, open the customized object class and get the customized obj
        |-----------------------------------------------------------------*/

        if ((int) (p_Class_CuDv = odm_open_class (CuDv_CLASS)) == FAIL)
        {
                LOG_SYSERR ("cfg_gga: open class CuDv failed\n");
                return (E_ODMOPEN);
        }

        /*-----------------------------------------------------------------
        |  next, open the predefined device object class and get the
        |  predefined device object for our device and parent
        |-----------------------------------------------------------------*/

        if (((int) (p_Class_PdDv = odm_open_class (PdDv_CLASS))) == FAIL)
        {
                LOG_SYSERR ("cfg_gga: open class PdDv failed.\n");
                odm_close_class (p_Class_CuDv);
                return(E_ODMOPEN);
        }

        sprintf (search_string, "name = '%s'", logical_name);

        rc = (int) odm_get_first (     p_Class_CuDv,
                                       search_string,
                                       &cusobj   );

        if ((rc == FAIL) || (rc == NULL))
        {
                LOG_SYSERR ("cfg_gga: Get CuDv failed.\n");
                odm_close_class (p_Class_CuDv);
                odm_close_class (p_Class_PdDv);
                return(E_NOCuDv);
        }

        /* Read Customized object of parent */
        sprintf (search_string, "name = '%s'", cusobj.parent );
        rc = (int) odm_get_first (     p_Class_CuDv,
                                       search_string,
                                       &parobj   );

        if ((rc == FAIL) || (rc == NULL))
        {
                LOG_SYSERR ("cfg_gga: Get CuDv failed.\n");
                odm_close_class (p_Class_CuDv);
                odm_close_class (p_Class_PdDv);
                return(E_PARENT);
        }

        sprintf (    search_string,
                     "uniquetype = '%s'",
                     cusobj.PdDvLn_Lvalue );

        rc = (int) odm_get_first( p_Class_PdDv,
                                  search_string,
                                  &s_PdDv_object  );

        if ((rc == FAIL) || (rc == NULL))
        {
                LOG_SYSERR ("cfg_gga: failed to find PdDv object.\n");
                odm_close_class (p_Class_CuDv);
                odm_close_class (p_Class_PdDv);
                return(E_ODMGET);
        }

        /* Copy the information we need from the ODM now */
        strncpy(dev_id, s_PdDv_object.devid, DEVID_MAX_LEN);

        strncpy(where, cusobj.connwhere, CONN_MAX_LEN);

        strncpy(bus_name, cusobj.parent, BUSN_MAX_LEN);

        /* Close down the files now */
        odm_close_class( p_Class_PdDv );
        odm_close_class( p_Class_CuDv );

        return(0);
}

