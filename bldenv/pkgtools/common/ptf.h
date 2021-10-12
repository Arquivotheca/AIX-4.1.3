/* @(#)50       1.9  src/bldenv/pkgtools/common/ptf.h, pkgtools, bos41J, 9512A_all 3/3/95 17:15:53 */
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

typedef enum {IFREQ, COREQ, PREREQ} reqtype; 


#define		PTFLEN		8	/* allow for terminating NULL */
#define		SUBSYSLEN	40
#define		FILESETLEN	60
#define		BUFSIZE		1024
#define		VRMFSIZE	16
#define		MODFIXLEN	4

typedef struct {
	char version[3];
	char release[3];
	char mod[5];
	char fix[5];
} vrmftype;

/*-----------------------------------------------
| Message variables ---- See processPtfmsg.c    |
------------------------------------------------*/
extern  char    *commandName;
extern  char    *AllFlagsNotSpecified;
extern  char    *BuildTypeNotSet;
extern	char	*CouldNotGetVRMF;
extern	char	*CouldNotOpenForVRMF;
extern  char    *InvalidFileFormat;
extern  char    *InvalidFileset;
extern  char    *InvalidPtf;
extern  char    *InvalidReq;
extern	char	*InvalidVRMF;
extern  char    *MultipleEntries;
extern  char    *MultiplePtfs;
extern  char    *MultipleFilesetsForFile;
extern  char    *MultipleFilesetForPtf;
extern  char    *FilesetNotFound;
extern  char    *PtfoptionsMismatch;
extern	char	*ReqNotInPtfOptFile;
extern  char    *SelfixTopNotSet;
extern  char    *StatFailed;
extern  char    *Usage;
extern  char    *VRMFNotFound;
extern  char    *WriteError;

/*--------------------------------------------
| Function Declarations.                      |
----------------------------------------------*/
void		addNewPtfInfo();
void		addInfo(char *buf, FILE *listFp, char *listFile);
int		addAparInfo (FILE *outFp, FILE *inFp, char *apar, char *fileName);
void		addRequisites (char *fileset, char *specialPtfType, char *lastCumPtf,
				 int overrideVRMF, char *top, char *internalTable, char *buildCycle);
void		addTreeReqs (char *listFile, reqtype reqType, char *top, 
				char *internalTable);
void		basePrereq (FILE *prereqFp, char *fileset, char *top, char *buildCycle);
void 		bumplevel( vrmftype vrmfentry, vrmftype *ptfvrmf, char *specialPtfType);
void    	createInfoFiles (char *top);
void		checkInternalTable(char *newPtf, char *fileset, char *internalTable);
int     	checkPtfOptions (char *currentPtf, char *fileset, char *vrmf, 
					char *fileName);
void    	checkSize (char * fileName);
void		createEmptyLists ();
void		createList (char *lastPtfDir, char *listFile, char *top, 
				char *internalTable, int fakePtfFlag);
void		createListFiles (char *filesetDir, char *lastPtf, char *top, 
				char *internalTable, int fakePtfFlag);
void		generateVRMF(char *fileset, char *bldcycle, 
				vrmftype *ptfvrmf, char *top, char *specialPtfType);
void    	getFileset (char *buf, char *fileset, char *fileName);
void    	getLastPtf (char *listFile, char *lastPtf);
void		getNextPtf (char *listFile, char *Ptf, char *nextPtf);
void    	getPtfInfo (char *ptfName, char *fileset, char *specialPtfType);
int		getValidPtf (char *Ptf, char *top, char *internalTable);
void    	getVRMF (char *fileset, char *top, char *vrmf, char *buildCycle);
void		incrstring(char * str);
void    	processPtf (char *fileset, char *top, int overrideFlag,
			 char *specialPtfType, char *internalTable, char *buildCycle, int fakePtfFlag);
char    	*stripBlanks (char *buf);
int		searchListFile ( char *listFileName, char *entry );
void    	splitVRMF (char *fileset, char *vrmf, vrmftype *vrmfentry);
int		stripComments (FILE *insfp, char *line);
void    	usage ();
