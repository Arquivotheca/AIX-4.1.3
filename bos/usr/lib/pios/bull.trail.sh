# @(#)35 1.1 src/bos/usr/lib/pios/bull.trail.sh, cmdpios, bos411, 9428A410j 4/28/94 07:53:51
#
# COMPONENT_NAME: (CMDPIOS) Printer Backend
#
# FUNCTIONS:
#
# ORIGINS: 83
#
# LEVEL 1, 5 Years Bull Confidential Information
#
# produces the "END" banner for the trailer for Bull print outs
#
if [ -x /bin/banner ]; then
    banner=/bin/banner
else
    banner=/bin/echo
fi
$banner "END"
exit 0
