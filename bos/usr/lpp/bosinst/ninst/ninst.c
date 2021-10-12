static char sccsid[] = "@(#)38	1.8  src/bos/usr/lpp/bosinst/ninst/ninst.c, bosinst, bos411, 9428A410j 5/11/94 07:12:46";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS: ninst - network install service client
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <varargs.h>
#include <sys/stat.h>


#define HEADER_VERSION "2.0"

#define CMDBUFLEN 16384          
#define LENBUF 16

int it;				/* r/w file descriptor to other machine */
int netsopen();			/* Open a stream connection */
char *progname;			/* argv[0] for error messages */
char buf[CMDBUFLEN];

main(argc,argv,envp)
int argc;
char *argv[];
char *envp[];

{
   int i;
   int readlen, rc;
   char *who  = argv[1];
   char *what = "instsrv";
   char lenbuf[LENBUF];

   progname = argv[0];
	
   if(argc < 3) {
      fprintf(stderr,"usage: %s <hostname>|<ip_address> <command>\n", progname);
      exit(1);
   }

   it = netsopen(who,what);
   if (it < 0) {
      fprintf(stderr, "ninst: cannot establish connection with machine '%s'\n\n", who);
      exit(12);
   }
/*   else fprintf(stderr, "ninst: connection established\n\n"); */

   output_header(envp);

   strcpy(buf,argv[2]);
   for(i = 3; i < argc; i++) {
      strcat(buf," ");
      strcat(buf,argv[i]);
   }

   sprintf(lenbuf,"%d ",strlen(buf));
   write(it,lenbuf,strlen(lenbuf));

   write(it,buf,strlen(buf));

   while((readlen = read(it,buf,CMDBUFLEN)) > 0) {
      rc = write(1,buf,readlen);
      if (rc != readlen) {
         /* Failure - write again to get correct ERRNO */
         write(1,buf,readlen);
         perror("write");
         exit(-1);
      }
   }

   if(readlen < 0)
     { 
      perror(argv[0]);
     }
   exit(0);
}

output_header(envp)
char *envp[];
{
   char strbuf[128];
   char *cp;
   char **envpptr;

#ifdef _IBMRT
   cp = "RT";
#endif _IBMRT

#ifdef _AIX
   cp = "R2";
#endif _AIX

#if !defined(_IBMRT) && !defined(_AIX)
   cp = "unknown";
#endif

   sprintf(strbuf,"Clients_Header_Version=%s\n",HEADER_VERSION);
   strcpy(buf,strbuf);

   sprintf(strbuf,"Clients_Client_IP=%s\n",getenv("cIPaddr"));
   strcat(buf,strbuf);

   sprintf(strbuf,"Clients_Type=%s\n",cp);
   strcat(buf,strbuf);

   for (envpptr = envp; *envpptr != NULL; envpptr++) {
      if ((strncmp(*envpptr, "IFS", 3)) != 0) {
         if ((strlen(buf)+strlen(*envpptr)+2) > CMDBUFLEN)
         {
            fprintf(stderr, "\n\nninst: Internal limit exceeded.\n");
            exit(10);
         }
         strcat(buf,*envpptr);
         strcat(buf,"\n");
      }
   }

   sprintf(strbuf,"%d ",strlen(buf));
   write(it,strbuf,strlen(strbuf));
   write(it,buf,strlen(buf));
}

/*
 * netsopen - This routine will open a stream connection with an
 *            existing server.  The two input parameters are strings
 *            containing network name or IP address, and service name.
 *            It will return -1 if the open can not succeed, or the
 *            number of the open socket if it does.
 */

int netsopen(who,what)
unsigned char *who;			/* Pointer to IP addr or system name*/
unsigned char *what;			/* Pointer to service name */

{
static struct sockaddr_in ssin;	/* Filled in by netsopen */
   register int i;		/* Standard index variable used alot */
   register int j;		/* Much like i */
   int rc;			/* The return code and misc. cntr */
   unsigned long port;		/* The resolved port number */
   unsigned long ip;		/* The resolved ip address */

   /* Initialize the sockaddr we will be playing with */
   bzero((char *)&ssin,sizeof(ssin));

   /* See if the service passed is really a service or just a port num */
   j = strlen(what);		/* Find out what we have to work with */
   if (!j) {			/* Is there a parameter? */
      fprintf(stderr, "ninst: No service passed\n");
      return(-1);		/* Return 'NO' */
   }
   rc = 0;			/* Initialize counter of digits */
   for (i=0;i<j;i++)		/* Look at ever char in service id */
      if (isdigit((int)what[i]))   /* Are we looking at a digit? */
         rc++;			/* YES - Good, count it.*/
   if (rc == j)			/* Are they all digits? */
      port = (unsigned) atoi(what);   /* YES - Get port number */
   else
      port = -1;		/* NO - Not that we must resolve */

   /* See if the address passed is an ip address or a name to resolve */
   if (who == NULL) {		/* Did they pass me a parameter */
      fprintf(stderr, "ninst: No address passed\n");
      return(-1);
   }
   ip = inet_addr(who);		/* Attempt to make it an ip address */

   /* Get the service number if it wasn't passed directly as a parameter*/
   if (port == -1) {
      struct servent *sent;	/* A pointer to the service entry */
      sent=getservbyname(what,"tcp"); /* Streams assume tcp, not udp */
      if (sent == NULL) {		/* the service was not found */
         fprintf(stderr, "ninst: Service %s not in local /etc/services\n", what);
         return(-1);
      }
      port = ntohs(sent->s_port); /* Get the port number we will use */
   }

   /* Get the ip address of the destination machine */
   if (ip == -1)		/* Do we already have the address */
   {				/*  NO - Look it up in host database */
      struct hostent *hp = NULL;   /* Only used by gethostbyname */
      hp = gethostbyname(who);  /* Try to look this guy up */
      if (hp == NULL) {		/* the name was not resolved */
         fprintf(stderr, "ninst: Cannot find host %s\n", who);
         return(-1);
      }
      ip = (unsigned long)*hp->h_addr_list[0];
      bcopy(hp->h_addr_list[0],(caddr_t)&ip,sizeof(ip));
      ssin.sin_family = hp->h_addrtype;  /* Use address type passed */
   }
   else {			/* the address is already kown */
      ssin.sin_family = AF_INET;  /*  just set the type to AF_INET */
   }

   /*
    * at this point, we have a valid ip address and port number
    */
/*   fprintf(stderr,"ninst: Trying ip address %s, port %d\n",inet_ntoa(ip),port); */
   ssin.sin_addr.s_addr = ip;	/* Fill in ip address, already nbo */
   ssin.sin_port = htons(port);	/* Fill in port address, cvt to nbo */
   rc = socket(ssin.sin_family, SOCK_STREAM, 0);
   if (rc < 0) {		/* socket system call failed */
      fprintf(stderr, "ninst: unable to create socket\n");
      return(-1);
   }
/*   else fprintf(stderr, "ninst: socket created successfully\n"); */

   /* Got the socket, try to connect with the remote host */
   if (connect(rc,(struct sockaddr *)&ssin,sizeof(ssin)) < 0) {
      close(rc);		/* the connect system call failed */
      fprintf(stderr, "ninst: unable to connect to socket\n");
      return(-1);
   }
/*   else fprintf(stderr, "ninst: connected to socket successfully\n"); */
   return(rc);			/* Return the socket address */
}
