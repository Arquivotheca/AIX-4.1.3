$ @(#)04 1.4.3.1  src/bos/usr/bin/smit/odm_msg/cmdtz.msg, cmdtz, bos411, 9434A411a 8/19/94 13:52:54

$
$ COMPONENT_NAME:  (CMDTZ) cmdstat.30
$
$ FUNCTIONS: (none)
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$set 30
$quote "
$ CUT is an acroynm for "Coordinated Universal Time"

1 "Change / Show Date, Time, & Time Zone"
2 "Use DAYLIGHT SAVINGS TIME?"
3 "CUT (Coordinated Universal Time) Time Zone"

4 "# Does this time zone go on\n\
# DAYLIGHT SAVINGS TIME?\n\
# "
$ Do not translate #, start each line with #, leave 1 space after #.
$ Last line must be only a # and a space.

5 "1 yes"
$ Do not translate 1, leave 1 space after 1.

6 "2 no"
$ Do not translate 1, leave 1 space after 1.

7 "\
(CUT0GDT)           Coordinated Universal Time   (CUT)\n\
(GMT0BST)           United Kingdom               (CUT)\n\
(AZOREST1AZOREDT)   Azores; Cape Verde        (CUT -1)\n\
(FALKST2FALKDT)     Falkland Islands          (CUT -2)\n\
(GRNLNDST3GRNLNDDT) Greenland; East Brazil    (CUT -3)\n\
(AST4ADT)           Central Brazil            (CUT -4)\n\
(EST5EDT)           Eastern U.S.; Colombia    (CUT -5)\n\
(CST6CDT)           Central U.S.; Honduras    (CUT -6)\n\
(MST7MDT)           Mountain U.S.             (CUT -7)\n\
(PST8PDT)           Pacific U.S.; Yukon       (CUT -8)\n\
(AST9ADT)           Alaska                    (CUT -9)\n\
(HST10HDT)          Hawaii; Aleutian          (CUT-10)\n\
(BST11BDT)          Bering Straits            (CUT-11)\n\
(NZST-12NZDT)       New Zealand               (CUT+12)\n\
(MET-11METDT)       Solomon Islands           (CUT+11)\n\
(EET-10EETDT)       Eastern Australia         (CUT+10)\n\
(JST-9JDT)          Japan                     (CUT +9)\n\
(KORST-9KORDT)      Korea                     (CUT +9)\n\
(WAUST-8WAUDT)      Western Australia         (CUT +8)\n\
(TAIST-8TAIDT)      Taiwan                    (CUT +8)\n\
(THAIST-7THAIDT)    Thailand                  (CUT +7)\n\
(TASHST-6TASHDT)    Tashkent; Central Asia    (CUT +6)\n\
(PAKST-5PAKDT)      Pakistan                  (CUT +5)\n\
(WST-4WDT)          Gorki; Central Asia; Oman (CUT +4)\n\
(MEST-3MEDT)        Turkey                    (CUT +3)\n\
(SAUST-3SAUDT)      Saudi Arabia              (CUT +3)\n\
(WET-2WET)          Finland                   (CUT +2)\n\
(USAST-2USADT)      South Africa              (CUT +2)\n\
(NFT-1DFT)          Norway; France            (CUT +1)"
$ Do not translate anything between the FIRST ( and
$ the FIRST ); leave AT LEAST ONE space after the 
$ FIRST ) on each line.  Do not split or continue lines!
$ Lines may be up to 70 characters long.

8 "\
(CUT0)           Coordinated Universal Time   (CUT)\n\
(GMT0)           United Kingdom               (CUT)\n\
(AZOREST1)       Azores; Cape Verde        (CUT -1)\n\
(FALKST2)        Falkland Islands          (CUT -2)\n\
(GRNLNDST3)      Greenland; East Brazil    (CUT -3)\n\
(AST4)           Central Brazil            (CUT -4)\n\
(EST5)           Eastern U.S.; Colombia    (CUT -5)\n\
(CST6)           Central U.S.; Honduras    (CUT -6)\n\
(MST7)           Mountain U.S.             (CUT -7)\n\
(PST8)           Pacific U.S.; Yukon       (CUT -8)\n\
(AST9)           Alaska                    (CUT -9)\n\
(HST10)          Hawaii; Aleutian          (CUT-10)\n\
(BST11)          Bering Straits            (CUT-11)\n\
(NZST-12)        New Zealand               (CUT+12)\n\
(MET-11M)        Solomon Islands           (CUT+11)\n\
(EET-10E)        Eastern Australia         (CUT+10)\n\
(JST-9)          Japan                     (CUT +9)\n\
(KORST-9)        Korea                     (CUT +9)\n\
(WAUST-8)        Western Australia         (CUT +8)\n\
(TAIST-8)        Taiwan                    (CUT +8)\n\
(THAIST-7)       Thailand                  (CUT +7)\n\
(TASHST-6)       Tashkent; Central Asia    (CUT +6)\n\
(PAKST-5)        Pakistan                  (CUT +5)\n\
(WST-4)          Gorki; Central Asia; Oman (CUT +4)\n\
(MEST-3)         Turkey                    (CUT +3)\n\
(SAUST-3)        Saudi Arabia              (CUT +3)\n\
(WET-2)          Finland                   (CUT +2)\n\
(USAST-2)        South Africa              (CUT +2)\n\
(NFT-1)          Norway; France            (CUT +1)"
$ Do not translate anything between the FIRST ( and
$ the FIRST ); leave AT LEAST ONE space after the 
$ FIRST ) on each line.  Do not split or continue lines!
$ Lines may be up to 70 characters long.

9 "Any changes made to the time zone will take effect\n\
at your next login session."


10 "YEAR (00-99)" 
11 "MONTH (01-12)" 
12 "DAY (01-31)\n" 
$ do not translate \n
13 "HOUR (00-23)" 
14 "MINUTES (00-59)" 
15 "SECONDS (00-59)" 
16 "Time zone"
17 "Does this time zone go on daylight savings time?\n" 
$ 50 columns per line.  Last line ends with \n
18 "yes"
19 "no"
20 "Old time zone"
21 "Change / Show Date & Time"
