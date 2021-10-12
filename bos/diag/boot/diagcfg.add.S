# @(#)86	1.4  src/bos/diag/boot/diagcfg.add.S, diagboot, bos41J, 9515A_all 4/5/95 10:11:55
*
*   COMPONENT_NAME: DIAGBOOT
*
*   FUNCTIONS: none
*
*   ORIGINS: 27
*
*   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
*   combined with the aggregated modules for this product)
*                    SOURCE MATERIALS
*
*   (C) COPYRIGHT International Business Machines Corp. 1989, 1995
*   All Rights Reserved
*   US Government Users Restricted Rights - Use, duplication or
*   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*

Config_Rules:
	phase = 1
	seq = 5
	rule = "/etc/methods/defsys"

Config_Rules:
	phase = 3
	seq = 5
	rule = "/etc/methods/defsys"

Config_Rules:
	phase = 3
	seq = 10
	rule = "/etc/methods/darcfgrule"

*Config_Rules:
*	phase = 3
*	seq = 10
*	rule = "/usr/lib/methods/defssar"

