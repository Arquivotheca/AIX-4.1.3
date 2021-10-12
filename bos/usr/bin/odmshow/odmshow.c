static char sccsid[] = "@(#)06  1.14  src/bos/usr/bin/odmshow/odmshow.c, cmdodm, bos411, 9428A410j 12/2/93 11:38:44";
/*
 * COMPONENT_NAME: (CMDODM) Object Data Manager
 *
 * FUNCTIONS: odmshow (main),  write_struct, count_obj
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*              include file for message texts          */
#include "odmcmd_msg.h"
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
#include <stdio.h>
#include <odmi.h>
#define INVALID -1
#define FALSE 0
#define TRUE 1

extern int odmerrno;
extern int odm_read_only;

char message [128];
char classname[128],colstr[128],dot_lcl[128];

FILE *fh;

/*
 * NAME: odmshow (main)
 *
 * FUNCTION:
 *      Describes an existing ODM object class in an ascii form outputted
 *      to standard output.  The output of this routine can then be sent
 *      to the 'odmcreate' command as input.
 *      Syntax for this command is:
 *             odmshow class1 [class2 class3 . . .]
 *
 * RETURNS:
 *      Exits with a 0 if successful, a positive number otherwise.
 */
main(argc,argv)
int argc;          /* number of parameters */
char *argv[];      /* pointer to array of parameters */
{
int i,j;
char *cp,*index();

#ifndef R5A
 setlocale(LC_ALL,"");
 scmc_catd = catopen(MF_CMD, NL_CAT_LOCALE);       /* Defect 116034 */
#endif
        if(argc < 2){
                fprintf(stderr,
                  catgets(scmc_catd, MS_odmshow, SHOW_MSG_0,
                  "usage: odmshow <classname> [<classname> <classname> ...]\n") );
                exit(-1);
                }
        fh = stdout;

        argc--;argv++;

	odm_read_only=1;

        odm_initialize ();
        for(i=0;i<argc;i++){
                strcpy(classname,argv[i]);
                write_struct(classname);
                }
        fclose(fh);
}


/*
 * NAME:       write_struct
 *
 * FUNCTION:   Prints out the description of a given object class.
 *
 * RETURNS:    Nothing.  Exits with a -1 if unsuccessful
 */
write_struct(class)
char *class;            /* Name of the object class */
{
char *a,*classgrp;
int i,rv,num,type;
struct Class *classp,*lclassp,*odm_mount_class();
struct ClassElem *e;

  classp = odm_mount_class(class);
  if((int)classp == -1){
        fprintf(stderr, catgets(scmc_catd, MS_odmshow, SHOW_MSG_1,
              "odmshow: Could not open class: %s,  status %d\n") ,
              class,odmerrno);
        exit(-1);
        }



        fprintf(fh,"class %s {\n",class);
        for(i=0;i<classp->nelem;i++) {
                e = &((classp->elem)[i]);
                type = (classp->elem)[i].type;
                switch(type){
                case ODM_SHORT:
                        sprintf(colstr,"\tshort %s;",
                                                  (classp->elem)[i].elemname);
                        break;
                case ODM_LONG:
                        sprintf(colstr,"\tlong %s;",
                                                    (classp->elem)[i].elemname);
                        break;
                case ODM_ULONG:
                        sprintf(colstr,"\tulong %s;",
                                                    (classp->elem)[i].elemname);
                        break;

                case ODM_DOUBLE:
                        sprintf(colstr,"\tdouble %s;",
                                                   (classp->elem)[i].elemname);
                        break;
                case ODM_CHAR:
                        sprintf(colstr,"\tchar %s[%d];",
                                (classp->elem)[i].elemname,
                                 (classp->elem)[i].size);
                        break;
                case ODM_VCHAR:
                        if((classp->elem)[i].size)
                                sprintf(colstr,"\tvchar %s[%d];",
                                        (classp->elem)[i].elemname,
                                        (classp->elem)[i].size);
                        else
                                sprintf(colstr,"\tvchar %s;",
                                        (classp->elem)[i].elemname);
                        break;
                case ODM_LONGCHAR:
                        sprintf(colstr,"\tlongchar %s[%d];",
                                (classp->elem)[i].elemname,
                                 (classp->elem)[i].size);
                        break;
                case ODM_BINARY:
                        sprintf(colstr,"\tbinary %s[%d];",
                                (classp->elem)[i].elemname,
                                 (classp->elem)[i].size);
                        break;
                case ODM_METHOD:
                        sprintf(colstr,"\tmethod %s[%d];",
                                (classp->elem)[i].elemname,
                                 (classp->elem)[i].size);
                        break;
                case ODM_LINK:
/* 65208 */
                        if ( (lclassp = (classp->elem)[i].link) != NULL ) {
                        	sprintf(colstr,"\tlink %s %s %s %s[%d];",
                                lclassp->classname,
                                lclassp->classname,
                                (classp->elem)[i].col,
                                (classp->elem)[i].elemname,
                                (classp->elem)[i].size);
                        	break;
                        }
                        else {
                                fprintf(stderr,
                                  catgets(scmc_catd, MS_odmshow, SHOW_MSG_8,"odmshow: Could not access linked object class.\n") );
                        	exit(49);
                        }

                default:
                        fprintf(stderr,
                                  catgets(scmc_catd, MS_odmshow, SHOW_MSG_2,
                                    "odmshow: descriptor %s unknown type %d") ,
                                     (classp->elem)[i].elemname,type);
                        exit(60);
                        }
                fprintf(fh,"%-46s/* offset: 0x%x ( %d) */\n",
                        colstr,
                        e->offset,e->offset);
                }
        fprintf(fh,"\t};\n");

  fprintf(fh,"/*\n");
  fprintf(fh, catgets(scmc_catd, MS_odmshow, SHOW_MSG_3,
              "\tcolumns:        %d\n\tstructsize:     0x%x (%d) bytes\n"),
                classp->nelem,
                classp->structsize,classp->structsize);
  fprintf(fh, catgets(scmc_catd, MS_odmshow, SHOW_MSG_4,
                                 "\tdata offset:    0x%x\n") ,classp->data);
  rv = (int) odm_open_class(classp);
  if(rv == -1){
        fprintf(fh, catgets(scmc_catd, MS_odmshow, SHOW_MSG_5,
                                "\tCould not open class: %s, status %d\n"),
                                               class,odmerrno);
        }
  else {
        rv = raw_addr_class(classp);
        if(rv == -1){
                fprintf(fh, catgets(scmc_catd, MS_odmshow, SHOW_MSG_6,
                              "\tCould not address class: %s, status %d\n"),
                                             class,odmerrno);
                }
        else {
                num = count_obj(classp,"");
                fprintf(fh,
                    catgets(scmc_catd, MS_odmshow, SHOW_MSG_7,
                     "\tpopulation:     %d objects (%d active, %d deleted)\n"),
                        classp->hdr->ndata,
                        num,
                        classp->hdr->ndata - num);
                odm_close_class(classp);
                }
        }
  fprintf(fh,"*/\n\n\n");
}

/*
 * NAME:       count_obj
 *
 * FUNCTION:   Determines the number of objects in the database.
 *
 * RETURNS:    Number of objects in the database
 */
count_obj(classp,crit)
struct Class *classp;   /* Pointer to the object class */
char *crit;             /* Selection criteria.         */
{
int ngot;
char *s,*p;
        ngot = 0;
        s = p = NULL;
        while(  p = odm_get_obj(classp,crit,p,!ngot)){
                s = p;
                ngot++;
                }
        if(s)free(s);
        return(ngot);
}
