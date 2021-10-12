static char sccsid[] = "@(#)75  1.2  src/bos/usr/lib/methods/ucfgdevice/rds_power_off_slot.c, cfgmethods, bos411, 9435A411a 8/30/94 08:51:16";
/*
 * COMPONENT_NAME: (CFGMETHODS) Configuration Functions
 *
 * FUNCTIONS: rds_power_off_device 
 *
 * ORIGINS: 83
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/sysmacros.h>
#include <sys/cfgdb.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mdio.h>

#include "cfgdebug.h"

int
rds_power_off_device(cusobj)
struct CuDv *cusobj;
{
  struct CuAt par_cuat;

  int cab_num, bus_num;
  int sid, lun;
  int disk;
  int status;
  int rc, i;
  int nvram;
  char ss[256];
  MACH_DD_IO param;

  if ((strncmp(cusobj->PdDvLn_Lvalue, "disk/scsi/", 10)) &&
      (strncmp(cusobj->PdDvLn_Lvalue, "tape/scsi/", 10)) &&
      (strncmp(cusobj->PdDvLn_Lvalue, "cdrom/scsi/", 11)))
    return(0);

  sprintf(ss,"name = '%s' AND attribute=bus_loc", cusobj->parent);
  rc = (int) odm_get_first(CuAt_CLASS, ss, &par_cuat);
  if (rc == -1) {
	/* ODM failure */
	DEBUG_0("ODM failure getting CuAt object\n");
	return(E_ODMGET);
  }
  if (rc == 0) {
	/* no CuAt object found */
	DEBUG_0("bus_loc attribute not found\n");
	return(E_NOATTR);
  }

  if ( !strcmp(par_cuat.value, "X-0"))
	return(0);

  DEBUG_0("CuAt of the father is taken\n");
  cab_num = par_cuat.value[0] & 0x0F;
  DEBUG_1("Cabinet number %d\n", cab_num);
  if ( par_cuat.value[2] == 'A' ) {
	DEBUG_0("Bus number : 0\n");
	bus_num = 0;
  }
  if ( par_cuat.value[2] == 'B' ) {
	DEBUG_0("Bus number : 1\n");
	bus_num = 1;
  }

  sid = (int) strtoul(cusobj->connwhere, NULL, 10);
  lun = (int) strtoul(strchr(cusobj->connwhere,',')+1,NULL,10);
  DEBUG_1("Sid off %d\n", sid);

  /* open nvram to enable ioctls usage */
  nvram = open("/dev/nvram", O_RDWR);
  if (nvram == -1) {
	DEBUG_0("Unable to open /dev/nvram\n");
	return(E_DEVACCESS);
  }

  status = MRDS_OFF;

  disk = sid | (bus_num << 4);

  param.md_size = 1;
  param.md_incr = MV_WORD;

  param.md_cbnum = cab_num;
  param.md_dknum = disk;
  param.md_data = (char *)&status;

  /* switch disk on or off */
  rc = ioctl(nvram, MRDSSET, &param);  
  if (rc == -1) {
	DEBUG_0("Error reading with RDS\n");
	close(nvram);
	return(E_DEVACCESS);
  }

  close(nvram);

  return(0);
}
