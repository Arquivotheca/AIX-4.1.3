/* @(#)90	1.1  R2/cmd/hia/w_msg.h, cmdhia, bos322 6/1/92 13:03:31 */

/*
 * COMPONENT_NAME: (CMDHIA) Messages include file
 *
 * FUNCTIONS: Global includes and declarations
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <nl_types.h>		/* AIX Message Facility include file	     */
#include <limits.h>		/* AIX Message Facility include file	     */

nl_catd catd;			/* Panel20's message catalog descriptor	     */
char regmsgnum[4];		/* array to store external (registered)	     */
				/* message number of a message               */
int *messargs[8];		/* array of arguments to be inserted into    */
				/* a message				     */
int count;			/* count of arguments to be inserted into    */
				/* a message				     */
int validmsg;                   /* flag to designate if setnum and/or msgnum */
                                /* are valid; 0 - invalid, 1 - valid         */

/****************************************************************************************
** BAW Array of default messages (same as messages in panel20.msg) that catgets will   **
** use when the panel20.cat file is not found (LANG=C), or message has been changed    **
** and catalog has not yet been updated in system.  If any changes are made to message **
** file, those changes must also be made here (note in panel20.msg file as well), and  ** 
** if any messages are added, the array dimension must be updated here as well         **
*****************************************************************************************/
char *def_msg[56] = 
{ "                                 Panel 20 - 01\n",
  "                       Configured 3270 Display Addresses\n",
  "      Link Speed           =  %1$s                  Broadcast Count    =  %2$s\n",
  "      Microcode Version    =   %1$s           Link Errors        =  %2$s\n",
  "                   Broadcast Frame  =  %1$s\n                             Data      %2$s\n",
  "         Device          Link           Link           Poll            SNRM\n",
  "         Name           Address        Status         Counter         Counter\n",
  "         ----           -------        ------         -------         -------\n",
  "           Status: Currently Reading HIA Device Information...\n\
    Down Arrow=Next Page  Up Arrow=Previous Page  Enter=Next Panel  F10=Quit\n\
          Note: It takes a few seconds to toggle between panels.\n",
  "                                 Panel 20 - 02\n",
  "                       Display of Poll and SNRM Counters\n",
  "                             3270 Display Addresses\n",
  "                              Graphic Addresses\n",
  "           Status: Currently Reading HIA Device Information...\n\
                     Enter=Next Panel   F10=Quit\n\
          Note: It takes a few seconds to toggle between panels.\n",
  "                                 Panel 20 - 03\n",
  "                            LINK STATISTICS SUMMARY\n",
  "                            -----------------------\n",
  "             1. Receive Time-Out                         %1$s *\n",
  "             2. Transmit Time-Out                        %1$s *\n",
  "             3. RNRs                                     %1$s\n",
  "             4. CRC Error                                %1$s *\n",
  "             5. Receive Overrun                          %1$s *\n",
  "             6. Receive Invalid                          %1$s *\n",
  "             7. Receive Residual Bits                    %1$s *\n",
  "             8. Transmit Underrun                        %1$s *\n",
  "             9. Invalid NR                               %1$s *\n",
  "            10. Invalid NS                               %1$s *\n",
  "            11. Unexpected Frame Type                    %1$s *\n",
  "            12. Good Transmit                            %1$s\n",
  "            13. Poll                                     %1$s\n",
  "            14. SNRM                                     %1$s\n",
  "            15. SNRM When Enabled                        %1$s\n",
  "            16. Invalid FCB                              %1$s\n",
  "                 * = Link Error as indicated on Panel 20 - 01\n",
  "           Status: Currently Reading HIA Device Information...\n\
                     Enter=Next Panel   F10=Quit\n\
          Note: It takes a few seconds to toggle between panels.\n",
  "0790-036 panel20: The value of the lower bond session address or\n\
\t\t  the number of sessions for the HIA adapter\n\
\t\t  cannot be retrieved.\n\
\t Use local problem reporting procedures.\n",
  "0790-037 panel20: The lower bond session address is out of range or\n\
\t\t  the number of sessions (with the first session starting\n\
\t\t  at the lower bond session address) is too large.\n",
  "0790-038 panel20: The poll system call has failed.\n\
\t Use local problem reporting procedures.\n",
  "           Status: Processing Workstation input. Please wait...\n\n",
  "           Status: The pressed key is not defined to Panel 20.\n\
                     Enter=Next Panel   F10=Quit\n\n",
  "0790-041 panel20: Cannot retrieve object from an ODM object class.\n\
\t Use local problem reporting procedures.\n",
  "0790-042 panel20: Cannot initialize ODM.\n\
\t Use local problem reporting procedures.\n",
  "0790-043 panel20: Cannot set the ODM default path.\n\
\t Use local problem reporting procedures.\n",
  "0790-044 panel20: Cannot terminate ODM.\n\
\t Use local problem reporting procedures.\n",
  "0790-045 panel20: The HIA device is not defined to the system.\n",
  "0790-046 panel20: The HIA device is not configured.\n",
  "",
  "",
  "",
  "0790-000 panel20: An internal set number or message number is not valid.\n\
\t\t  The set number is %1$d and the message number is %2$d.\n\
\t Please record this information and report the error using local\n\
\t problem reporting procedures.\n",
  "                         Configured Display Addresses\n",
  "      Device       Channel       Link         Link        Poll         SNRM\n",
  "       Name        Address      Address      Status      Counter      Counter\n",
  "       ----        -------      -------      ------      -------      -------\n",
  "           Status: The pressed key is not defined to Panel 20.\n\
    Down Arrow=Next Page  Up Arrow=Previous Page  Enter=Next Panel  F10=Quit\n\n",
  "Usage:  panel20 [AdapterName]\n"
};
