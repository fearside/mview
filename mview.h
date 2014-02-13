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

/********************************************
 * include
 ********************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>

#ifdef DEBUG
# ifdef HAVE_PROFILE
#  define MARK
#  include <prof.h>
# endif
#endif


/********************************************
 * constant number and string
  ********************************************
*/
#define CR		'\r'
#define NEWLINE		'\n'
#define EQUAL		'='
#define COMMA		','
#define SPACE		' '
#define BRACKET1	'<'
#define BRACKET2	'['

#define FROM		0
#define TO		1
#define DATE		2
#define STR_SRC		"src:["
#define STR_DST		"dst:["
#define STR_DATE	"date:["
#define STR_SIZE	"Size:"
#define STR_SRC_LENGTH	sizeof(STR_SRC)
#define STR_DST_LENGTH	sizeof(STR_DST)
#define STR_DATE_LENGTH	sizeof(STR_DATE)
#define STR_SIZE_LENGTH	sizeof(STR_SIZE)


/********************************************
 * macro
 ********************************************
*/
#define NEWLINE	'\n'


#define Emalloc(dst, size) \
if ((dst = malloc(size)) == NULL) { \
	sys_err(" **error** malloc error, no memory is available.", \
		SOURCE, __LINE__, 0); \
} \
else { \
	bzero(dst, size); \
}

#define Calloc(dst, num, size) \
if ((dst = calloc(num, size)) == NULL) { \
	sys_err(" **error** calloc error, no memory is available.", \
		SOURCE, __LINE__, 0); \
}

#define Realloc(dst, size) \
if ((dst = realloc(dst, size)) == NULL) { \
	sys_err(" **error** realloc error, no memory is available.", \
		SOURCE, __LINE__, 0); \
}

#define Efree(src) free(src)

#define Estrdup(dst, src) \
if ((dst = strdup(src)) == NULL) { \
	sys_err(" **error** malloc error, no memory is available.", \
		SOURCE, __LINE__, 0); \
}

#define Fopen(pfd, file, mode) \
if ((pfd = fopen(file, mode)) == NULL) { \
	sys_err(" ***error*** file open failure", SOURCE, __LINE__, 0); \
}

#define Fclose(pfd) \
if (fclose(pfd) != 0) { \
	sys_err(" ***error*** file close failure", SOURCE, __LINE__, 0); \
}

#ifdef HAVE_FGETLN
#define Fgets(buf, size, file) \
memset(buf, NULL, sizeof(buf)); \
if (buf = fgetln(file, sizeof(buf)) < 0) { \
	sys_err(" ***error*** fgetln failure", SOURCE, __LINE__, 0); \
}
#else
#define Fgets(buf, size, file) \
memset(buf, NULL, sizeof(buf)); \
if (fgets(buf, size, file) < 0) { \
	sys_err(" ***error*** fgets failure", SOURCE, __LINE__, 0); \
}
#endif

#define	OFFSET(type, field) \
	((unsigned int)&(((type *)NULL)->field))

/********************************************
* function
********************************************
*/

extern int sys_err(const char *, const char *, long int, int);

extern char *getlog(FILE *);
extern char *getfield(int , int);
extern int getnfield(int);
extern void setnfield(int , int);

/* end of header */
