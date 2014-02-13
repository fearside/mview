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
#file     :  viewlog.c
#contents :  print envelope sender/receiver and write down mail-body
#version  :  1.02
#higher module : mview.h (include file)
#lower  module : see Makefile if needed in detail
###############################################################################
#maintenance history
#create  :  2005/02/24  Tsuyoshi SAKAMOTO  create this program
#modify  :  2011/01/07  Masato Akiyama     change to c99 style format
#modify  :  2011/02/25  Masato Akiyama     add to search receiver address
#update  :  yyyy/mm/dd  - author -         - comments -
###############################################################################
*/


/********************************************
 * include file
 ********************************************
*/
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "mview.h"


/********************************************
 * macro
 ********************************************
*/
#define SOURCE		"viewlog.c"

#define NULL_SENDER	"<S>"
#define NULL_RECEIVER	"<R>"
#define OUT_PREFIX	"dump_"


/********************************************
 * type definition
 ********************************************
*/
typedef struct _addr {
	char address[256];
	struct _addr *next;
} Addr;

typedef struct _header {
	char sender[256];
	struct _addr *next;
	char *date;
	int write;
} Header;


/********************************************
 * global variable
 ********************************************
*/
static FILE *pin;	/* input file */
static FILE *pout;	/* output file */

/*
 * option flag
*/
int oflag 	= 0;	/* option -o */

/*
 * max length of output file name
*/
enum {
	MAX_PREFIX_LENGTH	= 5,
	MAX_SUFFIX_LENGTH	= 16
};

/*
 * matching result
*/
enum {
	UNMATCH		= 0,
	P_MATCH		= 1,
	W_MATCH		= 2
};

/*
 * write down or not
*/
enum {
	ON		= 1,
	OFF		= 0
};

/*
 * operation
*/
enum {
	NOOP		= 0,
	OPEN		= 1,
	CLOSE		= 2,
	WRITE		= 3
};


/********************************************
 * proto type
 ********************************************
*/
static void print_usage (void);
static void print_time (struct timeval *, struct timeval *);
static void print_env (FILE *, Header *);
static int match (Header *, Header *);
static int decide (char *, Header *, Header *);
static void freeall (Header *);



/*****************************************************************************
 * program section
 *****************************************************************************
*/
/********************************************
 * print usage
 ********************************************
*/
void print_usage ()
{
	int max = MAX_PREFIX_LENGTH;

	fprintf(stdout,
		"usage: viewlog [-h] [-d YYYYMMDDHHMMSS] [-o output-prefix] [-r receiver] [-s sender] file\n");
	fprintf(stdout,
		"options:\n");
	fprintf(stdout,
		"        -h<elp>     print out help\n");
	fprintf(stdout,
		"        -d<ate>     pick up only specified the date\n");
	fprintf(stdout,
		"        -o<ouput>   output file name prefix(less equal %d characters \n", max);
	fprintf(stdout,
		"        -r<eceiver> pick up only specified the receiver\n");
	fprintf(stdout,
		"        -s<ender>   pick up only specified the sender\n");

	exit(1);
}


/********************************************
 * print time of excusion
 ********************************************
*/
void print_time (struct timeval *s, struct timeval *e)
{
	/*
	 * ctime return char's pointer with a line feed,
	 * so I do not set line feed, "\n"
	*/
	fprintf(stdout, "Start Time: %s", ctime(&(s->tv_sec)));
	fprintf(stdout, "End   Time: %s", ctime(&(e->tv_sec)));

	if (s->tv_sec == e->tv_sec) {
		fprintf(stdout, "Eraps(ms): %lu\n", (e->tv_usec - s->tv_usec));
	}
	else {
		fprintf(stdout, "Eraps(s): %lu\n", (e->tv_sec - s->tv_sec));
	}

	return;
}

/********************************************
 * print envelope
 ********************************************
*/
void print_env (FILE *o, Header *p)
{
	int tos;		/* Number of receiver address */
	Addr *s;		/* Temporary pointer to search receiver address */

	tos = getnfield (TO);
	s = p->next;

	fprintf(o, "src:[%s]\n", p->sender);
	fprintf(o, "dst:[");
	for (int i = 0 ; i < tos -1 ; i++) {
		fprintf(o, "%s ", s->address);
		s = s->next;
	}
	fprintf(o, "%s]\n" , s->address);
	fprintf(o, "date:[%s]\n", p->date);
}

/********************************************
 * matching sender/receiver/date
 ********************************************
*/
int match (Header *l, Header *o)
{
	/*
	 * UNMATCH(0): non-operation, pass through by next message
	 * P_MATCH(1): print out to sdout
	 * W_MATCH(2): write down
	*/

	int tos;	/* Number of receiver address */
	Addr *s;	/* Temporary pointer to search receiver address */

	if (l->sender == NULL || l->next->address == NULL || l->date == NULL) {
		return UNMATCH;
	}

	if (*o->sender != NULL) {
		/* obsoleted
		fprintf(stderr, "[%03d] l->sender(%s), o->sender(%s)\n",
			strncmp(l->sender, o->sender, strlen(o->sender)),
			l->sender, o->sender);
		*/
		if (!strncmp(l->sender, o->sender, strlen(o->sender))) {
			return oflag ? W_MATCH : P_MATCH;
		}
		return UNMATCH;
	}
	else if (*o->next->address != NULL) {
		tos = getnfield (TO);
		s = l->next;

		for (int i = 0 ; i < tos ; i++) {
			if (!strncmp(s->address, o->next->address, strlen(o->next->address))) {
				return oflag ? W_MATCH : P_MATCH;
			}
			s = s->next;
		}
		return UNMATCH;
	}
	else if (o->date) {
		if (!strncmp(l->date, o->date, strlen(o->date))) {
			return oflag ? W_MATCH : P_MATCH;
		}
		return UNMATCH;
	}

	/*
	 * if not set option '-s' or '-r', print out or write down all
	 * of envelopes
	*/
	return oflag ? W_MATCH : P_MATCH;
}

/********************************************
 * make a decision whether to write or not
 ********************************************
*/
int decide (char *p, Header *l, Header *o)
	/* input buffer made by getlog() */
	/* envelope data of log */
	/* envelope data of option */
{
	static unsigned long int idx = 0;
	char *q;
	int rm;		/* return code of match() */
	int tos;	/* Numer of receiver address */
	Addr *s;	/* Temporary pointer to search receiver address */

	/*
	 * src:[   set sender
	 * dst:[   set recevier
	 * date:[  set date
	 * Size:   close file and clear pout
	*/
	tos = getnfield(TO);

	if (!strncmp(p, STR_SRC, strlen(STR_SRC))) {
		q = getfield(0 , FROM);
		//Estrdup(l->sender, (*q == NULL ? NULL_SENDER : q));
		if (*q == NULL)
			strcpy (l->sender , NULL_SENDER);
		else {
			if (strlen (q) >= 256) {
				fprintf (stdout , "** Too long address(%s)\n" , q);
				fprintf (stdout , "** Fail to copy sender address **");

				return NOOP;
			}
			strcpy (l->sender , q);
		}
		return NOOP;
	}
	else if (!strncmp(p, STR_DST, strlen(STR_DST))) {
		s = l->next;

		for (int i = 0 ; i < tos ; i++) {
			if (s->next == NULL) {
				Emalloc (s->next , sizeof (Addr));
			}
			q = getfield (i , TO);
			//Estrdup (s->address , (*q == NULL ? NULL_RECEIVER : q));
			if(*q == NULL)
				strcpy (s->address , NULL_RECEIVER);
			else {
				if (strlen(q) >= 256) {
					fprintf (stdout , "** Too long address(%s)\n" , q);
					fprintf (stdout , "** Fail to copy receiver address **");

					return NOOP;
				}	
			strcpy (s->address , q);
			}
			s = s->next;
		}
		return NOOP;
	}
	else if (!strncmp(p, STR_DATE, strlen(STR_DATE))) {
		Estrdup(l->date, getfield(0 , DATE));
		rm = match(l, o);
		if (rm == W_MATCH || rm == P_MATCH) {
			fprintf(stdout, "%06lu %s ", ++idx, l->sender);

			s = l->next;
			for (int i = 0 ; i < tos ; i++ ) {
				fprintf(stdout, "%s ", s->address);
				s = s->next;
			}
			fprintf(stdout, "%s\n", l->date);
			if (rm == W_MATCH) {
				l->write = ON;
				return OPEN;
			}
		}
		return NOOP;
		/* obsoleted code
		if ((rm = match(l, o)) == W_MATCH) {
			l->write = ON;

			t = l->next;
			for (int i = 0 ; i < tos ; i++ ) {
				fprintf(stdout, "%s ", t->address);
				t = t->next;
			}
			return OPEN;
		}
		else if (rm == P_MATCH) {
			l->write = OFF;

			t = l->next;
			for (int i = 0 ; i < tos ; i++ ) {
				fprintf(stdout, "%s ", t->address);
				t = t->next;
			}
		}
		return NOOP;
		*/
	}
	else if (!strncmp(p, STR_SIZE, strlen(STR_SIZE))) {
		if (l->write == ON) {
			return CLOSE;
		}
		return NOOP;
	}

	if (l->write == ON) {
		return WRITE;
	}

	return NOOP;
}

/********************************************
 * free all information
 ********************************************
*/
void freeall (Header *p)
{
	Addr *t;	/* Temporary pointer to erase Addr */

	if (*p->sender != NULL) {
		bzero (p->sender , 256);
	}
	for (t = p->next ; t != NULL ; ) {
		if (*t->address != NULL) {
			bzero (t->address , 256);
			t = t->next;
		}
		else
			break;
	}
	if (p->date) {
		Efree(p->date);
	}
	memset(p, NULL, sizeof(Header));

	return;
}

/********************************************
 * main routine
 ********************************************
*/
int main (int argc, char **argv)
{
	char ch;		/* getopt */

	char *input;		/* input file name */
	char *ibuff;		/* input ibuffer */
	size_t osize;		/* output file name size */
	char *output;		/* output entire file name */
	char *out_prefix;	/* output file name prefix */
	unsigned long int out_suffix;	/* output file name suffix */

	Header log;		/* envelope data of mail */
	Header opt;		/* option '-r' or '-s' or '-d' */

	int rt;			/* return code for "decide()" */

	struct timeval stp;	/* time of starting */
	struct timeval etp;	/* time of ending */

	/******************************************
	 * initialize
	 ******************************************
	*/
	input = NULL;
	ibuff = NULL;
	output = NULL;
	out_prefix = NULL;
	out_suffix = 0;
	memset(&log, NULL, sizeof(Header));
	memset(&opt, NULL, sizeof(Header));


	Emalloc (log.next , sizeof (Addr));

	/*
	 * get time
	*/
	if (gettimeofday(&stp, NULL)) {
		sys_err(" gettimeofday failure", SOURCE, __LINE__, 0);
	}

	/*
	 * get options
	*/
	while ((ch = getopt(argc, argv, "d:h:o:r:s:")) != -1) {
		switch((char)ch) {
		case 'd':
			Estrdup(opt.date, optarg);
			break;
		case 'r':
			Emalloc (opt.next , sizeof (Addr));
			strcpy (opt.next->address , optarg);
			//Estrdup(opt.next->address, optarg);
			break;
		case 's':
			strcpy (opt.sender , optarg);
			//Estrdup(opt.sender, optarg);
			break;
		case 'o':
			oflag = ON;
			Estrdup(out_prefix, optarg);
			if (strlen(out_prefix) > MAX_PREFIX_LENGTH) {
				out_prefix[MAX_PREFIX_LENGTH + 1] = NULL;
			}
			break;
		case 'h':
		default:
			print_usage();
			break;
		}
	}

	/*
	 * set STDIN if not set file name or set "-"
	*/

	/*
	 * output file name
	*/
	if (!oflag) {
		Estrdup(out_prefix, OUT_PREFIX);
	}
	osize = MAX_PREFIX_LENGTH + MAX_SUFFIX_LENGTH + 1;
	Emalloc(output, osize);
	memset(output, NULL, osize);

	for (int i = optind ; i < argc ; i++) {
		/******************************************
		 * initialize
		 ******************************************/
		input = NULL;

		Estrdup(input, argv[i]);
		if (( pin = fopen ( input , "r")) != NULL ) {

			/******************************************
			 * main
			 ******************************************
			 */
			/*unsigned long int line = 0; obsoleted */
			for (; (ibuff = getlog(pin)) != NULL; ) {
				/* fprintf(stderr, "%lu %s\n", ++line, ibuff); obsoleted */
				if ((rt = decide(ibuff, &log, &opt)) == WRITE) {
					fprintf(pout, "%s\n", ibuff);
				}
				else if (rt == OPEN) {
					snprintf(output, osize, "%s%lu",
							 out_prefix, ++out_suffix);
					Fopen(pout, output, "w");
					print_env(pout, &log);
				}
				else if (rt == CLOSE) {
					//freeall(&log);
					log.write = NOOP;
					Fclose(pout);
				}
				else {	/* NOOP */
					continue;
				}
			}

			Fclose(pin);
		}
	}
		/*
	* get time
	*/
	if (gettimeofday(&etp, NULL)) {
		sys_err(" gettimeofday failure", SOURCE, __LINE__, 0);
	}
        else {
		print_time(&stp, &etp);
	}

	return 0;
}

/* end of source */
