static char sccsid[] = "@(#)58        1.11  src/bos/usr/lib/methods/cfgeth/cfgeth.c, sysxient, bos411, 9428A410j 5/18/94 16:36:32";
/*
**   COMPONENT_NAME: SYSXIENT
**
**   FUNCTIONS: build_dds
**              download_microcode
**              query_vpd
**              define_children
**              device_specific
**
**   ORIGINS: 27
**
**   (C) COPYRIGHT International Business Machines Corp. 1990,1994
**   All Rights Reserved
**   Licensed Materials - Property of IBM
**   US Government Users Restricted Rights - Use, duplication or
**   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*/

/*
 *   cfgeth.c - Configure Method for Integrated Ethernet Adapters
*/

#include <stdio.h>
#include <cf.h>         /* Error codes */

#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/stat.h>
#include <sys/sysconfig.h>
#include <sys/device.h> 
#include <sys/cdli_entuser.h>        
#include <sys/ndd.h>
#include <sys/err_rec.h>
#include "cfg_ndd.h"
#include "ciepkg.h"   /* header file for cfgcie method, see define_children() */

#include "cfgdebug.h"
#include "pparms.h"

#include <ient/i_entdds.h>

#ifdef CFGDEBUG
#include <errno.h>
#endif

static  struct    attr_list *alist=NULL;   /* PdAt attribute list      */
int     how_many;                          /* Used by getattr routine. */
int     byte_cnt;                          /* Count of attributes retrieved */

#define GETATT(A,B,C)    getatt(alist,C,A,B,&byte_cnt)

#define VPD_LENGTH   256

/*
 * NAME: build_dds()
 * 
 * FUNCTION:    Builds the dds for the ethernet device.
 * 
 * EXECUTION ENVIRONMENT:
 *
 *      This process is invoked from the generic config device routine
 *      to build the DDS structure for an ethernet adapter.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int build_dds(logical_name,odm_info,dds_ptr,dds_length)
char *logical_name;
ndd_cfg_odm_t *odm_info;
char **dds_ptr;
long *dds_length;
{
    ient_dds_t      *ddi_ptr;
    struct  CuDv    cust_obj;
    struct  CuDv    bus_obj;
    struct  CuAt    *CuAt_obj;
    struct  PdAt    PA_obj;
    struct  attr_list   *bus_attr_list = NULL;
    char    *parentdesc, srchstr[256], utype[UNIQUESIZE], *ptr, tmp_str[5];
    int     i, rc;
    int     num_bus_attrs;

    /* declarations for get vpd for network address */
    char    busdev[32];
    int     fd;             /* machine device driver file descriptor */


    DEBUG_0("build_dds(): BEGIN build_dds()\n")

    *dds_length=sizeof(ient_dds_t);

    if ((ddi_ptr = (ient_dds_t *) malloc(sizeof(ient_dds_t)))
                                                  == (ient_dds_t *) NULL)
    {
        DEBUG_0 ("build_dds(): Malloc of dds failed\n")
        return E_MALLOC;
    }

    *dds_ptr=(uchar *) ddi_ptr;

    /* set default value for all attributes to zero */
    bzero(ddi_ptr,sizeof(ient_dds_t));           /* zero out dds structure */

    /* Get customized object */
    sprintf(srchstr,"name = '%s'",logical_name);
    if ((rc = (int) odm_get_first(CuDv_CLASS, srchstr, &cust_obj)) == 0)
        return E_NOCuDv;
    else if (rc == -1)
        return E_ODMGET;
    strcpy(utype,cust_obj.PdDvLn_Lvalue);

    /* Get slot */
    DEBUG_0("build_dds(): getting slot from CuDv\n")
        ddi_ptr->slot = atoi(cust_obj.connwhere) - 1;

    DEBUG_1("build_dds(): slot=%d\n",ddi_ptr->slot)

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
    /*              Regular Attributes                               */
    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
    /*
    **  Get attribute list
    */
    if ((alist = get_attr_list(logical_name, utype, &how_many,16)) == NULL)
        return(E_ODMGET);

    if(rc = GETATT(&ddi_ptr->intr_level, 'i', "bus_intr_lvl"))
        return(rc);

    if(rc = GETATT(&ddi_ptr->intr_priority, 'i', "intr_priority"))
        return(rc);

    if(rc = GETATT(&ddi_ptr->xmt_que_size,'i', "xmt_que_size"))
        return(rc);

    /*
    ** bus_mem_addr attribute only valid for deskside box
    ** if attribute does not exist, ok, continue
    ** if read error, then return error
    */

    rc = GETATT(&ddi_ptr->bus_mem_addr, 'a', "bus_mem_addr");
    if (rc != 0 && rc != E_NOATTR)
    {
        DEBUG_1("build_dds(): error reading bus_mem_addr attribute, rc = %d\n",
                rc)
        return(rc);
    }

    DEBUG_1("build_dds(): bus_mem_addr attribute = %x\n",ddi_ptr->bus_mem_addr)


    if (rc = GETATT(&ddi_ptr->tcw_bus_mem_addr, 'a', "dma_bus_mem"))
        return(rc);

    if (rc = GETATT(&ddi_ptr->io_port, 'a', "bus_io_addr"))
        return(rc);

    if (rc = GETATT(&ddi_ptr->dma_arbit_lvl, 'i', "dma_lvl"))
        return(rc);

    if (rc = GETATT(tmp_str, 's', "use_alt_addr"))
        return(rc);

    ddi_ptr->use_alt_addr = (tmp_str[0] == 'y') ? 1 : 0;

    if (ddi_ptr->use_alt_addr)
    {
        rc = GETATT(ddi_ptr->alt_addr, 'b', "alt_addr");
        if (byte_cnt != 6)
        {
            if (rc > 0)
                return(rc);
            else
            {
                DEBUG_1("build_dds(): getatt() bytes=%d\n",rc)
                return E_BADATTR;
            }
        }

        DEBUG_6("build_dds(): alt_addr=%02x %02x %02x %02x %02x %02x\n",
            (int) ddi_ptr->alt_addr[0],(int) ddi_ptr->alt_addr[1],
            (int) ddi_ptr->alt_addr[2],(int) ddi_ptr->alt_addr[3],
            (int) ddi_ptr->alt_addr[4],(int) ddi_ptr->alt_addr[5])
    }

    /* Get logical name */
    DEBUG_1("build_dds(): logical name = %s\n",logical_name)
    strcpy(ddi_ptr->lname,logical_name);

    DEBUG_1("build_dds(): logical_name = %4s\n",ddi_ptr->lname)

    /* create alias name, "en" appended with sequence number */
    rc = strncmp(odm_info->preobj.prefix,logical_name,
                                          strlen(odm_info->preobj.prefix));

    strcpy(ddi_ptr->alias, "en");

    if (rc == 0)
    {
        /* logical name created from prefix, copy only extension */
        strcat(ddi_ptr->alias,logical_name+strlen(odm_info->preobj.prefix));
    }
    else
    {
        /* logical name not created from prefix, append entire string */
        strcat(ddi_ptr->alias,logical_name);
    }

    DEBUG_1("build_dds(): alias name is %s\n",ddi_ptr->alias)


    /* Attributes from other Object Classes/Places */

    /* Get tcw bus memory size */
    DEBUG_1("build_dds(): uniquetype = %s\n",utype)
    sprintf(srchstr, "uniquetype = '%s' AND attribute = '%s'", utype,
                                                             "dma_bus_mem");

    DEBUG_1("build_dds(): criteria = '%s'\n",srchstr)

    if ((rc = (int)odm_get_first(PdAt_CLASS, srchstr, &PA_obj)) == 0)
        return E_NOPdOBJ;

    else if (rc == -1)
        return E_ODMGET;

    DEBUG_0("build_dds(): odm_get_obj() succeeded\n")
    DEBUG_1("build_dds(): PA_obj.width = %s\n",PA_obj.width)

    if ((rc = convert_att(&ddi_ptr->tcw_bus_mem_size,'l',PA_obj.width,'n'))!=0)
        return E_BADATTR;

    DEBUG_0("build_dds(): convert_att() succeeded\n")
    DEBUG_1("build_dds(): tcw_bus_mem_size = %ld\n",ddi_ptr->tcw_bus_mem_size)

    /*
    **  Get bus type, bus id, and bus number.
    */

    rc = Get_Parent_Bus(CuDv_CLASS, cust_obj.parent, &bus_obj);
    if (rc)
    {
        if (rc == E_PARENT)
            rc = E_NOCuDvPARENT;
        return (rc);
    }

    if ((bus_attr_list = get_attr_list(bus_obj.name, bus_obj.PdDvLn_Lvalue,
                                        &num_bus_attrs, 4)) == NULL)
        return (E_ODMGET);

    if (rc = getatt(bus_attr_list, "bus_type", &ddi_ptr->bus_type, 'i'))
        return (rc);

    if (rc = getatt(bus_attr_list, "bus_id", &ddi_ptr->bus_id, 'l'))
        return (rc);

    ddi_ptr->bus_id |= 0x800C0000;

    DEBUG_1("build_dds(): bus_id = 0x%x\n", ddi_ptr->bus_id)

    /*
    ** Call device specific subroutine to get
    ** network address from the system hardware
    */

    sprintf(busdev, "/dev/%s",bus_obj.name) ;

    if (0 > (fd = open(busdev, O_RDWR)) )
    {
        perror("[busquery]open()");
        fprintf(stderr, "Unable to open %s\n", busdev) ;
        return(E_DEVACCESS);
    }
    else
    {
        rc = search_eth_addr(ddi_ptr->eth_addr);
        if (rc != 0)
        {
            /* error in reading VPD or ethernet address */
            DEBUG_1("build_dds(): return from get vpd = %d\n",rc)
            return(rc);
        }

        DEBUG_1("build_dds(): get ethernet address rc=%d\n",rc)
        DEBUG_6("build_dds(): eth_addr=%02x %02x %02x %02x %02x %02x\n",
                (int) ddi_ptr->eth_addr[0],(int) ddi_ptr->eth_addr[1],
                (int) ddi_ptr->eth_addr[2],(int) ddi_ptr->eth_addr[3],
                (int) ddi_ptr->eth_addr[4],(int) ddi_ptr->eth_addr[5])
        close(fd);
    }


    #ifdef CFGDEBUG
    hexdump(ddi_ptr,(long) sizeof(ient_dds_t));
    #endif

    return 0;
}


/*
 * NAME:
 *      download_microcode()
 * 
 * FUNCTION:
 *      There is no microcode.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int download_microcode(logical_name, odm_info, cfg_k)
char              *logical_name;
ndd_cfg_odm_t     *odm_info;
struct  cfg_kmod  *cfg_k;
{
    DEBUG_0("download_microcode(): NULL function returning 0\n")
    return(0);
}

/*
 * NAME:
 *      query_vpd()
 * 
 * FUNCTION:
 *      Retrieve Vital Product Data (VPD) from the adapter card in
 *      order to store it in the database from later use.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int query_vpd(logical_name, odm_info, cfg_k, vpd)
char              *logical_name;
ndd_cfg_odm_t     *odm_info;
struct  cfg_kmod  *cfg_k;
char              *vpd;
{
    char key[] = "ETHERNET";

    get_vpd(vpd, key);
    return(0);
}

/*
 * NAME:
 *      define_children()
 *
 * FUNCTION:
 *      There are no children.
 *      The comio emulator device will be verified if installed, and if so
 *      its configure routine will be called.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int define_children(logical_name, odm_info, phase)
char *logical_name;
ndd_cfg_odm_t *odm_info;
int  phase;
{

    int rtn;
    struct stat stat_buf;
    char    sstring[256];           /* parameter temp space */

    DEBUG_0("define_children(): BEGIN routine\n")

    /*
    ** Begin section for configuring comio emulator for ethernet
    ** if it has been installed.  If configuration method for
    ** emulator is present (code has been installed), execute it.
    ** If file has not been installed, continue with no error.
    */
    sprintf(sstring,"lslpp -l %s > /dev/null 2>&1", CFG_EMULATOR_LPP);
    rtn = system(sstring);

    if (rtn == 0)     /* directory exists */
    {
        rtn = stat(CFG_COMIO_EMULATOR, &stat_buf);
        if (rtn == 0)          /* file exists */
        {
            /* call specified method with parameters */
            sprintf( sstring, " -l %s ", logical_name);
            DEBUG_2("cfgeth: calling %s %s\n",CFG_COMIO_EMULATOR, sstring)

            if (odm_run_method(CFG_COMIO_EMULATOR, sstring, NULL, NULL))
            {
                fprintf(stderr,"cfgeth: can't run %s\n", CFG_COMIO_EMULATOR);
                return(E_ODMRUNMETHOD);
            }
        }
        else
        {
            /* package installed, but file missing, return error */
            return(E_ODMRUNMETHOD);
        }
    }

    /* End section for configuring comio emulator for ethernet */

    return(0);
}

/*
 * NAME: device_specific
 *
 * FUNCTION:This is a null function for the time being and returns success
 * always
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function operates as a device dependent subroutine called by the
 *      generic configure method for all devices.
 *
 * (NOTES:)
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS: Returns 0 (success), if called.
 */

int device_specific(logical_name, odm_info, cfg_k)
char              *logical_name;
ndd_cfg_odm_t     *odm_info;
struct  cfg_kmod  *cfg_k;
{
    DEBUG_1("device_specific: NULL subroutine for %s\n",logical_name)
    return(0);

}


/*
 * NAME:
 *      search_eth_addr()
 *
 * FUNCTION:
 *      Searchs for and copies out the ethernet network address from
 *      the vpd string.
 *
 * DESCRIPTION:
 *      Called by the device specific routine get_data_power()
 *
 * INPUT:
 *      pointer to storage to put ethernet address
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */
int search_eth_addr(p_addr)
char    p_addr[];
{
    int i, j;
    int na_found;
    char vpd[VPD_LENGTH];
    char key[] = "ETHERNET";

    DEBUG_0("search_eth_addr(): start of routine\n")

    if (get_vpd(vpd, key))
        return(E_VPD);

    #ifdef CFGDEBUG
    hexdump(vpd,VPD_LENGTH);
    #endif

    /*
    **  find the network address
    */

    i = na_found = 0;
    while ( i < ( VPD_LENGTH - 2 ))
    {
        if ((vpd[i] == '*' ) && (vpd[i + 1] == 'N' ) && (vpd[i + 2] == 'A' ))
        {
            /* put Network Address in DDS  */
            i += 4;
            for (j = 0; j < ENT_NADR_LENGTH; j++,i++)
                p_addr[ j ] = vpd[ i ];

            na_found = 1;
            break;
        }
        i++;
    }

    if (na_found == 0)
    {
        DEBUG_0("search_eth_addr(): no address found in VPD\n")
        return (6);
    }
    else
    {
        return(0);
    }
}
