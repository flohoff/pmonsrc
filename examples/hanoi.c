/* $Id: hanoi.c,v 1.2 1996/01/16 14:16:58 chris Exp $ */
#include "stdio.h"
#include "termio.h"

/*************************************************************
*  hanoi.c
*	Solves the towers of hanoi problem graphically, requires
*	that PMON knows about your terminal type.
*/

#define SLOW

#define  GOOD		 0
#define  BAD		 1
#define  nil             0
#define  false           0
#define  true            1

    /* Towers */
#define maxcells         18
#define  BLANK           0
#define  POLE            20


    /* Towers */ /*
    discsizrange = 1..maxcells; */
#define    stackrange   3
/*    cellcursor = 0..maxcells; */
struct    element {
            int discsize;
            int next;
        };
/*    emsgtype = packed array[1..15] of char;
*/

    /* Towers */
int 	stack[stackrange+1];
struct element    cellspace[maxcells+1];
int    freelist,
       movesdone;
int    display[stackrange+1][20];
int pause_var;

    /*  Program to Solve the Towers of Hanoi */


int get_default_display(height)
int height;
{
  if (height<=15)
    {
      return(POLE);
    }
  else
    {
      return(BLANK);
    }
}

int get_height_top_disk(pole)
int pole;
{ int i,temp,found;
  found = 0;
  for (i=19;i>=0;i--)
    {
      temp = i;
      if (display[pole][i] != 0) {
        found = 1;
	break;
      }
    }
  if (!found) temp = -1;
  return(temp);

}

int get_size(pole, height)
int pole, height;
{
  return(display[pole][height]);
}

int get_size_top_disk(pole)
int pole;
{
  return(get_size(pole, get_height_top_disk(pole)));
}


print_disk(disk_size)
int disk_size;
{
  switch(disk_size) {
  case BLANK: printf("                       ");
    break;
  case POLE: printf("           |           ");
    break;
  case 1: printf("           0           ");
    break;
  case 2: printf("          111          ");
    break;
  case 3: printf("         22222         ");
    break;
  case 4: printf("        3333333        ");
    break;
  case 5: printf("       444444444       ");
    break;
  case 6: printf("      55555555555      ");
    break;
  case 7: printf("     6666666666666     ");
    break;
  case 8: printf("    777777777777777    ");
    break;
  case 9: printf("   88888888888888888   ");
    break;
  case 10: printf("  9999999999999999999  ");
    break;
  case 11: printf(" 000000000000000000000 ");
    break;
  }
}


pause(number)
int number;
{ int i;
#ifdef SLOW
 for (i=0;i<=number;i++) pause_var += i;
#endif
}

init_display()
{
  int i,j;
  for (i=0;i<=3;i++)
    {
      for (j=0;j<=19;j++)
	{
	  display[i][j] = 0;
	}

    }

}

default_display()
{ int w,h,x,size;
  for (w=1;w<=3;w++)
    {
      for (h=0;h<=19;h++)
	{
	  if (w==1) {
	    x=0;
	  } else if (w==2) {
	    x=26;
	  } else {
	    x=52;
	  }
      ttctl(STDOUT,TT_CM,x,22-h);
      size = display[w][h];
      if (size != 0) {
	print_disk(size);
      } else {
	print_disk(get_default_display(h));
      }
	}
    }
}

move_up(pole, height, size)
int pole, height, size;
{ int h,x;
  if (pole==1) {
    x=0;
  } else if (pole==2) {
    x=26;
  } else {
    x=52;
  }

  for (h=height; h<=18; h++)
  {
      ttctl(STDOUT,TT_CM,x,22-h-1);
      print_disk(size);
      pause(100);
      ttctl(STDOUT,TT_CM,x,22-h);
      print_disk(get_default_display(h));
      pause(10000);
  }
  ttctl(STDOUT,TT_CM,x,22-19);
  print_disk(BLANK);
}

move_dn(pole, height, size)
int pole, height, size;
{ int h,x;
  if (pole==1) {
    x=0;
  } else if (pole==2) {
    x=26;
  } else {
    x=52;
  }
  ttctl(STDOUT,TT_CM,x,22-19);
  print_disk(size);

  for (h=19; h>=(height+1); h--)
  {
      pause(10000);
      ttctl(STDOUT,TT_CM,x,22-h+1);
      print_disk(size);
      pause(100);
      ttctl(STDOUT,TT_CM,x,22-h);
      print_disk(get_default_display(h));
  }
}

move_over(pole_src, pole_dst, size)
int pole_src, pole_dst, size;
{ int x, y, src_x, dst_x;
  y  = 22-19;
  if (pole_src==1) {
    src_x=0;
  } else if (pole_src==2) {
    src_x=26;
  } else {
    src_x=52;
  }
  if (pole_dst==1) {
    dst_x=0;
  } else if (pole_dst==2) {
    dst_x=26;
  } else {
    dst_x=52;
  }


  if (dst_x>src_x)
  {
    for (x = src_x; x <= dst_x; x++)
    {
        ttctl(STDOUT,TT_CM,x,y);
        print_disk(size);
        pause(5000);
    }

  } else {
    for (x = src_x; x >= dst_x; x--)
    {
        ttctl(STDOUT,TT_CM,x,y);
        print_disk(size);
        pause(5000);
    }

  }

}

move_graphic(from, to)
int from, to;
{ int src_h, dst_h, size;
   src_h = get_height_top_disk(from);
   dst_h = 1 + get_height_top_disk(to);
   size = get_size(from, src_h);
   move_up(from, src_h, size);
   move_over(from, to, size);
   move_dn(to, dst_h, size);
   display[from][src_h] = 0;
   display[to][dst_h] = size;
   pause(1000);
}



    Error (emsg) char *emsg;
        {
        printf(" Error in Towers: %s\n",emsg);
        };

    Makenull (s)
        {
        stack[s]=0;
        };

    int Getelement ()
        {
        int temp;
        if ( freelist>0 )
            {
            temp = freelist;
            freelist = cellspace[freelist].next;
            }
        else
            Error("out of space   ");
        return (temp);
        };

    Push(i,s) int i, s;
        {
        int errorfound, localel;
        errorfound=false;
        if ( stack[s] > 0 )
            if ( cellspace[stack[s]].discsize<=i )
                {
                errorfound=true;
                Error("disc size error");
                };
        if ( ! errorfound )
            {
            localel=Getelement();
            cellspace[localel].next=stack[s];
            stack[s]=localel;
            cellspace[localel].discsize=i;
            }
        };

    Init (s,n) int s, n;
        {
        int discctr,x;
        Makenull(s);
	x = 0;
        for ( discctr = n; discctr >= 1; discctr-- ) {
            Push(discctr,s);
	    display[s][x++]=discctr;
	  }
        };

    int Pop (s) int s;
        {
         int temp, temp1;
        if ( stack[s] > 0 )
            {
            temp1 = cellspace[stack[s]].discsize;
            temp = cellspace[stack[s]].next;
            cellspace[stack[s]].next=freelist;
            freelist=stack[s];
            stack[s]=temp;
            return (temp1);
            }
        else
            Error("nothing to pop ");
        };

    Move (s1,s2) int s1, s2;
        {
	move_graphic(s1,s2);
        Push(Pop(s1),s2);
        movesdone=movesdone+1;
        };

    tower(i,j,k) int i,j,k;
        {
        int other,n;

	ioctl(0,FIONREAD,&n);
	if (n > 0) {
		end_gmode();
		exit();
		}

        if ( k==1 )
            Move(i,j);
        else
            {
            other=6-i-j;
            tower(i,other,k-1);
            Move(i,j);
            tower(other,j,k-1);
            }
        };


Towers ()    { /* Towers */
    int i, rc;
    rc = GOOD;
    for ( i=1; i <= maxcells; i++ )
        cellspace[i].next=i-1;
    freelist=maxcells;
    init_display();
    Init(1,11);
    default_display();
    Makenull(2);
    Makenull(3);
    movesdone=0;
    while (1) {
    tower(1,2,11);
    pause(100000);
    tower(2,3,11);
    pause(100000);
    tower(3,1,11);
    pause(100000);
    }
    }; /* Towers */




#ifdef PMCC
main()
#else
hanoi()
#endif
{
int n, i, error, loops;

loops = 1;
error = GOOD;
start_gmode();

/*
ttctl(STDOUT,TT_CM,20,10);
print_disk(5);
printf("\n");
*/
while ((loops <= 1) && (!error))
{
	
	if (!error)
	{	
		error = Towers();
	}
	

	printf("\n");
	++loops;
}

if (error)
{
	printf("Error in loop: %d\n",loops-1);
} else {

}

return 0;
}

start_gmode()
{
int n;

if (ttctl(STDOUT,TT_CLR) == -1) { /* clear screen */
	printf("terminal type not set\n");
	exit(1);
	}
ttctl(STDOUT,TT_CUROFF);
}

end_gmode()
{

ttctl(STDOUT,TT_CURON);
}




