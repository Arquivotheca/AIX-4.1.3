static char sccsid[] = "@(#)22	1.8.1.3  src/bos/usr/ccs/lib/libsrc/srcodm.c, libsrc, bos411, 9428A410j 11/9/93 16:23:42";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	getdbfields,putdbfields,update_obj,readrec,src_odm_init
 *	src_odm_terminate
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <stdio.h>
#include "odmi.h"
#include "srcodm.h"

static char *prev_path = NULL;
static int odm_lock_id=-1;

void getdbfields(fieldview)
struct fieldview *fieldview;
{
	for(;fieldview->c_addr!=0;fieldview++)
		memcpy(fieldview->c_addr,fieldview->db_addr,fieldview->size);
}

void putdbfields(fieldview)
struct fieldview *fieldview;
{
	for(;fieldview->c_addr!=0;fieldview++)
		memcpy(fieldview->db_addr,fieldview->c_addr,fieldview->size);
}

void prev_odm_path()
{
	int svodmerrno;
	char *p;
	svodmerrno=odmerrno;
	p = odm_set_path(prev_path);
	if((p != NULL) && (p != -1)) {
		free(p);
	}
	odmerrno=svodmerrno;
}

int update_obj(class,objview,criteria)
struct class *class;
struct objview *objview;
char *criteria;
{
        int rc=0;
        void *getrc;
        int firstnext=1;

        getrc = odm_get_obj(class,criteria,objview->db_rec,firstnext);        
        while(rc==0 && getrc != NULL && getrc != -1)
        {
                firstnext=0;
                putdbfields(objview->fieldview);
                rc=odm_change_obj(class,objview->db_rec);
                getrc = odm_get_obj(class,criteria,objview->db_rec,firstnext);
        }
        if(rc==0 && firstnext == 0 && getrc == NULL) {
                return(1);
        }
        if(getrc == -1) {
                return(-1);
        }
        return(rc);

}

int readrec(class,objview,criteria,firstnext)
struct class *class;
struct objview *objview;
char *criteria;
int firstnext;
{
	void *getrc;

	getrc = odm_get_obj(class,criteria,objview->db_rec,firstnext);
	if(getrc != NULL && getrc != -1) {
		getdbfields(objview->fieldview);
		return(1);
	}
	return((int)getrc);
}

int src_odm_init()
{

	if(odm_initialize() == -1)
		return(-1);
	if((prev_path=odm_set_path(SRC_ODM_PATH)) == (char *)-1)
		return(-1);
	if((odm_lock_id=odm_lock(SRC_ODM_LOCK,ODM_WAIT)) == -1)
	{
		prev_odm_path();
		free(prev_path);
		prev_path = NULL;
		return(-1);
	}
	
	return(0);
}

void src_odm_terminate(terminate)
int terminate;
{
	int svodmerrno;
	svodmerrno=odmerrno;
	prev_odm_path();
	if (prev_path != NULL && prev_path != -1) {
		free(prev_path);
		prev_path = NULL;
	}
	if(odm_lock_id!=(-1))
	{
		odm_unlock(odm_lock_id);
		odm_lock_id=-1;
	}
	if(terminate)
		odm_terminate();
	odmerrno=svodmerrno;
}
