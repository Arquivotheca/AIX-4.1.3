/* @(#)60	1.9   6/16/90 00:25:16 */
#ifndef _H_CONFIGREC
#define _H_CONFIGREC

/*
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager - 60
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>    /* include for definition of unique_id struct */

/* Format of the DASD configuration record                              */

/*
 * Seek Profile (for DASD connected to AT adapter or ESDI adapter.
 */
struct seek_profile {
        ushort  distance;
        ushort  constant;
        ushort  slope; };

/*
 * Disk configuration record
 */
struct config_rec {
        uint    config_rec_id;
	  /* Identifies the configuration record as present and valid.
	     This field contains the value 0xF8E9DACB. */
        int     data_capacity;
	  /* Contains the number of sectors available after formatting
	     (includes bad sectors). */
        ushort  reserved0;
	  /* Reserved field */
        char    interleave_factor;
	  /* Contains the interleave factor to be used to read anything
	     other than the IPL record, the configuration record, or the
	     backup configuration record. */
        char    sector_size;
	  /* Contains the number of bytes per sector modula 256.  This
	     field contains the value 0x02 (i.e., 512 bytes per sector). */
        ushort  last_cyl;
	  /* Contains the number of data cylinders minus 1.  Cylinders are
	     numbered from 0 to last_cyl.  The total number of cylinders
	     is last_cyl+2, where the last cylinder is the CE cylinder. */
        char    last_head;
	  /* Contains the number of heads minus 1.  Heads are numbered
	     from 0 to last_head. */
        char    last_sector;
	  /* Contains the number of sectors per track.  Sectors are
	     numbered from 1 to last_sector. */
        char    write_precomp;
        char    device_status;
	  /* Used by the loadable POST routine after the configuration
	     record is read from the physical volume.  When the
	     configuration record is written, the value of this field
	     is 0x00. */
        ushort  ce_cyl;
	  /* Contains the number of the CE cylinder (used for
	     diagnostics). */
        ushort  end_of_life;
	  /* The value in this field determines the number of defects that
	     force a physical volume to be considered unusable.  A value
	     of zero allows the AIX operating system to set this value;
	     a value of all ones disables bad block relocation for the
	     physical volume. */
	struct  seek_profile seek[5];
        char    manufacturer_id[3];
	  /* The first two bytes of this field indicate the size of the
	     physical volume (40 Mbytes, 70 Mbytes, etc.).  The last byte
	     indicates the ID of the manufacturer.  Manufacturer ID values
	     from 0x00 through 0x7F are reserved for IBM-supported
	     manufacturers and are included in Diagnostic Control Program
	     tests.  Manufacturer ID values greater than 0x7F are not
	     checked by IBM diagnostics. */
	char    reserved1;
	  /* Reserved field */
        ushort  service_request_number;
	  /* This field contains a value that is used by the diagnostics
	     facility to identify the disk driver. */
	char    device_characteristic;
	  /* This field contains an ASCII character that, combined with
	     the physical volume formatted data capacity, identifies the
	     physical volume. */
	char reserved2;
	  /* Reserved field */
	struct unique_id pv_id;
	  /* The unique identifier for this physical volume.  This field
	     may contain the value 0x0000000000000000, in which case the
	     AIX operating system will supply its own ID, or it may
	     contain a manufacturer-supplied unique number. */
	char reserved3 [436];
	  /* Reserved field */
	};

#endif /* _H_CONFIGREC */
