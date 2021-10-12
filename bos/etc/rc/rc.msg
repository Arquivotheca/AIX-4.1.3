$ @(#)22	1.3  src/bos/etc/rc/rc.msg, cfgetc, bos411, 9428A410j 7/15/92 15:29:47
$
$ COMPONENT_NAME: (CFGETC) Configuration files in /etc
$
$ FUNCTIONS:   
$
$ ORIGINS: 27  
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when  
$ combined with the aggregated modules for this product)  
$                  SOURCE MATERIALS  
$
$ (C) COPYRIGHT International Business Machines Corp. 1991  
$ All Rights Reserved  
$
$ US Government Users Restricted Rights - Use, duplication or  
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  

$quote "         define quote character.
$len 

$ Error messages for CFGETC are assigned to 1315-xxx.        
$ Informational messages do not use error numbers.

$set 1        Define initial set#
$ Translators: do not translate \n

1	"Starting Multi-user Initialization\n"

2	" Performing auto-varyon of Volume Groups \n"
$ Do not translate varyon	

3	" Activating all paging spaces \n"

4	" Performing all automatic mounts \n"

5	"Multi-user initialization completed\n"

6       "Write system start up record to /usr/adm/sa/sa`date`\n"
$ not used

7       "Write system start up record to /usr/adm/sa/sa"
