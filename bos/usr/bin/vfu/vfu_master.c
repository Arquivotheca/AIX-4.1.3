#ifndef lint
static char sccsid[] = "@(#)82 1.3 src/bos/usr/bin/vfu/vfu_master.c, cmdpios, bos411, 9437C411a 9/16/94 08:04:05";
#endif
/*
 * COMPONENT_NAME: (CMDPIOS)
 *
 * FUNCTIONS: vfu_master.c
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/*                      
*
*			    	MASTER MODULE
*			       ( vfu_master.c )
*
*	The aim is to control in-line command and to perform any necessary 
*	operation to save format file . The verbose option triggles interactive
*       facility .
*
*
*	WARNING : DO NOT ERASE ALL MESSAGES COMMENTED , THAT EASES MAINTENANCE 
*	
*	Author : BUI Q. M.
*	BULL S.A 
*	VERSION : 1.0 June 1990
*
*       Modified on 90/08/26, for use of printer PR88 and PR54
*       Modified on 90/09/13, for automatic printer-dependent file recoginition
*       Corrected on 90/09/15, for preserving file size upon saving when using
*       scrollbar mode
*       Modified on 90/09/21, for correcting incompatibility between printer
*       type configured and printer type of the file specified in command
*       Modified on 90/10/15, for correcting message display in case of
*       more than one occurence of any option encounted
*
*/

#include 	<curses.h>
#include 	<sys/types.h>
#include 	<dirent.h>
#include 	<unistd.h>
#include 	<malloc.h>
#include 	<memory.h>
#include 	<sys/dir.h>
#include 	<sys/stat.h>
#include 	<fcntl.h>
#include 	<string.h>
#include 	<ctype.h>
#include 	"vfu.h" 

#ifndef DEBUGLEVEL
#define DEBUGLEVEL 	3
#endif

static short 	fidesc,debug,mydebug,hotdebug,dialogue=0;

/* Messages */
static char 	USAGE[]= 	"USAGE: vfu [-p<printer_type>] [-d<country>] [-v] [-f<filename>] [-l<length>] [ -c<i>:<linestop> [ -c<j>:<linestop> ] ]\nlength = 1..144 , i = 1..12,country=f(e),printer_type=88(54a)(54b)";

#define NB_VFU_MSG 68

char *msgtab_f[NB_VFU_MSG] = {
"Argument doit etre superieur a 1 ou erronne ou trop grand",
"Chiffre uniquement",
"Canal 1(12) deja initialise, argument ignore",
"Warning : selection d'une meme ligne interdite",
"Warning : nom de fichier donne par defaut",
"Cette option est ignoree",
"Nom de fichier est tronque a",
"Nom de fichier par defaut est",
"Longueur de fichier est initialisee a la valeur par defaut",
"Longueur de fichier par defaut est",
"Argument inexistant, analyse du suivant",
"Fichier existe, le surcharger ? (oO/nN) ->",
"Plusieurs lignes de saut pour le canal 1 interdit",
"Plusieurs lignes de saut pour le canal 12 interdit",
"Premier caractere du nom doit etre un caractere",
"repertoire inexistant, le creer pour sauvegarder ce fichier",
"Limite atteinte, pas de changement",
"Echappement du mode curses",
"Fin de vfu",
"Fichier d'extension 'vfu' introuvable dans le repertoire courant",
"Longueur du format insignifiant, ignore",
"Ligne de saut 1 reservee a Haut de Page (canal 1)",
"-ieme ligne de saut reservee a Bas de Page (canal 12)",
"Ligne de saut 1 reservee a Haut de Page (canal 1), commande ineffective",
"-ieme ligne de saut reservee a Bas de Page (canal 12), commande ineffective",
"Nom de fichier inexistant, par defaut c'est",
"Chemin erronne ou droit refuse, essayez de nouveau un autre ...",
"Longueur est a changer ? (oO/nN) ->",
"Fichier existe, choisissez un autre nom",
"Le nom de fichier est donne par defaut",
"Erreur dans le codage de la page",
"Canaux et longueur non initialises",
"Numero canal excedant 2 digits ",
"Ligne de saut invalide ",
"Longueur imaginaire ",
"Repertoire courant inaccessible",
"Chemin erronne ou droit refuse",
"Variable d'environment 'VFU' erronnee ou droit refuse",
"Erreur : nombre argument",
"Digits attendus",
"Numero canal invalide",
"Option invalide",
"Numero canal invalide",
"Erreur de syntaxe : ':' attendu",
"Allocation de memoire impossible",
"Adresse correspondante a cette valeur inaccessilble",
"Codage impossible",
"Ouverture de fichier impossible",
"Creation de fichier impossible",
"Sauvegarde du fichier echouee",
"Le nom du fichier est donne par defaut",
"Le nom du fichier et sa longueur sont donnes par defaut",
"Caractere interdit",
"Caractere illegal ",
"Le champ complet doit etre saisi",
"caractere(s) au moins ",
"Tapez sur Enter si vous voudriez sauvegarder votre fichier",
"Tapez sur Enter si vous voudriez consulter un fichier format dans le repertoire courant",
"Tapez sur Enter si vous voudriez faire defiler dix lignes vers le haut",
"Tapez sur Enter si vous voudriez faire defiler dix lignes vers le bas",
"Tapez sur Enter si vous voudriez remettre a zero tout le fichier, soyez sur de votre intention",
"Tapez sur Enter si vous voudriez simplement quitter le programme",
"Fermeture du repertoire impossible",
"Type d'imprimante invalide",
"Type d'imprimante par defaut pourrait etre",
"Operande attendue entre les virgules",
"Canal reserve, commande ineffective",
"Ligne de saut reservee, commande ineffective"
};

char *msgtab_e[NB_VFU_MSG] = {
"Argument must be a number greater than 1 or corrupted  or toobig",
"Numeric character only",
"Channel 1(12) already set, skip to next",
"Warning : same line selected prohibited, not taken into account here",
"Warning : filename has been set to default name",
"This option is ignored",
"Filename is truncated at",
"Default filename is",
"Formlength is set to default value",
"Default length is",
"Inexistent argument, skip to next if any",
"File exists, overwrite ? (yY/nN) ->",
"Many linestops for channel 1 prohibited",
"Many linestops for channel 12 prohibited",
"First character of filename must be a character",
"directory not found, must be created for vfu temporary files salvage",
"Limit reached, no change made",
"Exit curses mode",
"Exit vfu -- Bye ",
"No file with 'vfu' extension found in current directory",
"Nonsense size file, not taken into account",
"Linestop 1 is reserved for Top Of Form (channel 1)",
"-th linestop is reserved for Bottom Of Form (channel 12)",
"Linestop 1 is reserved for Top Of Form (channel 1), command ineffective",
"-th linestop is reserved for Bottom Of Form (channel 12), command ineffective",
"Nonexistent filename, default is",
"Pathname corrupted or permission denied, try again please ...",
"Formlength is to be changed ? (yY/nN) ->",
"Filename exists, choose another one please",
"Default name is taken into account",
"Something wrong in mapping page ",
"Channel and formlength are not set up ",
"Channel number exceeds 2 digits ",
"Imaginary line number ",
"Imaginary formlength ",
"Working directory search fails ",
"Pathname corrupted or permission denied ",
"Environment variable 'VFU' corrupted or permision denied ",
"Error : arg count",
"Digits expected ",
"Invalid option ",
"Invalid channel number ",
"Invalid linestop number ",
"Syntax error : ':' expected ",
"Memory allocation fails ",
"Cannot get address corresponding to this value ",
"Cannot translate code ",
"Cannot open file ",
"Cannot creat file ",
"Trouble on saving file ",
"Default filename is given",
"Default filename and formlength are given",
"Prohibited character",
"Illegal character",
"Entire field to be filled up",
"character(s) at least ",
"Strike Enter key if you would like to save your file ",
"Strike Enter key if you would like to pick up some vfu format file in your current directory, if any",
"Strike Enter key if you would like to scroll ten lines up this displayed page ",
"Strike Enter key if you would like to scroll ten lines down this displayed page ",
"Strike Enter key if you would like to clear entirely this displayed page, make sure that this is your appropriated action",
"Strike Enter key if you would like to quit ",
"Trouble on closing directory",
"Invalid printer type ",
"Default printer type would be",
"Operand expected between commas",
"Channel reserved, command innefective",
"Linestop reserved, command innefective"
};

static char 	**msgtab;

/* Printer coding : pr88 -> 88
                    pr54 -> 54
                    .........
                    and so on */
static int 	vfu_printer=0,vfu_lpi=0;

/*********************************************************************
 *                  MAIN INTERFACE FOR OTHER MODULES                 * 
 *********************************************************************/

int 
get_debug()
{
	return(debug);
}

int 
get_mydebug()
{
	return(mydebug);
}

int 
get_country_dialogue()
{
	return(dialogue);
}

int 
get_printer_type()
{
	return(vfu_printer);
}

void 
set_printer_type(new)
int 	new;
{
	vfu_printer = new;
}

int 
get_printer_lpi()
{
	return(vfu_lpi);
}

void 
set_printer_lpi(new)
int 	new;
{
	vfu_lpi = new;
}

char 
**get_adr_map()
{
	return(msgtab);
}

/*********************************************************************
 *                   VFU TOOLS FOR DEBUGGING                         *
 *********************************************************************/

/* Viewing davfu array */
static void 
vfu_view_davfu(target,lg)
unsigned char 	*target;
{

	register int i;

	if (!dialogue)
		fprintf(stdout,"\nVIEWING DAVFU .........\n");
	else
		fprintf(stdout,"\nTRACE DU DAVFU .........\n");

	for(i=0;i<lg;i++) 
		fprintf(stdout,"line %d : %02x\n",i+1,target[i]);
}

/* View page */
static void 
vfu_view_page(target,heigth,large)
char 	**target;
int 	heigth,large;
{
	register int line,col;

	if (!dialogue) {
		fprintf(stdout,"\nVIEWING PAGE .........\n");
		fprintf(stdout,"Line                  C H A N N E L\n");
	}
	else {
		fprintf(stdout,"\nTRACE DE L'IMAGE DU FICHIER .........\n");
		fprintf(stdout,"Ligne               C   A   N   A   L\n");
	}
	fprintf(stdout,"           1  2  3  4  5  6  7  8  9  10 11 12\n");
	fprintf(stdout,"          _____________________________________\n");
	fprintf(stdout,"         |                                     |\n");
	for(line=0;line<heigth;line++) {
		fprintf(stdout,"     %4.d| ",line+1);
		for(col=0;col<large;++col) 
				fprintf(stdout,"%c  ",target[line][col]);
		fprintf(stdout,"|\n",target[line][large]);
	}
	fprintf(stdout,"         |_____________________________________|\n");
}

/* Messages initialization */
void 
vfu_init_msgtab()
{

	if (dialogue)
		msgtab = msgtab_f;
	else
		msgtab = msgtab_e;

}

/********************************************************************
 *                      VFU TOOL FOR PARSING                        *
 ********************************************************************/

/* Parse arguments related to option 'c' in in-line command ;
   syntax error and illegal values garble user command */
static void 
vfu_parse_channel(argconf)
char 	*argconf; /* option "c"'s argument */
{
	char 	*pacf,*pacf2;
	int 	val1,val2,i,shot=1;
	int 	mlines,oldl,koma=0;
	char 	*lset,*cset,**page_t,*get_adr_lset(),*get_adr_cset(),**get_adr_page_t();
	char unsigned *davfu_array,*get_adr_davfu();
	int 	folen;
	WINDOW 	*msgw,*vfu_get_adr_window();
	char 	hunter;

	msgw = vfu_get_adr_window(W_MSG);
	
	/* catch global variables addresses and value */
	lset = get_adr_lset();
	cset = get_adr_cset();
	page_t = get_adr_page_t();
	davfu_array = get_adr_davfu();
	folen = get_folength();
	hunter = ' ';
	
	if ((pacf2 =(char *)malloc((strlen(argconf)+1)*sizeof(char))) == NULL )
		syserr(msgw,msgtab[44]);
	strcpy(pacf2,argconf);

	/* koma count and check abnormal syntax */
	for(i=0;pacf2[i]!='\0';++i) {
		if ((hunter == ',') && (pacf2[i] == ',')) {
			fprintf(stdout,"--> %s\n",pacf2);
			warning(msgw,msgtab[65]);
			/* warning(msgw,"Operand expected between comma\n"); */
		}
		hunter = pacf2[i];
		if (pacf2[i] == ',') 
			++koma;
	}

	/* Check in advance if argument is in form : -ci:j,k,l...
	   therefore mlines is set */
	if (koma > 0 )
		mlines = 1 ;
	else 
		mlines = 0 ;

	/* Discard spurious channel argument */
	if (!isdigit(*argconf))
		warning(msgw,msgtab[1]);
		/* warning(msgw,CORRUPTCH); */

	/* Syntax check */
	if ((pacf=strtok(argconf,":")) == NULL ) {
		fprintf(stderr,"--> %s\n",pacf2);
		warning(msgw,msgtab[43]);
		/* warning(msgw,"SYNTAX ERROR : ':' expected -- Bye\n"); */
	}

	/* Check number of digits in channel argument */
	if (strlen(pacf) >2) {
		if (dialogue)
			fprintf(stderr,"--> %s a l'argument %s\n",pacf,pacf2);
		else
			fprintf(stderr,"--> %s in argument %s\n",pacf,pacf2);
		warning(msgw,msgtab[32]);
		/* warning(msgw,"Canal number exceeds 2 digits -- Bye\n"); */
	}
	
	/* Store this length */
	oldl = strlen(pacf)+strlen(":");
	/* channel number check */
	val1 = vfu_getn(pacf);

	if (val1 < 1 || val1 > 12) {
		fprintf(stderr,"--> %s\n",pacf2);
		fprintf(stderr,"--> %s : ",pacf);
		warning(msgw,msgtab[41]);
		/* warning(msgw,"Illegal channel number -- Bye\n"); */
	}

	/* Channel 1 or 12 must be set up with only one linestop 
	if (val1 == TOP || val1 == BOTTOM) { 
		fprintf(stderr,"--> %s\n",pacf2);
		fprintf(stderr,"--> %s : ",pacf);
		fprintf(stdout,"%s\n",msgtab[2]);
		fprintf(stdout,"Channel 1(12) already set , skip to next \n");
		return;
	}
	*/
	cset[val1-1]= 'd'; /* 'd' for 'done' */

loop:
	/* Form : -ci:j,k,l... */
	if (mlines)  {
		if (shot > 1) {
			/* get next token */
			if (koma > 0) 
				pacf = strtok(NULL,",");
			/* get last token */
			else if (koma == 0){
				pacf = strrchr(pacf2,',');
				/* skip ',' */
				++pacf;
			}
		}
		else  
			/* get first token without ':' ahead */
			pacf = strtok(argconf+oldl,",");

		if (!pacf) {
			fprintf(stdout,"--> %s\n",pacf2);
			warning(msgw,msgtab[65]);
			/* warning(msgw,"Operand expected between comma\n"); */
		}
	}		
	/* Form : -ci:j */
	else {
		/* get token */
		if ((pacf = strchr(pacf2,':')) == NULL ) 
			warning(msgw,msgtab[43]);
			/* warning(msgw,"Syntax error, ':'expected -- Bye\n"); */
		/* skip ':' */
		++pacf;
	}

	if (!isdigit(*pacf)) {
		fprintf(stdout,"--> %s\n",pacf2);
		warning(msgw,msgtab[39]);
		/* warning(msgw,"Digits expected\n"); */
	}

	if (koma < 0) 
		return;

	/* Linestop number check */
	val2 = vfu_getn(pacf);
	/* line number check */
	if ( val2 < 0 || val2 > folen) {
		fprintf(stderr,"--> %s\n",pacf2);
		fprintf(stderr,"--> %s : ",pacf);
		warning(msgw,msgtab[42]);
		/* warning(msgw,"Imaginary line number -- Bye \n"); */
	}

	if ( val2 == 1) {
		fprintf(stderr,"--> %s\n",pacf2);
		fprintf(stderr,"--> %s : ",pacf);
		fprintf(stdout,"%s\n",msgtab[21]);
		if (dialogue)
			fprintf(stdout,"Votre choix au canal %d est ineffective\n",val1);
		else
			fprintf(stdout,"Your choice for channel %d is garbled\n",val1);
		goto zap;
	}
	if ( val2 == folen) {
		fprintf(stderr,"--> %s\n",pacf2);
		fprintf(stderr,"--> %s : %d",pacf,folen);
		fprintf(stdout,"%s\n",msgtab[22]);
		/* fprintf(stdout,"This linestop is reserved for Bottom Of Form ,"); */
		if (dialogue)
			fprintf(stdout,"votre choix au canal %d est ineffective\n",val1);
		else
			fprintf(stdout,"your choice for channel %d is garbled\n",val1);
		goto zap;
	}
	/* Prevent selection on same line
	if (lset[val2-1] == 'd' ) {
		fprintf(stdout,"--> %s , argument %s\n",pacf,pacf2);
		fprintf(stdout,"%s\n",msgtab[3]);
		fprintf(stdout,Warning : same line prohibited , not taken in account\n");
		goto zap;
	}
	else
		lset[val2-1] = 'd'; set done 
	*/
	lset[val2-1] = 'd'; /* set done */ 

	/* impact only if limit not reached */
	if (vfu_hard_code(msgw,davfu_array,2*folen,val2,val1))
		page_t[val2-1][val1-1] = 'X'; /* print page to show */

zap:
	++shot;
	--koma;
	if (!mlines) 
		return;
	goto loop;

}

/********************************************************************
 *                      VFU INLINE HELP MENU                        *
 ********************************************************************/

/* English help menu */
static void
vfu_help_onlineE()
{

	char answer;

	fprintf(stdout,"\t\t\t------------------------------\n");
	fprintf(stdout,"\t\t\t------------ HELP ------------\n");
	fprintf(stdout,"\t\t\t------------------------------\n");
	fprintf(stdout,"\tUse arrow keys to move anywhere in vfu page or scrollbar menu\n");
	fprintf(stdout,"\t            OR\n");
	fprintf(stdout,"\tStrike key into vfu page :\n");
	fprintf(stdout,"    u or U or - to move up one row\n");
	fprintf(stdout,"    d or D or + to move down one row\n");
	fprintf(stdout,"    p or P to scroll up ten rows \n");
	fprintf(stdout,"    n or N to scroll down ten rows \n");
	fprintf(stdout,"    l or L or < to move backward one column\n");
	fprintf(stdout,"    r or R or > or space bar to move forward one column\n");
	fprintf(stdout,"    z or Z or 0 to zero array location\n");
	fprintf(stdout,"    x or X to impact array\n");
	fprintf(stdout,"    s or S to save file\n");
	fprintf(stdout,"    q or Q to quit program \n");
	fprintf(stdout,"    e or E to escape and use scrollbar menu below vfu page;\n");
	fprintf(stdout,"    when choice is done ,type return\n");
	fprintf(stdout,"\t\t\t\t\t Version 1.0 June 1990\n\n");
	fprintf(stdout,"\tHit return to continue ...\n\n");
	answer = getchar();

}

/* French help menu */
static void
vfu_help_onlineF()
{

	char answer;

	fprintf(stdout,"\t\t\t------------------------------\n");
	fprintf(stdout,"\t\t\t------------ AIDE ------------\n");
	fprintf(stdout,"\t\t\t------------------------------\n");
	fprintf(stdout,"\tUtilisez les touches flechees pour deplacer le cursor dans la page vfu ou dans le menu barre deroulante\n");
	fprintf(stdout,"\t             OU\n");
	fprintf(stdout,"\tLorsque vous etes dans la page vfu , vous pouvez appuyer sur la touche :\n");
	fprintf(stdout,"    u ou U ou - pour monter le curseur d'une ligne\n");
	fprintf(stdout,"    d ou D ou + pour descendre le curseur d'une ligne\n");
	fprintf(stdout,"    p ou P pour defiler une page de dix lignes vers le bas\n");
	fprintf(stdout,"    n ou N pour defiler une page de dix lignes vers le haut\n");
	fprintf(stdout,"    l ou L ou < pour deplacer le curseur d'une colonne a gauche \n");
	fprintf(stdout,"    r ou R ou > ou barre d'espacement pour deplacer le curseur d'une colonne a droite \n");
	fprintf(stdout,"    z ou Z ou 0 pour remettre a zero la selection de la ligne de saut\n");
	fprintf(stdout,"    x ou X pour marquer la ligne de saut du canal\n");
	fprintf(stdout,"    s ou S pour sauvegarder le fichier format\n");
	fprintf(stdout,"    q ou Q pour quitter le programme\n");
	fprintf(stdout,"    e ou E pour utiliser le menu barre deroulante;\n");
	fprintf(stdout,"    puis tapez retour chariot lorsque le choix est fait;\n");
	fprintf(stdout,"\t\t\t\t\t Version 1.0 Juin 1990\n\n");
	fprintf(stdout,"\tTapez retour chariot pour continuer...\n\n");
	answer = getchar();

}

/* Check incompatibility between printer-type specified in in-line 
   command and those of format file, if any */
int
vfu_check_intruder(win,filename)
WINDOW	*win;
char	*filename;
{
	struct stat	bufstatfn;	
	int	foo,t;
	unsigned char hook;

	if (stat(filename,&bufstatfn) == 0) {
		/* check incompatibility between pflag and printer type of 
		   existing file and correct it, if any */
		if (bufstatfn.st_size % 2 == 0) 
			vfu_printer = 88;
		if (bufstatfn.st_size % 2 != 0) {
			vfu_printer = 54;
			if ((foo=open(filename,O_RDONLY)) < 0)
				syserr(win,msgtab[47]);
			for(t=0;t<2;t++)
				read(foo,&hook,sizeof(hook));
			close(foo);
			vfu_lpi = (hook==0xEC)?6:8;
		}
		return(1);
	}
	return(0);
}

/*********************************************************************
 *                             MAIN BLOCK                            *
 *********************************************************************/

main(argc,argv)
int 	argc;
char 	**argv;
{
 	static char *SCCS = " %SRCS% "; 
	short 		vflag=0,
			fflag=0,
			pflag=0,
			lflag=0,
	    		cflag=0,
	    		dflag=0,
	    		xflag=0,
			pr_file=0,
			dfltln=0;
	int 		i,na=0,nb=0;
	int 		fd,j;
	int 		mysize,lgdavfu;
	struct stat 	bufstatfn;
	char 		answer,*confstr;
	char 		debstr[4],*wdir,mywdir[BUFSIZE],c;
	char 		*str1,*get_adr_tmpstr1(),
	     		*str2,*get_adr_tmpstr2(),
	     		*fnpath,*get_adr_fnpanic(),
	     		*lset,*get_adr_lset(),
	     		**page_t,**get_adr_page_t();
	unsigned char 	*davfu_array,*get_adr_davfu();
	int 		folen,in_default=0;
	WINDOW 		*win=(WINDOW *) 0,*curses_mode,*vfu_get_adr_window(),*get_curses_mode();
	char		*vfuenv,*myenv;
	char		country[10];
	char		copra[BUFSIZE];


	str1 = get_adr_tmpstr1();
	for(i=0;i<MAXFILENAME+4+1;i++)
		*(str1+i) = ' ';
	str2 = get_adr_tmpstr2();
	for(i=0;i<MAXDIGIT+1;i++)
		*(str1+i) = ' ';
	fnpath = get_adr_fnpanic();
	for(i=0;i<BUFSIZE+1;i++)
		*(fnpath+i) = ' ';
	folen = get_folength();
	
	vfu_dispatch_msg();

	/* Message dialogue set flag */
	if ((getenv("LANG")) != NULL) {
		strcpy(country,getenv("LANG"));
		if  (strcmp(country,"fr_FR") == 0 || strcmp(country,"Fr_FR") == 0)
			dialogue = 1;
	}
	vfu_init_msgtab();
	vfu_trap_adrmap();
	vfu_dispatch_msg();

	if (argc < 2) {
		vfu_printer = 88;
		/* make temporary filename tmp$$$$$ , $$$$$ = pid */
		sprintf(str1,"%s\0",mktemp("vfuXXXXXX"));
		fprintf(stdout,"%s %s%s\n",msgtab[7],str1,SAME);
		/* fprintf(stdout,"Default length is "); */
		folen = DFLTLENGTH;
		fprintf(stdout,"%s %d\n",msgtab[9],folen);
		sprintf(fnpath,"%s%s%s%s\0",PANICDIR,"/",str1,SAME);
		set_folength(folen);
		vfu_make_room_all(folen);
		vfu_init_all(folen);
		if (access(PANICDIR,F_OK) < 0) {
			fprintf(stdout,"%s ",PANICDIR);
			warning(win,msgtab[15]);
			/* warning(win,"directory doesn't exist,has to be created for vfu temporary files salvage\n"); */
		}	
		/* Write file */
		if ((fidesc = open(fnpath,O_RDWR|O_CREAT|O_TRUNC,S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) < 0) {
			fprintf(stderr,"--> %s\n",fnpath);
			syserr(win,msgtab[47]);
			/* syserr(win,"Cannot open file -- Bye\n"); */
		}
		davfu_array = get_adr_davfu();
		vfu_save_file(win,fidesc,davfu_array);
		close(fidesc);
		exit(0);
	}

	/************** parse command ****************/

	while ( argc > 1 && argv[1][0] == '-') {
		if (argv[1][1] == '\0') 
			break;

		switch (argv[1][1]) {

			/* printer type */
			case 'p' :
				/* take into account the first, if there are 
				   more than one occurence */
				if (pflag || vfu_printer)
					break;

				pflag = 1;
				/* case with argument */
				if (argv[1][2] != '\0') {
					
					if (strcmp((argv[1]+2),"88") == 0) {
						if (fflag && vfu_printer == 88)
							break;
						vfu_printer = 88;
						vfu_trap_tprinter();
						fprintf(stdout,"Configuration type : PR88\n");
					}
#ifdef BULL_PRG
					/*
					 * on BOSX the printer actually
					 * supporting the VFU is the PR88
					*/
					if (strcmp((argv[1]+2),"88") != 0) {
					/* if spurious value */
					fprintf(stdout,"--> %s : %s\n %s PR88\n",argv[1]+2,msgtab[63],msgtab[64]);
					vfu_printer = 88;
					vfu_trap_tprinter();
					}
#else
					if ( (strlen(argv[1]+2) == 2 && strcmp((argv[1]+2),"54") == 0) || (strlen(argv[1]+2) == 3 && strcmp((argv[1]+2),"54a") == 0) ) {
						if (fflag && vfu_printer == 54 && vfu_lpi == 6)
							break;
						vfu_printer = 54;
						vfu_lpi = 6;
						vfu_trap_tprinter();
						fprintf(stdout,"Configuration type : PR54, %d lines/inch\n",vfu_lpi);
					}
						
					if (strlen(argv[1]+2) == 3 && strcmp((argv[1]+2),"54b") == 0)  {
						if (fflag && vfu_printer == 54 && vfu_lpi == 8)
							break;

						vfu_printer = 54;
						vfu_lpi = 8;
						vfu_trap_tprinter();
						fprintf(stdout,"Configuration type : PR54, %d lines/inch\n",vfu_lpi);
					}

					if (strcmp((argv[1]+2),"88") != 0 && strcmp((argv[1]+2),"54") != 0 && strcmp((argv[1]+2),"54a") != 0 && strcmp((argv[1]+2),"54b") != 0 ) {
					/* if spurious value */
					fprintf(stdout,"--> %s : %s\n %s PR88\n",argv[1]+2,msgtab[63],msgtab[64]);
					vfu_printer = 88;
					vfu_trap_tprinter();
					}
#endif
				} /* if */
				/* default case */
				else {
					/* if argument not specified */
					vfu_printer = 88;
					if (!lflag && !cflag)
						fprintf(stdout,"%s PR88\n",msgtab[64]);
					vfu_trap_tprinter();
				}
				/* the goal of these codes is to correct the printer type with regard to the file which exists in directory specified in argument of '-f' flag */ 
				if (fflag) {
					char	*crunch,*korrect1,*korrect2;

					if (str1[0] == '/')
						strcpy(copra,str1);
					else {
						strcpy(copra,"./");
						strcat(copra,str1);
					}
					strcat(copra,SAME);
					if (vfu_check_intruder(win,copra)) {
						korrect1 = (dialogue)?"configure est corrige au type":"configured is corrected to the type";
						korrect2 = (dialogue)?"du fichier":"of file";
						crunch = "88";
						if (vfu_printer == 54) {
							if (vfu_lpi == 8) 
								crunch = "54b";
							if (vfu_lpi == 6) 
								crunch = "54a";
						}
						fprintf(stderr,"Printer type %s %s %s%s : PR%s\n",korrect1,korrect2,str1,SAME,crunch);
						vfu_trap_tprinter();
					}
				pr_file = 1;
				}
				break;

			/* country language */
			case 'd' :
				/* take into account the first, if there are 
				   more than one occurence */
				if (dflag)
					break;

				dflag = 1;
				if (argv[1][2] != '\0' && (argv[1][2] == 'f' || argv[1][2] == 'F')) 
					dialogue = 1;
				else 
					dialogue = 0;
				vfu_init_msgtab();
				vfu_trap_adrmap();
				vfu_dispatch_msg();
				break;
 
			/* Use of friendly-user interface */
			case 'v':
				vflag = 1;
				break;

			/* Ease user for naming file, the printer type of the
			   file, if any, has the highest priority over those 
                           set by user in command */
			case 'f':
				/* take into account the first, if there are 
				   more than one occurence
				   just output message*/
				if (fflag) {  /* already passed here */
					if (!na)
						fprintf(stdout,"%s\n",msgtab[4]);
					fprintf(stdout,"--> %s ",argv[1]);
					fprintf(stdout,": %s\n",msgtab[5]);
					++na;
					break;
				}

				fflag = 1;
				/* case pflag is not specified, set to 
                                   default value */
				if (!pflag)
					vfu_printer = 88;

				/* catch name as it is */
				if ( strlen(argv[1]) > 2 ) {
					/* catch filename */
					i=strlen(argv[1]+2);
					if (i > MAXFILENAME) {
						fprintf(stdout,"%s\n",msgtab[6]);
						/* fprintf(stdout,"Filename is truncated at "); */
						fprintf(stdout,"%d\n",MAXFILENAME);
						strncpy(str1,argv[1]+2,MAXFILENAME);
					}
					else 
						sprintf(str1,"%s\0",argv[1]+2);
					
				}
				else { 
					/* filename omitted */
					fflag = 1;
					/* make temporary filename vfu$$$$$ , $$$$$ = pid */
					sprintf(str1,"%s\0",mktemp("vfuXXXXXX"));
					fprintf(stdout,"%s %s%s\n",msgtab[7],str1,SAME);
					/* fprintf(stdout,"Default filename is "); */
					in_default = 1;
				}	
				/*
				mypt = str1;
				for(myi=0;*mypt != '\0';myi++)  
					str1[myi] = tolower(*mypt++);
				*/

				if (str1[0] ==  '/')
					strcpy(copra,str1);
				else {
					strcpy(copra,"./");
					strcat(copra,str1);
				}
				strcat(copra,SAME);
				/* force to reset printer type to the value of 
				   the file, if any */
				vfu_check_intruder(win,copra);

				/* the goal of these codes is to correct the 
                                   printer type with regard to the file which 
                                   exists in directory specified in argument 
                                   of '-f' flag */ 
				if (pflag || vfu_printer) {
					char	*crunch,*korrect1,*korrect2;

					if (vfu_check_intruder(win,copra)) {
						korrect1 = (dialogue)?"configure est corrige au type":"configured is corrected to the type";
						korrect2 = (dialogue)?"du fichier":"of file";
						crunch = "88";
						if (vfu_printer == 54) {
							if (vfu_lpi == 8) 
								crunch = "54b";
							if (vfu_lpi == 6) 
								crunch = "54a";
						}
						fprintf(stderr,"Printer type %s %s %s%s : PR%s\n",korrect1,korrect2,str1,SAME,crunch);
						vfu_trap_tprinter();
					}
				pr_file = 1;
				}
				break;
			
			case 'l':
				if (!vfu_printer) {
					vfu_printer = 88;
					vfu_trap_tprinter();
					if (!cflag && !pflag)
						fprintf(stdout,"%s PR88\n",msgtab[64]);
				}
				/* take into account the first, if there are 
				   more than one occurence */
				if (lflag) {
					char *yes;

					if (dfltln == 1) {
						fprintf(stdout,"%s\n",msgtab[8]);
						/* fprintf(stdout,"Formlength is set to default value\n"); */
					}
					else {
						if (!nb) {
							yes =(dialogue)?"Longueur deja initialisee":"Formlength already set";
							fprintf(stdout,"%s\n",yes);
						}
					}

					fprintf(stdout,"--> %s ",argv[1]);
					fprintf(stdout,": %s\n",msgtab[5]);
					/* fprintf(stdout,"This option is ignored\n"); */
					++nb;
					break;
				}
				lflag = 1;

				/* make default length if necessary */
				if (strlen(argv[1]) == 2) {
					fprintf(stdout,"%s %d\n",msgtab[9],DFLTLENGTH);
					/* fprintf(stdout,"Default length is "); */
					/* the aim of this code portion is to prevent from no 'c' option */
					sprintf(str2,"%d\0",DFLTLENGTH);	
					folen = DFLTLENGTH ;
					set_folength(folen) ;
					set_all_is_set(IS_ON);
					vfu_make_room_all(folen);
					vfu_init_all(folen);
					dfltln = 1;
					break;
				}
				else {
					if (!isdigit(argv[1][2])) {
						fprintf("--> %s\n",argv[1]);
						warning(win,msgtab[39]);
						/* warning(win,"Digits expected -- Bye\n"); */
					}
					sprintf(str2,"%s\0",argv[1]+2);	
					folen = vfu_getn(argv[1]+2);
					set_folength(folen);
				}
				
				if (folen < 3 || folen > MAXFORMLENGTH) 
					warning(win,msgtab[34]);
					/* warning(win,"Imaginary formlength\n"); */
				/* make enough room and init arrays */
				lgdavfu = (vfu_printer == 88)?folen*2+5:2*folen+4;
				set_all_is_set(IS_ON);
				vfu_make_room_all(folen);
				vfu_init_all(folen);
				break;

			/* Must have formlength in order to set up davfu array,
                           so that if not in command , assume this as a big 
                           form one */
			case 'c':
				cflag = 1;

				if (!vfu_printer) {
					vfu_printer = 88;
					vfu_trap_tprinter();
					if (!lflag && !pflag)
						fprintf(stdout,"%s PR88\n",msgtab[64]);
				}
				if (strlen(str1) != 0) {
					if (str1[0] ==  '/')
						strcpy(copra,str1);
					else {
						strcpy(copra,"./");
						strcat(copra,str1);
					}
					strcat(copra,SAME);
				}
				/* if file exists, check */
				if (vfu_check_intruder(win,copra))
					goto dealer;

				vfu_trap_tprinter();
				if (!lflag) { 
					lflag = 1;
					folen = DFLTLENGTH;
					set_folength(folen) ;
					sprintf(str2,"%d\0",folen);	
					/* make enough room and init arrays */
					set_all_is_set(IS_ON);
					vfu_make_room_all(folen);
					vfu_init_all(folen);
					fprintf(stdout,"%s %d\n",msgtab[8],DFLTLENGTH);
					/* fprintf(stdout,"Default length is "); */
				}

				if (strlen(argv[1]) == 2) {
					fprintf(stdout,"%s",msgtab[10]);
					/* fprintf(stdout,"Inexistent argument , skip to next if any\n"); */
					break;
				}
				goto myparse;

			/* reset to file's printer type if necessary */
			dealer:
				if (strlen(copra) != 0) {
					stat(copra,&bufstatfn);
					/* if lflag not set, use old size */
					if (bufstatfn.st_size % 2 == 0)
						if (!lflag)
							folen = (bufstatfn.st_size-2)/2 - 1;
					if (bufstatfn.st_size % 2 != 0)
						if (!lflag)
							folen = (bufstatfn.st_size-3)/2;
				}
				sprintf(str2,"%d\0",folen);	
				set_folength(folen);
				lgdavfu = (vfu_printer == 88)?folen*2+5:2*folen+4;
				vfu_trap_tprinter();
				set_all_is_set(IS_ON);
				vfu_make_room_all(folen);
				vfu_init_all(folen);
				/*
				if (!lflag) {
					set_all_is_set(IS_ON);
					vfu_make_room_all(folen);
					vfu_init_all(folen);
				}
				else
					vfu_expand_room_all(mysize,folen,MAXCHANNEL);

				*/
				pr_file = 1;
			myparse:
				/* make arrays if no lflag set at the beginning */
				if ( (confstr=(char *)malloc((strlen(argv[1]+2)+1)*sizeof(char))) == NULL) 
					syserr(win,msgtab[44]); /* exit here */
				/* catch its argument */
				strcpy(confstr,argv[1]+2);
				vfu_parse_channel(confstr);
				free(confstr);
				break;

			/* fire user !!! */
			default:
				fprintf(stderr,"%s",msgtab[40]);
				/* fprintf(stderr,"INVALID OPTION "); */
				fprintf(stderr,"(%s)\n",argv[1]);
				fprintf(stderr,"%s\n",USAGE);
				exit(2);	
		} /* switch */
	argc--;
	argv++;
	}

	/* Default printer type must be set here, if pflag and pr_file not found*/
	if (!pflag && !pr_file) {
		vfu_printer = 88;
		vfu_trap_tprinter();
		if (!lflag && !cflag)
			fprintf(stdout,"%s PR88\n",msgtab[64]);
	}

	if (vflag) {
		if (dialogue)
			fprintf(stdout,"Analyse de la commande achevee avec succes...\n");
		else 
			fprintf(stdout,"Successful command parsing ...\n");
	}
	
	/* Get array's addresses */
	page_t = get_adr_page_t();
	lset = get_adr_lset();
	davfu_array = get_adr_davfu();

	/************** set debug variables ****************/

	if ((vfuenv=(char *)malloc(BUFSIZE*sizeof(char))) == NULL)
		syserr(win,msgtab[44]); /* exit here */

	/* If VFU variable is set in environment , check write permission */
	if (getenv("VFU") != NULL) {
		DIR 	*rv;
		int 	dirp;

		strcpy(vfuenv,getenv("VFU"));
		/* check pathname */
		if ((rv=opendir(vfuenv)) == NULL)  {
			free(vfuenv);
			syserr(win,msgtab[37]); /* exit here */
			/* syserr(win,"Pathname given in environment variable VFU is corrupted or permision denied -- Bye\n"); */
		}
		close(rv) ;
	}
	else {
		putchar(07);
		if (vflag) {
			if (dialogue)
				fprintf(stdout,"LA VARIABLE D'ENVIRONNEMENT VFU N'EST PAS INITIALISEE , LE REPERTOIRE CIBLE EST LE REPERTOIRE COURANT\n"); 
			else
				fprintf(stdout,"VFU ENVIRONMENT VARIABLE NOT FOUND , TARGET DIRECTORY IS CURRENT DIRECTORY\n");
		}
		if ((myenv=getcwd(vfuenv,BUFSIZE)) == NULL)
			warning(win,msgtab[35]);
			/* warning(win,"Working directory search fails -- Bye\n"); */
	}

	if (!xflag) {
		/* Set debug variables for any debugging facility */
		/* Catch absolute path of working directory */
		/* Catch debug value if file exist , if not set it to default value */
		strcpy(mywdir,vfuenv);
		strcat(mywdir,"/DEBFILE.VFU");
		if ((fd=open(mywdir,O_RDONLY)) < 0) {
			debug = DEBUGLEVEL;
			mydebug = debug +1;
		}
		else {
			read(fd,debstr,sizeof(debstr)) ;
			close(fd);
			if (strlen(debstr) > 1) {
				debug = atoi(&debstr[0]);
				mydebug = atoi(debstr+1);
			}
			hotdebug = 1;
		}
	}

	/* If not in verbose mode, do clean job before saving file */
	 if (!vflag) {
		/* At this stage if fflag = false , set to default name */
		/* No fflag or no argument given and in_default is set */
		if (!fflag || in_default) {
			if (access(PANICDIR,F_OK) < 0) {
				fprintf(stdout,"%s ",PANICDIR);
				warning(win,msgtab[15]);
				/* warning(win,"directory doesn't exist,has to be created for vfu temporary files salvage\n"); */
			}	
			sprintf(str1,"%s",mktemp("vfuXXXXXX"));
			sprintf(fnpath,"%s%s%s%s\0",PANICDIR,"/",str1,SAME);
			/* fprintf(stdout,"%s ",msgtab[7]);
			fprintf(stdout,"Default filename is "); 
			fprintf(stdout,"%s%s\n",str1,SAME); */
			goto endup;
		}
		else {
			/* check pathname */
			if (strchr(str1,'/') == NULL)
				sprintf(fnpath,"%s%s%s\0",vfuenv,"/",str1);
			else {

				if (str1[0] != '/') 
					sprintf(fnpath,"%s%s%s\0",vfuenv,"/",str1);
				else 
					sprintf(fnpath,"%s\0",str1);

			}
			strcat(fnpath,SAME);

			/* if do not exist then creat */
			if ((fd=open(fnpath,O_RDONLY)) > 0) { 
				close(fd);
				fprintf(stderr,"%s ",msgtab[11]);
				/* fprintf(stdout,"File exists , overwrite ? (yY/nN) ->  "); */
				scanf("\n%c",&answer);
				if (answer == 'N' || answer == 'n')
					goto schluss;
			}
		} /* else fflag */
	free(vfuenv);

	endup :
		/* No channel set, create default file with 66 length */
		if (!cflag) {
			if (!lflag) {
				fprintf(stdout,"%s\n",msgtab[31]);
				/* syserr(win,"Channel and formlength are not set up -- Bye\n"); */
				folen = DFLTLENGTH;
			}
			lgdavfu = (vfu_printer == 88)?folen*2+5:2*folen+4;
			set_folength(folen);
			set_all_is_set(IS_ON);
			vfu_make_room_all(folen);
			vfu_init_all(folen);
		}

		/* Write file */
		if ((fidesc = open(fnpath,O_RDWR|O_CREAT|O_TRUNC,S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) < 0) {
			fprintf(stderr,"--> %s\n",fnpath);
			syserr(win,msgtab[47]);
			/* syserr(win,"Cannot open file -- Bye\n"); */
		}
		davfu_array = get_adr_davfu();
		vfu_save_file(win,fidesc,davfu_array);
		close(fidesc);
		goto schluss;

	 } /* if vflag */ 

	if (!dialogue) {
		fprintf(stdout,"\nDIRECTORY FOR FILES SALVAGE WOULD BE %s NOW\n",vfuenv);
		fprintf(stdout,"BEWARE : do not give %s as basename in Filename field because your given filename will be appended further .\n",vfuenv);
		fprintf(stdout,"If filename is omitted , default is given and target salvage directory is %s .\n",PANICDIR);
		fprintf(stdout,"Be sure that %s exists and have all rights and move all format files into appropriated directory. \n",PANICDIR);
		fprintf(stdout,"Directories and subdirectories mentionned must exist too and read_write_execute rights should have been set.\n");
		fprintf(stdout,"Hit return to continue ...\n\n");
	}
	else {
		fprintf(stdout,"\nREPERTOIRE DE SAUVEGARDE POURRAIT ETRE MAINTENANT %s\n",vfuenv);
		fprintf(stdout,"ATTENTION : ne pas donner %s comme racine du nom de fichier format ,il sera concatene automatiquement durant le traitement .\n",vfuenv);
		fprintf(stdout,"Si le nom du fichier est omis, le systeme en donne par defaut et la sauvegarde est faite dans le repertoire %s . \n",PANICDIR);
		fprintf(stdout,"Verifiez l'existence du repertoire %s et ses droits et transferez les fichiers format qui s'y trouvent dans votre repertoire approprie.\n",PANICDIR);
		fprintf(stdout,"Assurez que vos repertoires et sous-repertoires existent et ont des droits r(ead)w(rite)x(ecute)\n");
		fprintf(stdout,"Tapez retour chariot (Enter) pour continuer ...\n\n");
	}
	answer = getchar();

	/* short help menu display */
	if (dialogue)
		vfu_help_onlineF();
	else
		vfu_help_onlineE();

	/* crucial computional stage */
	vfu_operate(vflag,fflag,lflag,cflag);

schluss :
	if ((curses_mode=get_curses_mode()) != (WINDOW *) 0) 
		vfu_escape_curses();

	DEBUG(mydebug) {
		fprintf(stdout,"\n\n\t.................DEBUG..................\n\n");
		folen = get_folength();
		lgdavfu = (vfu_printer == 88)?folen*2+5:2*folen+4;
		davfu_array = get_adr_davfu();
		vfu_view_davfu(davfu_array,lgdavfu);
		page_t = get_adr_page_t();
		vfu_view_page(page_t,folen,MAXCHANNEL);
	}

	free(davfu_array);
	free(page_t);
	free(lset);
	exit(0);
}
