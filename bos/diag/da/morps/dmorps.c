static char sccsid[] = "@(#)62  1.8.4.4  src/bos/diag/da/morps/dmorps.c, damorps, bos41J, 9517B_all 4/25/95 17:38:33";
/*
 *   COMPONENT_NAME: DAMORPS
 *
 *   FUNCTIONS: bridge_box
 *              chk
 *              cleanup
 *              de_adapter
 *              disp_menu
 *              main
 *              portable
 *              serial_disk1
 *              serial_disk2
 *              serial_disk2_3
 *              retrn_frub
 *              retrn_goal
 *              system_unit
 *		getDiskDrawer
 *		getMediaDrawer
 *		expansion_7013
 *		disk_drawer_7134
 *		tower_7131
 *
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <nl_types.h>
#include <limits.h>
#include <locale.h>
#include <sys/buf.h>
#include <sys/devinfo.h>
#include <sys/cfgodm.h>
#include <memory.h>

#include <diag/diago.h>
#include <diag/tm_input.h>
#include <diag/tmdefs.h>               /* diagnostic modes and variables  */
#include <diag/da.h>                   /* FRU Bucket structure definition */
#include <diag/diag_exit.h>                            /* DA return codes */
#include "morps.h"
#include "morps_msg.h"
#include <diag/dcda_msg.h>

struct fru_bucket frub[] =
{					/* Adapter/device failure */
        { "", FRUB_ENCLDA, 0x950 , 0 , 0 ,	/* DEVICE_ADAP */
		{
			{ 90, "","",0,DA_NAME,NONEXEMPT},
			{ 10, "","",0,PARENT_NAME,EXEMPT},
		},
	},
					/* Device failure */
	{ "", FRUB_ENCLDA, 0x950 , 0 , 0 ,	/* HDA_TYPE */
		{
			{ 70, "de","",DEC,0,0},
			{ 20, "","",0,PARENT_NAME,EXEMPT},
			{ 10,"hda","",HDA,0,0},
		},
	},
					/* Power supply failure */
	{ "", FRUB_ENCLDA, 0x950 , 0 , 0 ,	/* POWR_SUPPLY */
		{
			{100,"power supply","",P_SUPPLY,0,0},
		},
	},
					/* Harrier 1 fan/power supply */
	{ "", FRUB_ENCLDA, 0x950 , 0 , 0 ,	/* frubTypeFanPowerSupply */
		{
			{90, "fan","",FANS,0,0},
			{10, "power supply","", P_SUPPLY,0,0 },
		},
	},

	{ "", FRUB_ENCLDA, 0x950 , 0 , 0 ,	/* frubTypeExtDiskette */
		{
			{ 90, "drive","",DRIVE,0,0},
			{ 10, "","",0,PARENT_NAME,EXEMPT},
		}
	},

	{ "", FRUB_ENCLDA, 0 , 0 , 0 ,	/* HARR2_SRN_1 */
		NULL
	},

	{ "", FRUB_ENCLDA, 0 , 0 , 0 ,	/* HARR2_SRN_2 */
		{
			{100, "","",0,DA_NAME,EXEMPT},
		}
	},

        { "", FRUB_ENCLDA, 0 , 0 , 0 ,	/* frubType7013PowerSupply */
		{
			{85,"power supply","",fMsgExpPowerSupply,0,0},
			{10,"relay unit","",fMsgRelay,0,0},
			{ 5,"fan","",fMsgFanAssembly,0,0},
		}
        },

        { "", FRUB_ENCLDA, 0 , 0 , 0 ,	/* frubTypeAcFanAssembly */
		{
			{100, "fan","",fMsgAcFanAssembly,NOT_IN_DB,EXEMPT},
		}
        },

        { "", FRUB_ENCLDA, 0 , 0 , 0 ,	/* frubTypeDcFanAssembly */
		{
			{100, "fan","",fMsgDcFanAssembly,NOT_IN_DB,EXEMPT},
		}
        },

        { "", FRUB_ENCLDA, 0 , 0 , 0 ,	/* frubTypeScsiBus */
		{
			{100, "SCSI bus","",fMsgScsiBus,NOT_IN_DB,EXEMPT},
		}
        }
};

void getDiskDrawer();
void getMediaDrawer();
void expansion_7013();
void disk_drawer_7134();
void serial_disk1();
void system_unit();
void bridge_box();
void portable();
void serial_disk2();
void tower_7131();

enum {devMissHdisk, devMissDskt, devMissMedia, devMissNum};

typedef int (*TestFunc)();

/*>>>IMPORTANT!! keep the function lists argreed with the displayed menus<<<*/

#define MENU_MAX_ENTRY 10
TestFunc funcEncl[][MENU_MAX_ENTRY] = {
	/* devMissHdisk */
	{system_unit, bridge_box, getDiskDrawer, portable, expansion_7013},
	/* devMissDskt */
	{system_unit, bridge_box, serial_disk1},
	/* devMissMedia */
	{system_unit, bridge_box, getMediaDrawer, expansion_7013},
};

TestFunc funcDiskEnclDrawer[] = {
	tower_7131,
	disk_drawer_7134,
	serial_disk1
};

TestFunc funcMediaEnclDrawer[] = {
	tower_7131,
	serial_disk1
};

struct msglist menuEncl[][MENU_MAX_ENTRY] = {
        {	/* devMissHdisk */
        	{MORPS,  msgEnclMenuHead },
                {MORPS,  msgEnclCPUEnclosure },
                {MORPS,  msgEnclBridgeBox },
                {MORPS,  msgEnclDiskDevDrawerTower },
                {MORPS,  msgEnclPortable },
                {MORPS,  msgEncl7013 },
                {MORPS,  Choose        },
                NULL
	},
        {	/* devMissDskt */
        	{MORPS,  msgEnclMenuHead },
                {MORPS,  msgEnclCPUEnclosure },
                {MORPS,  msgEnclBridgeBox },
                {MORPS,  msgEnclDevDrawerTower },
                {MORPS,  Choose        },
                NULL
	},
        {	/* devMissMedia */
        	{MORPS,  msgEnclMenuHead },
                {MORPS,  msgEnclCPUEnclosure },
                {MORPS,  msgEnclBridgeBox },
                {MORPS,  msgEnclDevDrawerTower },
                {MORPS,  Choose        },
                NULL
	}
};

struct msglist menuDiskEnclDrawer[] =
	{	{MORPS,  msgEnclMenuHead },
		{MORPS,  msgEncl7131 },
		{MORPS,  msgEncl7134 },
                {MORPS,  msgEnclOther },
                {MORPS,  Choose        },
                NULL
	};

struct msglist menuMediaEnclDrawer[] =
	{	{MORPS,  msgEnclMenuHead },
		{MORPS,  msgEncl7131 },
                {MORPS,  msgEnclOther },
                {MORPS,  Choose        },
                NULL
	};

struct msglist opening_msg[ ] =
	{	{MORPS,  LOCATION      },
		{MORPS,  Yes_option    },
                {MORPS,  No_option     },
                {MORPS,  Choose        },
                NULL
	};

struct msglist bridge_1[ ] =
	{	{MORPS,  BRIDGE_MENU_1  },
                {MORPS,  Yes_option    },
                {MORPS,  No_option     },
                {MORPS,  Choose        },
                NULL
	};

struct msglist bridge_2[ ] =
	{	{MORPS,  BRIDGE_MENU_2  },
                {MORPS,  Yes_option    },
                {MORPS,  No_option     },
                {MORPS,  Choose        },
                NULL
	};

struct msglist bridge_3[ ] =
	{	{MORPS,  BRIDGE_MENU_3  },
                {MORPS,  Yes_option    },
                {MORPS,  No_option     },
                {MORPS,  Choose        },
                NULL
	};

struct msglist bridge_4[ ] =
	{	{MORPS,  BRIDGE_MENU_4  },
                {MORPS,  Yes_option    },
                {MORPS,  No_option     },
                {MORPS,  Choose        },
                NULL
	};

struct msglist bridge_5[ ] =
	{	{MORPS,  BRIDGE_MENU_5  },
                {MORPS,  Yes_option    },
                {MORPS,  No_option     },
                {MORPS,  Choose        },
                NULL
	};
struct msglist bridge_6[ ] =
	{	{MORPS,  BRIDGE_MENU_6  },
                {MORPS,  Yes_option    },
                {MORPS,  No_option     },
                {MORPS,  Choose        },
                NULL
	};

struct msglist bridge_7[ ] =
	{	{MORPS,  BRIDGE_MENU_7  },
                {MORPS,  Yes_option    },
                {MORPS,  No_option     },
                {MORPS,  Choose        },
                NULL
	};

struct msglist bridge_8[ ] =
	{	{MORPS, BRIDGE_MENU_8   },
                {MORPS, Yes_option      },
                {MORPS, No_option       },
                {MORPS, Choose          },
                NULL
	};

struct msglist harrier_1[ ] =
       {	{MORPS,  HARR1_MENU_1A },
                {MORPS,  Yes_option    },
                {MORPS,  No_option     },
                {MORPS,  Choose        },
                NULL
       };

struct msglist harrier_2[ ] =
       {	{MORPS,  HARR1_MENU_2  },
                {MORPS,  Yes_option    },
                {MORPS,  No_option     },
                {MORPS,  Choose        },
                NULL
       };

struct msglist harrier_3[ ] =
       {	{MORPS,  HARR1_MENU_3A },
                {MORPS,  Yes_option    },
                {MORPS,  No_option     },
                {MORPS,  Choose        },
                NULL
       };

struct msglist harrier_4[ ] =
       {	{MORPS,  HARR1_MENU_4A },
                {MORPS,  Yes_option    },
                {MORPS,  No_option     },
                {MORPS,  Choose        },
                NULL
       };

struct msglist harrier_5[ ] =
       {	{MORPS,  HARR1_MENU_5A },
                {MORPS,  Yes_option    },
                {MORPS,  No_option     },
                {MORPS,  Choose        },
                NULL
       };

struct msglist harrier_6[ ] =
       {	{MORPS,  HARR1_MENU_6  },
                {MORPS,  Yes_option    },
                {MORPS,  No_option     },
                {MORPS,  Choose        },
                NULL
       };

struct msglist harrier_7[ ] =
       {	{MORPS,  HARR1_MENU_7  },
                {MORPS,  Yes_option    },
                {MORPS,  No_option     },
                {MORPS,  Choose        },
                NULL
       };

/* Harrier2 controller messages */

struct msglist harr2_C0[ ] =
        {	{MORPS, HARR2_MENU_C0 },
                {MORPS, Yes_option    },
                {MORPS, No_option     },
                {MORPS, Choose        },
                NULL
        };

struct msglist harr2_C1[ ] =
        {	{MORPS, HARR2_MENU_C1 },
                {MORPS, Yes_option    },
                {MORPS, No_option     },
                {MORPS, Choose        },
                NULL
        };

struct msglist harr2_C2[ ] =
        {	{MORPS, HARR2_MENU_C2 },
                {MORPS, Yes_option    },
                {MORPS, No_option     },
                {MORPS, Choose        },
                NULL
        };

struct msglist harr2_C3[ ] =
        {	{MORPS, HARR2_MENU_C3 },
                {MORPS, Yes_option    },
                {MORPS, No_option     },
                {MORPS, Choose        },
                NULL
        };

struct msglist harr2_C4[ ] =
        {	{MORPS, HARR2_MENU_C4 },
                {MORPS, Yes_option    },
                {MORPS, No_option     },
                {MORPS, Choose        },
                NULL
        };

struct msglist harr2_C5[ ] =
        {	{MORPS, HARR2_MENU_C5 },
                {MORPS, Yes_option    },
                {MORPS, No_option     },
                {MORPS, Choose        },
                NULL
        };

struct msglist harr2_C6[ ] =
        {	{MORPS, HARR2_MENU_C6 },
                {MORPS, Yes_option    },
                {MORPS, No_option     },
                {MORPS, Choose        },
                NULL
        };



/* Harrier2 DASD messages */

struct msglist harr2_D0[ ] =
        {	{MORPS, HARR2_MENU_D0 },
                {MORPS, Yes_option    },
                {MORPS, No_option     },
                {MORPS, Choose        },
                NULL
        };

struct msglist harr2_D1[ ] =
        {	{MORPS, HARR2_MENU_D1 },
                {MORPS, Yes_option    },
                {MORPS, No_option     },
                {MORPS, Choose        },
                NULL
        };

struct msglist harr2_D2[ ] =
        {	{MORPS, HARR2_MENU_D2 },
                {MORPS, Yes_option    },
                {MORPS, No_option     },
                {MORPS, Choose        },
                NULL
        };

struct msglist harr2_D3[ ] =
        {	{MORPS, HARR2_MENU_D3 },
                {MORPS, Yes_option    },
                {MORPS, No_option     },
                {MORPS, Choose        },
                NULL
        };

struct msglist harr2_D4[ ] =
        {	{MORPS, HARR2_MENU_D4 },
                {MORPS, Yes_option    },
                {MORPS, No_option     },
                {MORPS, Choose        },
                NULL
        };

struct msglist portable_1[ ] =
       {	{MORPS,  PORTA_MENU_1  },
                {MORPS,  Yes_option    },
                {MORPS,  No_option     },
                {MORPS,  Choose        },
               	NULL
       };

struct msglist portable_2[ ] =
       {	{MORPS,  PORTA_MENU_2  },
                {MORPS,  Yes_option    },
                {MORPS,  No_option     },
                {MORPS,  Choose        },
                NULL
       };

struct msglist portable_3[ ] =
       {	{MORPS,  PORTA_MENU_3  },
                {MORPS,  Yes_option    },
                {MORPS,  No_option     },
                {MORPS,  Choose        },
                NULL
       };

struct msglist portable_4[ ] =
       {	{MORPS,  PORTA_MENU_4  },
                {MORPS,  Yes_option    },
                {MORPS,  No_option     },
                {MORPS,  Choose        },
                NULL
       };

struct msglist portable_5[ ] =
       {	{MORPS,  PORTA_MENU_5  },
                {MORPS,  Yes_option    },
                {MORPS,  No_option     },
                {MORPS,  Choose        },
                NULL
       };

struct msglist portable_6[ ] =
       {	{MORPS,  PORTA_MENU_6  },
                {MORPS,  Yes_option    },
                {MORPS,  No_option     },
                {MORPS,  Choose        },
                NULL
       };
struct msglist gen_men1[ ] =
        {	{MORPS, WHERE_R_YOU1    },
                {MORPS, Yes_option      },
                {MORPS, No_option       },
                {MORPS, Choose          },
                NULL
        };
struct msglist gen_men2[ ] =
        {	{MORPS, WHERE_R_YOU     },
                {MORPS, DASD_DRAWER     },
                {MORPS, SYS_UNIT        },
                {MORPS, Choose          },
                NULL
        };
struct msglist gen_men3[ ] =
        {	{MORPS, WHERERYOU_2A   },
                {MORPS, Yes_option      },
                {MORPS, No_option       },
                {MORPS, Choose          },
                NULL
        };
/* -------------------------------------------------------------------------*/
/* >>> 7134 menus <<< */
struct msglist menu7134PowerSwitch[ ] =
        {	{MORPS, msg7134PowerSwitch},
                {MORPS, Yes_option},
                {MORPS, No_option},
                {MORPS, Choose},
                NULL
        };
struct msglist menu7134PowerIndicator[ ] =
        {	{MORPS, msg7134PowerIndicator},
                {MORPS, Yes_option},
                {MORPS, No_option},
                {MORPS, Choose},
                NULL
        };
struct msglist menu7134FanPowerSupplyIndicator[ ] =
        {	{MORPS, msg7134FanPowerSupplyIndicator},
                {MORPS, Yes_option},
                {MORPS, No_option},
                {MORPS, Choose},
                NULL
        };
struct msglist menu7134PowerCord[ ] =
        {	{MORPS, msg7134PowerCord},
                {MORPS, Yes_option},
                {MORPS, No_option},
                {MORPS, Choose},
                NULL
        };
struct msglist menu7134FanIndicator[ ] =
        {	{MORPS, msg7134FanIndicator},
                {MORPS, Yes_option},
                {MORPS, No_option},
                {MORPS, Choose},
                NULL
        };
struct msglist menu7134CheckIndicator[ ] =
        {	{MORPS, msg7134CheckIndicator},
                {MORPS, Yes_option},
                {MORPS, No_option},
                {MORPS, Choose},
                NULL
        };
/* -------------------------------------------------------------------------*/
/* >>> 7013 menus <<< */
struct msglist menu7013PowerIndicator[ ] =
        {	{MORPS, msg7013PowerIndicator},
                {MORPS, Yes_option},
                {MORPS, No_option},
                {MORPS, Choose},
                NULL
        };
/* -------------------------------------------------------------------------*/
/* >>> menugoals <<< */
struct msglist men_goals[]=
        {	{MORPS_M, BRIDGE_MENU_GOL_1 },
                {MORPS_M, BRIDGE_MENU_GOL_2 },
                {MORPS_M, BRIDGE_MENU_GOL_3 },
                {MORPS_M, BRIDGE_MENU_GOL_4 },
                {MORPS_M, PORTA_MENU_GOL_1  },
                {MORPS_M, PORTA_MENU_GOL_2  },
                {MORPS_M, PORTA_MENU_GOL_3  },
                {MORPS_M, PORTA_MENU_GOL_4  },
                {MORPS_M, PORTA_MENU_GOL_5  },
                {MORPS_M, HARR1_GOAL_1      },
                {MORPS_M, HARR1_GOAL_2      },
                {MORPS_M, HARR1_GOAL_3      },
                {MORPS_M, HARR2_GOAL_C0     },
                {MORPS_M, HARR2_GOAL_D0     },
                {MORPS_M, HARR2_GOAL_C1     },
                {MORPS_M, msg7134PowerSwitchGoal},
                {MORPS_M, msg7134PowerCordGoal},
                {MORPS_M, msg7131PubsReference},
                NULL
        };
/* increase the MAX_NUM_MENU_GOAL as the menugoal list grows */
#define MAX_NUM_MENU_GOAL	18

/* -------------------------------------------------------------------------*/
nl_catd	catd;
struct	tm_input        tminput;
int     f_code;
char    *f_type;
int     globol_rc = 0;
int     device_type;
extern  getdainput();
extern	nl_catd diag_catopen(char *, int);

/* the default menuType of the application is ASL_DIAG_LIST_CANCEL_EXIT_SC.
 * When we need to display a different type, change this global variable
 * to that type. Reassign the menuType to its default when done*/
int	menuType = ASL_DIAG_LIST_CANCEL_EXIT_SC;

/**/
/*
 * NAME: main
 *
 * FUNCTION: This procedure is the entry point for the Missing Options
 *      Resolution Procedures (MORPs). It determines the type
 *      of device that is missing,the location of the device and then
 *      calls the approriate routine to try and resolve the missing
 *      device.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This procedure is forked and execed by the diagnostic controller.
 *
 * (NOTES:) The inputs to this routine is from the TM_INPUT object class.
 *
 *
 * (RECOVERY OPERATION:) In the event of a software error that prevents
 *      this routine from functioning, control will be returned to the
 *      diagnostic controller having set DA_SETRC_ERROR(DA_ERROR_OTHER).
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOOTES.
 *
 * RETURNS: NONE
 */

main(argc,argv,envp)
int argc;
char **argv;
char **envp;
{
        int     rc;
        int     rc1;
        int     menu_rc;
        char    criteria[40];
        struct  listinfo     obj_info;
        struct  CuDv    *cudv;
	struct  CuAt    *cuat;
	int     bc;

        void    cleanup();

        setlocale(LC_ALL,"");

        if (init_dgodm() == -1){		/* Initialize the ODMI */
                /* Signal ODM did not initialize */
                globol_rc = -1;
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                cleanup();
        }

        if (getdainput(&tminput) == -1){        /* Get my DA input parameters*/
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                cleanup();
        }
                                               /* Open catalog file */
        if ((catd = diag_catopen(MF_MORPS, 0)) == CATD_ERR){
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                cleanup();
        }

        if (diag_asl_init() == -1){
                globol_rc = -2;
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                cleanup();
        }

        /* Set tests to FULL */
        DA_SETRC_TESTS(DA_TEST_FULL);

        /* Determine the device type */
        /* Get the fru code from the LED value in Pre-define class */

        sprintf(criteria,"name = %s",tminput.dname);
        cudv = get_CuDv_list(CuDv_CLASS,criteria,&obj_info,1,2);
        if ((cudv == (struct CuDv *) -1) || (cudv == (struct CuDv *) NULL)){
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                cleanup();
        }

        f_code = cudv->PdDvLn->led;
	if(f_code == 0) {
		/* No LED number in PdDv so check for type Z */
		/* attribute                                 */

		if ((cuat = (struct CuAt *)getattr(cudv->name,"led",0,&bc))
		    == (struct CuAt *)NULL) {
		    /* Error from CuAt */
		    DA_SETRC_ERROR(DA_ERROR_OTHER);
		    cleanup();
		}

		f_code = (int)strtoul(cuat->value,NULL,0);
	}

        f_type = cudv->PdDvLn->class;

	/* special cases */
        if (!strcmp(cudv->PdDvLn->type,"8mm5gb") &&
                (de_adapter(tminput.parent) == TRUE)) 
                        f_code = 0x914;	/* correct the LED value for DIFF */

        switch (f_code) {
		case HARRIER2:
                	serial_disk2();
        		break;
		case 0x970:
			bridge_box();
			break;
	}

        if (strcmp(f_type, "disk") == 0)
                device_type = devMissHdisk;
        else if (strcmp(f_type, "diskette") == 0)
		device_type = devMissDskt;
	else
		device_type = devMissMedia;

	/* Display & get the type of enclosure */
	menuType = ASL_DIAG_LIST_CANCEL_EXIT_HELP_SC;
        menu_rc = disp_menu(0x950000+device_type, 0,
			menuEncl[device_type], NOO, NULL);
	menuType = ASL_DIAG_LIST_CANCEL_EXIT_SC;
	/* Call the device's MAPs */
	(*funcEncl[device_type][menu_rc-1])();
}/* end main */
/**/
/*
 * NAME:
 *
 * FUNCTION:
 *
 * NOTES:
 *
 * RETURNS:
 *
 */
int de_adapter(parent)
char *parent;
{
        struct  CuVPD *cuvpd;
        struct  listinfo cuvpd_info;
        char    criteria[128];

        sprintf(criteria, "name=%s and vpd like *SCSI-2DIFF*", parent);
        cuvpd = get_CuVPD_list(CuVPD_CLASS, criteria, &cuvpd_info, 1, 2);
        if ((cuvpd == (struct CuVPD *)(-1)) ||
            (cuvpd == (struct CuVPD *) NULL)) {
                return(FALSE);
        }

        return(TRUE);

} /* end de_adapter() */

/**/
/*
 * NAME: disp_menu()
 *
 * FUNCTION:	This function is resposible for displaying all messages
 *			to interface to the user.
 *		If response is 'NOO' and 'report' flag is 'YESS' then
 *			return a menu goal
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */
int disp_menu(menu_num, msg_num, list, report, goal)
long	menu_num;
int	msg_num;
int	report;
int	goal;
struct	msglist list[];
{
       	long	menu_rc = 0;
        void	retrn_goal();
        char	string[1024];
        ASL_SCR_TYPE menutypes = DM_TYPE_DEFAULTS;
        ASL_SCR_INFO uinfo[MENU_MAX_ENTRY];

        (void) memset(uinfo, 0, sizeof(uinfo));
        menutypes.cur_index = 1;
        menu_rc = chk(diag_display(menu_num,catd,list,DIAG_MSGONLY,
			ASL_DIAG_LIST_CANCEL_EXIT_SC,&menutypes,uinfo));
        sprintf(string,uinfo[msg_num].text,tminput.dname,tminput.dnameloc,
                  tminput.dname, tminput.dnameloc);
        uinfo[msg_num].text = (char *) malloc(strlen(string)+1);
        strcpy(uinfo[msg_num].text,string);

        menu_rc = chk(diag_display(menu_num,catd,NULL,DIAG_IO,
			menuType,&menutypes,uinfo));

	while (menu_rc == DIAG_ASL_HELP) {
		diag_hmsg(catd,MSG_HELP_SET,msgEnclHelp,NULL);
        	menu_rc = chk(diag_display(menu_num,catd,NULL,DIAG_IO,
				menuType,&menutypes,uinfo)); 
	}

        if ((menutypes.cur_index == NOO) && (report == YESS))
                retrn_goal(goal);

        return (menutypes.cur_index);
}

/**/
/*
 * NAME: portable()
 *
 * FUNCTION: 
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */
void portable()
{
        int	menu_rc;
        void    retrn_frub();

        /* Is a module installed ?*/
        menu_rc = disp_menu(0x950400,0,portable_6,TRUE,PORTA_MENU_GOL_5);

        /* Is the power on light glowing?*/
        menu_rc = disp_menu(0x950401, 0,portable_1,FALSE,NULL);
        if (menu_rc == YESS) {
                if (tminput.advanced == ADVANCED_TRUE) {
                        /* Is the signal cable properly plugged?*/
                        menu_rc = disp_menu(0x950402, 0,portable_4,
					TRUE,PORTA_MENU_GOL_3);
                        /* SCSI Address correct */
                        menu_rc = disp_menu(0x950403, 0,portable_5,
					TRUE,PORTA_MENU_GOL_4);
                }
                if (f_code == 0x955)
                	retrn_frub(DEV_ADAP_FAILURE, HDA_TYPE);
                else
                	retrn_frub(DEV_ADAP_FAILURE, DEVICE_ADAP);
        }
        else
        {
                /* Is the power switch on?*/
		menu_rc=disp_menu(0x950404,0,portable_2,TRUE,PORTA_MENU_GOL_1);

                /* Is the power cord plugged */
                menu_rc=disp_menu(0x950405,0,portable_3,TRUE,PORTA_MENU_GOL_2);
                f_code = PORTA_PSUPPLY;
                retrn_frub(P_SUPPLY_FAILURE, POWR_SUPPLY);
        }
}

/**/
/*
 * NAME: bridge_box()
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 */

void bridge_box()
{
        void    retrn_frub();
        int     menu_rc = YESS;

        /*If the missing device is not external dskt drv.*/
        if (device_type != devMissDskt) {
                /* Is the power on light glowing?*/
                menu_rc = disp_menu(0x950500,0,bridge_1,NOO,NULL);
        }
        if (menu_rc == YESS) {
                if (tminput.advanced == ADVANCED_TRUE) {
                        /* Is signal cable properly plugged?      */
                        (void) disp_menu(0x950501,0,bridge_5,YESS,BRIDGE_MENU_GOL_4);
                        if (device_type != devMissDskt) {
                                /* SCSI address correct */
                                (void) disp_menu(0x950502,0,bridge_4,YESS,
                                                  BRIDGE_MENU_GOL_3);
                                if (f_code == 0x955)
					retrn_frub(DEV_ADAP_FAILURE, HDA_TYPE);
                                else
					retrn_frub(DEV_ADAP_FAILURE, DEVICE_ADAP);
                        }
                        else
                        {
                                /* Is the power switch on? */
                                (void) disp_menu(0x950503,0,bridge_6,YESS,
                                                        BRIDGE_MENU_GOL_1);
                                /* Is the power cord plugged */
                                (void) disp_menu(0x950504,0,bridge_7,YESS,
                                                        BRIDGE_MENU_GOL_2);
                                f_code = DSKT_PSUPPLY;
                                retrn_frub(DEV_ADAP_FAILURE,DEVICE_ADAP_PSUPPLY);/* Power Supply */
                        }
                }
                else    /* (customer mode and light on) */
                {
                        if (device_type != devMissDskt)
                        {
                                if (f_code == 0x955)
                                	retrn_frub(DEV_ADAP_FAILURE,HDA_TYPE);
                                else
                                	retrn_frub(DEV_ADAP_FAILURE,DEVICE_ADAP);
                        }
                }
        }

	/* Is the power switch on? */
        if (device_type == devMissDskt)
                (void) disp_menu(0x950503,0,bridge_6,YESS,BRIDGE_MENU_GOL_1);
        else {
                (void) disp_menu(0x950503,0,bridge_2,YESS,BRIDGE_MENU_GOL_1);
                if ((f_code == 0x970) && (tminput.advanced == ADVANCED_TRUE)) {
                        (void) disp_menu(0x950510,0,bridge_8,YESS,BRIDGE_MENU_GOL_1);
                }
        }

        /* Is the power cord plugged */
        if (device_type == devMissDskt) {
                (void) disp_menu(0x950504,0,bridge_7,YESS,BRIDGE_MENU_GOL_1);
                f_code = DSKT_PSUPPLY;
        	retrn_frub(DEV_ADAP_FAILURE,DEVICE_ADAP_PSUPPLY);
        }
        else {
                (void) disp_menu(0x950506,0,bridge_3,YESS,BRIDGE_MENU_GOL_2);
        	f_code = BRDG_PSUPPLY;
        	retrn_frub(P_SUPPLY_FAILURE,P_SUPPLY_FAILURE);
        }
}

/**/
/*
 * NAME: serial_disk1()
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 */
void serial_disk1()
{
        void    retrn_frub();
        int     menu_rc;

	/* All Lights off ?*/
        menu_rc = disp_menu(0x950100, 0, harrier_1, NOO ,NULL);
        if (menu_rc == NOO) {	/* Is the Fans Good light on ?*/
                menu_rc = disp_menu(0x950101, 0, harrier_3, NOO, NULL);
                if (menu_rc == NOO) {
                        f_code = HARR1_FAN;
                        retrn_frub(HARR1_FAN_FAILURE,FAN);	/* FAN */
                }
                else
                {                     /* Is the Compartment light on ?*/
                        menu_rc = disp_menu(0x950102,0,harrier_5,NOO,NULL);
                        if (menu_rc == YESS) {
                                if (f_code ==0x955)	
                                	retrn_frub(DEV_ADAP_FAILURE,HDA_TYPE);
                                else
                                	retrn_frub(DEV_ADAP_FAILURE,DEVICE_ADAP);
                        }
                        else {
                                /* Is the compartment switch on?*/
                                (void) disp_menu(0x950103,0,harrier_6,YESS,
                                                        HARR1_GOAL_2);
                                frub[POWR_SUPPLY].frus[0].fmsg=DRW_PSUPPLY;
                                f_code = HARR1_PSUPPLY;
                                retrn_frub(P_SUPPLY_FAILURE,POWR_SUPPLY);
                        }
                }
        }
        else {
                /* Is the power switch on? */
                (void) disp_menu(0x950104, 0, harrier_2,YESS, HARR1_GOAL_1);
                if (tminput.advanced == ADVANCED_TRUE) {
                        /* Is Mains Power Present Light On*/
                        menu_rc = disp_menu(0x950105,0,harrier_4,NOO,NULL);
                        if (menu_rc == NOO) {
                                frub[POWR_SUPPLY].frus[0].fmsg=RACK_PSUPPLY;
                                f_code = RACK_SUPPLY;
                                retrn_frub(P_SUPPLY_FAILURE,POWR_SUPPLY);
                        }
                        else {
                                frub[POWR_SUPPLY].frus[0].fmsg=DRW_PSUPPLY;
                                f_code = HARR1_PSUPPLY;
                                retrn_frub(P_SUPPLY_FAILURE,POWR_SUPPLY);
                        }
                }
                else {
                        frub[POWR_SUPPLY].frus[0].fmsg=DRW_PSUPPLY;
                        f_code = HARR1_PSUPPLY;
                        retrn_frub(P_SUPPLY_FAILURE,POWR_SUPPLY);
                }
        }
}

/**/
/*
 * NAME: serial_disk2()
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 */
void serial_disk2()
{
        void    retrn_frub(); /* exit point */
	struct	CuVPD	*cuvpd;
        struct  listinfo     obj_info;
        char    criteria[40];
	char	*string;
	char	tm_fields[5]={NULL};
        int     menu_rc;

        if (strcmp(f_type,"disk")==0) { /* Harrier2 DASD MORPS */
                /* Power Good light on ? */
                menu_rc = disp_menu(0x950210,0,harr2_D0,NOO,NULL);
                if (menu_rc == NOO) {
                        /* Power Switch on ? */
                        (void) disp_menu(0x950211,0,harr2_D1,YESS,HARR2_GOAL_D0);
                        f_code = 0xC001;
                        retrn_frub(HARR2_SRN_RMSG,HARR2_SRN_1);

                }

                /* DASD Check Indicator on ? */
                menu_rc = disp_menu(0x950213,0,harr2_D2,NOO,NULL);
                if (menu_rc == YESS) {
                        f_code = 0xC002;
                        retrn_frub(HARR2_SRN_RMSG,HARR2_SRN_1);

                }

                /* Ready light on ? */
                menu_rc = disp_menu(0x950215,0,harr2_D3,NOO,NULL);
                if (menu_rc == NOO) {
                        f_code = 0xC003;
                        retrn_frub(HARR2_SRN_RMSG,HARR2_SRN_1);

                }

                /* Display show DASD number(s) ? */
                menu_rc = disp_menu(0x950217,0,harr2_D4,NOO,NULL);
                if (menu_rc == YESS) {
                        f_code = 0xC004;
                        retrn_frub(HARR2_SRN_RMSG,HARR2_SRN_1);

                }

                /* Config problem ! */
                f_code = 0xC005;
                retrn_frub(HARR2_SRN_RMSG,HARR2_SRN_1);

        }
        else {
                /* Harrier2 controller MORPS */

                /* Display showing '-' ? */
                menu_rc = disp_menu(0x950200,0,harr2_C0,NOO,NULL);
                if (menu_rc == NOO) {
                        /* Power switch on ? */
                        (void) disp_menu(0x950201,0,harr2_C1,YESS,HARR2_GOAL_C0);
                        /* Display blank ? */
                        menu_rc = disp_menu(0x950202,0,harr2_C2,NOO,NULL);
                        if (menu_rc == NOO) {
				/* Get the VPD data and determine the	*/
				/* type of controller in the system.	*/
				
        			sprintf(criteria,"name = %s" ,tminput.dname);
				cuvpd = get_CuVPD_list(CuVPD_CLASS,criteria,
					&obj_info,1,2);
        			if (cuvpd == (struct CuVPD *) -1 ||
					cuvpd == (struct CuVPD *) NULL) {
					/* No data in CuVPD or an error	*/
					/* status returned		*/
					DA_SETRC_ERROR(DA_ERROR_OTHER);
					cleanup();
				}

				/* Search the VPD for a TM Field and	*/
				/* copy the controller type into a 	*/
				/* tm_field				*/

				string=strstr(cuvpd->vpd,"9333-");
				sprintf(tm_fields,"%c%c%c",string[5],
					string[6],string[7]);

				/* If the value of tm_field == 0 then	*/
				/* we have an old controller.		*/

				if (atoi(tm_fields)==0) {
                                	f_code = 0xC012;
                                	retrn_frub(HARR2_SRN_RMSG,HARR2_SRN_1);
				}
				else
				/* we have one of the following:	*/
				/* 4 port controller - one switch	*/
				/* 8 port controller - two switches	*/
					serial_disk2_3();

                        }
                        /* Rack power light on ? */
                        menu_rc = disp_menu(0x950204,0,harr2_C3,NOO,NULL);
                        if (menu_rc == NOO) {
                                /* SRN 950-500 */
                                f_code = RACK_SUPPLY;
                                retrn_frub(HARR2_PS_RMSG,HARR2_SRN_2);

                        }
                        else {
                                /* SRN C011 */
                                f_code = 0xC011;
                                retrn_frub(HARR2_SRN_RMSG,HARR2_SRN_1);
                        }
                }

                /* Cable or Config problem ! */

                /* SRN C013 */
                f_code = 0xC013;
                retrn_frub(HARR2_SRN_RMSG,HARR2_SRN_1);
        }
        cleanup();
} /* end serial_disk2() */

/**/
/*
 * NAME: retrn_frub()
 *
 * FUNCTION: 
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 */

void retrn_frub(r_msg,type)
int   r_msg;
int   type;
{
        int	rc1;
        int	rc2;
        void	cleanup();

        strcpy(frub[type].dname,tminput.dname);

        if (type == HDA_TYPE) {
                strcpy(frub[type].frus[0].floc,tminput.dnameloc);
                strcpy(frub[type].frus[2].floc,tminput.dnameloc);
        }

        frub[type].rmsg = r_msg;
        rc1 = insert_frub(&tminput, &frub[type]);
        frub[type].rcode = f_code;

        if (type == HARR2_SRN_1)
                frub[type].sn = -1;
        else
                frub[type].sn = MISSING_OPTION;

        rc2 = addfrub(&frub[type]);

        if ((rc1 !=0) ||(rc2 != 0))
                DA_SETRC_ERROR(DA_ERROR_OTHER);
        else {
                DA_SETRC_STATUS(DA_STATUS_BAD);
                cleanup();
        }
}

/**/
/*
 * NAME: cleanup()
 *
 * FUNCTION: 
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */
void    cleanup()
{
        if (globol_rc != -1)
                term_dgodm();
        if (catd != CATD_ERR)
                catclose(catd);
        if (globol_rc != -2)
                diag_asl_quit(NULL);
        DA_EXIT();
}

/**/
/*
 * NAME: system_unit()
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 */

void    system_unit()
{
        if (f_code == 0x955)
        	retrn_frub(DEV_ADAP_FAILURE,HDA_TYPE);
        else
        	retrn_frub(DEV_ADAP_FAILURE,DEVICE_ADAP);
}

/**/
/*
 * NAME: retrn_goal()
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *	call cleanup() which then exit program
 */
void    retrn_goal(goal)
int     goal;
{
        int     menu_rc;
        ASL_SCR_TYPE menutypes = DM_TYPE_DEFAULTS;
        ASL_SCR_INFO uinfo[MAX_NUM_MENU_GOAL];
        char    string[1024];

        (void) memset(uinfo, 0, sizeof(uinfo));

        menu_rc = diag_display(0x000000,catd,men_goals, DIAG_MSGONLY,
                             ASL_DIAG_LIST_CANCEL_EXIT_SC,&menutypes,uinfo);
        sprintf(string,uinfo[goal -1].text,tminput.dname,tminput.dnameloc,
                  tminput.dname, tminput.dnameloc);
        uinfo[goal -1].text = (char *) malloc(strlen(string) +1);
        strcpy(uinfo[goal -1].text,string);
        menugoal(uinfo[goal -1].text);
        DA_SETRC_STATUS(DA_STATUS_BAD);
        cleanup();
}

/**/
/*
 * NAME: chk()
 *
 * FUNCTION: Designed to check ASL return code and take an appropriate action.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

long chk(asl_stat)
long    asl_stat;
{
        switch (asl_stat) {
                case DIAG_ASL_OK:
                        break;
                case DIAG_MALLOCFAILED:
                case DIAG_ASL_ERR_NO_SUCH_TERM:
                case DIAG_ASL_ERR_NO_TERM:
                case DIAG_ASL_ERR_INITSCR:
                case DIAG_ASL_ERR_SCREEN_SIZE:
                case DIAG_ASL_FAIL: 
                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                        cleanup();
                        break;
                case DIAG_ASL_CANCEL: 
                        DA_SETRC_USER(DA_USER_QUIT);
                        cleanup();
                        break;
                case DIAG_ASL_EXIT:
                        DA_SETRC_USER(DA_USER_EXIT);
                        cleanup();
                        break;
                default:
                        break;
        }
	return (asl_stat);
}
/**/
/*
 * NAME: serial_disk2_3()
 *
 * FUNCTION: Designed to display a scpecific messages 9333-1 controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

serial_disk2_3()
{
	int	menu_rc=0;

	/* Is the display contain H or L*/
	menu_rc= disp_menu(0x950221,0,harr2_C4, NOO,NULL);
	if (menu_rc ==NOO) {
		/* Is the display flashing P followed by another character */
		menu_rc=disp_menu(0x950223,0,harr2_C5, NOO,NULL);
		if (menu_rc==NOO) {
			/* Call Service; Report SRN C012		*/
			f_code = 0xC012;
			retrn_frub(HARR2_SRN_RMSG,HARR2_SRN_1);
		}
		else {
			/* Is the cable properly connected		*/
			/* If the answer is no disp_menu does not retrun*/
			/* it will issue a menu goal asking the user to	*/
			/* connect the cable properly and restart the 	*/
			/* system.					*/
			if (tminput.advanced == ADVANCED_TRUE)
				(void) disp_menu(0x950224,0,harr2_C6,YESS,
					HARR2_GOAL_C1);

			/* report SRN C101 */
			f_code=0xC101;
			retrn_frub(HARR2_SRN_RMSG,HARR2_SRN_1);
		}
	}
	else {
		/* switch fault has been detected report an SRN */
		f_code=0xC100;
		retrn_frub(HARR2_SRN_RMSG,HARR2_SRN_1);
	}
}

/**/
/*---------------------------------------------------------------------------
 *
 * NAME: getDiskDrawer()
 *
 * FUNCTION: gets input from users for the type of enclosure of
 *	     the missing disk drive
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

void getDiskDrawer()
{
	int	menu_rc;

	/* which type of drawer is the disk in?*/
        menu_rc = disp_menu(0x950010, 0, menuDiskEnclDrawer, NOO ,NULL);
        (*funcDiskEnclDrawer[menu_rc-1])();
}

/**/
/*---------------------------------------------------------------------------
 *
 * NAME: getMediaDrawer()
 *
 * FUNCTION: gets input from users for the type of enclosure of
 *	     the missing media
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

void getMediaDrawer()
{
	int	menu_rc;

	/* which type of drawer or tower is the media in?*/
        menu_rc = disp_menu(0x950011, 0, menuMediaEnclDrawer, NOO ,NULL);
        (*funcMediaEnclDrawer[menu_rc-1])();
}

/**/
/*---------------------------------------------------------------------------
 *
 * NAME: expansion_7013()
 *
 * FUNCTION: MORPS for 7013 expansion unit (Pegasus J Series)
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
*/
void expansion_7013()
{
	int	menu_rc;

        menu_rc = disp_menu(0x950590,0,menu7013PowerIndicator,NOO,NULL);
	if (menu_rc == NOO){
		f_code=0x203;
		retrn_frub(P_SUPPLY_FAILURE, frubType7013PowerSupply);
	}

	retrn_frub(DEV_ADAP_FAILURE, DEVICE_ADAP);
}

/**/
/*---------------------------------------------------------------------------
 *
 * NAME: disk_drawer_7134()
 *
 * FUNCTION: MORPS for 7134 disk drawer (Saracen)
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
*/
void disk_drawer_7134()
{
	int	menu_rc;

	/* power switch on? */
	(void)disp_menu(0x950600,0,menu7134PowerSwitch,YESS,msg7134PowerSwitchGoal);
	/* power indicator on? */
        menu_rc = disp_menu(0x950601,0,menu7134PowerIndicator,NOO,NULL);
        if (menu_rc == NOO) {
		/* Fan-Power supply assembly power indicators on? */
        	menu_rc = disp_menu(0x950602,0,menu7134FanPowerSupplyIndicator,NOO,NULL);
        	if (menu_rc == YESS) {	/* AC fan assembly problem */
			f_code = 0x301;
			retrn_frub(DEV_ADAP_FAILURE, frubTypeAcFanAssembly);
		}
		/* power cord connected? */
		(void)disp_menu(0x950603,0,menu7134PowerCord,YESS,msg7134PowerCordGoal);
		f_code = 0x500;		/* input power problem */
		retrn_frub(P_SUPPLY_FAILURE, POWR_SUPPLY);
	}
	/* fan indicator on? */
       	menu_rc = disp_menu(0x950604,0,menu7134FanIndicator,NOO,NULL);
       	if (menu_rc == YESS) {		/* AC fan assembly problem */
       		menu_rc = disp_menu(0x950605,0,menu7134CheckIndicator,NOO,NULL);
       		if (menu_rc == NOO) {	/* SCSI bus problem */
			f_code = 0x201;
			retrn_frub(DEV_ADAP_FAILURE, frubTypeScsiBus);
		}
		f_code = 0x301;		/* AC fan assembly problem */
		retrn_frub(DEV_ADAP_FAILURE, frubTypeAcFanAssembly);
	}
	f_code = 0x302;			/* DC fan assembly problem */
	retrn_frub(DEV_ADAP_FAILURE, frubTypeDcFanAssembly);
}

/**/
/*---------------------------------------------------------------------------
 *
 * NAME: tower_7131()
 *
 * FUNCTION: MORPS for 7131 Media Tower
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
*/
void tower_7131()
{
	void	cleanup();
	void	retrn_goal();

	/* refer to pubs. */
	retrn_goal(msg7131PubsReference);
	cleanup();
}
