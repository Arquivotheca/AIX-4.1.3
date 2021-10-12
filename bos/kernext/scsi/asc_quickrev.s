#**********************************************************************
#   COMPONENT_NAME: SYSXSCSI
#
#   FUNCTIONS:  qrev_writel
#		qrev_writes
#		qrev_readl
#		qrev_reads
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#**********************************************************************

#**********************************************************************
#
#  NAME: qrev_writel
#
#  FUNCTION:  store byte reversed long
#
#       qrev_writel(src, &dest);
#
#  INPUT STATE:
#     r3 = 32-bit value
#     r4 = address to store byte reversed value at
#
#  RETURNED VALUE:
#     none
#
#  EXECUTION ENVIRONMENT:
#       any
#
#
#**********************************************************************

        S_PROLOG(qrev_writel)

        stbrx   r3, 0, r4               # byte reversed r3 => (r4)

        S_EPILOG
        FCNDES(qrev_writel)

#**********************************************************************
#
#  NAME: qrev_writes
#
#  FUNCTION:  store byte reversed short
#
#       qrev_writes(src, &dest);
#
#  INPUT STATE:
#     r3 = 16-bit value
#     r4 = address to store byte reversed value at
#
#  RETURNED VALUE:
#     none
#
#  EXECUTION ENVIRONMENT:
#       any
#
#
#**********************************************************************

        S_PROLOG(qrev_writes)

        sthbrx   r3, 0, r4               # byte reversed r3 => (r4)

        S_EPILOG
        FCNDES(qrev_writes)

#**********************************************************************
#
#  NAME: qrev_readl
#
#  FUNCTION:  read byte reversed long
#
#       dest = qrev_readl(&src);
#
#  INPUT STATE:
#     r3 = address of 32-bit value to byte reverse
#
#  RETURNED VALUE:
#     byte reversed 32-bit value
#
#  EXECUTION ENVIRONMENT:
#       any
#
#
#**********************************************************************

        S_PROLOG(qrev_readl)

        lbrx   r3, 0, r3               # byte reversed (r3) => r3

        S_EPILOG
        FCNDES(qrev_readl)

#**********************************************************************
#
#  NAME: qrev_reads
#
#  FUNCTION:  read byte reversed short
#
#       dest = qrev_reads(&src);
#
#  INPUT STATE:
#     r3 = address of 16-bit value to reverse
#
#  RETURNED VALUE:
#     bytes reversed 16-bit value
#
#  EXECUTION ENVIRONMENT:
#       any
#
#
#**********************************************************************

        S_PROLOG(qrev_reads)

        lhbrx   r3, 0, r3               # byte reversed (r3) => r3

        S_EPILOG
        FCNDES(qrev_reads)

