/*
 * Copyright (c) 2005, Tsuyoshi Sakamoto <skmt.japan@gmail.com>,
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE. 
*/

/*
###############################################################################
#program  :  Mail Statistics
#system   :  unix, C language
#file     :  getlog.c
#contents :  getlog(), getfield(), getnfield()
#version  :  1.01
#higher module : mview.c
#lower  module : none
###############################################################################
#maintenance history
#create  :  2005/02/24  Tsuyoshi SAKAMOTO  create this program
#update  :  yyyy/mm/dd  - author -         - comments -
###############################################################################
*/

/********************************************
 * include file
 ********************************************
*/
#include "mview.h"

/********************************************
 * macro
 ********************************************
*/
#define SOURCE		"getlog.c"


/********************************************
 * global variable
 ********************************************
*/
static char *log	= NULL;
static char *slog	= NULL;
static char **field	= NULL;
static int f_nfield	= 0;
static int t_nfield	= 0;
static int d_nfield	= 0;
static size_t lsize	= 1;
static size_t fsize	= sizeof(field);

/********************************************
 * prototype
 ********************************************
*/
int getnfield(int);
void setnfield(int , int);
char *getfield(int , int);
char *getlog(FILE *);
static char *tolowerall(char *);
static void split(char *);


/********************************************
 * split
 ********************************************
*/
void split(char *p)
{
	int n;		/* number of field */
	int type;	/* type of envelope header */
	char *q;	/* starting pointer of each fields */
	char **r;	/* work field */

	if (field == NULL) {
		fsize *= 2;
		Emalloc(field, fsize);
	}

	if (!strncmp(p, STR_SRC, strlen(STR_SRC))) {
		p += (STR_SRC_LENGTH - 1);
		type = FROM;
	}
	else if (!strncmp(p ,STR_DST, strlen(STR_DST))) {
		p += (STR_DST_LENGTH - 1);
		type = TO;
	}
	else if (!strncmp(p, STR_DATE, strlen(STR_DATE))) {
		p += (STR_DATE_LENGTH - 1);
		type = DATE;
	}
	else {
		return;	/* not found */
	}

	n = 0;
	q = p;
	r = field;
	for (; *p != ']' && *p != NULL; ++p) {
		if (*p == SPACE) {
			*p = NULL;
			if (n == (fsize/sizeof(field) - 1)) {
				fsize *= 2;
				Realloc(field, fsize);
				r = field;
			}
			r[n++] = q;
			q = ++p;
		}
	}
	
	if (p == NULL) {
		return;
	}
	
	*p = NULL;	/* clear bracket */
	r[n++] = q;
	setnfield (n , type);

	return;
}

/********************************************
 * lower all
 ********************************************
*/
char *tolowerall (char *p)
{
	char *q;

	q = p;

	for (; *q != NULL; ++q) {
		*q = tolower(*q);
	}

	return p;
}

/********************************************
 * set nfield
 ********************************************
*/
void setnfield(int nfield , int type)
{
	if (type == FROM)
		f_nfield = nfield;
	else if (type == TO)
		t_nfield = nfield;
	else if (type == DATE)
		d_nfield = nfield;
}

/********************************************
 * get nfield
 ********************************************
*/
int getnfield(int type)
{
	if (type == FROM)
		return f_nfield;
	else if (type == TO)
		return t_nfield;
	else if (type == DATE)
		return d_nfield;
}

/********************************************
 * get field
 ********************************************
*/
char *getfield (int index, int type)
{
	if (index < 0 || index >= getnfield(type)) {
		return NULL;
	}

	return field[index];
}

/********************************************
 * get log
 ********************************************
*/
char *getlog (FILE *in)
{
	char *t;	/* working pointer of "log" */
	int c;		/* input */
	int n;		/* index of "log" stream */

	if (log == NULL) {
		lsize *= 2;
		Emalloc(log, lsize);
		Emalloc(slog, lsize);
	}

	t = log;
	for (n = 0; (c = fgetc(in)) != EOF && c != NEWLINE; ++n) {
		if (n == lsize) {
			lsize *= 2;
			Realloc(log, lsize);
			Realloc(slog, lsize);

			t = log;
		}
		t[n] = (char )c;
	}
	t[n] = NULL;	/* termination */

	memcpy(slog, log, lsize);

	split(tolowerall(slog));

	return (c == EOF && n == 0) ? NULL : log;
}


/********************************************
 * debug section
 ********************************************
 *
 * following code is the driver for getlog().
 * if you want to test getlog() only, you can do "make getlog".
 *
*/
#ifdef DEBUG_GETLOG

int main (int argc, char **argv)
{
	exit(0);
}

#endif

/* end of source */
