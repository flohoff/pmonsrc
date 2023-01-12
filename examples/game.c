/* $Id: game.c,v 1.2 1996/01/16 14:16:57 chris Exp $ */
#include <string.h>
#include <ctype.h>
#define loop for (;;)
#define cmpstr(x,y) ((strcmp(x,y) == 0) ? 1 : 0)

/*************************************************************
*  game.c
*	A stripped down version of Adventure
*/

/*	  N, NE,  E, SE,  S, SW,  W, NW, UP, DN   */

int map[][10] = {
	{ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{ 3,  0,  6,  0,  4,  0,  2,  0,  0,  0},
	{ 1,  0,  8,  0,  2,  0,  4,  0,  0,  0},
	{ 2,  0,  1,  0,  4,  0,  7,  0,  0,  0},
	{ 1,  0,  3,  0,  5,  0,  2,  0,  0,  0},
	{ 2,  0,  6,  0,  4,  0,  1,  0,  0,  0},
	{ 2,  0,  7,  0,  8,  0,  5,  0,  0,  0},
	{ 0,  0,  0,  0,  0,  0,  6,  0,  0,  0},
	{ 6,  0,  3,  0,  9,  0,  4,  0,  0,  0},
	{ 8,  0,  3,  0, 10,  0,  4,  0,  0,  0},
	{ 9,  0,  0,  0,  0,  0,  0,  0,  0, 11},	/* rm10 */
	{ 0,  0, 10,  0,  0,  0, 12,  0, 10,  0},
	{ 0,  0, 11,  0,  0,  0, 13,  0,  0,  0},
	{ 0,  0, 12,  0,  0,  0, 14,  0, 14,  0},
	{ 0,  0, 13,  0,  0,  0, 15,  0, 15, 13},
	{ 0,  0, 14,  0,  0,  0, 16,  0,  0,  0},
	{ 0,  0, 15,  0,  0,  0,  0,  0,  0, 17},
	{22,  0,  0,  0, 18,  0, 19,  0, 16, 22},
	{17,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{ 0,  0, 17,  0,  0,  0, 20,  0,  0,  0},
	{ 0,  0, 19,  0,  0,  0, 21,  0,  0,  0},	/* rm20 */
	{20,  0, 20,  0, 88,  0, 29,  0,  0,  0},
	{25,  0, 17,  0, 23, 83, 24,  0, 17,  0},
	{22,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{ 0,  0, 22,  0,  0,  0, 31,  0, 31,  0},
	{26,  0,  0,  0, 22,  0,  0,  0,  0, 33},
	{ 0,  0, 28,  0, 25,  0, 27,  0,  0,  0},
	{ 0,  0, 26,  0,  0,  0,  0,  0,  0,  0},
	{ 0,  0,  0,  0,  0, 26,  0,  0,  0,  0},
	{31,  0, 21,  0,  0,  0, 30,  0, 21, 31},
	{31,  0, 29,  0, 94,  0,  0,  0,  0,  0},	/* rm30 */
	{32,  0, 24,  0, 30,  0, 29,  0,  0,  0},
	{31,  0,  0,  0,  0,  0,  0,  0,  0,  0}
	};


char *dirs[] = {
	"North", "North East", "East", "South East", 
	"South", "South West", "West", "North West", 
	"Up", "Down"
	};

typedef struct OBJS {
	int loc;
	char *name;
	char *desc;
	} OBJS;

OBJS object[] = {
	{28,"AXE","a small axe"},
	{18,"GOLD","a large gold nugget"},
	{25,"BARS","some bars of silver"},
	{23,"JEWELRY","some precious jewelry"},
	{24,"COINS","many coins"},
	{12,"CAGE","a wicker cage"},
	{15,"BIRD","a bird singing"},
	{21,"DIAMONDS","several diamonds"},
	{7,"LAMP","a brass lamp"},
	{7,"KEYS","a bunch of keys"},
	{7,"FOOD","a packet of tasty food"},
	{7,"BOTTLE","a bottle of water"},
	{0,"",""}
	};

#define IN_HAND 0

char *descrip[] = {
	"dummy",
	"You're in forest.",
	"You're in forest.",
	"You're in forest.",
	"You're in forest.",
	"You're at hill at end of road.",
	"You're at road near house.",
	"You're inside house.",
	"You're beside a stream in a valley.",
	"You're near the slit in the streambed.",
	"You're directly above a grate in a dry streambed.", /* rm10 */
	"You're below the grate.",
	"You're in cobble crawl.",
	"You're in the debris room.",
	"You're in an awkward sloping E/W canyon.",
	"You're in the bird chamber.",
	"You're at the top of a small pit.",
	"You're in hall of mists.",
	"You're in the nugget of gold room.",
	"You're on east side of fissure.",
	"You're on the west side of the fissure.",	/* rm20 */
	"You're at the west end of the hall of mists.",
	"You're in the hall of the Mountain King.",
	"You're in the south chamber.",
	"You're in the west chamber.",
	"You're at a N/S passage at a hole in the floor.",
	"You're at \"Y2\"",
	"You're near a window overlooking the pit.",
	"The passage here is blocked by a recent cavein.",
	"You're at east end of long hall.",
	"You're at west end of long hall.",		/* rm30 */
	"You're at a crossover of a high N/S canyon and a low E/W one.",
	"Dead end."
	};

int posn;

main()
{
int dest;
char cmd[10],arg1[10];

printf("Welcome to TINY caves, you may direct me with commands:\n");
printf("N, NE,  E, SE,  S, SW,  W, NW, UP, DOWN, or QUIT\n\n");
posn = 6;
loop {
	if(!getresp(cmd,arg1)) continue;
	if (cmpstr(cmd,"N")) dest = map[posn][0];
	else if (cmpstr(cmd,"NE")) dest = map[posn][1];
	else if (cmpstr(cmd,"E")) dest = map[posn][2];
	else if (cmpstr(cmd,"SE")) dest = map[posn][3];
	else if (cmpstr(cmd,"S")) dest = map[posn][4];
	else if (cmpstr(cmd,"SW")) dest = map[posn][5];
	else if (cmpstr(cmd,"W")) dest = map[posn][6];
	else if (cmpstr(cmd,"NW")) dest = map[posn][7];
	else if (cmpstr(cmd,"UP")) dest = map[posn][8];
	else if (cmpstr(cmd,"U")) dest = map[posn][8];
	else if (cmpstr(cmd,"PLUGH")) {
		if (posn == 26) dest = 7;
		else {
			printf("Nothing happens\n");
			continue;
			}
		}
	else if (cmpstr(cmd,"DOWN") || cmpstr(cmd,"D")) {
		if (godown()) dest = map[posn][9];
		else continue;
		}
	else if (cmpstr(cmd,"GET") || cmpstr(cmd,"TAKE")) { 
		getobj(arg1);
		continue;
		}
	else if (cmpstr(cmd,"PUT") || cmpstr(cmd,"DROP")) {
		putobj(arg1);
		continue;
		}
	else if (cmpstr(cmd,"INVENTORY") || cmpstr(cmd,"INV")) {
		inventory();
		continue;
		}
	else if (cmpstr(cmd,"LOOK") || cmpstr(cmd,"L")) {
		look(posn); 
		continue;
		}
	else if (cmpstr(cmd,"QUIT")) break;
	else { 
		printf("EH?\n"); 
		continue; 
		}
		
	if (dest == 0) printf("There is no way to go in that direction\n");
	else if (dest > 32) {
		printf("Well, it looked as though there was a way through,\n");
		printf("but it doesn't look like there is after all.\n");
		}
	else posn = dest; 
	}
}

int getresp(cmd,arg1)
char *cmd,*arg1;
/* describe your current location and get input */
/* returns 0 if the input line was blank */
{
char buf[40],*p;

if (objlocn("LAMP") == IN_HAND || posn <= 11) {
	printf("%s\n\n",descrip[posn]);
	objshere(posn);
	}
else printf("It's very dark down here, I can't see a thing!\n");

printf(">");
gets(buf);
if (buf[0] == 0) return(0);
strtoupper(buf);
p = getword(cmd,buf);
getword(arg1,p);
return(1);
}

int godown()
/* return 1 if down is permitted, else 0 */
{

if (posn == 10 && objlocn("KEYS") != IN_HAND) {
	printf("I'm sorry the grate is locked\n");
	return(0);
	}
return(1);
}

getobj(p)
char *p;
/* pick-up the object specified in 'p' */
{
int num;

num = objnum(p);
if (num == -1) 
	printf("I see no %s here\n",p);
else if (object[num].loc == 0) 
	printf("You're already carrying the %s\n",p);
else if (object[num].loc != posn)
	printf("I see no %s here\n",p);
else {
	if (cmpstr(p,"BIRD") && objlocn("CAGE") != 0) {
		printf("You can't catch a bird without a cage!\n");
		return;
		}
	object[num].loc = 0;
	printf("OK\n");
	}
}

int objlocn(p)
char *p;
/* return the room number that the object 'p' is in */
/* returns -1 if the object name is unrecognized */
{
int i;

i = 0;
while (object[i].desc[0] != 0) {
	if (cmpstr(object[i].name,p)) return(object[i].loc);
	i++;
	}
return(-1);
}

int objnum(p)
char *p;
/* return the object-number of object 'p' */
/* returns -1 if the object name is unrecognized */
{
int i;

i = 0;
while (object[i].desc[0] != 0) {
	if (cmpstr(object[i].name,p)) return(i);
	i++;
	}
return(-1);
}

putobj(p)
char *p;
/* put down the object 'p' */
{
int num;

num = objnum(p);
if (num == -1) 
	printf("You're not carrying %s\n",p);
else if (object[num].loc != 0) 
	printf("You're not carrying %s\n",p);
else {
	object[num].loc = posn;
	printf("OK\n");
	}
}

inventory()
/* list the objects being carried */
{
int i;

i = 0;
while (object[i].desc[0] != 0) {
	if (object[i].loc == 0) 
		printf("        You are holding %s\n",object[i].desc);
	i++;
	}
}

look(n)
int n;
/* describe the exits from room 'n', and the objects here */
{
int i;

if (objlocn("LAMP") != IN_HAND && posn > 11) {
	printf("I'm sorry, I can't see a thing, it's too dark\n");
	return;
	}
printf("From where I am standing, I can see exits to the:\n");
for (i=0;i<10;i++) {
	if (map[n][i] != 0) printf("        %s\n",dirs[i]);
	}
printf("\n");
objshere(n);
}

objshere(n)
int n;
/* print a list of the objects in room 'n' */
{
int i;

i = 0;
while (object[i].desc[0] != 0) {
	if (object[i].loc == n) 
		printf("        I can see %s here\n",object[i].desc);
	i++;
	}
}

