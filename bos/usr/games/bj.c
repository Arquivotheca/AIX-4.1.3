static char sccsid[] = "@(#)33	1.6  src/bos/usr/games/bj.c, cmdgames, bos411, 9428A410j 6/15/90 21:26:20";
/*
 * COMPONENT_NAME: (CMDGAMES) unix games
 *
 * FUNCTIONS: bj
 *
 * ORIGINS: 3, 27
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

/*
 *      Compile as:  cc bj.c -lm -o bj
 */

#include	<stdio.h>
#include	<signal.h>
#include	<setjmp.h>

struct	card	{
	char	rank;			/* 2-9,	10=T, 11=J, 12=Q, 13=K,	14=A */
	char	suit;			/* 0=S,	1=H, 2=C, 3=D */
} deck[52];

char	curcrd[] = "XX";		/* current card */
int	nxtcrd;				/* next	card; index into `deck'	*/
long	action;				/* total bucks bet */
long	bux;				/* signed gain/loss */
int	ins;				/* player took insurance against bj */
int	shuff;				/* A shuffle has occured */
int	wager =	2;			/* bet per hand	*/
int	split;
jmp_buf	jbuf;				/* place to stash a stack frame */

main()
{
	register	r, s, aced;
	register	d1, d2, p1, p2, dtot;
	int		ptot[2];
	char		ddown[3];
	long		t;
	extern		leave(void);

	signal(SIGINT, (void (*)(int))leave);
	if(signal(SIGQUIT, SIG_IGN) != SIG_IGN)
	signal(SIGQUIT, (void (*)(int))leave);
	time(&t);
	srand(getpid() + (int)((t>>16) + t));	/* set random number seed */
	printf("\nWelcome To Black Jack!\n");
	for(r =	2; r < 15; r++)			/* initialize the deck */
		for(s =	0; s < 4; s++)	{
			deck[nxtcrd].rank = r;
			deck[nxtcrd++].suit = s;
		}
	shuffle();				/* and shuffle it */
	setjmp(jbuf);	/* longjmps to here from win(), lose() & draw() */
	printf("\nNew hand\n");
	if(shuff) {
		printf("** Shuffle deck **\n");
		shuff = 0;
	}
	ins = split = aced = 0;
	printf("Dealer shows ");
	d1 = getcard(0);
	d2 = getcard(1);
	strcpy(ddown, curcrd);
	if(d1 > 10)
		d1 = 10;
	if(d2 > 10)
		d2 = 10;
	dtot = d1 + d2;
	printf("\nYou show ");
	p1 = getcard(0);
	putchar('+');
	p2 = getcard(0);
	putchar('\n');
	if(d1 == 1)			/* dealer has an ace showing */
		if(getresp("Insurance"))
			ins++;
	if((aced = (d1 == 1 || d2 == 1)) && dtot == 11) { /* dealers got a bj */
		printf("Dealer has %s for blackjack!\n", ddown);
		if(ins)
			draw();
		else
			lose();
	}
	if(p1 == p2)
		if(getresp("Split pair"))
			split++;
	if(p1 > 10)
		p1 = 10;
	if(p2 > 10)
		p2 = 10;
	if(split) {
		wager *= 2;
		printf("First down card: ");
		p2 = getcard(0);
		if(p2 > 10)
			p2 = 10;
		putchar('\n');
		r = play(p1, p2);
		printf("Second down card: ");
		p2 = getcard(0);
		if(p2 > 10)
			p2 = 10;
		putchar('\n');
		s = play(p1, p2);
		if(r == 22 && s == 22)		/* two blackjacks! */
			win();
		if(r < 0 && s < 0)		/* two busts! */
			lose();
		ptot[0] = r;
		ptot[1] = s;
	}
	else
		switch(ptot[0] = play(p1, p2)) {
			case -1:
				lose();
			case 22:
				win();
		}
	printf("Dealer has %s", ddown);
	if(aced && dtot < 12 && dtot > 5)
		dtot += 10;
	while(dtot < 17) {
		putchar('+');
		d2 = getcard(0);
		if(d2 > 10)
			d2 = 10;
		if(d2 == 1 && !aced)
			aced = 1;
		dtot += d2;
		if(aced && dtot < 12 && dtot > 5)
			dtot += 10;
	}
	printf(" = ");
	if(dtot > 21) {
		printf("bust\n");
		win();
	}
	printf("%d\n", dtot);
	if(!(split && ptot[0] != ptot[1])) {
		if(ptot[0] > dtot)
			win();
		if(dtot > ptot[0])
			lose();
		draw();
	}
	if(dtot > ptot[0]) {
		if(dtot > ptot[1])
			lose();
		if(dtot == ptot[1]) {
			wager /= 2;
			lose();
		}
		draw();
	}
	if(ptot[0] > dtot) {
		if(ptot[1] > dtot)
			win();
		if(ptot[1] == dtot) {
			wager /= 2;
			win();
		}
		draw();
	}
	wager /= 2;
	if(dtot > ptot[1])
		lose();
	if(dtot < ptot[1])
		win();
	draw();
}

shuffle()
{
	int		rancomp();

	qsort(deck, 52, sizeof(struct card), rancomp);
	nxtcrd = 0;			/* reset deck index */
}

rancomp(p1, p2)
char *p1, *p2;				/* these are unused but	necessary */
{
	return rand() - (32767/2);
}

status()
{
	printf("\nAction $%ld\nYou're ", action);
	if(bux == 0)
		printf("even\n");
	else if(bux > 0)
		printf("up $%ld\n", bux);
	else
		printf("down $%ld\n", -bux);
}

getcard(f)
register f;
{
	register	r, s;
	static		char	brix[] = "TJQKA";
	static		char	suits[]	= "SHCD";

	if(nxtcrd > 51)			/* no more! */
		shuffle();
	r = deck[nxtcrd].rank;
	s = deck[nxtcrd++].suit;
	if(r > 9)
		curcrd[0] = brix[r - 10];
	else
		curcrd[0] = '0' + r;
	curcrd[1] = suits[s];
	if(!f)			/* not dealer's down card, so we can show it */
		printf("%s", curcrd);
	if(r ==	14)			/* ace */
		return(1);
	else
		return(r);
}

leave(void)
{
	status();
	printf("\nBye!\n\n");
	exit(0);
}

lose()
{
	action += wager;
	if(ins)			/* took insurance, dealer didn't have bj */
		wager += 1;
	printf("You lose $%d\n", wager);
	bux -= wager;
	wager = 2;
	status();
	longjmp(jbuf, 0);
}

win()
{
	action += wager;
	if(ins)
		wager -= 1;
	printf("You win $%d\n", wager);
	bux += wager;
	wager = 2;
	status();
	longjmp(jbuf, 0);
}

draw()
{
	action += wager;
	printf("You break even\n");
	wager = 2;
	status();
	longjmp(jbuf, 0);
}

getresp(s)
char *s;
{
	char		buf[120];

	printf("%s? ", s);
	gets(buf);
	if(feof(stdin)) leave();  /* he hit cntl-d */
	if(buf[0] == 'y')
		return(1);
	else
		return(0);
}

play(p1, p2)
register p1, p2;
{
	register	ptot, acep, p;

	ptot = p1 + p2;
	if((acep = (p1 == 1 || p2 == 1)) && ptot == 11 ) {
		printf("You have blackjack!\n");
		wager += 1;
		return(22);
	}
	if(ptot == 10 || ptot == 11)
		if(getresp("Double down")) {
			wager *= 2;
			printf("Next card is ");
			p1 = getcard(0);
			putchar('\n');
			if(p1 > 10)
				p1 = 10;
			if(p1 == 1 && ptot == 10)
				p1 = 11;
			ptot += p1;
			goto stick;
		}
	while(getresp("Hit")) {
		printf("Next card is ");
		p = getcard(0);
		putchar('\n');
		if(p == 1 && !acep)
			acep = 1;
		if(p > 10)
			p = 10;
		if((ptot += p) > 21) {
			printf("You bust!\n");
			return(-1);
		}
	}
stick:
	if(ptot < 12 && acep)
		ptot += 10;
	printf("You have %d\n", ptot);
	acep = 0;
	return(ptot);
}
