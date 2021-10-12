/*
 *   COMPONENT_NAME: sysxcie
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * These defines are used by the LAN unconfigure methods to determine if
 * the comio emulator is installed, and if so, to run the appropriate
 * configure or unconfigure method.
 */
#define CFG_EMULATOR_LPP        "bos.compat.lan"
#define CFG_COMIO_EMULATOR      "/usr/lib/methods/cfgcie"
#define UCFG_COMIO_EMULATOR     "/usr/lib/methods/ucfgcie"


