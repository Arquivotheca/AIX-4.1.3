/* @(#)61       1.10  src/bos/diag/tu/sun/suntu.h, tu_sunrise, bos411, 9437A411a 7/20/94 17:13:40 */
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: MSG
 *              MSG2
 *              MSG3
 *              MSG4
 *              MSG5
 *              MSGn1
 *              MSGn2
 *              MSGn3
 *              MSGn4
 *              MSGn5
 *              MSGn5SUM
 *              msg_menu
 *              nkMSG3
 *              nkMSG5
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/****************************************************************************
        File:   suntu.h
*****************************************************************************/

#define    COMERR         99
#define TRUE    1
#define FALSE   0
#define msg_menu(a,b,c,d,e,f) \
{       if (no_msgmenu>TRUE) msgout (a,b,c,d,e,f); \
        else if (no_msgmenu) msgfile(a,b,c,d,e,f); \
        else                 msgmenu(a,b,c,d,e,f); \
}

short no_msgmenu=1;                     /* default for command history file is OFF                      */

#define MSG1(str)                                                \
        { msg_menu(str,str,str,str,str,str);}
#define MSG2(s1,s2)                                     \
        { msg_menu(s1,s2,"","","","");}
#define MSG3(s1,s2,s3)                                  \
        { msg_menu(s1,s2,s3,"","","");}
#define MSG4(s1,s2,s3,s4)                                       \
        { msg_menu(s1,s2,s3,s4,"","");}
#define MSG5(s1,s2,s3,s4,s5)                            \
        { msg_menu(s1,s2,s3,s4,s5,"");}
#define MSGn1(s1,n1)                                            \
        { char ss1[50];                                 \
        sprintf(ss1,"%s=0x%X",s1,n1);                   \
        msg_menu(ss1,ss1,ss1,ss1,ss1,ss1);}
#define MSGn2(s1,n1,s2,n2)                                      \
        { char ss1[50],ss2[50];                         \
         sprintf(ss1,"%s=0x%X",s1,n1);                  \
        sprintf(ss2,"%s=0x%X",s2,n2);                   \
        msg_menu(ss1,ss2,"","","","");}
#define MSGn3(s1,n1,s2,n2,s3,n3)                        \
        { char ss1[50],ss2[50],ss3[50];                 \
        sprintf(ss1,"%s=0x%X",s1,n1);                   \
        sprintf(ss2,"%s=0x%X",s2,n2);                   \
        sprintf(ss3,"%s=0x%X",s3,n3);                   \
        msg_menu(ss1,ss2,ss3,"","","");}
#define MSGn4(s1,n1,s2,n2,s3,n3,s4,n4)                  \
        { char ss1[50],ss2[50],ss3[50],ss4[50];         \
        sprintf(ss1,"%s=0x%X",s1,n1);                   \
        sprintf(ss2,"%s=0x%X",s2,n2);                   \
        sprintf(ss3,"%s=0x%X",s3,n3);                   \
        sprintf(ss4,"%s=0x%X",s4,n4);                   \
        msg_menu(ss1,ss2,ss3,ss4,"","");}
#define MSGn5(s1,n1,s2,n2,s3,n3,s4,n4,s5,n5)            \
        { char ss1[50],ss2[50],ss3[50],ss4[50],ss5[50]; \
        sprintf(ss1,"%s=0x%X",s1,n1);                   \
        sprintf(ss2,"%s=0x%X",s2,n2);                   \
        sprintf(ss3,"%s=0x%X",s3,n3);                   \
        sprintf(ss4,"%s=0x%X",s4,n4);                   \
        sprintf(ss5,"%s=0x%X",s5,n5);                   \
        msg_menu(ss1,ss2,ss3,ss4,ss5,"");}

#define MSGn5SUM(s0,s1,n1,s2,n2,s3,n3,s4,n4,s5,n5)              \
        { char ss0[50],ss1[50],ss2[50],ss3[50],ss4[50],ss5[50]; \
        sprintf(ss0,"CMDFILE=%s",s0);                   \
        sprintf(ss1,"%s=%s",s1,((n1==0xA)?"FB-A":((n1==0xB)?"FB-B":((n1==0xC)?"WID/OV":"CLIPPING"))));                  \
        sprintf(ss2,"%s=0x%X",s2,n2);                   \
        sprintf(ss3,"%s=0x%X",s3,n3);                   \
        sprintf(ss4,"%s=0x%X",s4,n4);                   \
        sprintf(ss5,"%s=0x%X",s5,n5);                   \
        msg_menu(ss0,ss1,ss2,ss3,ss4,ss5);}

#define nkMSG5(s1,s2,s3,s4,s5)                          \
        { nokeymenu(s1,s2,s3,s4,s5);}
#define nkMSG3(s1,s2,s3)                                \
        { nokeymenu(s1,s2,s3,"","");}

/* menu selection ID's */
#define CMDFILE         '!'     /* 0x21 */
#define FKEYCMD         '@'     /* 0x40 */
#define BLmenu          '+'     /* 0x2B */
#define PAXmenu         '-'     /* 0x2D */
#define MImenu          '&'     /* 0x2A */
#define MCmenu          '.'     /* 0x2A */
#define LOmenu          '#'     /* 0x2A */
#define CDmenu          '*'     /* 0x2A */
#define SCmenu          '('     /* 0x2A */
#define FEmenu          ')'     /* 0x2A */
#define VPmenu          '%'     /* 0x2A */
#define ABmenu          '$'     /* 0x2A */

/* for SUNRISE MENUs */
#define MCIO            '0'     /* 0x30 */
#define MIAMIREGS       '1'     /* 0x31 */
#define LOCALREGS       '2'     /* 0x32 */
#define CODECREGSMEM    '3'     /* 0x33 */
#define CODECREGS       '4'     /* 0x34 */
#define CODECINITMENU   '5'     /* 0x35 */
#define PX2070REGSMENU  '6'     /* 0x36 */
#define FRONTEND        '7'     /* 0x37 */
#define VPD             '8'     /* 0x38 */
#define SUNTU           '9'     /* 0x39 */

/* SUNRISE DEFINES ... */
/* Address Map for Sunrise */
#define MIbaseaddr  0x1FFA0000      /* for Miami     */
#define CDbaseaddr  0x1DFF0000      /* for CoDec     */
#define FEbaseaddr  0x18F80000      /* for Front-End */
#define VPbaseaddr  0x1FFF0000      /* for VPD       */
#define SCbaseaddr  0x1EFF0000      /* for Scalar    */
#define DMAbaseaddr 0x2FE00000      /* for DMA Port  */
#define HIU_0 (SCbaseaddr + 0x00)
#define HIU_1 (SCbaseaddr + 0x04)
#define HIU_2 (SCbaseaddr + 0x08)
#define HIU_3 (SCbaseaddr + 0x0c)
#define HIU_4 (SCbaseaddr + 0x10)
#define HIU_CSU HIU_0
#define HIU_OCS HIU_1
#define HIU_RIN HIU_2
#define HIU_RDT HIU_3
#define HIU_MDT HIU_4

/* Local Bus Registers */
#define _LBStatus   0x1CFF        /* for Local bus status register      */
#define LBStatus    0x1CFF0000    /* for Local bus status register      */
#define _LBControl  0x1BFF        /* for Local bus control register     */
#define LBControl   0x1BFF0000    /* for Local bus control register     */
#define CNTreg      0x1DE00000    /* for Word Count register            */

/* Front-end PCD8584 Registers */
#define _IIC_DATA   0x0
#define _IIC_STATUS 0x4

/* Miami microchannel register access */
#define _COMMAND     0x0000
#define _ATTN        0x0004
#define _SCP         0x0005
#define _GAID        0x000a
#define _HSBR        0x000c
#define _MDATA       0x0010
#define _CONF1       0x0014
#define _CONF2       0x0018
#define _CONF3       0x001C

/* Miami local register access */
#define _POS_Setup1  0x0000
#define _POS_Setup2  0x0004
#define _CRDID       0x000C
#define _PROC_CFG    0x0010
#define _RSR         0x0014
#define _XPOS        0x0018
#define _LBPE        0x0020
#define _CBSP        0x2010
#define _CAR1        0x3000
#define _SAR1        0x3004
#define _BCR1        0x3008
#define _CCR1        0x300C
#define _BMAR1       0x3010
#define _LAP1        0x3014
#define _BMSTAT1     0x3018
#define _BMCMD1      0x301C
#define _CAR2        0x4000
#define _SAR2        0x4004
#define _BCR2        0x4008
#define _CCR2        0x400C
#define _BMAR2       0x4010
#define _LAP2        0x4014
#define _BMSTAT2     0x4018
#define _BMCMD2      0x401C

#define         INTPOS0         0x96
#define         INTPOS1         0x8F
#define         NUPATTERN       4

#ifdef  CL560B  /* if card has CCUBE-560 chip */
   #define  CODECHUFFYDCMIN        0xE600
   #define  CODECHUFFYDCMAX        0xE65C
   #define  CODECHUFFCDCMIN        0xEE00
   #define  CODECHUFFCDCMAX        0xEE5C
   #define  CODECHUFFYACMIN        0xE000
   #define  CODECHUFFYACMAX        0xE5FC
   #define  CODECHUFFCACMIN        0xE800
   #define  CODECHUFFCACMAX        0xEDFC
#else           /* if card has CCUBE-550 chip */
   #define  CODECHUFFYDCMIN        0xEC00
   #define  CODECHUFFYDCMAX        0xEC7C
   #define  CODECHUFFCDCMIN        0xFC00
   #define  CODECHUFFCDCMAX        0xFC7C
   #define  CODECHUFFYACMIN        0xE000
   #define  CODECHUFFYACMAX        0xEAFC
   #define  CODECHUFFCACMIN        0xF000
   #define  CODECHUFFCACMAX        0xFAFC
#endif

#define  CODECQUANTMIN          0xB800
#define  CODECQUANTMAX          0xBBFC
#define  CODECINITREGSMIN       0x9008
#define  CODECREGSMIN           0x0000
#define  CODECREGSMAX           0x7FFC
#define  VPDBASE_MIN            0x0000
#define  VPDCODEC_MIN           0x0400
#define  VPDREGSMAX             0x07FF

#define     CODEC_NOTPRESENT    0x200   /* EOI mask bit of LBStatus register */
                                        /* for checking CODEC card presents  */

/* External define */
int pio_write(unsigned int addr, unsigned int data, int tl);
int pio_read(unsigned int addr, unsigned int *data, int tl);
int scalar_write(unsigned int addr, unsigned int data, int tl);
int scalar_read(unsigned int addr, unsigned int *data, int tl);
int pio_mcwrite(unsigned int addr, unsigned int data, int tl);
int pio_mcread(unsigned int addr, unsigned int *data, int tl);
int  write2chip(char chip, unsigned int reg, unsigned int val,int tl);          /* writes (reg)=val for pax,TB,BL(VorW) */
int  readchip(char chip, unsigned int val, int tl, unsigned int *data);           /* reads val=(reg_  for pax,TB,BL(rorR) */
unsigned int lswap (unsigned int num);
