static char sccsid[] = "@(#)00  1.1.1.1  src/bos/diag/tu/sky/a0f0.c, tu_sky, bos411, 9428A410j 10/29/93 15:33:56";
/*
 *   COMPONENT_NAME : (tu_sky) Color Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: a0f0
 *		combine
 *		get_device_num
 *		htoa
 *		reverse
 *		vpd_crc
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include "skytu.h"
#include <sys/types.h>
#include <sys/sysconfig.h>
#include <sys/devinfo.h>
#include <sys/ldr.h>
#include <sys/device.h>
#include <sys/stat.h>
#include <sys/intr.h>
#include <sys/errno.h>
#include <sys/display.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/mode.h>
#include <unistd.h>
#include <sys/types.h>
#include <nl_types.h>
#include <sys/cfgdb.h>
#include <sys/sysmacros.h>
#include <sys/devinfo.h>
#include <sys/device.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <odmi.h>
#define asciivpd      "VPD"

/**************************************************************************
* Name     : A0F0()                                                       *
* Function : Verify the Vital Product Data                                *
*                                                                         *
**************************************************************************/
a0f0()
{ int rc;
  byte jnkchar;
  lword  totlen;
  lword  crcval;
  lword  vram_address;
  byte   str_base_addr[12];
  lword  y;
  struct  cfg_dd          cfgdata;        /* Conifg DD ioctl arg  */
  char    qvpd_data[256];
  lword    maj_min[2];     /* major and minor device numbers */
  char    skyx[5];         /* skyway instance sky0,sky1,sky2,sky3 */

  do 
  {
    /* get major (maj_min[0]) and minor (maj_min[1]) device numbers */
    vram_address = getvbaseadd();
    vram_address &= 0x0FFFFFFF; /* clear out the segreg */
    htoa(vram_address, str_base_addr); /* convert hex address into string */
    rc = get_device_num(str_base_addr, maj_min);
    if (rc == -1) break;

    /* setup the configuration data structure */
    cfgdata.kmid   = 0;
    cfgdata.devno  = makedev(maj_min[0],maj_min[1]);
    cfgdata.cmd    = CFG_QVPD;
    cfgdata.ddsptr = (char *) qvpd_data;
    cfgdata.ddslen = 256;

    /* config ioctl to call the SKY DD config entry point */
    rc = sysconfig( SYS_CFGDD, &cfgdata, sizeof(struct cfg_dd) );
    if (rc < 0)
    {
      CatMsgs("Post sysconfig: rc="); CatMsgx(rc); CatMsgs("\n");
      CatMsgs("maj_min[0] = "); CatMsgx(maj_min[0]); CatMsgs("\n");
      CatMsgs("maj_min[1] = "); CatMsgx(maj_min[1]); CatMsgs("\n");
      CatErr(VPD | THISTUFAIL);
      return(VPD | THISTUFAIL);
    }

    jnkchar = qvpd_data[4];
    qvpd_data[4] = '\0';
    if (strcmp(asciivpd,qvpd_data+1) != 0)
    { CatMsgs("VPD Compare Failure:\n--Expected String = ");
      CatMsgs(asciivpd);
      CatMsgs("\n--String Received = ");
      CatMsgs(qvpd_data);
      CatErr(VPD | THISTUFAIL);
      return(VPD | THISTUFAIL);
    }
    qvpd_data[4] = jnkchar;
   

    /* Get the Total VPD Length */
    totlen = qvpd_data[4] * 0x100 + qvpd_data[5];
    totlen *= 2; 

    /* Get the CRC Value */
    crcval = qvpd_data[6] * 0x100 + qvpd_data[7];

    /* Do a CRC Verification on the VPD Information */
      rc = (word) vpd_crc(qvpd_data+8, totlen);
  } while (FALSE);



  if (rc  == 0xFFFF)
  { /* ODM error from get_device_num() */
    CatErr(VPD | SYS_FAILUR);
    return(VPD | SYS_FAILUR);
  }
  else if (rc  != ((word)crcval))
  { 
    CatMsgs("Vital Product Data CRC Compare Failure:\n--CRC Calculated = 0x");
    CatMsgx(rc);
    CatMsgs("\n--CRC Read From VPD = 0x");
    CatMsgx(crcval);
    CatMsgs("\n");
    CatErr(VPD | THISTUFAIL);
    return(VPD | THISTUFAIL);
  }
  else 
  { rc = GOOD_OP;  /* to clear rc of vpd_crc() call */
  }

  if (rc > ERR_LEVEL)
  { CatErr(VPD | SUBTU_FAIL);
    return(VPD | SUBTU_FAIL);
  }
  else
  { CatErr(VPD | GOOD_OP);
    return(VPD | GOOD_OP);
  }
}

/**************************************************************************
* Name     : vpd_crc()                                                    *
* Function : Does a CRC on Vital Product Data                             *
*            (as defined in 'RIOS System Architecture: VPD; Version 1.62) *
*                                                                         *
**************************************************************************/
#define crc_mask 0xFF07
vpd_crc(pbuff, length)
char pbuff[];
int  length;
{ union acccum
  { short  whole;
    struct bites
    { char msb;
      char lsb;
    } bite;
  } avalue, dvalue;
  char datav;
  int i;
  dvalue.whole = 0xFFFF;

  for (i=0; length>0; i++, length--)
  { 
    datav = *(pbuff+i);
    avalue.bite.lsb = (datav ^ dvalue.bite.lsb);
    dvalue.bite.lsb = avalue.bite.lsb;
    avalue.whole    = ((avalue.whole * 16) ^ dvalue.bite.lsb);
    dvalue.bite.lsb= avalue.bite.lsb;
    avalue.whole   <<= 8;

    avalue.whole   >>= 1;
    avalue.bite.lsb ^= dvalue.bite.lsb;
    avalue.whole   >>= 4;

    avalue.whole     = combine(avalue.bite.lsb, avalue.bite.msb);
    avalue.whole     = ((avalue.whole & crc_mask) ^ dvalue.bite.lsb);
    avalue.whole     = combine(avalue.bite.lsb, avalue.bite.msb);
    avalue.bite.lsb ^= dvalue.bite.msb;
    dvalue.whole   = avalue.whole;
  } /* end for i */
  return(dvalue.whole);
} /* end vpd_crc */




combine(bite1, bite2)
char bite1,bite2;
{ short rc;
  rc = ((bite1 << 8) | bite2);
  return(rc);
}


get_device_num(sbase_addr,maj_min)
char sbase_addr[];   /*Address of the device in character string format */
lword maj_min[];
{ char *attributes[] =  { "bus_addr_start", "bus_mem_start", "vram_start",
	               	  "dma1_start", "dma2_start", "dma3_start",
		          "dma4_start", "dma_channel", "int_level",
		          "int_priority", ""
			};

  char   *prefix = "gda";
  char	 sstring[MAX_CRITELEM_LEN];  /* search string */
  struct CuDv  *cudv;            /* ptr to customized device Class */
  struct listinfo cudv_info;
  struct CuAt *cuat;
  struct CuDvDr cudvdr;
  /*struct Class	*cudv_class;	** ptr to Class object */
  /*struct Class	*cuat_class;	** ptr to Class object */
  int    rc;			/* return code */
  int    how_many;
  int	 i,selection;
 
  sprintf(tinfo.info.msg_buff,"In get_device_num..............\n");
  /* Initialize the ODM.  */
  if ( odm_initialize() < 0 )
  { sprintf(tinfo.info.msg_buff,"ODM initialization failed");
    odm_terminate();
    return(-1);
  }

  /* Find all devices matching the prefix */
  sprintf(sstring,"name LIKE %s*", prefix); 
  if ((cudv = odm_get_list(CuDv_CLASS,sstring,&cudv_info,1,1)) < 0)
  { sprintf(tinfo.info.msg_buff,"name LIKE %s*\n", prefix); 
    CatMsgs("odm_get_list failed");
    odm_terminate();
    return(-1);
  }
  /*If number of objects returned is zero then no "prefix" devices found*/
  if (!cudv_info.num)
  { sprintf(tinfo.info.msg_buff,"name LIKE %s*\n", prefix); 
    CatMsgs("No ODM skyway devices found: cudv_info.num = "); 
    CatMsgx(cudv_info.num); 
    CatMsgs("\n");
    odm_terminate(); 
    return(-1); 
  }

  /* Now match the vram address with the existing devices to select
     the skyway and get the major/minor number
  */
  selection = 2;     /*  We need vram_mem_start attribute */
  rc = -1;	   /*  Set return code to failed 	*/
  for(i=0;i<cudv_info.num;i++,cudv++)
  { 
    /* if status = 0 skep device that may have been removed */
    if (cudv->status != AVAILABLE) continue;

    cuat = getattr(cudv->name,attributes[selection],FALSE,&how_many);
    if(cuat == NULL)
    { sprintf(tinfo.info.msg_buff,"Couldn't get Customized Attributes");
      odm_terminate(); 
      return(-1); 
    }
    if(!strcmp(cuat->value,sbase_addr))
    { sprintf(tinfo.info.msg_buff,
	      "ODM VRAM Addresses Don't Match in A0F0");
      odm_terminate(); 
      return (-1);
    }
    sprintf(sstring,"resource = devno and value3 = %s",cudv->name);
    if((odm_get_first(CuDvDr_CLASS,sstring,&cudvdr)) < 0)
    { CatMsgs("Couldn't get CuDvDr information on Pass");
      CatMsgx(i+1);
      CatMsgs(" of ");
      CatMsgx(cudv_info.num);
      CatMsgs("\n");
      CatMsgs("CuDvDr_CLASS=");
      CatMsgs("sstring=");
      CatMsgs(sstring);
      CatMsgs("\n");
    }
    else
    { rc = 0;
      CatMsgs("Found CuDvDr information on Pass");
      CatMsgx(i+1);
      CatMsgs(" of ");
      CatMsgx(cudv_info.num);
      CatMsgs("\n");
      odm_terminate();
      break;
    }
  }
  if( rc != 0)
  { CatMsgs("--Failed to find device match for vram_start on ");
    CatMsgx(i);
    CatMsgs(" passes of ");
    CatMsgx(cudv_info.num);
    CatMsgs("\n");
    return(-1);
  }

  maj_min[0] = atoi(cudvdr.value1); 
  maj_min[1] = atoi(cudvdr.value2); 
  /*
  sprintf(tinfo.info.msg_buff,
          "logical name = %s\n  major devno = %s\n  minor devno = %s\nlogical name = %s\n  maj_min0 = %d\n  maj_min1 = %d\n=================================\n",
           cudv->name, cudvdr.value1, cudvdr.value2,
           cudv->name, maj_min[0], maj_min[1]);
  */
  return(0);
}

htoa(n,s)
ulong n;
char *s;
{
int i,sign;
static tr[16] = { '0','1','2','3','4','5','6','7','8','9',
		  'a','b','c','d','e','f'};

  if ((sign=n) < 0)
    n = -n;

  i = 0;
  do {

    s[i++] = tr[n % 16];

  } while ((n/=16) > 0);

  if (sign < 0)
    s[i++] = '-';

  s[i] = '\0';
  reverse(s);
}

reverse(s)
char s[];
{
int c,i,j;

  for (i=0,j=strlen(s)-1; i<j; i++,j--) {
    c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
}
