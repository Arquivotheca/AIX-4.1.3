static char sccsid[] = "@(#)15	1.12  src/bos/usr/ccs/lib/libcfg/cfdefmsg.c, libcfg, bos411, 9428A410j 6/9/94 16:41:29";
/*
 * COMPONENT_NAME: (LIBCFG)  Device configuration library
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *	MODULE: cfdefmsg.c
 *	CONTAINS: default messages strings for device configuration
 *		  routines and methods.
 */

/*----------------------------- problem messages -----------------------------*/
char *meth_err_msg[] =
{
"Method successful completion.\n",
"\t0514-001 System error:\n",
"\t0514-002 Cannot initialize the ODM.\n",
"\t0514-003 Cannot obtain the device configuration database lock.\n",
"\t0514-004 Cannot open an object class in the device\n\
\t         configuration database.\n",
"\t0514-005 Cannot close an object class in the device\n\
\t         configuration database.\n",
"\t0514-006 Cannot retrieve an object from the device\n\
\t         configuration database.\n",
"\t0514-007 Cannot update an object in the device \n\
\t         configuration database.\n",
"\t0514-008 Cannot add an object to the device configuration database.\n",
"\t0514-009 Cannot delete an object from the device\n\
\t         configuration database.\n",
"\t0514-010 Error returned from odm_run_method.\n",
"\t0514-011 Invalid arguments:\n",
"\t0514-012 Cannot open a file or device.\n",
"\t0514-013 Logical name is required.\n",
"\t0514-014 Device class, subclass, and type required.\n",
"\t0514-015 Parent or connection required.\n",
"\t0514-016 Parent or connection not required.\n",
"\t0514-017 The following attributes are not valid for the\n\
\t         specified device:\n",
"\t0514-018 The values specified for the following attributes \n\
\t         are not valid:\n",
"\t0514-019 This method does not support the -P flag.\n",
"\t0514-020 This method does not support the -T flag.\n",
"\t0514-021 This method does not support a parent or \n\
\t         connection change.\n",
"\t0514-022 The specified connection is not valid.\n",
"\t0514-023 The specified device does not exist in the \n\
\t         customized device configuration database.\n",
"\t0514-024 The specified predefined information does not exist \n\
\t         in the predefined device configuration database.\n",
"\t0514-025 Cannot perform the requested function because the\n\
\t         parent of the specified device does not exist.\n",
"\t0514-026 It is illegal to define another device of this type.\n",
"\t0514-027 The specified device is in the wrong state to\n\
\t         perform the requested function.\n",
"\t0514-028 Cannot perform the requested function because the\n\
\t         parent of the specified device is not in a correct state.\n",
"\t0514-029 Cannot perform the requested function because a\n\
\t         child device of the specified device is not in a correct state.\n",
"\t0514-030 Cannot perform the requested function because\n\
\t         the specified device is dependent on another device which is\n\
\t         not in a correct state.\n",
"\t0514-031 A device is already configured at the specified location.\n",
"\t0514-032 Cannot perform the requested function because the\n\
\t         specified device is dependent on another device which does\n\
\t         not exist.\n",
"\t0514-033 Cannot perform the requested function because the\n\
\t         specified device is dependent on an attribute which does\n\
\t         not exist.\n",
"\t0514-034 The following attributes do not have valid values:\n",
"\t0514-035 Cannot perform the requested function because of\n\
\t         missing predefined information in the device configuration\n\
\t         database.\n",
"\t0514-036 Cannot perform the requested function because of\n\
\t         missing customized information in the device configuration\n\
\t         database.\n",
"\t0514-037 Cannot generate a logical name.\n",
"\t0514-038 Error loading kernel extension.\n",
"\t0514-039 Error unloading kernel extension.\n",
"\t0514-040 Error initializing a device into the kernel.\n",
"\t0514-041 Error terminating device from driver.\n",
"\t0514-042 Error getting or assigning a major number.\n",
"\t0514-043 Error getting or assigning a minor number.\n",
"\t0514-044 Cannot make a special file.\n",
"\t0514-045 Error building a DDS structure.\n",
"\t0514-046 A file containing microcode or adapter software was\n\
\t         not accessable.\n",
"\t0514-047 Cannot access a device.\n",
"\t0514-048 Error downloading microcode or software.\n",
"\t0514-049 Error getting VPD for a device.\n",
"\t0514-050 Cannot perform the requested function because the\n\
\t         specified device was not detected.\n",
"\t0514-051 Device to be configured does not match the physical\n\
\t         device at the specified connection location.\n",
"\t0514-052 Bus resources could not be resolved for a device.\n",
"\t0514-053 Error returned from sys_config.\n",
"\t0514-054 There is not enough memory available now.\n",
"\t0514-055 Cannot release devno because it does not exist.\n",
"\t0514-056 Requested devno is already in use.\n",
"\t0514-057 File has wrong status.\n",
"\t0514-058 Cannot remove a special file.\n",
"\t0514-059 Cannot create symbolic link.\n",
"\t0514-060 Cannot create another process.\n",
"\t0514-061 Cannot find a child device.\n",
"\t0514-062 Cannot perform the requested function because the\n\
\t         specified device is busy.\n",
"\t0514-063 The specified device driver name is too long.\n",
"\t0514-064 Cannot perform the requested function because the\n\
\t         AIO kernel extension is permanent and cannot be unloaded.\n",
"\t0514-065 Cannot perform the requested instance number function.\n",
"\t0514-066 Parent required.\n",
"\t0514-067 There are no slots available for a device.\n",
"\t0514-068 Cause not known.\n"
};
