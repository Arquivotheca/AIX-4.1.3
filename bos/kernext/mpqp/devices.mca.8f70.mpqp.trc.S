# @(#)98	1.1  src/bos/kernext/mpqp/devices.mca.8f70.mpqp.trc.S, sysxmpqp, bos411, 9436B411a 9/6/94 00:07:35
#
#   COMPONENT_NAME: SYSXMPQP
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
* /etc/trcfmt
+ 224 1.0 "MPQPDD" \
        $DD7


+ DD2 1.0 "" \
        "errno:" X1 "devno:" X4

* DD subroutine to display mpqp trace
+ DD7 1.0 "" \
        {{ $loc = X1 }} \
        $loc, \
        01  { "entry_open   :" \
                $DD2 "devflag:" X4 "chan:" X4 "p_ext:" X4 } , \
        02  { "exit_open    :" \
                $DD2 {{ $openerr = X4 }} } , \
        03  { "entry_close  :" \
                $DD2 "chan:" X4 "  " } , \
        04  { "exit_close   :" \
                $DD2 {{ $openerr = X4 }} } , \
        05  { "entry_read   :" $DD2 "bufptr:" X4 "chan:" X4 "ext:" X4 } , \
        06  { "exit_read    :" $DD2 "bufptr:" X4 "chan:" X4 "status:" X4  \
                {{ $openerr = X4 }} } , \
        07  { "entry_write  :" $DD2 "bufptr:" X4 "chan:" X4 "ext:" X4 } , \
        08  { "exit_write   :" $DD2 "bufptr:" X4 "chan:" X4 "status:" X4  \
                {{ $openerr = X4 }} } , \
        09  { "entry_ioctl  :" \
                $DD2 "op:" X4 "flag:" X4 "chan:" X4 "ext:" X4 } , \
        0A  { "exit_ioctl   :" \
                $DD2 {{ $openerr = X4 }} } , \
        0B  { "entry_select :" \
                $DD2 "events" X4 "chan:" X4 } , \
        0C  { "exit_select  :" \
                $DD2 "reventp:" X4 "chan:" X4  {{ $openerr = X4 }} } , \
        0D  { "entry_config :" $DD2 "op:" X4 } , \
        0E  { "exit_config  :" $DD2 {{ $openerr = X4 }} } , \
        11  { "entry_mpx    :" $DD2 } , \
        12  { "exit_mpx     :" \
                $DD2 "nameptr:" X4 "chan:" D4 "openflag:" X4 \
                {{$openerr = X4 }} } , \
        15  { "entry_intr   :" $DD2 } , \
        16  { "exit_intr    :" $DD2 "status:" X4 } , \
        19  { "entry_cstart :" \
                $DD2 "parm1:" X4 "parm2:" X4 "parm3:" X4 "parm4:" X4 } , \
        1A  { "exit_cstart  :" \
                $DD2 "parm#:" X4 "Parmval:" X4  {{ $suberr = X4 }} } , \
        21  { "ID:" O1 A4.8     \
                "d1=" X4 " d2=" X4 " d3=" X4 " d4=" X4 } , \
        70  { "entry_halt   :" $DD2 } , \
        71  { "exit_halt    :" $DD2 "status:" X4 } , \
        72  { "entry_getstat:" $DD2 "devflag:" X4 "chan:" X4 } , \
        73  { "exit_getstat :" \
                $DD2 "code:" X4 "opt[0]:" X4 "opt[1]" X4 "opt[2]" X4  } , \
        74  { "exit_kread   :" \
                $DD2 "openid:" X4 "status:" X4 "bufptr:" X4 } , \
        75  { "exit_kstat   :" \
                $DD2 "openid:" X4 "code:" X4 "opt[0]:" X4 opt[1] X4 } , \
        76  { "exit_ktx_fn   :" \
                $DD2 "openid:" X4 } , \
        77  { "entry_chgparm:" \
                $DD2 "rcv timer:" X4 "Poll addr:" X4 "Select Addr:" X4 } , \
        78  { "exit_chgparm :" \
                $DD2 } ,  \
        79  { "entry_start_ar:"\
                $DD2 } , \
        7A  { "exit_start_ar :"\
                $DD2 } , \
        7b  { "entry_flushport:"\
                $DD2 } , \
        7c  { "exit_flushport :"\
                $DD2 } , \
        7d  { "entry_adaptquery:"\
                $DD2 } , \
        7e  { "exit_adaptquery :"\
                $DD2 } , \
        7f  { "entry_query_stat:"\
                $DD2 } , \
        80  { "exit_query_stat :"\
                $DD2 } , \
        81  { "entry_stop_ar:"\
                $DD2 } , \
        82  { "exit_stop_ar :"\
                $DD2 } , \
        83  { "entry_trace_on:"\
                $DD2 } , \
        84  { "exit_trace_on :"\
                $DD2 } , \
        85  { "entry_stop_port:"\
                $DD2 } , \
        86  { "exit_stop_port :"\
                $DD2 } , \
        87  { "entry_traceoff:"\
                $DD2 } , \
        88  { "exit_traceoff :"\
                $DD2 } \
        $openerr, \
             0x000000f0 { "Suberror :Adapter number too big" }, \
             0x000000f1 { "Suberror :There is no ACB" }, \
             0x000000f2 { "Suberror :No offlevel intr. structure" }, \
             0x000000f3 { "Suberror :Cannot register interrupt" }, \
             0x000000f4 { "Suberror :No port dds " }, \
             0x000000f5 { "Suberror :Channel too big" }, \
             0x000000f6 { "Suberror :Channel busy" }, \
             0x000000f7 { "Suberror :No mbuf available" }, \
             0x000000f8 { "Suberror :No transmit chain" }, \
             0x000000f9 { "Suberror :Adapter already opened" }, \
             0x000000fa { "Suberror :Cannot set up POS REG" }, \
             0x000000fd { "Suberror :Error in uiomove" }, \
             0x000000fe { "Suberror :Port not open" }, \
             0x000000ff { "Suberror :Port not started" }, \
             0x00000100 { "Suberror :Pin code failed " }, \
             0x00000101 { "Suberror :Add entry failed in devswadd " }, \
             0x00000102 { "Suberror :Port already opened" }, \
             0x000000e3 { "Suberror :None" } \
        $suberr, \
             0x000000e0 { "Suberror : Physical link invalid" }, \
             0x000000e1 { "Suberror : Data protocol invalid" }, \
             0x000000e2 { "Suberror : Baud rate invalid" }, \
             0x000000e3 { "Suberror : None" } \
