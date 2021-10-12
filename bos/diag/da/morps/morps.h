/* @(#)63       1.6  src/bos/diag/da/morps/morps.h, damorps, bos411, 9435B411a 8/31/94 10:53:53 */
/*
 *   COMPONENT_NAME: DAMORPS
 *
 *   FUNCTIONS: Header
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#define DASD_DRW_X      1
#define MEDIA_DRW_X     1
#define SYSTEM_UNIT_    2
#define PORTABLE_X      1
#define BRIDGE_X        2
#define RACK_X          3
#define HDISK           1
#define DSKT            2
#define MEDIA           3
#define YESS            1
#define NOO             2
#define FAN             3

/* frub's types *
 * IMPORTANT!!! Keep the order of this list agreed with the frub defined */
enum {	DEVICE_ADAP, HDA_TYPE, POWR_SUPPLY,
	frubTypeFanPowerSupply, frubTypeExtDiskette,
	HARR2_SRN_1, HARR2_SRN_2,
	frubType7013PowerSupply,
	frubTypeAcFanAssembly, frubTypeDcFanAssembly, frubTypeScsiBus
};

/* rCode */
#define BRDG_PSUPPLY    0x200
#define PORTA_PSUPPLY   0x100
#define HARR1_FAN       0x300
#define HARR1_PSUPPLY   0x400
#define RACK_SUPPLY     0x500
#define MISSING_OPTION  0x950
#define DSKT_PSUPPLY    0x600
#define DEVICE_ADAP_PSUPPLY     4

/* harrier2 defines */

#define HARRIER2        0x870

