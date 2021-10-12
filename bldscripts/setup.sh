#!/bin/sh
# @(#)31        1.64  src/bldscripts/setup.sh, ade_build, bos41J, 9511A_all 3/3/95 09:16:26
#
#   COMPONENT_NAME: bldprocess
#
#   ORIGINS: 27
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#

#
# This is the setup script for building the source tree from scratch
# using as little as possible from the environment already installed on
# the current machine.  The basic process is to create the "environment"
# as we go along, which requires that this script "understand" all of the
# interdependencies between components and their environment.  When porting
# the sources to a "unknown" machine, this script is the place to start
# making changes.  Good Luck!
#

. `/usr/bin/dirname $0`/common_funcs.sh
. `/usr/bin/dirname $0`/common.sh

#-----------------------------------------------------------------------------
#
# If arguments are passed to setup.sh, then only build the tool types
# specified (i.e. BOS_TOOLS, GOS_TOOLS).  Look at src/bldenv/Makefile
# to see what tool types are available (those ending with _TOOLS).
#
if [ $# -gt 0 ]
then
    BUILDVAR=""

    while [ $# -gt 0 ]
    do
        BUILDVAR="${BUILDVAR} -D $1"

        shift 1
    done   
fi

#
# Locally define the absolute path to the object directory.
#
objdir=${BASE}/obj/${target_machine}

#
# Constrain search paths
#
OS_PATH=/bin:/usr/bin
PATH=${ODE_TOOLS}/bin:.:${OS_PATH}
OBJECTDIR=../obj/${target_machine}

export PATH
export OBJECTDIR

/usr/bin/echo "SETUP started at `/usr/bin/date`"

#
# Define system information for boot strapping the compilers.
#
BLDENV_TOOLS=${ODE_TOOLS}

export BLDENV_TOOLS

#
# Undefine ODE_TOOLS until all the tools have been built.
#
unset ODE_TOOLS

#
# Make sure that the object directory exists before building anything.
#
[ -d ${objdir} ] || mkdir -p ${objdir}

#
# Create the srcpath tool.  This tool is used by scripts in the build
# environment to find the path of where source files resides.
#
[ -d ${BLDENV_TOOLS}/bin ] || mkdir -p ${BLDENV_TOOLS}/bin

srcpath_script >${BLDENV_TOOLS}/bin/srcpath
chmod +x ${BLDENV_TOOLS}/bin/srcpath

#
# copy the locales into ode_tools so they are ready once the 
# compilers have been bootstrapped.  ( You can find the 
# bootlocales function in common_funcs.sh )
#

bootlocales

#
# Export the C++ header files.
#

includes="sys/types.h sys/limits.h sys/m_types.h stream.h iostream.h \
          stdio.h stdiostream.h fstream.h memory.h string.h standards.h \
          va_list.h iomanip.h float.h generic.h"

vpath=xlC/usr/lpp/xlC/include
export_base=${BASE}/export/${target_machine}
expdir=bldenv/usr/include/xlC

for next_include in $includes
do
    #
    # Create the export directory if it does not exist.
    #
    export_dir=`dirname $export_base/$expdir/$next_include`

    if [ ! -d $export_dir ]
    then
        mkdir -p $export_dir
    fi

    #
    # Export the next header file.
    #
    cp -p `srcpath $vpath/$next_include` $export_dir
done

#
# create usr/bin for linking cc, xlc, etc.
#
idir=usr/bin

if [ ! -d ${BLDENV_TOOLS}/$idir ]
then
    mkdir -p ${BLDENV_TOOLS}/$idir
fi

#
# Install C++ libraries and runtime object files.
#
idir=usr/lpp/xlC/lib
vpath=xlC/$idir

if [ ! -d ${BLDENV_TOOLS}/$idir ]
then
    mkdir -p ${BLDENV_TOOLS}/$idir
fi

cp `srcpath $vpath/crt0.o`  ${BLDENV_TOOLS}/$idir
cp `srcpath $vpath/gcrt0.o`  ${BLDENV_TOOLS}/$idir
cp `srcpath $vpath/mcrt0.o`  ${BLDENV_TOOLS}/$idir
cp `srcpath $vpath/crt0_r.o`  ${BLDENV_TOOLS}/$idir
cp `srcpath $vpath/gcrt0_r.o`  ${BLDENV_TOOLS}/$idir
cp `srcpath $vpath/mcrt0_r.o`  ${BLDENV_TOOLS}/$idir
cp `srcpath $vpath/libC.a`   ${BLDENV_TOOLS}/$idir
cp `srcpath $vpath/libCns.a` ${BLDENV_TOOLS}/$idir
cp `srcpath $vpath/libctpd.a` ${BLDENV_TOOLS}/$idir

#
# Create the export directory if it does not exist.
#
export_dir=$export_base/usr/ccs/lib

if [ ! -d $export_dir ]
then
    mkdir -p $export_dir
fi

#
# Export libdemangle.a
#
cp -p `srcpath $vpath/libdemangle.a` $export_dir

#
# Install the C++ compiler.
#
idir=usr/lpp/xlC/bin
vpath=xlC/$idir

if [ ! -d ${BLDENV_TOOLS}/$idir ]
then
    mkdir -p ${BLDENV_TOOLS}/$idir
fi

cp `srcpath $vpath/xlC` ${BLDENV_TOOLS}/$idir
chmod +x ${BLDENV_TOOLS}/$idir/xlC

#
# setup links for the various compilers where they will be 
# called from, ( usr/bin/compilers ) to the real compiler, ( xlC )
# in usr/lpp/xlC/bin
#

if [ ! -d ${BLDENV_TOOLS}/usr/bin/compilers ]
then
     mkdir -p ${BLDENV_TOOLS}/usr/bin/compilers
fi

ln ${BLDENV_TOOLS}/$idir/xlC ${BLDENV_TOOLS}/usr/bin/compilers/cc
chmod +x ${BLDENV_TOOLS}/usr/bin/compilers/cc
ln ${BLDENV_TOOLS}/$idir/xlC ${BLDENV_TOOLS}/usr/bin/compilers/xlc
chmod +x ${BLDENV_TOOLS}/usr/bin/compilers/xlc
ln ${BLDENV_TOOLS}/$idir/xlC ${BLDENV_TOOLS}/usr/bin/compilers/cc_r
chmod +x ${BLDENV_TOOLS}/usr/bin/compilers/cc_r
ln ${BLDENV_TOOLS}/$idir/xlC ${BLDENV_TOOLS}/usr/bin/compilers/xlc_r
chmod +x ${BLDENV_TOOLS}/usr/bin/compilers/xlc_r
ln ${BLDENV_TOOLS}/$idir/xlC ${BLDENV_TOOLS}/usr/bin/compilers/xlC
chmod +x ${BLDENV_TOOLS}/usr/bin/compilers/xlC

cp `srcpath $vpath/c++filt` ${BLDENV_TOOLS}/$idir
chmod +x ${BLDENV_TOOLS}/$idir/c++filt

idir=usr/bin

cp `srcpath $vpath/xlC` ${BLDENV_TOOLS}/$idir
chmod +x ${BLDENV_TOOLS}/$idir/xlC

idir=usr/lpp/xlC/exe
vpath=xlC/$idir

if [ ! -d ${BLDENV_TOOLS}/$idir ]
then
    mkdir -p ${BLDENV_TOOLS}/$idir
fi

cp `srcpath $vpath/xlCentry` ${BLDENV_TOOLS}/$idir
chmod +x ${BLDENV_TOOLS}/$idir/xlCentry

cp `srcpath $vpath/xlcentry` ${BLDENV_TOOLS}/$idir
chmod +x ${BLDENV_TOOLS}/$idir/xlcentry

cp `srcpath $vpath/munch` ${BLDENV_TOOLS}/$idir
chmod +x ${BLDENV_TOOLS}/$idir/munch

cp `srcpath $vpath/xlCinline` ${BLDENV_TOOLS}/$idir
chmod +x ${BLDENV_TOOLS}/$idir/xlCinline

cp `srcpath $vpath/xlCcode` ${BLDENV_TOOLS}/$idir
chmod +x ${BLDENV_TOOLS}/$idir/xlCcode

idir=usr/lib
vpath=xlC/usr/ccs/lib

if [ ! -d ${BLDENV_TOOLS}/$idir ]
then
    mkdir -p ${BLDENV_TOOLS}/$idir
fi

cp `srcpath $vpath/cpp` ${BLDENV_TOOLS}/$idir
chmod +x ${BLDENV_TOOLS}/$idir/cpp

#
# Also copy cpp to /usr/bin/compilers/cpp so that we can use the same
# script to invoke the compiler as weel as cpp
#

cp ${BLDENV_TOOLS}/$idir/cpp ${BLDENV_TOOLS}/usr/bin/compilers/cpp
chmod +x ${BLDENV_TOOLS}/usr/bin/compilers/cpp

#
# Install the C++ message catalogs.
#
vpath=xlC/usr/lpp/xlC/exe/default_msg
idir=usr/lib/nls/msg/En_US

if [ ! -d ${BLDENV_TOOLS}/$idir ]
then
    mkdir -p ${BLDENV_TOOLS}/$idir
fi

cp `srcpath $vpath/xlCfe.cat`   ${BLDENV_TOOLS}/$idir
cp `srcpath $vpath/xlCsmsg.cat` ${BLDENV_TOOLS}/$idir
cp `srcpath $vpath/xlCdmsg.cat` ${BLDENV_TOOLS}/$idir
cp `srcpath $vpath/xlCmsg.cat`  ${BLDENV_TOOLS}/$idir
cp `srcpath $vpath/hdrvhelp.cat`  ${BLDENV_TOOLS}/$idir

idir=usr/lib/nls/msg/C

if [ ! -d ${BLDENV_TOOLS}/$idir ]
then
    mkdir -p ${BLDENV_TOOLS}/$idir
fi

echo "No C messages" >${BLDENV_TOOLS}/$idir/C_msg_cat

#
# Install the fortran compiler
#

cp `srcpath xlf/usr/bin/xlf` ${BLDENV_TOOLS}/usr/bin/xlf
chmod 755 ${BLDENV_TOOLS}/usr/bin/xlf

if [ ! -d ${BLDENV_TOOLS}/usr/lpp/xlf/bin ]
then
    mkdir -p ${BLDENV_TOOLS}/usr/lpp/xlf/bin
fi

cp `srcpath xlf/usr/lpp/xlf/bin/xlfentry` ${BLDENV_TOOLS}/usr/lpp/xlf/bin/xlfentry
chmod 755 ${BLDENV_TOOLS}/usr/lpp/xlf/bin/xlfentry
cp `srcpath xlf/usr/lib/libxlf90.a` ${BLDENV_TOOLS}/usr/lib/libxlf90.a
chmod 644 ${BLDENV_TOOLS}/usr/lib/libxlf90.a
cp `srcpath xlf/usr/lib/libxlf.a` ${BLDENV_TOOLS}/usr/lib/libxlf.a
chmod 644 ${BLDENV_TOOLS}/usr/lib/libxlf.a

#
# Install the "path-setting" compiler scripts
#

comp_script_path=bldenv/compilers/xlC
idir=usr/bin

if [ ! -d ${BLDENV_TOOLS}/$idir ]
then
    mkdir -p ${BLDENV_TOOLS}/$idir
fi

cp `srcpath $comp_script_path/xlC.sh`  ${BLDENV_TOOLS}/$idir/xlC
chmod +x ${BLDENV_TOOLS}/$idir/xlC
ln -f ${BLDENV_TOOLS}/$idir/xlC ${BLDENV_TOOLS}/$idir/xlc
chmod +x ${BLDENV_TOOLS}/$idir/xlc
ln -f ${BLDENV_TOOLS}/$idir/xlc ${BLDENV_TOOLS}/$idir/cc
chmod +x ${BLDENV_TOOLS}/$idir/cc
ln -f ${BLDENV_TOOLS}/$idir/xlc ${BLDENV_TOOLS}/$idir/cc_r
chmod +x ${BLDENV_TOOLS}/$idir/cc_r
ln -f ${BLDENV_TOOLS}/$idir/xlc ${BLDENV_TOOLS}/$idir/xlc_r
chmod +x ${BLDENV_TOOLS}/$idir/xlc_r
ln -f ${BLDENV_TOOLS}/$idir/xlc ${BLDENV_TOOLS}/$idir/xlC_r
chmod +x ${BLDENV_TOOLS}/$idir/xlC_r
ln -f ${BLDENV_TOOLS}/$idir/xlc ${BLDENV_TOOLS}/$idir/cpp
chmod +x ${BLDENV_TOOLS}/$idir/cpp


if [ ! -d ${BLDENV_TOOLS}/etc ]
then
    mkdir -p ${BLDENV_TOOLS}/etc
fi

#
# Install the C++ template files which will be used by the
# create_cfgs script, and then the create_cfgs script itself.
#
cp `srcpath $comp_script_path/xlC.cfg.tmplt` ${BLDENV_TOOLS}/etc
cp `srcpath $comp_script_path/bldenv.xlC.cfg.tmplt` ${BLDENV_TOOLS}/etc
cp `srcpath $comp_script_path/boot.xlC.cfg.tmplt` ${BLDENV_TOOLS}/etc
cp `srcpath $comp_script_path/dce.xlC.cfg.tmplt` ${BLDENV_TOOLS}/etc

cp `srcpath $comp_script_path/create_cfgs.sh` ${BLDENV_TOOLS}/$idir/create_cfgs
chmod +x ${BLDENV_TOOLS}/$idir/create_cfgs

#
# Install the fortran template files which will be used by the
# create_cfgs script.
#
comp_script_path=bldenv/compilers/fortran
cp `srcpath $comp_script_path/xlf.cfg.tmplt` ${BLDENV_TOOLS}/etc
cp `srcpath $comp_script_path/bldenv.xlf.cfg.tmplt` ${BLDENV_TOOLS}/etc
cp `srcpath $comp_script_path/boot.xlf.cfg.tmplt` ${BLDENV_TOOLS}/etc

#
# Install the vfs template file which will be used by the
# create_cfgs script.
#
comp_script_path=bldenv/vfs
cp `srcpath $comp_script_path/vfs.tmplt` ${BLDENV_TOOLS}/etc

#
# modify the configuration files to have the correct paths
#

(cd ${BLDENV_TOOLS}; usr/bin/create_cfgs)

#
# Define which compiler configuration file to use to bootstrap 
# build the bldenv.
#
XLC_CFG=bootstrap.xlC.cfg
XLF_CFG=bootstrap.xlf.cfg

export XLC_CFG
export XLF_CFG

#
# Temporarily set MD until md is built.
#
MD=true

export MD

#
# Retrieve the symbol value for GENPATH.
#
get_tool_symbol GENPATH

#
# Go ahead and build make.
#
if [ -x ${BLDENV_TOOLS}/bin/make ] && [ -x ${GENPATH} ]
then
    #
    # Use make to build make.
    #
    SUBDIRS=bldenv/make

    walk_subdirs build
    [ "$?" = 0 ] || exit 1
else
    #
    # Temporarily set GENPATH until the genpath tool is built.  This is
    # done because the the OSF make files use this tool and until it is
    # built the following message is displayed:
    #
    #     "genpath returned a non-zero status".
    #
    GENPATH=true

    export GENPATH

    #
    # Define how to use the compiler for boot strapping make.
    #
    CC="${BLDENV_TOOLS}/usr/bin/cc -F${BLDENV_TOOLS}/etc/${XLC_CFG}"

    export CC

    #
    # Boot strap build the make command.
    #
    context=power_aix
    MAKETOP=${SOURCEBASE}/
    MAKESUB=bldenv/make

    export context
    export MAKETOP
    export MAKESUB 

    [ -d ${objdir}/${MAKESUB} ] || mkdir -p ${objdir}/${MAKESUB}
    [ -d ${BLDENV_TOOLS}/bin ]  || mkdir -p ${BLDENV_TOOLS}/bin

    (cd ${objdir}/${MAKESUB}; sh -x `srcpath ${MAKESUB}/bootstrap.sh`)

    #
    # Undefine variables only needed to build make.  Variable information is
    # set through the Makeconf file.
    #
    unset CC
    unset context
    unset MAKETOP
    unset MAKESUB
fi

#
# Install make.
#
cp ${objdir}/bldenv/make/make ${BLDENV_TOOLS}/bin

#
# Force the shell to re-evaluate the location for make.  This is done
# just in case make is resident on the system.
#
hash -r make

#
# Temporarily set MAKEPATH until the makepath tool is built.
#
MAKEPATH=${objdir}/bldenv/makepath/makepath

export MAKEPATH

#
# _M4FLAGS_ is temporarily set because the _M4FLAGS_ set in the OSF make
# files use the -I flag which is not supported by the system m4 command.
#
_M4FLAGS_="-B32768 -DINCLML=. -DINCL=."

export _M4FLAGS_

#
# Build all tools that use the running machine binder.
#
SUBDIRS=bldenv/makepath

walk_subdirs build install
[ "$?" = 0 ] || exit 1

SUBDIRS=bldenv/includes

walk_subdirs export
[ "$?" = 0 ] || exit 1

SUBDIRS=bldenv/md

walk_subdirs build install
[ "$?" = 0 ] || exit 1

#
# Generate a dependency file for the make, makepath and md commands now that
# md is built.
#
(cd ${objdir}/bldenv/make;     ${MD} -rm -all .)
(cd ${objdir}/bldenv/makepath; ${MD} -rm -all .)
(cd ${objdir}/bldenv/md;       ${MD} -rm -all .)

LD_OTHER_SYMBOL=LD_BINDER
LD_OTHER_SYMBOL2=LD_GLINK

SUBDIRS="bldenv/genpath bldenv/findfile bldenv/mkcatdefs \
         bldenv/gencat bldenv/libc bldenv/ld"

walk_subdirs build install
[ "$?" = 0 ] || exit 1

#
# unset LD_BINDER and LD_GLINK because these are not used once LD is
# defined and having LD_BINDER defined causes _LD_PATHS_ to be
# appended to the CC line.
#
unset LD_BINDER
unset LD_GLINK

#
# Allow XLC_CFG and XLF_CFG to be redefined for use by the 4.x compilers and
# binder.
#
unset XLC_CFG
unset XLF_CFG

#
# Allow _M4FLAGS_ to be redefined for use by the 4.x m4 command.
#
unset _M4FLAGS_

#
# Build all the tools that will use the 4.x binder.
#
SUBDIRS="bldenv/rm bldenv/mv bldenv/cp bldenv/mkdir bldenv/echo \
         bldenv/cat bldenv/chmod bldenv/date bldenv/test bldenv/basename \
         bldenv/false bldenv/true bldenv/tr bldenv/grep bldenv/egrep \
         bldenv/fgrep bldenv/ed bldenv/sed bldenv/ar bldenv/tar \
         bldenv/expr bldenv/touch bldenv/cut bldenv/cmp bldenv/yacc \
         bldenv/lex bldenv/awk bldenv/as bldenv/m4 bldenv/what bldenv/wc \
         bldenv/xargs bldenv/libodm bldenv/odmcreate bldenv/odmadd \
	 bldenv/odmget bldenv/odmchange bldenv/brand bldenv/genxlt \
	 bldenv/lint1 bldenv/ls"

#
# Define all the tool symbols that depend upon the value of another
# tool symbol to correctly define the tool symbol.
#
YACC_OTHER_SYMBOL=YACCPAR
LEX_OTHER_SYMBOL=LEXER
BRAND_OTHER_SYMBOL=BRANDDICT

walk_subdirs build install
[ "$?" = 0 ] || exit 1

#
# Build COSE tools that are used to build the other COSE tools.
#
if [ -f "`srcpath gos/desktop/Makefile`" ]
then
    dthelp=bldenv/gos/desktop/dthelp

    #
    #  Build the libraries first.
    #
    SUBDIRS="$dthelp/util $dthelp/util2"

    walk_subdirs export
    [ "$?" = 0 ] || exit 1

    UTIL_OTHER_SYMBOL=DTCONTEXT
    UTIL2_OTHER_SYMBOL=DTCONTEXT2
    BUILD_OTHER_SYMBOL=DTBUILD
    BUILD2_OTHER_SYMBOL=DTBUILD2
    ELTDEF_OTHER_SYMBOL=DTELTDEF

    SUBDIRS="$dthelp/util $dthelp/util2 $dthelp/build $dthelp/build2 $dthelp/eltdef"

    walk_subdirs build install
    [ "$?" = 0 ] || exit 1
fi

#
# Build the rest of the build environment.
#
(cd bldenv; make -r ${BUILDVAR} build_all)
[ "$?" = 0 ] || exit 1

(cd bldenv; make -r ${BUILDVAR} install_all)
[ "$?" = 0 ] || exit 1

/usr/bin/echo "SETUP ended at `/usr/bin/date`"

chmod +x ${LOCPATH}/*

exit 0
