$ @(#)07	1.13  src/bos/usr/lib/methods/defif/defif.msg, cmdnet, bos411, 9428A410j 6/15/90 22:29:08
$
$ Messages for DEFIF.c 
$
$ COMPONENT_NAME:  CMDNET   Network Configuration  
$
$ ORIGINS: 27 
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
$ combined with the aggregated modules for this product) 
$                  SOURCE MATERIALS 
$ (C) COPYRIGHT International Business Machines Corp. 1989. 
$ All Rights Reserved 
$
$ US Government Users Restricted Rights - Use, duplication or 
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp. 
$
$ len 
$quote "         define quote character.
$
$ Do not translate defif.
$
$ Message numbers have been assigned a range of 014-037.
$
$set MS_DEFIF        Define initial set#

BADADD "0821-014 defif: Cannot add a customized interface record.\n"
BADADDDEP "0821-015 defif: Cannot add a customized dependency record.\n"
BADCLASS "0821-016 defif: The class specified must be if."
$ Do not translate CuDv.
GETADAPT "0821-020 defif: Cannot get records from PdCn.\n"
$ Do not translate PdCn.
GETPDADAP "0821-022 defif: Cannot get predefined adaptor.\n"
$ Do not translate PdAt.
$ predefined means previously defined.
GETPDAT "0821-023 defif: Cannot get records from PdAt.\n"
$ Do not translate PdAt.
GETPDDV "0821-024 defif: Cannot get records from PdDv.\n"
OPENCUDP "0821-025 defif: Cannot open CuDep.\n"
$ Do not translate CuDep.
OPENCUDV "0821-026 defif: Cannot open CuDv.\n"
$ Do not translate CuDv.
PDADAPNUM "0821-029 defif: Expected one predefined adaptor, retreived %d.\n"
USAGE1 "\nusage:\t%s -c class -s subclass -t type\n"
USAGE2 "\t%s -h\n"
WRONGNUM "0821-030 defif: The number of predefined if statements is not correct.\n"
NOPDIF "0821-031 defif: Cannot find interface: %s\n"
CANTGETCUDV "0821-032 defif: Cannot get record for adaptor %s from CuDv.\n"
CANTGETPDAT "0821-033 defif: Cannot get record for if_keyword attribute from PdAt.\n\tuniquetype = %s/%s/%s\n"
CANTGETPDDV "0821-034 defif: Cannot find predefined device with uniquetype = %s/%s/%s\n"
CANTMKLGNM  "0821-035 defif: failure in making logical name\n"
