# @(#)79	1.9  src/bos/etc/security/group.S, cfgsauth, bos411, 9428A410j 3/26/91 14:07:59
#  COMPONENT_NAME: CFGSAUTH
#
#  ORIGINS: 27
#
#  (C) COPYRIGHT International Business Machines Corp. 1989, 1991
#  All Rights Reserved
#  Licensed Materials - Property of IBM
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

system:
	admin = true

staff:
	admin = false

bin:
	admin = true

sys:
	admin = true

adm:
	admin = true

uucp:
	admin = true

mail:
	admin = true

security:
	admin = true

cron:
	admin = true

printq:
	admin = true

audit:
	admin = true

ecs:
	admin = true

usr:
	admin = false

nobody:
	admin = false

