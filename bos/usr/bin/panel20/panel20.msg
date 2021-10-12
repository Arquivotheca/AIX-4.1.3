$ @(#)99        1.5  src/bos/usr/bin/panel20/panel20.msg, cmdhia, bos411, 9428A410j 7/21/92 10:42:11
$
$ COMPONENT_NAME: (CMDHIA) Panel20 Messages
$
$ FUNCTIONS: Panel20 Messages source file
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregrated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1986, 1990
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$ The component number for all error messages in this file is 0790.

$quote "
$set 1

$               GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate %1$d %1$ld %1$s %1$c etc.
$ These are output formats for file names, directory
$ names, permission codes, command names, and similar,
$ depending on their context within each message.
$ The program replaces %1$d %2$d etc with a decimal number.
$ The program replaces %1$ld %2$ld etc with a long decimal number.
$ The program replaces %1$s %2$s %3$s etc with a string of characters.
$ The program replaces %1$c etc with a character.
$
$ Do not translate any of the following words. They are AIX and
$ other system names and acronyms that identify system calls,
$ program labels, parameters, and other features that are
$ identified by these same characters regardless of language:
$ SNRM  HIA  ODM  F10  RNR  CRC  NR  NS  FCB  panel20 poll
$
$ Messages 1 through 35, 39, 40, and 51 through 54 are headings or 
$ insertions for tabular data. When translating, be sure that you do 
$ not move or run over into the next field. Adjust number of spaces to
$ keep other fields in their original place. The translations should 
$ not add any new lines. Keep the headings/insertions centered.
$
$    OTHER COMMENTS FOR DEVELOPERS
$ If any changes are made to this file, the corresponding changes must
$ also be made to the def_msg array of default messages in w_msg.h.  This
$ array is used for default message given to the catgets call, and the 
$ array is indexed by the message number (-1).  This is done so that if 
$ the user is using LANG=C (so might not have panel20.cat), or changes have
$ been made to this file, but not translated and put into the system 
$ release, the user will see the default messages (built into the executable)
$ rather than some error message.      
$ Also, any changes made to this file should be added to the end.  For new
$ messages, just add them to the end with a new number.  For changes to 
$ existing messages, the message should be changed and added to the end
$ with a new number (the code should be updated correspondingly to get the 
$ proper message) and a comment put by the original message indicating that
$ there is a changed message now being used. 

1 "                                 Panel 20 - 01\n"

$ msg #2 is no longer used, changed and now is msg #51
2 "                       Configured 3270 Display Addresses\n"

3 "      Link Speed           =  %1$s                  Broadcast Count    =\
  %2$s\n"
$ This is a 1 line message.

4 "      Microcode Version    =   %1$s           Link Errors        =  %2$s\n"

5 "                   Broadcast Frame  =  %1$s\n\
                             Data      %2$s\n"

$ msg #6 is no longer used, changed and now is msg #52
6 "         Device          Link           Link           Poll\
            SNRM\n"

$ This is a 1 line message.
$ msg #7 is no longer used, changed and now is msg #53
7 "         Name           Address        Status         Counter\
         Counter\n"

$ This is a 1 line message.
$ msg #8 is no longer used, changed and now is msg #54
8 "         ----           -------        ------         -------\
         -------\n"
$ This is a 1 line message.

9 "           Status: Currently Reading HIA Device Information...\n\
    Down Arrow=Next Page  Up Arrow=Previous Page  Enter=Next Panel  F10=Quit\n\
          Note: It takes a few seconds to toggle between panels.\n"

10 "                                 Panel 20 - 02\n"

11 "                       Display of Poll and SNRM Counters\n"

12 "                             3270 Display Addresses\n"

13 "                              Graphic Addresses\n"

14 "           Status: Currently Reading HIA Device Information...\n\
                     Enter=Next Panel   F10=Quit\n\
          Note: It takes a few seconds to toggle between panels.\n"

15 "                                 Panel 20 - 03\n"

16 "                            LINK STATISTICS SUMMARY\n"

17 "                            -----------------------\n"

18 "             1. Receive Time-Out                         %1$s *\n"

19 "             2. Transmit Time-Out                        %1$s *\n"

20 "             3. RNRs                                     %1$s\n"

21 "             4. CRC Error                                %1$s *\n"

22 "             5. Receive Overrun                          %1$s *\n"

23 "             6. Receive Invalid                          %1$s *\n"

24 "             7. Receive Residual Bits                    %1$s *\n"

25 "             8. Transmit Underrun                        %1$s *\n"

26 "             9. Invalid NR                               %1$s *\n"

27 "            10. Invalid NS                               %1$s *\n"

28 "            11. Unexpected Frame Type                    %1$s *\n"

29 "            12. Good Transmit                            %1$s\n"

30 "            13. Poll                                     %1$s\n"

31 "            14. SNRM                                     %1$s\n"

32 "            15. SNRM When Enabled                        %1$s\n"

33 "            16. Invalid FCB                              %1$s\n"

34 "                 * = Link Error as indicated on Panel 20 - 01\n"

35 "           Status: Currently Reading HIA Device Information...\n\
                     Enter=Next Panel   F10=Quit\n\
          Note: It takes a few seconds to toggle between panels.\n"

36 "0790-036 panel20: The value of the lower bond session address or\n\
\t\t  the number of sessions for the HIA adapter\n\
\t\t  cannot be retrieved.\n\
\t Use local problem reporting procedures.\n"

37 "0790-037 panel20: The lower bond session address is out of range or\n\
\t\t  the number of sessions (with the first session starting\n\
\t\t  at the lower bond session address) is too large.\n"

38 "0790-038 panel20: The poll system call has failed.\n\
\t Use local problem reporting procedures.\n"

39 "           Status: Processing Workstation input. Please wait...\n\n"

40 "           Status: The pressed key is not defined to Panel 20.\n\
                     Enter=Next Panel   F10=Quit\n\n"

41 "0790-041 panel20: Cannot retrieve object from an ODM object class.\n\
\t Use local problem reporting procedures.\n"

42 "0790-042 panel20: Cannot initialize ODM.\n\
\t Use local problem reporting procedures.\n"

43 "0790-043 panel20: Cannot set the ODM default path.\n\
\t Use local problem reporting procedures.\n"

44 "0790-044 panel20: Cannot terminate ODM.\n\
\t Use local problem reporting procedures.\n"

45 "0790-045 panel20: The HIA device is not defined to the system.\n"

46 "0790-046 panel20: The HIA device is not configured.\n"

50 "0790-000 panel20: An internal set number or message number is not valid.\n\
\t\t  The set number is %1$d and the message number is %2$d.\n\
\t Please record this information and report the error using local\n\
\t problem reporting procedures.\n"

$ msg #51 used to be msg #2 which is no longer used
51 "                         Configured Display Addresses\n"

$ msg #52 used to be msg #6 which is no longer used
52 "      Device       Channel       Link         Link        Poll         SNRM\n"

$ msg #53 used to be msg #7 which is no longer used
53 "       Name        Address      Address      Status      Counter      Counter\n"

$ msg #54 used to be msg #8 which is no longer used
$ This is a 1 line message.
54 "       ----        -------      -------      ------      -------      -------\n"

$ msg #55 is a new msg
55 "           Status: The pressed key is not defined to Panel 20.\n\
    Down Arrow=Next Page  Up Arrow=Previous Page  Enter=Next Panel  F10=Quit\n\n"

$ msg #56 is a new msg
56 "Usage:  panel20 [AdapterName]\n"

$ Please do not change the following lines

$set 9
3 ""
