static char sccsid[] = "@(#)94	1.1  src/bos/usr/lbin/tty/mon-cxma/tables.c, sysxtty, bos411, 9428A410j 6/23/94 15:27:59";
/*
 * COMPONENT_NAME: (sysxtty) System Extension for tty support
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 */
#include "tables.h"

char *speeds[] = {
    "Speed = 2400  ",
    "Speed = 4800  ",
    "Speed = 9600  ",
    "Speed = 14.2K ",
    "Speed = 19.2K ",
    "Speed = 38.4K ",
    "Speed = 57.6K ",
    "Speed = 64K   ",
    "Speed = 76.8K ",
    "Speed = 115K  ",
    "Speed = 155K  ",
    "Speed = 230K  ",
    "Speed = 460K  ",
    "Speed = 920K  ",
    "Speed = 921K  ",
    "Speed = 1.2M  ",
    "Speed = 1.843M",
    "Speed = 2.458M",
    "Speed = 3.686M",
    "Speed = 7.373M",
    "Speed = 10M   ",
};

char *wires[] = {
    "4-wire ",
    "8-wire "
};

char *connections[] = {
    "Direct Connection",
    "Synchronous Connection"
};

char *clocks[] = {
    "internal clock,   ",
    "self-clocked,     ",
    "ext clock RS-422, ",
    "ext clock RS-232, "
};
    

struct table table[] = {
/*0*/	{S115K,  WIRE8, CLOCK_INTERNAL, DIRECT_CONNECT},
   	{S230K,  WIRE4, CLOCK_SELF, DIRECT_CONNECT},
   	{S460K,  WIRE4, CLOCK_SELF, DIRECT_CONNECT},

/*3*/ 	{S2400,  WIRE8, CLOCK_INTERNAL, DIRECT_CONNECT},
   	{S4800,  WIRE8, CLOCK_INTERNAL, DIRECT_CONNECT},
   	{S9600,  WIRE8, CLOCK_INTERNAL, DIRECT_CONNECT},
   	{S19_2K, WIRE8, CLOCK_INTERNAL, DIRECT_CONNECT},
   	{S38_4K, WIRE8, CLOCK_INTERNAL, DIRECT_CONNECT},
  	{S57_6K, WIRE8, CLOCK_INTERNAL, DIRECT_CONNECT},
   	{S76_8K, WIRE8, CLOCK_INTERNAL, DIRECT_CONNECT},
   	{S115K,  WIRE8, CLOCK_INTERNAL, DIRECT_CONNECT},
   	{S230K,  WIRE8, CLOCK_INTERNAL, DIRECT_CONNECT},
   	{S460K,  WIRE8, CLOCK_INTERNAL, DIRECT_CONNECT},
   	{S920K,  WIRE8, CLOCK_INTERNAL, DIRECT_CONNECT},
   	{S1_2M,  WIRE8, CLOCK_INTERNAL, DIRECT_CONNECT},

/*15*/ 	{S2400,  WIRE8, CLOCK_RS422, SYNC_CONNECT},
 	{S4800,  WIRE8, CLOCK_RS422, SYNC_CONNECT},
   	{S9600,  WIRE8, CLOCK_RS422, SYNC_CONNECT},
   	{S19_2K, WIRE8, CLOCK_RS422, SYNC_CONNECT},
   	{S38_4K, WIRE8, CLOCK_RS422, SYNC_CONNECT},
   	{S57_6K, WIRE8, CLOCK_RS422, SYNC_CONNECT},
   	{S76_8K, WIRE8, CLOCK_RS422, SYNC_CONNECT},
   	{S115K,  WIRE8, CLOCK_RS422, SYNC_CONNECT},
   	{S230K,  WIRE8, CLOCK_RS422, SYNC_CONNECT},
 	{S460K,  WIRE8, CLOCK_RS422, SYNC_CONNECT},
   	{S920K,  WIRE8, CLOCK_RS422, SYNC_CONNECT},
   	{S1_2M,  WIRE8, CLOCK_RS422, SYNC_CONNECT},

/*27*/ 	{-1}, {-1}, {-1}, {-1}, {-1}, {-1},

   	{S14_4K, WIRE8, CLOCK_RS422, SYNC_CONNECT},
   	{-1},

/*35*/ 	{S2400,  WIRE8, CLOCK_RS232, SYNC_CONNECT},
   	{S4800,  WIRE8, CLOCK_RS232, SYNC_CONNECT},
   	{S9600,  WIRE8, CLOCK_RS232, SYNC_CONNECT},
   	{S14_4K, WIRE8, CLOCK_RS232, SYNC_CONNECT},
   	{S19_2K, WIRE8, CLOCK_RS232, SYNC_CONNECT},
 	{S38_4K, WIRE8, CLOCK_RS232, SYNC_CONNECT},
   	{S57_6K, WIRE8, CLOCK_RS232, SYNC_CONNECT},
   	{S64K,   WIRE8, CLOCK_RS232, SYNC_CONNECT},
   	{S76_8K, WIRE8, CLOCK_RS232, SYNC_CONNECT},
/*44*/ 	{-1}, {-1}, {-1}, {-1},	{-1}, {-1}, {-1}, {-1},
/*52*/	{-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
/*60*/ 	{S115K,    WIRE4, CLOCK_SELF, DIRECT_CONNECT},
   	{S115K,    WIRE4, CLOCK_SELF, DIRECT_CONNECT},
   	{S230K,    WIRE4, CLOCK_SELF, DIRECT_CONNECT},
   	{S230K,    WIRE4, CLOCK_SELF, DIRECT_CONNECT},
 	{S460K,    WIRE4, CLOCK_SELF, DIRECT_CONNECT},
   	{S460K,    WIRE4, CLOCK_SELF, DIRECT_CONNECT},
   	{S921K,    WIRE4, CLOCK_SELF, DIRECT_CONNECT},
   	{S921K,    WIRE4, CLOCK_SELF, DIRECT_CONNECT},
   	{S1_843M,  WIRE4, CLOCK_SELF, DIRECT_CONNECT},
   	{S1_843M,  WIRE4, CLOCK_SELF, DIRECT_CONNECT},
/*70*/ 	{S1_843M,  WIRE8, CLOCK_INTERNAL, DIRECT_CONNECT},
   	{S2_458M,  WIRE8, CLOCK_INTERNAL, DIRECT_CONNECT},
	{S3_686M,  WIRE8, CLOCK_INTERNAL, DIRECT_CONNECT},
   	{S7_373M,  WIRE8, CLOCK_INTERNAL, DIRECT_CONNECT},
   	{S10M,     WIRE8, CLOCK_INTERNAL, DIRECT_CONNECT}
};
