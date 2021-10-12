static char sccsid[] = "@(#)60	1.7  src/bos/usr/games/quiz.c, cmdgames, bos411, 9428A410j 10/14/92 08:59:25";
/*
 * COMPONENT_NAME: (CMDGAMES) unix games
 *
 * FUNCTIONS: quiz
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define NF 10
#define NL 300
#define NC 200
#define SL 100
#define NA 10
#define RS 100

int tflag;
int xx[NL];
char score[NL];
int rights;
int wrongs;
int guesses;
FILE *input;
int nl = 0;
int na = NA;
int inc;
int ptr  = 0;
int nc = 0;
char *line = NULL;
int   line_size = 0;
char response[RS];
char *tmp[NF];
int select[NF];

readline()
{
    /* Read one line at a time */
    /* Leading and trailing blanks are removed */

	char *t;
	register c;
	int count;
loop:
	for (t=line, count = 0; c=getc(input), c!=EOF; t++) {
		if (line_size <= count) {
			line_size = ((count / 256) + 1) * 256;
			if ((line = (char *)realloc(line,line_size)) == NULL) {
				fprintf(stderr,"Internal realloc error\n");
				exit(1);
			}
			t = line + count;
		}
		count++;
		*t = c;
		nc++;
		if(*t==' '&&(t==line||t[-1]==' '))
			t--;
		if(*t=='\n') {
			if(t[-1]=='\\')		/*inexact test*/
				continue;
			while(t>line&&t[-1]==' ')
				*--t = '\n';
			*++t = 0;
			return(1);
		}
		if(t-line>=NC) {
			printf("Too hard for me\n");
			do {
				*line = getc(input);
				if(*line==0377)
					return(0);
			} while(*line!='\n');
			goto loop;
		}
	}
	return(0);
}

char *eu;
char *ev;

cmp(u,v)
      /* See if specified categories are valid */
char *u, *v;
{
	int x;
	eu = u;
	ev = v;
	x = disj(1);
	if(x!=1)
		return(x);
	return(eat(1,0));
}

disj(s)
{
	int t, x;
	char *u;
	u = eu;
	t = 0;
	for(;;) {
		x = string(s);
		if(x>1)
			return(x);
		switch(*ev) {
		case 0:
		case ']':
		case '}':
			return(t|x&s);
		case '|':
			ev++;
			t |= s;
			s = 0;
			continue;
		}
		if(s) eu = u;
		if(string(0)>1)
			return(2);
		switch(*ev) {
		case 0:
		case ']':
			return(0);
		case '}':
			return(1);
		case '|':
			ev++;
			continue;
		default:
			return(2);
		}
	}
}

string(s)
{
	int x;
	for(;;) {
		switch(*ev) {
		case 0:
		case '|':
		case ']':
		case '}':
			return(1);
		case '\\':
			ev++;
			if(*ev==0)
				return(2);
			if(*ev=='\n') {
				ev++;
				continue;
			}
		default:
			if(eat(s,*ev)==1)
				continue;
			return(0);
		case '[':
			ev++;
			x = disj(s);
			if(*ev!=']' || x>1)
				return(2);
			ev++;
			if(s==0)
				continue;
			if(x==0)
				return(0);
			continue;
		case '{':
			ev++;
			x = disj(s);
			if(*ev!='}'||x>1)
				return(2);
			ev++;
			continue;
		}
	}
}

eat(s,c)
char c;
{
	if(*ev!=c)
		return(2);
	if(s==0) {
		ev++;
		return(1);
	}
	if(fold(*eu)!=fold(c))
		return(0);
	eu++;
	ev++;
	return(1);
}

fold(c)
      /* Fold upper case letters into lower case */
      /* Leave the character as it is if it is in lower case */

char c;
{
	if(c<'A'||c>'Z')
		return(c);
	return(c|040);
}

publish(t)
char *t;
{
	ev = t;
	pub1(1);
}

pub1(s)
{
      /* Print out characters */

	for(;;ev++){
		switch(*ev) {
		case '|':
			s = 0;
			continue;
		case ']':
		case '}':
		case 0:
			return;
		case '[':
		case '{':
			ev++;
			pub1(s);
			continue;
		case '\\':
			if(*++ev=='\n')
				continue;
		default:
			if(s)
				putchar(*ev);
		}
	}
}

segment(u,w)
        /* Put segments of a line in the input file into separate temporary locations */
        /* For example, file name (or complete path name) goes to tmp[0], */
        /* first category to tmp[1] and second category to tmp[2] */

char *u, *w[];
{
	char *s;
	int i;
	char *t;
	s = u;
	for(i=0;i<NF;i++) {
		u = s;
		t = w[i];
		while(*s!=':'&&*s!='\n'&&s-u<SL) {
			if(*s=='\\')  {
				if(s[1] == '\n') {
					s += 2;
					continue;
				}
				*t++ = *s++;
			}
			*t++ = *s++;
		}

		while(*s!=':'&&*s!='\n')
			s++;
		*t = 0;
		if(*s++=='\n') {
			return(i+1);
		}
	}
	printf("Too many facts about one thing\n");
	return(0);
}

perm(u,m,v,n,p)
int p[];
char *u[], *v[];
{
	int i, j;
	int x;
	for(i=0;i<m;i++) {
		for(j=0;j<n;j++) {
			x = cmp(u[i],v[j]);
			if(x>1) badinfo();
			if(x==0)
				continue;
			p[i] = j;
			goto uloop;
		}
		return(0);
uloop:		;
	}
	return(1);
}

find(u,m)
     /* Find categories on info file */

char *u[];
{
	int n;
	while(readline()){
		n = segment(line,tmp);
		if(perm(u,m,tmp+1,n-1,select))
			return(1);
	}
	return(0);
}

readindex()
{
	xx[0] = nc = 0;
	while(readline()) {
		xx[++nl] = nc;
		if(nl>=NL) {
			printf("I've forgotten some of it;\n");
			printf("I remember %d items.\n", nl);
			break;
		}
	}
}

talloc()
{
      /* Allocate storage */

	int i;
	for(i=0;i<NF;i++)
		tmp[i] = malloc((size_t)SL);
	line_size = 256;
	line = malloc(line_size);
}

main(argc,argv)
char *argv[];
{
        extern char *optarg;
        int opterr = 0;
        int bflag = 0, iflag = 0;
        int c;
	register j;
	int i;
	int x;
	int z;
	char *info;
	long tm;
	extern done(void);
	int count;
	info = "/usr/games/lib/quiz/index";
	time(&tm);
	inc = (int)tm&077774|01;

          /* Analyze options */

        while ((c = getopt(argc, argv, "i:t")) != EOF)
		switch(c)  {
		case 'i':
			if(argc>2) 
                           info = optarg;
                        iflag++;
                        break;
		case 't':
			tflag = 1;
            	        bflag++;
			break;
                case '?':
                        opterr++;
                        break;
	}
        if(opterr) {
           fprintf(stderr,"usage: quiz -t [-i file] [category1][category2] \n");
           exit(2);
        }
        if(iflag) {
		argc -= 2;
		argv += 2;
        }
        if(bflag) {
		argc--;
		argv++;
        }
	input = fopen(info,"r");
	if(input==NULL) {
		printf("No info\n");
		exit(0);
	}
	talloc();
	if(argc<=2)
		instruct(info);
	signal(SIGINT, (void (*)(int))done);
	argv[argc] = 0;
	if(find(&argv[1],argc-1)==0)
		dunno();
	fclose(input);
	input = fopen(tmp[0],"r");
	if(input==NULL)
		dunno();
	readindex();
	if(!tflag || na>nl)
		na = nl;
	stdout->_flag |= _IONBF;
	for(;;) {
		i = next();
		fseek(input,xx[i]+0L,0);
		z = xx[i+1]-xx[i];
		if (line_size < z) {
			if (line)
				free(line);
			line_size = ((z / 256) + 1) * 256;
			if ((line = (char *)malloc(line_size)) == NULL) {
				fprintf(stderr,"Internal malloc error\n");
				exit(1);
			}
		}
		for(j=0;j<z;j++)
			line[j] = getc(input);
		segment(line,tmp);
		if(*tmp[select[0]] == '\0' || *tmp[select[1]] == '\0') {
			score[i] = 1;
			continue;
		}
		publish(tmp[select[0]]);
		printf("\n");
		for(count=0;;count++) {
			if(query(response)==0) {
				publish(tmp[select[1]]);
				printf("\n");
				if(count==0) wrongs++;
				score[i] = tflag?-1:1;
				break;
			}
			x = cmp(response,tmp[select[1]]);
			if(x>1) badinfo();
			if(x==1) {
				printf("Right!\n");
				if(count==0) rights++;
				if(++score[i]>=1 && na<nl)
					na++;
				break;
			}
			printf("What?\n");
			if(count==0) wrongs++;
			score[i] = tflag?-1:1;
		}
		guesses += count;
	}
}

query(r)
char *r;
{
	char *t;
	int i = 0;
	for(t=r;;t++) {
		if(i++ >= RS)
			t--;
		if(read(0,t,1)==0)
			done();
		if(*t==' '&&(t==r||t[-1]==' '))
			t--;
		if(*t=='\n') {
			while(t>r&&t[-1]==' ')
				*--t = '\n';
			break;
		}
	}
	*t = 0;
	return(t-r);
}

next()
{
	int flag;
	inc = inc*3125&077777;
	ptr = (inc>>2)%na;
	flag = 0;
	while(score[ptr]>0)
		if(++ptr>=na) {
			ptr = 0;
			if(flag) done();
			flag = 1;
		}
	return(ptr);
}

done(void)
{
       /* Print score of quiz */

	printf("\nRights %d, wrongs %d, ", rights, wrongs);
	if(guesses)
		printf("extra guesses %d, ", guesses);
	printf("score %d%%\n",100*rights/(rights+wrongs));
	exit(0);
}
instruct(info)
       /* Give instructions for the game */

char *info;
{
	int i, n;
	printf("Subjects:\n\n");
	while(readline()) {
		printf("-");
		n = segment(line,tmp);
		for(i=1;i<n;i++) {
			printf(" ");
			publish(tmp[i]);
		}
		printf("\n");
	}
	printf("\n");
	input = fopen(info,"r");
	if(input==NULL)
		abort();
	readline();
	segment(line,tmp);
	printf("For example,\n");
	printf("    quiz ");
	publish(tmp[1]);
	printf(" ");
	publish(tmp[2]);
	printf("\nasks you a ");
	publish(tmp[1]);
	printf(" and you answer the ");
	publish(tmp[2]);
	printf("\n    quiz ");
	publish(tmp[2]);
	printf(" ");
	publish(tmp[1]);
	printf("\nworks the other way around\n");
	printf("\nType empty line to get correct answer.\n");
	exit(0);
}

badinfo(){
	printf("Bad info %s\n",line);
}

dunno()
{
	printf("I don't know about that\n");
	exit(0);
}
