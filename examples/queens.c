/* $Id: queens.c,v 1.2 1996/01/16 14:17:05 chris Exp $ */
/*************************************************************
 * This is a version of the queens benchmark that uses text graphics 
 *  to display the solutions.
 */

#include "stdio.h"
#include "termio.h"

#define  false           0
#define  true            1
#define  NUM_TIMES            1

int last_x[9];
unsigned char board_l[8][8];
char board_d[8][8];
int first_time;
char strings[10][11];

int pause_var;

int item_number;

init_display()
{   
item_number = 1;
ttctl(STDOUT,TT_CLR);
}

init_string()
{   


strcpy(&strings[0][0],"+--------+");
strcpy(&strings[1][0],"|        |");
strcpy(&strings[2][0],"|        |");
strcpy(&strings[3][0],"|        |");
strcpy(&strings[4][0],"|        |");
strcpy(&strings[5][0],"|        |");
strcpy(&strings[6][0],"|        |");
strcpy(&strings[7][0],"|        |");
strcpy(&strings[8][0],"|        |");
strcpy(&strings[9][0],"+--------+");

}


pause(number)
int number;
{ int i;
 for (i=0;i<=number;i++) pause_var += i;
}

print_string()
{ int x;
	x = (item_number-1) * 9 + 1;
display_goto(x,5);
printf("%s\n",&strings[0][0]);
display_goto(x,6);
printf("%s\n",&strings[1][0]);
display_goto(x,7);
printf("%s\n",&strings[2][0]);
display_goto(x,8);
printf("%s\n",&strings[3][0]);
display_goto(x,9);
printf("%s\n",&strings[4][0]);
display_goto(x,10);
printf("%s\n",&strings[5][0]);
display_goto(x,11);
printf("%s\n",&strings[6][0]);
display_goto(x,12);
printf("%s\n",&strings[7][0]);
display_goto(x,13);
printf("%s\n",&strings[8][0]);
display_goto(x,14);
printf("%s\n",&strings[9][0]);

pause(4);

}

board_in_string(x)
int x[];
{   int i, j;

for (i=0; i<=7; i++)
{
	for (j=0; j<=7; j++)
	{
		if (board_l[i][j]==0) {
			strings[i+1][j+1] = ' ';
		} else if ((x[i+1] == j+1)) {
			strings[i+1][j+1] = 'Q';
		} else {
			strings[i+1][j+1] = '.';
		}
	}
}
}

display_goto(x, y)
int     x, y;
  {

	ttctl(1,TT_CM,x,y);

  }



int on_diag(x,y,i,j)
int x,y,i,j;
{  int x_dim, y_dim, x_inc, y_inc;

x_dim= x;
y_dim= y;
x_inc = 1;
y_inc = 1;
while ((x_dim<=7) && (y_dim<=7) && (x_dim>=0) && (x_dim>=0))
{
    if ((x_dim == i) && (y_dim == j)) {
	return(true);
    }
    x_dim+=x_inc;
    y_dim+=y_inc;
}

x_dim= x;
y_dim= y;
x_inc = 1;
y_inc = -1;
while ((x_dim<=7) && (y_dim<=7) && (x_dim>=0) && (x_dim>=0))
{
    if ((x_dim == i) && (y_dim == j)) {
	return(true);
    }
    x_dim+=x_inc;
    y_dim+=y_inc;
}

x_dim= x;
y_dim= y;
x_inc = -1;
y_inc = 1;
while ((x_dim<=7) && (y_dim<=7) && (x_dim>=0) && (x_dim>=0))
{
    if ((x_dim == i) && (y_dim == j)) {
	return(true);
    }
    x_dim+=x_inc;
    y_dim+=y_inc;
}

x_dim= x;
y_dim= y;
x_inc = -1;
y_inc = -1;
while ((x_dim<=7) && (y_dim<=7) && (x_dim>=0) && (x_dim>=0))
{
    if ((x_dim == i) && (y_dim == j)) {
	return(true);
    }
    x_dim+=x_inc;
    y_dim+=y_inc;
}
return(false);

}

int power(number, pwr)
int number, pwr;
{
	int x, i;
	x = number;
	if (pwr == 0)
	{
	    return(1);
	}
	else if (pwr == 1)
	{
	    return(number);
	}
	else
	{
	    for (i=2;i<=pwr;i++)
	    {
		x *= number;
	    }
	    return(x);
	}

}




make_mask(number, char_p)
int number;
unsigned char *char_p;
{
    *char_p = power(2, number - 1);
}

erase_number(number)
int number;
{   unsigned char mask,mask2;
    int i,j;

	make_mask(number, &mask);
	mask2 = 0xff - mask;
	for (i=0; i<=7; i++)
	{
		for (j=0; j<=7; j++)
		{
			if ((board_l[i][j] & mask) != 0)
			{
				board_l[i][j] = board_l[i][j] & mask2;
			}
		}
	}
}

int add_number(number, x, y)
int number, x, y;
{   unsigned char mask;
    int i,j;

	make_mask(number, &mask);
	y = y-1;
	for (i=0; i<=7; i++)
	{
		for (j=0; j<=7; j++)
		{
			if ((i==x) || (j==y) || (on_diag(x,y,i,j)))
			{
				board_l[i][j] = board_l[i][j] | mask;
			}
		}
	}
	return(false);
}

display_board(x)
int x[];
{	int i, new;
for (i=0; i<=7; i++)
{
	if (x[i+1] != last_x[i+1])
	{
		new = i;
		break;
	}
}
for (i=new; i<=7; i++)
{
	if (last_x[i+1] != 0)
	{
		erase_number(last_x[i+1]);
	}
}
for (i=new+1; i<=7; i++)
{
	x[i+1]=0;
}
add_number(x[new+1],new,x[new+1]);
for (i=0; i<=7; i++)
{
	last_x[i+1] = x[i+1];
}
board_in_string(x);
print_string();


}

    /* The eight queens problem, solved 50 times. */
/*
        type    
            doubleboard =   2..16;
            doublenorm  =   -7..7;
            boardrange  =   1..8;
            aarray      =   array [boardrange] of boolean;
            barray      =   array [doubleboard] of boolean;
            carray      =   array [doublenorm] of boolean;
            xarray      =   array [boardrange] of boardrange;
*/

        Try(i, q, a, b, c, x) int i, *q, a[], b[], c[], x[];
            {
            int     j;
            if (first_time) {
		j = item_number - 1;
		first_time = false;
	    } else {
            	j = 0;
	    }
            *q = false;
            while ( (! *q) && (j != 8) )
                { j = j + 1;
                *q = false;
                if ( b[j] && a[i+j] && c[i-j+7] )
                    { x[i] = j;
                    b[j] = false;
                    a[i+j] = false;
                    c[i-j+7] = false;
                    if ( i < 8 )
                        {
			display_board(x);
			Try(i+1,q,a,b,c,x);
                        if ( ! *q )
                            { b[j] = true;
                            a[i+j] = true;
                            c[i-j+7] = true;
                            }
                        }
                    else *q = true;
                    }
                }
            }
        
    Doit ()
        {
        int i,j,q;
        int a[9], b[17], c[15], x[10];
        i = 0 - 7;
        while ( i <= 16 )
            { if ( (i >= 1) && (i <= 8) ) {
		    a[i] = true;
		    x[i] = false;
		    last_x[i] = false;
		}
            if ( i >= 2 ) b[i] = true;
            if ( i <= 7 ) c[i+7] = true;
            i = i + 1;
            }
	for (i=0; i<=7; i++)
	{
		for (j=0; j<=7; j++)
		{
			board_l[i][j] = 0;
			board_d[i][j] = ' ';
		}
	}
	init_display();
	init_string();
	first_time = true;
        Try(1, &q, b, a, c, x);
	display_board(x);
	item_number++;


        i = 0 - 7;
        while ( i <= 16 )
            { if ( (i >= 1) && (i <= 8) ) {
		    a[i] = true;
		    x[i] = false;
		    last_x[i] = false;
		}
            if ( i >= 2 ) b[i] = true;
            if ( i <= 7 ) c[i+7] = true;
            i = i + 1;
            }
	for (i=0; i<=7; i++)
	{
		for (j=0; j<=7; j++)
		{
			board_l[i][j] = 0;
			board_d[i][j] = ' ';
		}
	}
	init_string();
	first_time = true;
        Try(1, &q, b, a, c, x);
	display_board(x);
	item_number++;


        i = 0 - 7;
        while ( i <= 16 )
            { if ( (i >= 1) && (i <= 8) ) {
		    a[i] = true;
		    x[i] = false;
		    last_x[i] = false;
		}
            if ( i >= 2 ) b[i] = true;
            if ( i <= 7 ) c[i+7] = true;
            i = i + 1;
            }
	for (i=0; i<=7; i++)
	{
		for (j=0; j<=7; j++)
		{
			board_l[i][j] = 0;
			board_d[i][j] = ' ';
		}
	}
	init_string();
	first_time = true;
        Try(1, &q, b, a, c, x);
	display_board(x);
	item_number++;


        i = 0 - 7;
        while ( i <= 16 )
            { if ( (i >= 1) && (i <= 8) ) {
		    a[i] = true;
		    x[i] = false;
		    last_x[i] = false;
		}
            if ( i >= 2 ) b[i] = true;
            if ( i <= 7 ) c[i+7] = true;
            i = i + 1;
            }
	for (i=0; i<=7; i++)
	{
		for (j=0; j<=7; j++)
		{
			board_l[i][j] = 0;
			board_d[i][j] = ' ';
		}
	}
	init_string();
	first_time = true;
        Try(1, &q, b, a, c, x);
	display_board(x);
	item_number++;


        i = 0 - 7;
        while ( i <= 16 )
            { if ( (i >= 1) && (i <= 8) ) {
		    a[i] = true;
		    x[i] = false;
		    last_x[i] = false;
		}
            if ( i >= 2 ) b[i] = true;
            if ( i <= 7 ) c[i+7] = true;
            i = i + 1;
            }
	for (i=0; i<=7; i++)
	{
		for (j=0; j<=7; j++)
		{
			board_l[i][j] = 0;
			board_d[i][j] = ' ';
		}
	}
	init_string();
	first_time = true;
        Try(1, &q, b, a, c, x);
	display_board(x);
	item_number++;


        i = 0 - 7;
        while ( i <= 16 )
            { if ( (i >= 1) && (i <= 8) ) {
		    a[i] = true;
		    x[i] = false;
		    last_x[i] = false;
		}
            if ( i >= 2 ) b[i] = true;
            if ( i <= 7 ) c[i+7] = true;
            i = i + 1;
            }
	for (i=0; i<=7; i++)
	{
		for (j=0; j<=7; j++)
		{
			board_l[i][j] = 0;
			board_d[i][j] = ' ';
		}
	}
	init_string();
	first_time = true;
        Try(1, &q, b, a, c, x);
	display_board(x);
	item_number++;


        i = 0 - 7;
        while ( i <= 16 )
            { if ( (i >= 1) && (i <= 8) ) {
		    a[i] = true;
		    x[i] = false;
		    last_x[i] = false;
		}
            if ( i >= 2 ) b[i] = true;
            if ( i <= 7 ) c[i+7] = true;
            i = i + 1;
            }
	for (i=0; i<=7; i++)
	{
		for (j=0; j<=7; j++)
		{
			board_l[i][j] = 0;
			board_d[i][j] = ' ';
		}
	}
	init_string();
	first_time = true;
        Try(1, &q, b, a, c, x);
	display_board(x);
	item_number++;


        i = 0 - 7;
        while ( i <= 16 )
            { if ( (i >= 1) && (i <= 8) ) {
		    a[i] = true;
		    x[i] = false;
		    last_x[i] = false;
		}
            if ( i >= 2 ) b[i] = true;
            if ( i <= 7 ) c[i+7] = true;
            i = i + 1;
            }
	for (i=0; i<=7; i++)
	{
		for (j=0; j<=7; j++)
		{
			board_l[i][j] = 0;
			board_d[i][j] = ' ';
		}
	}
	init_string();
	first_time = true;
        Try(1, &q, b, a, c, x);
	display_board(x);
	item_number++;


        if ( ! q )
            printf (" Error in Queens.\n");
        }

Queens ()
    {
    int i;
    for ( i = 1; i <= NUM_TIMES; i++ ) Doit();
    }





main()
{

printf("  Queens\n"); 
while (true) {
    Queens();
}

}




