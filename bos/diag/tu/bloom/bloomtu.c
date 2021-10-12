static char sccsid[] = "@(#)39  1.4  src/bos/diag/tu/bloom/bloomtu.c, tu_bloom, bos41J, 9523B_all 6/6/95 15:19:29";
/*
 *   COMPONENT_NAME: TU_BLOOM
 * 
 *   FUNCTIONS: adapter_close
 *              adapter_open
 *              print_error
 *              getadapter_attr
 *              err_exit
 *              exectu
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef SIXDG
#include <string.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <cf.h>
#include <sys/types.h>
#include <sys/nvdd.h> 
#include <sys/errno.h>
#include <sys/sysconfig.h>
#include <sys/param.h>
#include <sys/diagex.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <sys/mdio.h>
#include <sys/ioacc.h>
#include "bcm.h"
#endif
#include <stdio.h>
#ifdef SIXDG
#include <fw_api.h>
#include <io_sub.h>
extern int diag_interact;
#endif
#include "bloomtu.h"
#include "ncr8xxreg.h"

/******************************************************************************
*
* NAME: print_error
*
* FUNCTION:  prints error information for error debugging.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: none.
*
* EXTERNAL PROCEDURES CALLED: printf.
*
******************************************************************************/
void print_error(adapter_struc_t *adapter_info, char *error_string)
{
#ifdef SIXDG
     if (diag_interact) {
        text_responsemsg_tr("Error PCI SCSI Adapter Test",error_string,
                            ENTER_RSP,ERROR_STYLE);
     }
#endif
#ifndef SIXDG
     switch (adapter_info->print) {
        case NO_PRINT:       break;
        case SCREEN_PRINT:
             fprintf(stderr,"%s",error_string);
             fflush(stderr);
             break;
        case FILE_PRINT:
             fprintf(adapter_info->fprint,"%s",error_string);
             fflush(adapter_info->fprint);
             break;
        default:
             break;
     }
#endif
}

#ifndef SIXDG
/*
 * NAME: get_scsi_id
 *
 * FUNCTION: this functions gets nvram location for scsi_id in the
 *      specified slot if it is different from the one passed.
 *
 * INPUTS: slotno,
 *
 * RETURNS: 0 if success else >0 Error code.
 *
 */
uint get_scsi_id(adapter_struc_t *adapter_info) {

MACH_DD_IO      mdd;
int             fd;
char            sid=7;
char            msg[100];

   sprintf(msg,"scsi id = %d\n", sid);
   print_error(adapter_info,msg);
   return (sid); 
   if((fd = open("/dev/nvram", O_RDWR, 0)) == -1) return E_OPEN;

   sprintf(msg,"bus_num = %d\n", adapter_info->diagex_dds->bus_id);
   print_error(adapter_info,msg);

   /* Decrement slot number found in database */
   mdd.md_addr = SCSI_BASE_ADDR + adapter_info->diagex_dds->slot_num;
   mdd.md_data = &sid;
   mdd.md_size = 1;
   mdd.md_incr = MV_BYTE;
   /* get the byte in NVRAM corresponding to the given slot */
   if (ioctl(fd,MIONVGET,&mdd) != -1) {
      /* of the byte based on the given bus_num                    */
      /* bus0 : id is in lower nibble */
      /* bus1 : id is in upper nibble */
      if ((BID_NUM(adapter_info->diagex_dds->bus_id)) == 1)  sid >>=4;
      sid &= 0x0f;
      close(fd);
      sprintf(msg,"scsi id = %d\n", sid);
      print_error(adapter_info,msg);
      return (sid); 
   } else {
      close(fd);
      return (E_DEVACCESS); /* ioctl failed */
   }
}
#endif

#ifndef SIXDG
/*
 * NAME: getadapter_attr
 *
 * FUNCTION: Builds the DDS (Defined Data Structure) for the
 *           Lace Adapter
 *
 * RETURNS: 0 - success
 *         >0 - failure
 *
 */
uint getadapter_attr(char *adapter_name,adapter_struc_t *adapter_info) {

   uint     rc;                     /* used for return codes */
   char    sstring[512];           /* working string                   */
   char    *lname;
   struct  PdAt  pdatobj;
   char    *ut;                    /* pointer to device's uniquetype */
   char    *pt;                    /* pointer to parent's uniquetype */

   struct Class *cusdev;           /* customized devices class ptr */
   struct CuDv cusobj;             /* customized device object storage */
   struct CuDv busobj;             /* customized device object storage */
   struct CuDv parobj;             /* customized device object storage */
   diagex_dds_t *dds;

   int lock_id;

   /*****                                                          */
   /***** Validate Parameters                                      */
   /*****                                                          */
   /* logical name must be specified */
   if (adapter_name == NULL) {
      print_error(adapter_info,"cfgad: logical name must be specified\n");
      return(BLOOM_ANAME_E);
   }

   /* start up odm */
   if (odm_initialize() == -1) {
      /* initialization failed */
      print_error(adapter_info,"cfgad: odm_initialize() failed\n");
      return(BLOOM_ODMINIT_E);
   }

   /* lock the database */
   if ((lock_id = odm_lock("/etc/objrepos/config_lock",0)) == -1) {
      print_error(adapter_info,"cfgad: odm_lock() failed\n");
      return(err_exit(BLOOM_ODMLOCK_E));
   }

   /* open customized devices object class */
   if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1) {
      print_error(adapter_info,"cfgad: open class CuDv failed\n");
      return(err_exit(BLOOM_ODMOPEN_E));
   }
   /* search for customized object with this logical name */
   sprintf(sstring, "name = '%s'", adapter_name);
   rc = (int)odm_get_first(cusdev,sstring,&cusobj);
   if (rc==0) {
      /* No CuDv object with this name */
      sprintf(sstring,"cfgad failed to find obj for %s\n",adapter_name);
      print_error(adapter_info,sstring);
      return(err_exit(BLOOM_ODMCUDV_E));
   } else if (rc==-1) {
     /* ODM failure */
     print_error(adapter_info,"cfgad: ODM failure getting CuDv object");
     return(err_exit(BLOOM_ODMGET_E));
   }

   sprintf(sstring, "name = '%s'", cusobj.parent);
   rc = (int)odm_get_first(cusdev,sstring,&parobj);
   if (rc==0) {
      /* Parent device not in CuDv */
      sprintf(sstring,"no parent in CuDv, %s\n",cusobj.parent);
      print_error(adapter_info,sstring);
      return(err_exit(BLOOM_PARENT_E));
   } else if (rc==-1) {
       /* ODM failure */
      sprintf(sstring,"failed getting  parent in CuDv, %s\n",cusobj.parent);
      print_error(adapter_info,sstring);
     return(err_exit(BLOOM_ODMCUDV_E));
    }
    /* parent must be available to continue */
    if (parobj.status != AVAILABLE) {
       print_error(adapter_info,"cfgad: parent is not AVAILABLE");
       return(err_exit(BLOOM_PARENT_E));
    }

   adapter_info->diagex_dds = (diagex_dds_t *) malloc( sizeof(diagex_dds_t));
   if (adapter_info->diagex_dds == NULL) {
      print_error(adapter_info,"adapter_info could not be allocated\n");
      return(BLOOM_MALLOC_E);       /* report allocation error */
   }
   /* Clear the dds  */
   memset( adapter_info->diagex_dds, 0, sizeof(diagex_dds_t) );

   strcpy(adapter_info->diagex_dds->parent_name,cusobj.parent);
   strcpy(adapter_info->diagex_dds->device_name,cusobj.name);

   if((int)(odm_open_class(CuAt_CLASS)) == -1){
       print_error(adapter_info,"bld_dds: can not open CuAt\n");
       return (BLOOM_ODMOPEN_E);
   }
   if((int)(odm_open_class(PdAt_CLASS)) == -1){
       print_error(adapter_info,"bld_dds: can not open PdAt\n");
       return (BLOOM_ODMOPEN_E);
   }
   lname=adapter_info->diagex_dds->device_name;
   dds=adapter_info->diagex_dds;
   ut = cusobj.PdDvLn_Lvalue;
   pt = parobj.PdDvLn_Lvalue;

   if ((rc=Get_Parent_Bus(cusdev, lname, &busobj))>0) {
      sprintf(sstring,"Could not get parent rc=%x\n",rc);
      print_error(adapter_info,sstring);
      return (BLOOM_PARENT_E);
   }

   /* get bus attributes */
   if((rc=getatt(&dds->bus_id,'l',CuAt_CLASS,PdAt_CLASS,busobj.name,
       busobj.PdDvLn_Lvalue,"bus_id",NULL))>0) {
      sprintf(sstring,"Could not get busid rc=%x\n",rc);
      print_error(adapter_info,sstring);
      return (BLOOM_BUSID_E);
   }

   /* get device attributes */
   if((rc=getatt(&dds->bus_io_addr,'l',CuAt_CLASS,PdAt_CLASS,lname,ut,
       "bus_io_addr",NULL))>0) {
      sprintf(sstring,"Could not get busio_addr in cuat rc=%x\n",rc);
      print_error(adapter_info,sstring);
      return (BLOOM_IOAD_E);
   }
   sprintf( sstring, "uniquetype = %s AND attribute = bus_io_addr",ut);
   if ((rc = (int)odm_get_first(PdAt_CLASS, sstring, &pdatobj) )==0 ) {
      sprintf(sstring,"Could not get busio_addr length in pdat rc=%x\n",rc);
      print_error(adapter_info,sstring);
      return( BLOOM_ODMPDAT_E );
   }
   else if ( rc == -1 ) return( BLOOM_ODMGET_E );
   adapter_info->bus_io_addr=adapter_info->diagex_dds->bus_io_addr;
   dds->bus_io_length = (ulong)strtol(pdatobj.width,(char **) NULL,0);
   if((rc=getatt(&dds->bus_intr_lvl,'i',CuAt_CLASS,PdAt_CLASS,lname,ut,
       "bus_intr_lvl",NULL))>0){
      sprintf(sstring,"Could not get intr level in cuat rc=%x\n",rc);
      print_error(adapter_info,sstring);
      return (BLOOM_INTR_E);
   }
   if((rc=getatt(&dds->intr_priority,'i',CuAt_CLASS,PdAt_CLASS,lname,ut,
       "intr_priority",NULL))>0) {
      sprintf(sstring,"Could not get intr priority in cuat rc=%x\n",rc);
      print_error(adapter_info,sstring);
      return (BLOOM_INTRPRIO_E);
   }
   if((rc=getatt(&adapter_info->scsi_id,'i',CuAt_CLASS,PdAt_CLASS,lname,ut,
       "id",NULL))>0){ 
      sprintf(sstring,"Could not get scsi id in cuat rc=%x\n",rc);
      print_error(adapter_info,sstring);
      return (BLOOM_INTRPRIO_E);
   }
/*   if((rc=getatt(&dds->dma_lvl,'i',CuAt_CLASS,PdAt_CLASS,lname,ut,
       "dma_lvl",NULL))>0) {
      sprintf(sstring,"Could not get dma level in cuat rc=%x\n",rc);
      print_error(adapter_info,sstring);
      return (BLOOM_DMAL_E);
   }
   if((rc=getatt(&dds->dma_bus_mem,'l',CuAt_CLASS,PdAt_CLASS,lname,ut,
       "dma_bus_mem",NULL))>0) {
      sprintf(sstring,"Could not get Dma Bus Mem in cuat rc=%x\n",rc);
      print_error(adapter_info,sstring);
      return (BLOOM_DMABMEM_E);
   }
   sprintf( sstring, "uniquetype = %s AND attribute = dma_bus_mem",ut);
   if( (rc = (int)odm_get_first(PdAt_CLASS, sstring, &pdatobj) )==0 ) {
      sprintf(sstring,"Could not get dma bus mem length in pdat rc=%x\n",rc);
      print_error(adapter_info,sstring);
      return( BLOOM_ODMPDAT_E );
   }
   else if ( rc == -1 ) return( BLOOM_ODMGET_E );
   dds->dma_bus_length = (ulong)strtol(pdatobj.width,(char **) NULL,0);
*/
   dds->slot_num = atoi(cusobj.connwhere);
   dds->intr_flags =0; 

   /* un lock the database */
   if (odm_unlock(lock_id) == -1) {
      print_error(adapter_info,"cfgad: odm_unlock() failed\n");
      return(err_exit(BLOOM_ODMLOCK_E));
   }

   odm_close_class(CuAt_CLASS);
   odm_close_class(PdDv_CLASS);
   odm_close_class(PdAt_CLASS);

   /*--------------------------------------------*/
   /*- acquire the scsi ids                     -*/
   /*-  from nvram area                         -*/
   /*--------------------------------------------*/
   if ((adapter_info->scsi_id=get_scsi_id(adapter_info))<0) {
      sprintf(sstring,"get scsi id failed rc=%x\n",adapter_info->scsi_id);
      print_error(adapter_info,sstring);
      return(BLOOM_SCSIID_E);
   }
   return (0);
}
#endif

#ifndef SIXDG
/*
 * NAME: err_exit
 *
 * FUNCTION: Closes any open object classes and terminates ODM.  Used to
 *           back out on an error.
 *
 * NOTES:
 *
 *   err_exit( exitcode )
 *      exitcode = The error exit code.
 *
 * RETURNS:
 *               None
 */
err_exit(uint exitcode) {
   /* Close any open object class */
   odm_close_class(CuDv_CLASS);
   odm_close_class(PdDv_CLASS);
   odm_close_class(CuAt_CLASS);

   /* Terminate the ODM */
   odm_terminate();
   return(exitcode);
}
#endif

/******************************************************************************
*
* NAME:  pcicfg_rd
*
* FUNCTION:  Read pci config registers 
*
* INPUT PARAMETERS:     adapter_info
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED:
*
*
******************************************************************************/
uint pcicfg_rd(adapter_struc_t *adapter_info,int count,int offset,uint *data) {

#ifndef SIXDG
   MACH_DD_IO   mdd;
#endif
   uint          i,rc;
   char         msg[100];

#ifndef SIXDG
   /* build mdd record */
   mdd.md_size = count;
   mdd.md_incr = MV_WORD;
   mdd.md_addr = offset;
   mdd.md_sla  = adapter_info->diagex_dds->slot_num;
   mdd.md_data = (char *) data;
   if ((rc=ioctl(adapter_info->cfg_fd, MIOPCFGET, &mdd))< 0)  {
      sprintf(msg,"Error in reading pci cfg regs at %x rc%x\n",offset,rc);
      print_error(adapter_info,msg);
      return(BLOOM_PCFGRD_E);
   }
   for (i=0;i<count;i++)  /* bigindian to little indian */
       data[i]= ((data[i]>>24) | ((data[i]>> 8) & 0x0000ff00) | 
                 ((data[i]<< 8) & 0x00ff0000)| (data[i]<<24));
#endif
#ifdef SIXDG
   *(ulong *) data = pci_config_read_32(adapter_info->sixdg_slot,offset);
#endif
   return(0);

}

/******************************************************************************
*
* NAME:  pcicfg_wrc
*
* FUNCTION:  write pci config registers 
*
* INPUT PARAMETERS:     adapter_info
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED:
*
*
******************************************************************************/
uint pcicfg_wrc(adapter_struc_t *adapter_info,int count,int offset,uint *data,
               uint data_mask) {
#ifndef SIXDG
   MACH_DD_IO   mdd;
#endif
   uint         i,rc;
   char         msg[100];
   uint         *swap_data;

   swap_data = (unsigned int *)malloc(count*sizeof(uint));
#ifndef SIXDG
   for (i=0;i<count;i++)  /* bigindian to little indian */
       swap_data[i]= ((data[i]>>24) | ((data[i]>> 8) & 0x0000ff00) | 
                 ((data[i]<< 8) & 0x00ff0000)| (data[i]<<24));
   /* build mdd record */
   mdd.md_size = count;
   mdd.md_incr = MV_WORD;
   mdd.md_addr = offset;
   mdd.md_sla  = adapter_info->diagex_dds->slot_num;
   mdd.md_data = (char *) swap_data;
   if ((rc=ioctl(adapter_info->cfg_fd, MIOPCFPUT, &mdd))< 0)  {
      sprintf(msg,"Error in writing pci cfg regs at %x offset data %x rc%x\n",
              offset,*data,rc);
      print_error(adapter_info,msg);
      free(swap_data);
      return(BLOOM_PCFGWR_E);
   }
#endif
#ifdef SIXDG
   pci_config_write_32(adapter_info->sixdg_slot,offset,
                                      *(ulong *)data);
#endif
   if (rc=pcicfg_rd(adapter_info,count,offset,swap_data)) {
      free(swap_data);
      return(rc);
   }
   for (i=0;i<count;i++) {
       if ((data_mask&data[i]) != (data_mask&swap_data[i])) {
          sprintf(msg,"Miscompare cfg regs at %x offset %x data %x rddata %x\n",
                  offset,i,data[i],swap_data[i]);
          print_error(adapter_info,msg);
          free(swap_data);
          return(BLOOM_CFGMIS_E);
       }
   }
   free(swap_data);
   return(0);
}

#ifndef SIXDG
/******************************************************************************
*
* NAME:  adapter_close
*
* FUNCTION:  Initialize an adapter to be used with diag_ex and
*            verify its usability.
*
* INPUT PARAMETERS:     adapter_name = device name of adapter.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED: cfgad, getad, diag_open, post_error,
*                             initialize_adapter, malloc.
*
******************************************************************************/
uint adapter_close(adapter_struc_t *adapter_info) {
    struct cfg_load cfg_ld;
    uint rc;
    char   msg[256];

    if ((rc=diag_close(adapter_info->handle) != DGX_OK)) {
        sprintf(msg,"Diag close failed %x\n",rc);
        print_error(adapter_info,msg);
        rc=BLOOM_DCLOSE_E;
        return(rc);
    }

    /***************************************************
      Unload kernel extentions interrupt handler.
     ***************************************************/
    cfg_ld.path = adapter_info->interrupt_routine;
    cfg_ld.libpath = NULL;
    if (sysconfig(SYS_QUERYLOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld))) {
        print_error(adapter_info,"SYSCONFIG QUERYLOAD failed");
        rc=BLOOM_INTR_QLOAD_E;
        return(rc);
    }
    if (!cfg_ld.kmid) {
       print_error(adapter_info,"Interrupt is not loaded\n");
       rc=BLOOM_INTH_E;
       return(rc);
    }
    if (sysconfig(SYS_KULOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld))) {
       print_error(adapter_info,"SYSCONFIG KULOAD failed");
       rc=BLOOM_INTR_ULOAD_E;
       return(rc);
    }

    if (adapter_info->print== FILE_PRINT)
       fclose(adapter_info->fprint); /* close debug file */
    close(adapter_info->cfg_fd); /* close /dev/busx handle */
    if ((rc=diagex_initial_state(adapter_info->diagex_dds->device_name))) 
       return(rc);
    else if (rc) return((0xF0000000 & BLOOM_RESDEV_E) | rc );
    free(adapter_info);
    return (rc);
}
#endif

#ifndef SIXDG
/******************************************************************************
*
* NAME:  adapter_open
*
* FUNCTION:  Initialize an adapter to be used with diag_ex and
*            verify its usability.
*
* INPUT PARAMETERS:     adapter_name = device name of adapter.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED: cfgad, getad, diag_open, post_error,
*                             initialize_adapter, malloc.
*
******************************************************************************/
uint adapter_open(char *adapter_name,adapter_struc_t **padapter_info){
    adapter_struc_t *adapter_info;
    struct  cfg_load cfg_ld;
    uint     rc, i;
    char    msg_string[100];
    uint    id;
    char    *interrupt_routine;

    /* allocate space for adapter configuration */
    adapter_info = (adapter_struc_t *) malloc(sizeof(adapter_struc_t));
    if (adapter_info == NULL ) {
       printf("No storage for adapter data structure\n");
       return(BLOOM_MALLOC_E);
    }
    /* Retrive environment settings for printing */
    adapter_info->print=NO_PRINT;
    if (getenv("BLOOM_SCREEN") != NULL) adapter_info->print=SCREEN_PRINT;
    if (getenv("BLOOM_FILE") != NULL) adapter_info->print=FILE_PRINT;
    if (adapter_info->print == FILE_PRINT) {
       if ((adapter_info->fprint=fopen("/tmp/.BLOOM_DEBUG","w"))== NULL) {
          adapter_info->print=SCREEN_PRINT;
          sprintf(msg_string, "Unable to create open file errno=%d\n", errno);
          print_error(adapter_info, msg_string);
          return(BLOOM_FOPEN_E);
       }
    }
    if ((rc= getadapter_attr(adapter_name,adapter_info))!=0) {
         sprintf(msg_string, "Unable to get ODM info: %d\n", rc);
         print_error(adapter_info, msg_string);
         return(rc);
    }

    /* unload the children and the adapter device drivers */
    if ((rc=diagex_cfg_state(adapter_info->diagex_dds->device_name))==-1) 
       return(rc);
    else if (rc) return((0xF0000000 & BLOOM_CHILD_E) | rc );

    /*passing data to intr*/
    adapter_info->diagex_dds->data_ptr=(char *)adapter_info;
    adapter_info->diagex_dds->d_count=sizeof(adapter_struc_t);

    dump_dds(adapter_info,sizeof(adapter_struc_t));
    dump_dds(adapter_info->diagex_dds,sizeof(diagex_dds_t));

    /***************************************************
      Load intrrupt routine kernel extentions if not already loaded.
      This code then passes the DDS to the kernel extension.
     ***************************************************/

    if ((interrupt_routine=(char *)getenv("DIAGX_SLIH_DIR")) == NULL) {
        print_error(adapter_info,"Can't find interrupt routine directory\n");
        return(BLOOM_INTR_DIR_E);
        
    }
    strcpy(adapter_info->interrupt_routine,interrupt_routine);
    strcat(adapter_info->interrupt_routine,"/bloom_intr");
    cfg_ld.path = adapter_info->interrupt_routine;
    cfg_ld.libpath = NULL;
    if (rc=sysconfig(SYS_KLOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld))) {
        sprintf(msg_string, "Unable to load intr routine rc %x errno %x\n", rc,
                errno);
        print_error(adapter_info, "Unable to load interrupt routine\n");
        return(BLOOM_INTR_LOAD_E);
    }

    adapter_info->diagex_dds->bus_type = BUS_BID;
    adapter_info->diagex_dds->kmid = (int )cfg_ld.kmid;

    if ((rc=diag_open(adapter_info->diagex_dds,&adapter_info->handle))!=DGX_OK){
       sprintf(msg_string,"diag_open failed rc=%x\n",rc);
       print_error(adapter_info, msg_string);
       adapter_close(adapter_info);
       return(BLOOM_DOPEN_E);
    }
    sprintf(msg_string,"diag handle: %x\n",adapter_info->handle);
    print_error(adapter_info,msg_string);
    *padapter_info=adapter_info;
    sprintf(msg_string,"/dev/%s",adapter_info->diagex_dds->parent_name);
    if ((adapter_info->cfg_fd=open(msg_string,O_RDWR))==-1)
       return (BLOOM_CFGOPEN_E);
    if ((rc=pcicfg_rd(adapter_info,DEV_VENDOR_ID_SIZE,DEV_VENDOR_ID_OFFS,&id)))
        return(rc);
    switch (id) {
        case P810_SIGNATURE:
            break;
        case P820_SIGNATURE:
            break;
        case P825_SIGNATURE:
            break;
        default:
            sprintf(msg_string,"Invalid device vendor id %x\n",id);
            print_error(adapter_info,msg_string);
            return(BLOOM_DEVID_E);
    }

    return (0);
}
#endif

#ifndef SIXDG
/******************************************************************************
*
* NAME:  read_cfg_io
*
* FUNCTION:  Initialize an adapter to be used with diag_ex and
*            verify its usability.
*
* INPUT PARAMETERS:     adapter_info
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED:
*
*
******************************************************************************/
uint read_cfg_io(adapter_struc_t *adapter_info) {
   uint         rc;
   uint        config[256/4];

   if (rc=pcicfg_rd(adapter_info,256/4,0,config)) return(rc);
   dump_dds(config,sizeof(config));
   return 0;
}
#endif

/******************************************************************************
*
* NAME:  test_cfg_reg
*
* FUNCTION:  
*
* INPUT PARAMETERS:     adapter_info
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED:
*
******************************************************************************/
uint test_cfg_reg(adapter_struc_t *adapter_info) {
   uint         rc;
   uint        data;

   data=STATUS_COMMAND;
   if (rc=pcicfg_wrc(adapter_info,STATUS_COMMAND_SIZE,STATUS_COMMAND_OFF,&data,
                    NO_MASK))
      return(rc);

   data=FIVEFIVE;
   if (rc=pcicfg_wrc(adapter_info,BASE_ADDR_SIZE,BASE_IOADDR_OFFSET,&data,
                    BASE_ADDR_MASK)) 
      return(rc);

   data=ABLEABLE;
   if (rc=pcicfg_wrc(adapter_info,BASE_ADDR_SIZE,BASE_IOADDR_OFFSET,&data,
                    BASE_ADDR_MASK)) 
      return(rc);

   data=RUNNINGD;
   if (rc=pcicfg_wrc(adapter_info,BASE_ADDR_SIZE,BASE_IOADDR_OFFSET,&data,
                    BASE_ADDR_MASK)) 
      return(rc);

   data=FIVEFIVE;
   if (rc=pcicfg_wrc(adapter_info,BASE_ADDR_SIZE,BASE_MEMADDR_OFFSET,&data,
                    BASE_ADDR_MASK)) 
      return(rc);

   data=ABLEABLE;
   if (rc=pcicfg_wrc(adapter_info,BASE_ADDR_SIZE,BASE_MEMADDR_OFFSET,&data,
                    BASE_ADDR_MASK)) 
      return(rc);

   data=RUNNINGD;
   if (rc=pcicfg_wrc(adapter_info,BASE_ADDR_SIZE,BASE_MEMADDR_OFFSET,&data,
                    BASE_ADDR_MASK)) 
      return(rc);

   /* set io address to be the right stuff from odm */
   data=adapter_info->bus_io_addr;
   if (rc=pcicfg_wrc(adapter_info,BASE_ADDR_SIZE,BASE_IOADDR_OFFSET,&data,
                    BASE_ADDR_MASK)) 
      return(rc);
   return 0;
}

/******************************************************************************
*
* NAME:  read_mem_io
*
* FUNCTION:  This flips the bytes so 
*
* INPUT PARAMETERS:     adapter_info
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED:
*
******************************************************************************/
uint read_mem_io(adapter_struc_t *adapter_info,int count,int offset,
                 uchar *data) {
   uint        rc;
   int         i;
   char        msg[256];
#ifdef SIXDG
   int         k=0;
#endif

   for (count--;count >=0;count--,offset++) {
#ifndef SIXDG
       if ((rc=diag_io_read(adapter_info->handle,IOCHAR,offset,&data[count],
          NULL ,PROCLEV) != DGX_OK)) {
          sprintf(msg,"memio read error at %x rc %x\n",offset,rc);
          print_error(adapter_info,msg);
          return(BLOOM_IOREGS_E);
       }
#endif
#ifdef SIXDG
      data[k]=in8(adapter_info->bus_io_addr+offset);
      k++;
#endif
   }
   return 0;
}

/******************************************************************************
*
* NAME:  write_mem_io
*
* FUNCTION:  flips the byetes
*
* INPUT PARAMETERS:     adapter_info
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED:
*
******************************************************************************/
uint write_mem_io(adapter_struc_t *adapter_info,int count,int offset,
                  uchar *data) {
   uint        j,rc;
   int         i;
   char        msg[256];
#ifdef SIXDG
   int         k=0;
#endif
   j=offset;
   for (i=count-1;i>=0;i--,j++){
#ifndef SIXDG
       if ((rc=diag_io_write(adapter_info->handle,IOCHAR,j,data[i],
                             NULL,PROCLEV) != DGX_OK)) {
          sprintf(msg,"memio write error at %x rc %x\n",j,rc);
          print_error(adapter_info,msg);
          return(BLOOM_IOREGS_E);
       }
#endif
#ifdef SIXDG
       out8(adapter_info->bus_io_addr+j,data[k]);
       k++;
#endif
   }
   return 0;
}

/******************************************************************************
*
* NAME:  writec_mem_io
*
* FUNCTION:  flips the byetes
*
* INPUT PARAMETERS:     adapter_info
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED:
*
******************************************************************************/
uint writec_mem_io(adapter_struc_t *adapter_info,int count,int offset,
                  uchar *data) {
   uint        j,rc;
   int         i;
   char        msg[256];
   char        *rddata;

   if(rc=write_mem_io(adapter_info,count,offset,data)) return(rc);
   rddata = (unsigned char *)malloc(count*sizeof(uchar));
   if((rc=read_mem_io(adapter_info,count,offset,rddata))) {
      free(rddata);
      return(rc);
   }
   for (i=0;i<count;i++) {
       if ((data[i] != rddata[i])) {
          sprintf(msg,"miscompare memio regs at %x offset %x data %x read %x\n",
                  offset,i,data[i],rddata[i]);
          print_error(adapter_info,msg);
          free(rddata);
          return(BLOOM_MEMIOMIS_E);
       }
   }
   free(rddata);
   return 0;
}

#ifndef SIXDG
/******************************************************************************
*
* NAME:  sediff
*
* FUNCTION:  Check for the adapter being single ended or differential 
*
* INPUT PARAMETERS:     adapter_info
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns se or diff 
*
* EXTERNAL PROCEDURES CALLED:
*
*
******************************************************************************/
uint sediff(adapter_struc_t *adapter_info) {
   uint         rc;
   uchar        ucdata;

   ucdata=0x0F;
   if(rc=write_mem_io(adapter_info,1,GPCNTL,&ucdata)) return(rc);
   if((rc=read_mem_io(adapter_info,1,GPREG,&ucdata))) return(rc);
   /* Single Ended  Adapter */
   if (ucdata&0x08) 
      return(BLOOM_DEVICE_SE);
   return (BLOOM_DEVICE_DIFF);
}
#endif

/******************************************************************************
*
* NAME:  test_memio_reg
*
* FUNCTION:  
*
* INPUT PARAMETERS:     adapter_info
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED:
*
******************************************************************************/
uint test_memio_reg(adapter_struc_t *adapter_info) {
   uint        rc;
   uint        uidata;
   uchar       ucdata;

   ucdata=0x40; /* reset the chip */
   if(rc=writec_mem_io(adapter_info,1,ISTAT,&ucdata)) return(rc);
   ucdata=0x00; /* unreset the chip */
   if(rc=writec_mem_io(adapter_info,1,ISTAT,&ucdata)) return(rc);
   ucdata=0xCA; /* check full arbitration bits and assert on parity bit */
   if(rc=writec_mem_io(adapter_info,1,SCNTL0,&ucdata)) return(rc);
   ucdata=0x11; /* check simple arbitration and target mode */
   if(rc=writec_mem_io(adapter_info,1,SCNTL0,&ucdata)) return(rc);
   ucdata=0x00;
   if(rc=writec_mem_io(adapter_info,1,SCNTL0,&ucdata))return(rc);
   ucdata=0xA0; /* extra clock disable halt on parity */
   if(rc=writec_mem_io(adapter_info,1,SCNTL1,&ucdata)) return(rc);
   ucdata=0x04; /* assert even scsi parity */
   if(rc=writec_mem_io(adapter_info,1,SCNTL1,&ucdata)) return(rc);
   ucdata=0x00;
   if(rc=writec_mem_io(adapter_info,1,SCNTL1,&ucdata)) return(rc);
   ucdata=0x2A; /* 20mHz enable wide divide by factor 1.5 */
   if(rc=writec_mem_io(adapter_info,1,SCNTL3,&ucdata)) return(rc);
   ucdata=0x55; /* divide by 3 */
   if(rc=writec_mem_io(adapter_info,1,SCNTL3,&ucdata)) return(rc);
   ucdata=0x00; 
   if(rc=writec_mem_io(adapter_info,1,SCNTL3,&ucdata)) return(rc);
   ucdata=0x0A; /* scsi destination id */
   if(rc=writec_mem_io(adapter_info,1,SDID,&ucdata)) return(rc);
   ucdata=0x05; /* scsi destination id */
   if(rc=writec_mem_io(adapter_info,1,SDID,&ucdata)) return(rc);
   ucdata=0x00;
   if(rc=writec_mem_io(adapter_info,1,SDID,&ucdata)) return(rc);
   ucdata=0xAA; /* scsi interrupt enable 0 */
   if(rc=writec_mem_io(adapter_info,1,SIEN0,&ucdata)) return(rc);
   ucdata=0x55; /* scsi interrupt enable 0 */
   if(rc=writec_mem_io(adapter_info,1,SIEN0,&ucdata)) return(rc);
   ucdata=0x00;
   if(rc=writec_mem_io(adapter_info,1,SIEN0,&ucdata)) return(rc);
   ucdata=0x02; /* scsi interrupt enable 1 */
   if(rc=writec_mem_io(adapter_info,1,SIEN1,&ucdata)) return(rc);
   ucdata=0x05; /* scsi interrupt enable 1 */
   if(rc=writec_mem_io(adapter_info,1,SIEN1,&ucdata)) return(rc);
   ucdata=0x00;
   if(rc=writec_mem_io(adapter_info,1,SIEN1,&ucdata)) return(rc);
   ucdata=0x2A; /* scsi chip id */
   if(rc=writec_mem_io(adapter_info,1,SCID,&ucdata)) return(rc);
   ucdata=0x45; /* scsi chip id */
   if(rc=writec_mem_io(adapter_info,1,SCID,&ucdata)) return(rc);
   ucdata=0x00;
   if(rc=writec_mem_io(adapter_info,1,SCID,&ucdata)) return(rc);
   ucdata=0xAA; /* scsi transfer register */
   if(rc=writec_mem_io(adapter_info,1,SXFER,&ucdata)) return(rc);
   ucdata=0x45; /* scsi transfer register */
   if(rc=writec_mem_io(adapter_info,1,SXFER,&ucdata)) return(rc);
   ucdata=0x00;
   if(rc=writec_mem_io(adapter_info,1,SXFER,&ucdata)) return(rc);
   ucdata=0xAA; /* dma command register */
   if(rc=writec_mem_io(adapter_info,1,DCMD,&ucdata)) return(rc);
   ucdata=0x55; /* dma command register */
   if(rc=writec_mem_io(adapter_info,1,DCMD,&ucdata)) return(rc);
   ucdata=0x00;
   if(rc=writec_mem_io(adapter_info,1,DCMD,&ucdata)) return(rc);
   uidata=0xAA55AA55; /* next dma address for command */
   if(rc=writec_mem_io(adapter_info,4,DNAD,(char *)&uidata)) return(rc);
   uidata=0xAA55FF00; /* next dma address for command */
   if(rc=writec_mem_io(adapter_info,4,DNAD,(char *)&uidata)) return(rc); 
   uidata=0x55AA55AA; /* next dma address for command */
   if(rc=writec_mem_io(adapter_info,4,DNAD,(char *)&uidata)) return(rc);
   uidata=0x00000000;
   if(rc=writec_mem_io(adapter_info,4,DNAD,(char *)&uidata)) return(rc);
    /* set MAN bit in DMODE, to allow us to test DSP w/o causing fetches */
   ucdata=0x01;
   if(rc=writec_mem_io(adapter_info,1,DMODE,&ucdata)) return(rc);
   uidata=0xAA55AA55; /* dma scripts pointer */
   if(rc=writec_mem_io(adapter_info,4,DSP,(char *)&uidata)) return(rc);
   uidata=0xAA55FF00; /* dma scripts pointer */
   if(rc=writec_mem_io(adapter_info,4,DSP,(char *)&uidata)) return(rc); 
   uidata=0x55AA55AA; /* dma scripts pointer */
   if(rc=writec_mem_io(adapter_info,4,DSP,(char *)&uidata)) return(rc);
   uidata=0x00000000;
   if(rc=writec_mem_io(adapter_info,4,DSP,(char *)&uidata)) return(rc);
   uidata=0xAA55AA55; /* dma scripts pointer save */
   if(rc=writec_mem_io(adapter_info,4,DSPS,(char *)&uidata)) return(rc);
   uidata=0xAA55FF00; /* dma scripts pointer save */
   if(rc=writec_mem_io(adapter_info,4,DSPS,(char *)&uidata)) return(rc); 
   uidata=0x55AA55AA; /* dma scripts pointer save */
   if(rc=writec_mem_io(adapter_info,4,DSPS,(char *)&uidata)) return(rc);
   uidata=0x00000000;
   if(rc=writec_mem_io(adapter_info,4,DSPS,(char *)&uidata)) return(rc);
   ucdata=0xAA; /* test dma mode register bits */ 
   if(rc=writec_mem_io(adapter_info,1,DMODE,&ucdata)) return(rc);
   ucdata=0x00; /* test dma mode register bits */
   if(rc=writec_mem_io(adapter_info,1,DMODE,&ucdata)) return(rc);
   ucdata=0x29; /* test Dma interrupt enable bits */
   if(rc=writec_mem_io(adapter_info,1,DIEN,&ucdata)) return(rc);
   ucdata=0x55; /* test Dma interrupt enable bits */
   if(rc=writec_mem_io(adapter_info,1,DIEN,&ucdata)) return(rc);
   ucdata=0x11; /* test dma control bits */
   if(rc=writec_mem_io(adapter_info,1,DCNTL,&ucdata)) return(rc); 
   ucdata=0x00;
   if(rc=writec_mem_io(adapter_info,1,DCNTL,&ucdata)) return(rc);
   return 0;
}

/******************************************************************************
*
* NAME:  test_dma_fifo
*
* FUNCTION:  
*
* INPUT PARAMETERS:     adapter_info
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED:
*
******************************************************************************/
uint test_dma_fifo(adapter_struc_t *adapter_info) {
  uchar       ucdata;
  uchar       chkdata;
  int 	      i,j;
  uint        rc;
  char        msg[256];

  ucdata=0x40; /* reset the chip */
  if(rc=writec_mem_io(adapter_info,1,ISTAT,&ucdata)) return(rc);
  ucdata=0x00; /* unreset the chip */
  if(rc=writec_mem_io(adapter_info,1,ISTAT,&ucdata)) return(rc);
  /* The proceeding section of code writes the entire chip DMA */
  /* FIFO and then performs a readback check.                  */
  if((rc=read_mem_io(adapter_info,1,CTEST3,&ucdata))) return(rc);
  ucdata |= CLEAR_DMA_FIFO; /* clear the dma fifo */
  if(rc=write_mem_io(adapter_info,1,CTEST3,&ucdata)) return(rc);
  /* check to make sure that fifo is empty */
  if((rc=read_mem_io(adapter_info,1,DSTAT,&ucdata))) return(rc);
  if (!(ucdata&0x80)) {
     sprintf(msg,"Fifo empty bit is not set for dstat %x\n",ucdata);
     print_error(adapter_info,msg);
     return (BLOOM_FIFOEC_E);
  }
  if((rc=read_mem_io(adapter_info,1,CTEST1,&ucdata))) return(rc);
  if ((ucdata&0x0F)) { /* check fifo  full bits */
     sprintf(msg,"Fifo full bits are set for dstat %x\n",ucdata);
     print_error(adapter_info,msg);
     return (BLOOM_FIFOFS_E);
  }
  for (i= CTEST4_DATA; i < CTEST4_DEND; i++) {
      ucdata =i;
      if(rc=writec_mem_io(adapter_info,1,CTEST4,&ucdata)) return(rc);
      for (j = 0; j < DMA_FIFO_SIZE; j++) {
          ucdata=(j+1)*(i+1)*2;
          if(rc=write_mem_io(adapter_info,1,CTEST6,&ucdata)) return(rc);
      }
  }
  /* Read DSTAT reg to check DMA FIFO Empty bit is not set. */
  if ((rc=read_mem_io(adapter_info,1,DSTAT,&ucdata))) return(rc);
  if (ucdata&0x80) { /* check fifo empty bit */
     sprintf(msg,"Fifo empty bit is still set for dstat %x\n",ucdata);
     print_error(adapter_info,msg);
     return (BLOOM_FIFOES_E);
  }
  if((rc=read_mem_io(adapter_info,1,CTEST1,&ucdata))) return(rc);
  if ((ucdata&0x0F)!=0x0F) { /* check fifo  full bits */
     sprintf(msg,"Fifo full bits are not set for dstat %x\n",ucdata);
     print_error(adapter_info,msg);
     return (BLOOM_FIFOFC_E);
  }

  /* read the data back and verify it */
  for (i= CTEST4_DATA; i < CTEST4_DEND; i++) {
      ucdata =i;
      if(rc=writec_mem_io(adapter_info,1,CTEST4,&ucdata)) return(rc);
      for (j=0;j<DMA_FIFO_SIZE;j++) {
          if((rc=read_mem_io(adapter_info,1,CTEST6,&ucdata))) return(rc);
          chkdata=(j+1)*(i+1)*2;
          if (ucdata!=chkdata) {
             sprintf(msg,"Fifo data miscompare %x with %x\n",ucdata,chkdata);
             print_error(adapter_info,msg);
             return (BLOOM_FIFOMIS_E);
          }
      }
  }

  if((rc=read_mem_io(adapter_info,1,CTEST3,&ucdata))) return(rc);
  ucdata |= CLEAR_DMA_FIFO; /* clear the dma fifo */
  if(rc=write_mem_io(adapter_info,1,CTEST3,&ucdata)) return(rc);
  return (0);
}  

/******************************************************************************
*
* NAME:  test_scsi_fifo
*
* FUNCTION:  
*
* INPUT PARAMETERS:     adapter_info
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED:
*
******************************************************************************/
uint test_scsi_fifo(adapter_struc_t *adapter_info) {
  uchar       ucdata;
  int 	      i,j;
  uint        rc;
  char        msg[256];

  ucdata=0x40; /* reset the chip */
  if(rc=writec_mem_io(adapter_info,1,ISTAT,&ucdata)) return(rc);
  ucdata=0x00; /* unreset the chip */
  if(rc=writec_mem_io(adapter_info,1,ISTAT,&ucdata)) return(rc);
  /* The following section of code writes the entire chip SCSI  */
  /* FIFO and then performs a readback check.                   */
  /* start check for even parity logic */
  ucdata = 0x04;
  if(rc=writec_mem_io(adapter_info,1,SCNTL1,&ucdata)) return(rc);
  /* set STW in STEST3 so we can write to the SCSI FIFO */
  ucdata = 0x01; /* clear scsi fifo */
  if(rc=writec_mem_io(adapter_info,1,STEST3,&ucdata)) return(rc);
  for (i = 0; i < SCSI_FIFO_SIZE; i++) {
      if((rc=read_mem_io(adapter_info,1,SSTAT1,&ucdata))) return(rc);
      if ((ucdata&0xF0) != (i<<4)) {
         sprintf(msg,"scsi Fifo no of bytes error %x\n",ucdata);
         print_error(adapter_info,msg);
         return (BLOOM_SCSIFIFO_FLAGS_E);
      }
      ucdata = 0xAA; 
      if(rc=write_mem_io(adapter_info,1,SODL1,&ucdata)) return(rc);
      ucdata = 0xAA; 
      if(rc=write_mem_io(adapter_info,1,SODL0,&ucdata)) return(rc);
  }
  /* set STR in STEST3 so we can read the SCSI FIFO */
  ucdata = 0x40; 
  if(rc=write_mem_io(adapter_info,1,STEST3,&ucdata)) return(rc);
  for (i = 0; i < SCSI_FIFO_SIZE; i++) {
      if((rc=read_mem_io(adapter_info,1,SSTAT1,&ucdata))) return(rc);
      if ((ucdata&0xF0) != ((SCSI_FIFO_SIZE-i)<<4)) {
         sprintf(msg,"scsi Fifo no of bytes error %x\n",ucdata);
         print_error(adapter_info,msg);
         return (BLOOM_SCSIFIFO_FLAGS_E);
      }
      if((rc=read_mem_io(adapter_info,1,SODL1,&ucdata))) return(rc);
      if (ucdata!=0xAA) {
         sprintf(msg,"scsi Fifo data miscompare %x\n",ucdata);
         print_error(adapter_info,msg);
         return (BLOOM_SCSIFIFO_MIS_E);
      }
      if((rc=read_mem_io(adapter_info,1,SODL0,&ucdata))) return(rc);
      if (ucdata!=0xAA) {
         sprintf(msg,"scsi Fifo data miscompare %x\n",ucdata);
         print_error(adapter_info,msg);
         return (BLOOM_SCSIFIFO_MIS_E);
      }
  }   /* end of loop */

  /* Now check the odd parity logic */
  ucdata = 0x00;
  if(rc=writec_mem_io(adapter_info,1,SCNTL1,&ucdata)) return(rc);
  /* set STW in STEST3 so we can write to the SCSI FIFO */
  ucdata = 0x01; /* clear scsi fifo */
  if(rc=writec_mem_io(adapter_info,1,STEST3,&ucdata)) return(rc);
  for (i = 0; i < SCSI_FIFO_SIZE; i++) {
      ucdata = 0x55; 
      if(rc=write_mem_io(adapter_info,1,SODL1,&ucdata)) return(rc);
      ucdata = 0x55; 
      if(rc=write_mem_io(adapter_info,1,SODL0,&ucdata)) return(rc);
  }
  /* set STR in STEST3 so we can read the SCSI FIFO */
  ucdata = 0x40; 
  if(rc=write_mem_io(adapter_info,1,STEST3,&ucdata)) return(rc);
  for (i = 0; i < SCSI_FIFO_SIZE; i++) {
      if((rc=read_mem_io(adapter_info,1,SODL1,&ucdata))) return(rc);
      if (ucdata!=0x55) {
         sprintf(msg,"scsi Fifo data miscompare %x\n",ucdata);
         print_error(adapter_info,msg);
         return (BLOOM_SCSIFIFO_MIS_E);
      }
      if((rc=read_mem_io(adapter_info,1,SODL0,&ucdata))) return(rc);
      if (ucdata!=0x55) {
         sprintf(msg,"scsi Fifo data miscompare %x\n",ucdata);
         print_error(adapter_info,msg);
         return (BLOOM_SCSIFIFO_MIS_E);
      }
  }   /* end of loop */

  /* read SCSI FIFO one more time.  This should cause an error because */
  /* it should be empty at this point.                                 */
  if((rc=read_mem_io(adapter_info,1,SODL,&ucdata))) return(rc);
  /* check for SCSI Gross Error in SIST0 */
  if((rc=read_mem_io(adapter_info,1,SIST0,&ucdata))) return(rc);
  if ((ucdata&0x08)!=0x08) {
     sprintf(msg,"scsi fifo underflow not reported sist0 %x\n",ucdata);
     print_error(adapter_info,msg);
     return (BLOOM_SCSIFIFO_UNDF_E);
  }
  ucdata=0x40; /* reset the chip */
  if(rc=writec_mem_io(adapter_info,1,ISTAT,&ucdata)) return(rc);
  ucdata=0x00; /* unreset the chip */
  if(rc=writec_mem_io(adapter_info,1,ISTAT,&ucdata)) return(rc);
  ucdata = 0x19;  /* loopback mode  and low level mode */
  if(rc=writec_mem_io(adapter_info,1,STEST2,&ucdata)) return(rc);
  ucdata = 0x08;  /* master parity enable */
  if(rc=writec_mem_io(adapter_info,1,CTEST4,&ucdata)) return(rc);
  ucdata = 0x08;  /* enable parity checking */
  if(rc=writec_mem_io(adapter_info,1,SCNTL0,&ucdata)) return(rc);
  ucdata = 0x04;  /* assert even scsi parity */
  if(rc=writec_mem_io(adapter_info,1,SCNTL1,&ucdata)) return(rc);
  ucdata = 0x00; 
  if(rc=write_mem_io(adapter_info,1,SODL1,&ucdata)) return(rc);
  ucdata = 0x77; 
  if(rc=write_mem_io(adapter_info,1,SODL0,&ucdata)) return(rc);
  ucdata = 0x44;  /* puts data on the scsi bus */
  if(rc=writec_mem_io(adapter_info,1,SCNTL1,&ucdata)) return(rc);
  /* data needs to be read back for checking parity */
  if ((rc=read_mem_io(adapter_info,1,SBDL1,&ucdata))) return(rc);
  if ((rc=read_mem_io(adapter_info,1,SBDL0,&ucdata))) return(rc);
  if ((rc=read_mem_io(adapter_info,1,ISTAT,&ucdata))) return(rc);
  if((rc=read_mem_io(adapter_info,1,SIST0,&ucdata))) return(rc);
  if ((ucdata&0x01) != 0x01) {
     sprintf(msg,"scsi parity error not reported sist0 %x\n",ucdata);
     print_error(adapter_info,msg);
     return (BLOOM_SCSIFIFO_PAR_E);
  }
  /* this will clear the interrupt status */
  if((rc=read_mem_io(adapter_info,1,SIST1,&ucdata))) return(rc);
  if((rc=read_mem_io(adapter_info,1,SIST0,&ucdata))) return(rc);
  adapter_info->intr_rec=FALSE;
  if ((rc=read_mem_io(adapter_info,1,ISTAT,&ucdata))) return(rc);
  ucdata = 0x01;  /* enable parity interrupt */
  if(rc=write_mem_io(adapter_info,1,SIEN0,&ucdata)) return(rc);
  ucdata = 0x00; 
  if(rc=write_mem_io(adapter_info,1,SODL1,&ucdata)) return(rc);
  ucdata = 0x77; 
  if(rc=write_mem_io(adapter_info,1,SODL0,&ucdata)) return(rc);
  ucdata = 0x44;  /* puts data on the scsi bus */
  if(rc=writec_mem_io(adapter_info,1,SCNTL1,&ucdata)) return(rc);
  /* data needs to be read back for checking parity */
  if ((rc=read_mem_io(adapter_info,1,SBDL1,&ucdata))) return(rc);
  if ((rc=read_mem_io(adapter_info,1,SBDL0,&ucdata))) return(rc);
  for ( i=0;i<INTR_TIMEOUT;i++) {
#ifndef SIXDG
      usleep(100);
#endif
#ifdef SIXDG
      sleep(100,MICROSECONDS);
#endif
      if (adapter_info->intr_rec) break;
  }
  if (adapter_info->intr_rec) {
     if ((adapter_info->sist0&0x01) != 0x01) {
        sprintf(msg,"scsi parity error not reported sist0 %x\n",ucdata);
        print_error(adapter_info,msg);
        return (BLOOM_SCSIFIFO_PAR_E);
     }
  } else {
     sprintf(msg,"Interrupt not received for parity error\n");
     print_error(adapter_info,msg);
     return (BLOOM_INTR_TIMEOUT);
  }
  ucdata=0x40; /* reset the chip */
  if(rc=writec_mem_io(adapter_info,1,ISTAT,&ucdata)) return(rc);
  ucdata=0x00; /* unreset the chip */
  if(rc=writec_mem_io(adapter_info,1,ISTAT,&ucdata)) return(rc);
  return (0);
}  

/******************************************************************************
*
* NAME:  ptc_test
*
* FUNCTION:  
*
* INPUT PARAMETERS:     adapter_info
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED:
*
******************************************************************************/
uint ptc_test(adapter_struc_t *adapter_info) {
  uint rc;  
  uchar ucdata;
  uchar msg[256];

  ucdata=0x0F;
  if(rc=writec_mem_io(adapter_info,1,GPCNTL,&ucdata)) return(rc);
  if ((rc=read_mem_io(adapter_info,1,GPREG,&ucdata))) return(rc);
  if ((ucdata&0x01)== 0x00) { /* no term power */
     sprintf(msg,"no term power gpreg %x\n",ucdata);
     print_error(adapter_info,msg);
     return (BLOOM_TERMPOWER_E);
  }
  return (0);
} 

/******************************************************************************
*
* NAME:  adapter_diag
*
* FUNCTION:  
*
* INPUT PARAMETERS:     adapter_info
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED:
*
******************************************************************************/
uint adapter_diag(adapter_struc_t *adapter_info) {
   uint         rc;

   if (rc=test_cfg_reg(adapter_info)) return(rc);
   if (rc=test_memio_reg(adapter_info)) return(rc);
   if (rc=test_dma_fifo(adapter_info)) return(rc);
   if (rc=ptc_test(adapter_info)) return(rc);
   if (rc=test_scsi_fifo(adapter_info)) return(rc);
   return 0;
}

/**************************************************************************/
/*                                                                        */
/* NAME:  scsi_move_byte_out                                              */
/*                                                                        */
/* FUNCTION:  Moves 1 byte of data onto and off of the scsi bus, handling */
/*            REQ/ACK handshaking for an asynchronous data out phase.     */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine assumes SCE is already set, and scsi bus control  */
/*         lines are also set appropriately.                              */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    diagnostic structure - diagnostic information.                      */
/*                                                                        */
/* INPUTS:                                                                */
/*    adapter_info - pointer                                              */
/*    data - 1 byte of data to write and read from scsi bus               */
/*    clear_atn - flag to indicate whether to clear the ATN signal        */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  EFAULT for data miscompare, O otherwise     */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
uint scsi_move_byte_out(adapter_struc_t *adapter_info,ushort data,
                        uchar clear_atn) {
  uint  rc;
  uchar ucdata;
  char  msg[256];

  /* Assert REQ to indicate target wants to receive data */
  if ((rc=read_mem_io(adapter_info,1,SOCL,&ucdata))) return(rc);
  ucdata |= 0x80;
  if (rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);

  /* Initiator writes data onto bus */
  ucdata = data >> 8;
  if (rc=writec_mem_io(adapter_info,1,SODL1,&ucdata)) return(rc);
  ucdata = data &0xFF;
  if (rc=writec_mem_io(adapter_info,1,SODL0,&ucdata)) return(rc);
  if ((rc=read_mem_io(adapter_info,1,SCNTL1,&ucdata))) return(rc);
  ucdata |= 0x40;
  if (rc=writec_mem_io(adapter_info,1,SCNTL1,&ucdata)) return(rc);

  if (clear_atn) {
     /* Clear ATN to indicate target can change state after */
     /* receiving the data. */
     if ((rc=read_mem_io(adapter_info,1,SOCL,&ucdata))) return(rc);
     ucdata &= 0xF7;
     if (rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);
  }

  /* assert ACK to acknowledge data was sent */
  if ((rc=read_mem_io(adapter_info,1,SOCL,&ucdata))) return(rc);
  ucdata |= 0x40;
  if (rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);

  /* Target receives data, compare data received */
  if ((rc=read_mem_io(adapter_info,1,SBDL1,&ucdata))) return(rc);
  if (ucdata !=  (data >> 8)) {
     sprintf(msg,"scsi data doesnot match recd %x expected %x\n",ucdata,
             (data>>8));
     print_error(adapter_info,msg);
     return (BLOOM_SCSIDATA_E);
  }
  if ((rc=read_mem_io(adapter_info,1,SBDL0,&ucdata))) return(rc);
  if (ucdata !=  (data & 0xFF)) {
     sprintf(msg,"scsi data doesnot match recd %x expected %x\n",ucdata,
             (data & 0xFF));
     print_error(adapter_info,msg);
     return (BLOOM_SCSIDATA_E);
  }

  /* deassert REQ */
  if ((rc=read_mem_io(adapter_info,1,SOCL,&ucdata))) return(rc);
  ucdata &= 0x7F;
  if (rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);

  /* deassert ACK */
  if ((rc=read_mem_io(adapter_info,1,SOCL,&ucdata))) return(rc);
  ucdata &= 0xBF;
  if (rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);

  return (0);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  scsi_move_byte_in                                               */
/*                                                                        */
/* FUNCTION:  Moves 1 byte of data onto and off of the scsi bus, handling */
/*            REQ/ACK handshaking for an asynchronous data in phase.      */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine assumes SCE is already set, and scsi bus control  */
/*         lines are also set appropriately.                              */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    diagnostic structure - diagnostic information.                      */
/*                                                                        */
/* INPUTS:                                                                */
/*    diag_ptr - diagnostic structure pointer                             */
/*    iocc_addr - base address for pio                                    */
/*    data - 1 byte of data to write and read from scsi bus               */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  EFAULT for data miscompare, O otherwise     */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
uint scsi_move_byte_in(adapter_struc_t *adapter_info,uchar data) {
  uint rc;  
  uchar ucdata;
  uchar cdata;
  char  msg[256];

  /* write data onto bus */
  ucdata=data>>8;
  if (rc=writec_mem_io(adapter_info,1,SODL1,&ucdata)) return(rc);
  ucdata=data & 0xFF ;
  if (rc=writec_mem_io(adapter_info,1,SODL0,&ucdata)) return(rc);
  if ((rc=read_mem_io(adapter_info,1,SCNTL1,&ucdata))) return(rc);
  ucdata &= 0xFB; /* clear assert even scsi parity */
  ucdata |= 0x40;
  if (rc=writec_mem_io(adapter_info,1,SCNTL1,&ucdata)) return(rc);

  /* assert REQ */
  if ((rc=read_mem_io(adapter_info,1,SOCL,&ucdata))) return(rc);
  ucdata |= 0x80;
  if (rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);
  if ((rc=read_mem_io(adapter_info,1,SOCL,&ucdata))) return(rc);

  /* assert ACK */
  if ((rc=read_mem_io(adapter_info,1,SOCL,&ucdata))) return(rc);
  ucdata |= 0x40;
  if (rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);

  /* deassert REQ */
  if ((rc=read_mem_io(adapter_info,1,SOCL,&ucdata))) return(rc);
  ucdata &= 0x7F;
  if (rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);

  /* compare data received */
  if ((rc=read_mem_io(adapter_info,1,SBDL1,&ucdata))) return(rc);
  if (ucdata !=  (data >> 8)) {
     sprintf(msg,"scsi data doesnot match recd %x expected %x\n",ucdata,
             (data>>8));
     print_error(adapter_info,msg);
     return (BLOOM_SCSIDATA_E);
  }
  if ((rc=read_mem_io(adapter_info,1,SBDL0,&ucdata))) return(rc);
  if (ucdata !=  (data & 0xFF)) {
     sprintf(msg,"scsi data doesnot match recd %x expected %x\n",ucdata,
             (data & 0xFF));
     print_error(adapter_info,msg);
     return (BLOOM_SCSIDATA_E);
  }

  /* deassert ACK */
  if ((rc=read_mem_io(adapter_info,1,SOCL,&ucdata))) return(rc);
  ucdata &= 0xBF;
  if (rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);

  return (0);
}

/******************************************************************************
*
* NAME:  wrap_ptc_test
*
* FUNCTION:  
*
* INPUT PARAMETERS:     adapter_info
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED:
*
******************************************************************************/
uint wrap_ptc_test(adapter_struc_t *adapter_info) {
  uint  rc;
  int   i;
  uchar ucdata;
  ushort command[6] = {0x1111, 0x2222, 0x4444, 0x8888, 0x55AA, 0x00FF};
  uchar msg[256];

  /* The chip reset set all the chip registers to their */
  /* default values.  We next set specific registers w/ */
  /* the non-default values needed for the loop test.   */
  ucdata=0x40; /* reset the chip */
  if(rc=writec_mem_io(adapter_info,1,ISTAT,&ucdata)) return(rc);
  ucdata=0x00; /* unreset the chip */
  if(rc=writec_mem_io(adapter_info,1,ISTAT,&ucdata)) return(rc);

  /* set chip, arbitrate for bus, and select the target */
  if (rc = arb_select(adapter_info,FALSE)) return rc;

  /* Successful selection phase */
  /* Get to message out phase, first by clearing SEL, */
  /* then by asserting MSG and CMD. */
  if((rc=read_mem_io(adapter_info,1,SOCL,&ucdata))) return(rc);
  ucdata &= 0xE8; /* clear SEL */
  if(rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);
  if((rc=read_mem_io(adapter_info,1,SOCL,&ucdata))) return(rc);
  ucdata |= 0x06;  /* assert msg and cmd */
  if(rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);

  /* Send identify message to the target */
  if (rc = scsi_move_byte_out(adapter_info,0x00C0,TRUE)) return rc;

  /* Get to the command phase by clearing MSG */
  /* and asserting CMD. */
  if((rc=read_mem_io(adapter_info,1,SOCL,&ucdata))) return(rc);
  ucdata &= 0xFB;  /* clear msg and assert cmd */
  if(rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);

  for (i=0; i < sizeof(command)/2; i++) {
      /* Send command byte to the target */
      if (rc = scsi_move_byte_out(adapter_info,command[i],FALSE)) return rc;
  }

  /* Get to data out phase by clearing MSG, C/D, and I/O */
  if((rc=read_mem_io(adapter_info,1,SOCL,&ucdata))) return(rc);
  ucdata &= 0xE8;  /* assert msg and cmd */
  if(rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);

  for (i=0; i < 16; i++) {
      if (rc = scsi_move_byte_out(adapter_info,((i*16)<<8)+(i*16),FALSE)) 
          return rc;
  }  

  /* Write the bus phases in the SOCL reg to bring the chip */
  /* to the command completion phase                        */
  /* Get to status phase (assert I/0 & C/D, deassert MSG) */
  if((rc=read_mem_io(adapter_info,1,SOCL,&ucdata))) return(rc);
  ucdata |= 0x03; 
  ucdata &= 0xFB;
  if(rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);
  if(rc=read_mem_io(adapter_info,1,SCNTL0,&ucdata)) return(rc);
  ucdata |= 0x01;  /* set target mode */
  if(rc=writec_mem_io(adapter_info,1,SCNTL0,&ucdata)) return(rc);

  /* indicate good status */
  if (rc = scsi_move_byte_in(adapter_info,0x0000)) return rc;

  /* Get to message in phase (assert I/0 & MSG) */
  if((rc=read_mem_io(adapter_info,1,SOCL,&ucdata))) return(rc);
  ucdata |= 0x05; 
  if(rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);

  /* indicate command completion */
  if (rc = scsi_move_byte_in(adapter_info,0x0000)) return rc;

  /* check for any scsi errors */
  if((rc=read_mem_io(adapter_info,1,SIST0,&ucdata))) return(rc);
  if ((ucdata&0xBF)) { /* scsi bus errors */
     sprintf(msg,"scsi bus errors sist0 %x\n",ucdata);
     print_error(adapter_info,msg);
     return (BLOOM_BUS_E);
  }
  if((rc=read_mem_io(adapter_info,1,SIST1,&ucdata))) return(rc);
  if ((ucdata)) { /* scsi bus timeouts */
     sprintf(msg,"scsi bus timeouts sist1 %x\n",ucdata);
     print_error(adapter_info,msg);
     return (BLOOM_BUS_E);
  }

  /* Go to bus free (deassert all signals) */
  ucdata = 0x00; 
  if(rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);

  /* Repeat test, going through the data in phase.  First, */
  /* set chip, arbitrate for bus, and select the initiator */
  if (rc = arb_select(adapter_info,FALSE)) return rc;

  /* Successful selection phase */
  /* Get to message out phase, first by clearing SEL, */
  /* then by asserting MSG, CMD, and I/O. */
  if((rc=read_mem_io(adapter_info,1,SOCL,&ucdata))) return(rc);
  ucdata &= 0xEF; /* clear SEL */
  if(rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);
  if((rc=read_mem_io(adapter_info,1,SOCL,&ucdata))) return(rc);
  ucdata |= 0x07;  /* assert msg and cmd */
  if(rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);

  /* send identify message to initiator */
  if (rc = scsi_move_byte_in(adapter_info,0x0080)) return rc;

  if(rc=read_mem_io(adapter_info,1,SCNTL0,&ucdata)) return(rc);
  ucdata |= 0x01;  /* set target mode */
  if(rc=writec_mem_io(adapter_info,1,SCNTL0,&ucdata)) return(rc);

  /* Move to data in phase by clearing MSG, CMD and ATN */
  if((rc=read_mem_io(adapter_info,1,SOCL,&ucdata))) return(rc);
  ucdata &= 0xE1; /* clear SEL */
  if(rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);

  for (i=0; i < 16; i++) {
      if (rc = scsi_move_byte_in(adapter_info,i)) return rc;
  }  

  /* Write the bus phases in the SOCL reg to bring the chip */
  /* to the command completion phase                        */

  /* Get to status phase (assert I/0 & C/D, deassert MSG) */
  if((rc=read_mem_io(adapter_info,1,SOCL,&ucdata))) return(rc);
  ucdata |= 0x03; 
  ucdata &= 0xFB; 
  if(rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);

  /* indicate good status */
  if (rc = scsi_move_byte_in(adapter_info,0x0000)) return rc;

  /* Get to message in phase (assert I/0 & MSG) */
  if((rc=read_mem_io(adapter_info,1,SOCL,&ucdata))) return(rc);
  ucdata |= 0x05; 
  if(rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);

  /* indicate command completion */
  if (rc = scsi_move_byte_in(adapter_info,0x0000)) return rc;

  /* check for any scsi errors */
  if((rc=read_mem_io(adapter_info,1,SIST0,&ucdata))) return(rc);
  if ((ucdata&0xBF)) { /* scsi bus errors */
     sprintf(msg,"scsi bus errors sist0 %x\n",ucdata);
     print_error(adapter_info,msg);
     return (BLOOM_BUS_E);
  }
  if((rc=read_mem_io(adapter_info,1,SIST1,&ucdata))) return(rc);
  if ((ucdata)) { /* scsi bus timeouts */
     sprintf(msg,"scsi bus timeouts sist1 %x\n",ucdata);
     print_error(adapter_info,msg);
     return (BLOOM_BUS_E);
  }

  /* Go to bus free (deassert all signals) */
  ucdata = 0x00; 
  if(rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);

  return (0);
} 

/**************************************************************************/
/*                                                                        */
/* NAME:  arb_select                                                      */
/*                                                                        */
/* FUNCTION:  Resets chip and starts loopback test, taking chip through   */
/*            arbitration and (re)selection phases.                       */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine runs a loop test of the chip scsi bus.  Signals   */
/*         and some register testing is done here.                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    diagnostic structure - diagnostic information.                      */
/*                                                                        */
/* INPUTS:                                                                */
/*    adapter_info - pointer                                              */
/*    target_role - indicates whether to act as target or initiator       */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, EFAULT otherwise     */
/*                                                                        */
/* ERROR DESCRIPTION:  pio problem or problem connecting to scsi bus      */
/*                                                                        */
/**************************************************************************/
int arb_select(adapter_struc_t *adapter_info,uchar target) {
  uint   rc;
  int    i;
  uchar ucdata;
  ushort resp_id;
  char        msg[256];

#ifndef SIXDG
  clock_t micro_start;
  clock_t micro_now;
#endif

  /* Set SCSI Control Enable, clear SLB and LOW */
   ucdata=0x80;  
  if(rc=writec_mem_io(adapter_info,1,STEST2,&ucdata)) return(rc);
  ucdata=0xC0|target; 
  if(rc=writec_mem_io(adapter_info,1,SCNTL0,&ucdata)) return(rc);
  /* Set destination ID */
  ucdata=adapter_info->scsi_id; 
  if(rc=writec_mem_io(adapter_info,1,SDID,&ucdata)) return(rc);
  /* Set card ID and enable selection and reselection */
  ucdata=0x60 | adapter_info->scsi_id; 
  if(rc=writec_mem_io(adapter_info,1,SCID,&ucdata)) return(rc);
  /* Set IDs chip responds to from SCSI bus */
  resp_id=0x01<<adapter_info->scsi_id; 
  ucdata = resp_id >> 8;
  if(rc=writec_mem_io(adapter_info,1,RESPID1,&ucdata)) return(rc);
  ucdata = resp_id & 0xFF;
  if(rc=writec_mem_io(adapter_info,1,RESPID0,&ucdata)) return(rc);
  /* Set target mode and WATN/ correctly, and start full arbitration */
  ucdata = 0xF8;
  if (target) ucdata=0xE9;
  if(rc=write_mem_io(adapter_info,1,SCNTL0,&ucdata)) return(rc);
  /* read ISTAT to see if we are connected to the bus  */
  /* to tell if we won arbitration                     */
  for ( i=0; i< 5 ; i++) { /* wait 1 second */
      if ((rc=read_mem_io(adapter_info,1,ISTAT,&ucdata))) return(rc);
      if (ucdata & 0x08) break;
#ifndef SIXDG
      micro_start=clock();
      do {
         micro_now=clock(); /* wait 1/4 second */
      } while ( (micro_now - micro_start ) < (CLOCKS_PER_SEC/4));
#endif
#ifdef SIXDG
     sleep(250,MILLISECONDS);
#endif
  }
  if ( (ucdata & 0x08) != 0x08 ) {
     sprintf(msg,"could not win arbitration on scsi bus iostat %x\n",ucdata);
     print_error(adapter_info,msg);
     return (BLOOM_SCSIARB_E);
  }

  /* Go to selection phase */
  if ((rc=read_mem_io(adapter_info,1,SBCL,&ucdata))) return(rc);
  if (ucdata < 0) {
     sprintf(msg,"could not go to selection phase sbcl %x\n",ucdata);
     print_error(adapter_info,msg);
     return (BLOOM_SCSIARB_E);
  }

  /* begin driving BSY */
  ucdata |= 0x20;
  if(rc=writec_mem_io(adapter_info,1,SOCL,&ucdata)) return(rc);
  for ( i=0; i< 5 ; i++) { /* wait 1 second */
      if ((rc=read_mem_io(adapter_info,1,SIST0,&ucdata))) return(rc);
      if (ucdata & 0x40) break;
#ifndef SIXDG
      micro_start=clock();
      do {
         micro_now=clock(); /* wait 1/4 second */
      } while ( (micro_now - micro_start ) < (CLOCKS_PER_SEC/4));
#endif
#ifdef SIXDG
     sleep(250,MILLISECONDS);
#endif
  }
  if ( (ucdata & 0x40) != 0x40 ) {
     sprintf(msg,"scsi arb sel Function not complete sist0 %x\n",ucdata);
     print_error(adapter_info,msg);
     return (BLOOM_SCSFCMP_E);
  }
  return (0);
}

/******************************************************************************
*
* NAME:  scsi_reset
*
* FUNCTION:  
*
* INPUT PARAMETERS:     adapter_info
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED:
*
******************************************************************************/
uint scsi_reset(adapter_struc_t *adapter_info) {
  uint rc;  
  uchar ucdata;
#ifndef SIXDG
  clock_t micro_start;
  clock_t micro_now;
#endif

  ucdata = 0x08;
  if (rc=writec_mem_io(adapter_info,1,SCNTL1,&ucdata)) return(rc);
#ifndef SIXDG
  micro_start=clock();
  do {
     micro_now=clock(); /* wait 1 second */
  } while ( (micro_now - micro_start ) < (CLOCKS_PER_SEC));
#endif
#ifdef SIXDG
     sleep(1,SECONDS);
#endif
  ucdata = 0x00;
  if (rc=writec_mem_io(adapter_info,1,SCNTL1,&ucdata)) return(rc);
  return (0);
} 

/*
 * NAME: exectu
 *
 * FUNCTION:  Execute a specific bloomer Test Unit.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called as a subroutine of a diagnostic
 *      application.
 *
 * NOTES:  This routine will accept commands to perform specific test
 *         units on CORVETTE adapters. Supported test units are:
 *
 *         BLOOM_INIT_ATU          (0x00) - intialize adapter for test units
 *         BLOOM_DIAG_ATU          (0x01) - command interface register test
 *         BLOOM_SCSI_ATU          (0x02) - Scsi and Term Power test
 *         BLOOM_TERM_ATU          (0x03) - terminate test
 *
 * DATA STRUCTURES:
 *
 * INPUTS:
 *
 * RETURNS:
 * RETURN VALUE DESCRIPTION:
 *
 *      BLOOM_SUCCESS           (0)    - test unit completed nornmally.
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 *
 * LOCAL PROCEDURES CALLED:
 *
 *
 */

int exectu(tucb_t *tucb) {
  uint rc=BLOOM_SUCCESS;
  char msg[256];
  
  switch (tucb->tu) {
#ifndef SIXDG
    case BLOOM_INIT_ATU:
     rc=adapter_open(tucb->name,&ahandle);
     break;
#endif
    case BLOOM_DIAG_ATU:
     rc=adapter_diag(ahandle);
     break;
    case BLOOM_SCSI_ATU:
     rc=wrap_ptc_test(ahandle);
     break;
#ifndef SIXDG
    case BLOOM_TERM_ATU:
     rc=adapter_close(ahandle);
     break;
    case BLOOM_RCFG_ATU:
     rc=read_cfg_io(ahandle);
     break;
    case BLOOM_SEDIFF_ATU:
     rc=sediff(ahandle);
     break;
#endif
    default:
     rc=BLOOM_TEST_E;
     break;
  }  /* end of switch on tu number */
  sprintf(msg,"TU no %x rc=%x\n",tucb->tu,rc);
  print_error(ahandle,msg);
  return (rc);
} /* end of exectu() --------------------------------------------------------*/
