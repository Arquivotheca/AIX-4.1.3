static char sccsid[] = "@(#)73	1.2  src/bos/usr/ccs/lib/libsrc/srcprint.c, libsrc, bos411, 9428A410j 11/9/93 16:23:45";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 * 	src_print_all_notify, src_print_all_subsvr, src_print_all_subsystem,
 * 	src_print_default_subsystem, src_print_names, src_print_one_notify
 * 	src_print_one_subsvr, src_print_one_subsystem, src_print_values
 *	
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
#include <src.h>
#include <srcobj.h>
#include <srcodm.h>
#include <odmi.h>

struct convert_elem {
short	elem_num;
short	max_value;
short	min_value;
char	**elem_val;
};

struct convert_elem *get_convert_subsys()
{
	static struct convert_elem convert[4];

	static char *restart[]={
		"",
		"-R",
		"-O"};
	static char *display[]={
		"-D",
		"-d"};
	static char *multi[]={
		"-Q",
		"-q"};
	static char *contact[]={
		"",
		"",
		"-S",
		"-K"};

	convert[3].elem_num=17;
	convert[3].elem_val=display;
	convert[2].elem_num=11;
	convert[2].elem_val=contact;
	convert[1].elem_num=10;
	convert[1].elem_val=multi;
	convert[0].elem_num=9;
	convert[0].elem_val=restart;
	
	return(convert);
}

void src_print_names(struct Class *class)
{
	struct ClassElem	*elem;
	int i;

	printf("#");
	for(i=class->nelem,elem=class->elem;i>0;i--,elem++)
		printf("%s:",elem->elemname);
	printf("\n");
}

void src_print_values(struct Class *class, char *value, struct convert_elem *convert)
{
	struct ClassElem	*elem;
	int i;

	for(i=0,elem=class->elem;i<class->nelem;i++,elem++)
		switch(elem->type)
		{
			case ODM_SHORT:
				if( (void *)convert != NULL && i==convert->elem_num)
				{
					printf("%s:",
(char *)(convert->elem_val[(int)(*(short *)(value+(elem->offset)))]));
					convert++;
				}
				else
					printf("%d:",*(short *)(value+(elem->offset)));

				break;
			case ODM_LONG:
				printf("%ld:",*(long *)(value+(elem->offset)));
				break;
			default:
				printf("%s:",(char *)(value+(elem->offset)));
				break;
		}
	printf("\n");
}
void src_print_all_subsystem()
{
	struct SRCsubsys subsys;
	char *rc;

	src_print_names(SRCSYSTEM);
	src_odm_init();
	rc=odm_get_obj(SRCSYSTEM,NULL,&subsys,ODM_FIRST);
	while(rc != NULL && rc != -1)
	{
	        src_print_values(SRCSYSTEM,(char *)&subsys,get_convert_subsys());
		rc=odm_get_obj(SRCSYSTEM,NULL,&subsys,ODM_NEXT);
	}
	src_odm_terminate(TRUE);
}
void src_print_all_subsvr()
{
	struct SRCsubsvr subsvr;
	char *rc;

	src_print_names(SRCSUBSVR);
	src_odm_init();
	rc=odm_get_obj(SRCSUBSVR,NULL,&subsvr,ODM_FIRST);
	while(rc != NULL && rc != -1)
	{
		src_print_values(SRCSUBSVR,(char *)&subsvr,NULL);
		rc=odm_get_obj(SRCSUBSVR,NULL,&subsvr,ODM_NEXT);
	}
	src_odm_terminate(TRUE);
}
void src_print_all_notify()
{
	struct SRCnotify notify;
	char *rc;

	src_print_names(SRCNOTIFY);
	src_odm_init();
	rc=odm_get_obj(SRCNOTIFY,NULL,&notify,ODM_FIRST);
	while(rc != NULL && rc != -1)
	{
		src_print_values(SRCNOTIFY,(char *)&notify,NULL);
		rc=odm_get_obj(SRCNOTIFY,NULL,&notify,ODM_NEXT);
	}
	src_odm_terminate(TRUE);
}

void src_print_one_subsystem(char * subsysname)
{
	struct SRCsubsys subsys;
	if(getssys(subsysname,&subsys) == 0)
	{
		src_print_names(SRCSYSTEM);
	        src_print_values(SRCSYSTEM,(char *)&subsys,get_convert_subsys());
	}
}

void src_print_one_subsvr(char * sub_type)
{
	struct SRCsubsvr subsvr;
	if(getsubsvr(sub_type,&subsvr) == 0)
	{
		src_print_names(SRCSUBSVR);
		src_print_values(SRCSUBSVR,(char *)&subsvr,NULL);
	}
}

void src_print_one_notify(char * name)
{
	struct SRCnotify notify;
	char criteria[256];
	void *rc;

	sprintf(criteria,"notifyname = '%s'",name);
	src_odm_init();
	rc = odm_get_first(SRCNOTIFY,criteria,&notify);
	if(rc != NULL && rc != -1)
	{
		src_print_names(SRCNOTIFY);
		src_print_values(SRCNOTIFY,(char *)&notify,NULL);
	}
	src_odm_terminate(TRUE);
}

void src_print_default_subsystem()
{
	struct SRCsubsys subsys;

	defssys(&subsys);

	src_print_names(SRCSYSTEM);
	src_print_values(SRCSYSTEM,(char *)&subsys,get_convert_subsys());
}
