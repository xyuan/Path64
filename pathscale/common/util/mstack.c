/*
 * Copyright (C) 2007. QLogic Corporation. All Rights Reserved.
 */

/*
 * Copyright 2004, 2005, 2006 PathScale, Inc.  All Rights Reserved.
 */

/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

   Path64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   Path64 is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Path64; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   Special thanks goes to SGI for their continued support to open source

*/


static char *Version = "$Source: /home/bos/bk/kpro64-pending/common/util/SCCS/s.mstack.c $ $Revision: 1.5 $";
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "mstack.h"

static int getsp ( int a );
static int fra ( int a );
static char *savestr ( char *str );
#ifndef MONGOOSE_BE
static void make_ftab ( void );
#endif /* MONGOOSE_BE */
static struct frec *search_in_ftab ( int adr );

#if 0  // WAS:  #if mips
static int ftab_problems = 0;
/*--------------------------------------------------------------
 * extremely dependent on mips calling convention. returns sp
 * of the calling routine
 *------------------------------------------------------------*/
static int getsp(int a)
{
  return (int) &a;
}
/*--------------------------------------------------------------
 * extremely dependent on mips calling convention. returns 
 * return address in the caller of getra. Intent is to find 
 * current value of pc.
 *------------------------------------------------------------*/
static int fra(int a)
{
  int *p = &a;
  return p[5];
}

static int getra(void)
{
  return fra(3);
}

struct frec {
    char *name, *file;
    int  addr, fsize, raofst;
};

#define STRSP_SIZE 512
static char *strsp;
static int strsp_left = 0;

static char *savestr(char *str)
{
    int l;
    char *x;

    l = strlen(str);
    if (strsp_left < (l+1)) {
	strsp = (char *) malloc(STRSP_SIZE);
	strsp_left = STRSP_SIZE;
    }
    x = strsp;
    strcpy(x, str);
    strsp += (l+1);
    strsp_left -= (l+1);
    return x;
}

static struct frec *ftab;
static struct frec *main_fr;
static struct frec *trst_fr;
static char *tmpname = "                              ";

#ifndef MONGOOSE_BE
static void make_ftab()
{
    FILE *fp;
    char *curfile = "", buf1[200], buf[200];
    int adr, j1, j2, j3, j4, j5, ro, fs, l, i, lc, ch;
    struct frec *cf;
    extern char **__Argv;

    sprintf(tmpname, "/tmp/TPF%1d", getpid());
    sprintf(buf, "odump -P %s > %s", __Argv[0], tmpname);
    system(buf);
    fp = fopen(tmpname, "r");
    unlink(tmpname);
    lc = 0;
    while ((ch = getc(fp)) != EOF) {
	if (ch == '\n')
	    lc++;
    }
    /* check for a problem with odump */
    if (lc < 100) {
      ftab_problems++;
      fclose(fp);
    }
    fseek(fp, 0, 0);
    ftab = (struct frec *) malloc(lc * sizeof(struct frec));
    cf = ftab;
    for (i=0; i<8; i++)
        fgets(buf, 200, fp);
    while (fgets(buf, 200, fp)) {
	l = strlen(buf);
	if (buf[l-2] == ']') {
	    sscanf(buf, " %s", buf1);
	    curfile = savestr(buf1);
	} else if (buf[l-2] != ':') {
	    sscanf(buf, "%s 0x%x %d %d %d 0x%x %d %d %d", buf1, &adr, &j1, &j2,
				&j3, &j4, &ro, &fs, &j5);
	    
	    cf->file = curfile;
	    cf->name = savestr(buf1);
	    cf->addr = adr;
	    cf->raofst = ro;
	    cf->fsize = fs;
	    if (strcmp(buf1, "main") == 0) main_fr = cf;
	    if (strcmp(buf1, "trace_stack") == 0) trst_fr = cf;
	    cf++;
	    fgets(buf, 200, fp);
	}
    }
    fclose(fp);
    cf->addr = 0x7fffffff;
}
#endif /* MONGOOSE_BE */
/*----------------------------------------------------------------------
 *	do it by binary search
 *--------------------------------------------------------------------*/
static struct frec *search_in_ftab(int adr)
{
    struct frec *f;

    /* handle problem with odump */
    if (ftab_problems) {
      return NULL;
    }
    f = ftab;
    while (f->addr <= adr)
	f++;
    return f-1;
}

static struct frec *this_func;

#ifndef MONGOOSE_BE
int trace_stack(prfunc, prfile)
    int prfunc, prfile;
{
    int sp, ra, fc;
    struct frec *cf;

#ifndef BACK_END
    return ( 1 ); /* STREICH: figure out what's wrong later... */
#endif /* BACK_END */

#define SPOFST -36
    /*sp = ((int) &sp) + SPOFST;*/
    fc = 0;
    sp = getsp(0);
    if (ftab_problems == 0 && ftab == NULL)
	make_ftab();
    /*cf = trst_fr;*/
    cf = search_in_ftab(getra());
    /* RAT: There is a bug in odump which sometimes causes it not
     *	to find the whole procedure table.  So search_in_ftab will
     *  sometimes fail (in a perfect world it would never fail).
     *	So check for a nil cf and return 0
     */
    if (cf == NULL) return fc;
    while (1) {
	if (prfunc) {
	    if (prfile) 
		printf("%s:", cf->file);
	    printf("%s\n", cf->name);
	}
	if (cf == main_fr)
	    break;
	ra = * ((int *)(sp + cf->fsize + cf->raofst));
	sp += cf->fsize;
	cf = search_in_ftab(ra);
	fc++;
    } 
    return fc;
}

#endif /* MONGOOSE_BE */

#else	/* not mips */

#if A_UX

struct x {
  struct x *next;
};

stack_lev(b)
  int b;
{
  struct x *l = (struct x *) (((int)(&b)) - 8);
  int a = 0;
  while (l) {
    a++;
    l = l->next;
  }
  return a;
}

trace_stack(a, b)
{
  return stack_lev()-1;
}

#else	/* not A_UX */

char **__Argv;

/*ARGSUSED*/
int trace_stack(a, b)
  int a;
  int b;
{
  return 1;	/* not implemented */
}

#endif	/* not A_UX */
#endif	/* not mips */

