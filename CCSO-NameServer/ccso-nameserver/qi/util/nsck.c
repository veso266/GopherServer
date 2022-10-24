/*
 * Copyright (c) 1985 Corporation for Research and Educational Networking
 * Copyright (c) 1988 University of Illinois Board of Trustees, Steven
 *		Dorner, and Paul Pomes
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the Corporation for
 *	Research and Educational Networking (CREN), the University of
 *	Illinois at Urbana, and their contributors.
 * 4. Neither the name of CREN, the University nor the names of their
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE TRUSTEES AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE TRUSTEES OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char  RcsId[] = "@(#)$Id: nsck.c,v 1.12 1994/08/18 16:41:50 p-pomes Exp $";
#endif

/*
 * This program checks the integrity of a CSO nameserver database.  It makes
 * sure almost everything is "on the up-and-up".  Specifically, it detects
 *  the following problems:
 *
 * + existence and permissions of all database files
 * - non-unique aliases
 * - no pointers to null leaves in the .bdx file
 * - entries in .bdx file in proper order
 * - no bad pointers into .seq file from .bdx file
 * - entries in .seq file in proper order
 * - proper counts in all files
 * + no bad pointers into .dir file from .idx file
 * + no pointers to deleted entries in .dir file from .idx file
 * + no bad pointers into .iov file from .idx file
 * + no shared pointers into .iov file from .idx and .iov files
 * + no bad pointers into .dov file from .dir file
 * + no shared pointers into .dov file from .dir file
 * + no bad pointers from .dov file into .dov file
 * + no shared pointers from .dov file into .dov file
 * - no entries whose data is longer than their entry chain
 *
 * Note that it DETECTS only; it does not (yet) correct problems.  Note
 * also that it does NOT check to see that each word the index claims to be
 * in an entry is in fact in that entry, nor does it check to see that every
 * word in indexed fields actually appear in the index with the proper
 * pointers.  The reason it does not do so is that such would amount to
 * doing a full reindex of the database, and would take hours.  Perhaps
 * at some time nsck will be extended to perform some or all of these
 * functions.
 *
 * Note also that this program avoids many of the normal database
 * operations, since they may give wrong information in the presence
 * of flaws.  That means that changes made in database structure will
 * require changes in this program.
 */
#include "protos.h"
#include <sys/types.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <grp.h>
#include <pwd.h>
#include "conf.h"

#ifndef L_SET
# define L_SET 0
#endif /* !L_SET */

#ifndef L_INCR
# define L_INCR 1
#endif /* !L_INCR */

/*
 * important definitions
 */
#define FATAL		2	/* an error that makes progress impossible */
#define ERROR		1	/* error that does not stop processing */
#define FINE		0	/* clean bill of health */
#define DEAD		1	/* for dead .dir entries */
#define REFERENCED	2	/* .dir entries that are referenced in .idx */
typedef char string[2048];	/* for various strings */

#define T	(trouble ? 0 : (trouble=ERROR)),printf
#define PERR(msg)	printf("%s%d: %s: %s\n",Me,__LINE__,msg,errno > sys_nerr ? "unknown error" : sys_errlist[errno])
#define REP_INT		1000	/* reporting interval for status messages */
#define HASNULL(x)	(!(x&0xff && x&0xff00 && x&0xff0000 && x&0xff000000))

/*
 * some functions that have to be declared...
 */

/*
 * some global variables
 */
static char *Me;		/* the name of this program */
int	WantInfo = 0;		/* do we want info messages? */
int	WantStatus = 0;	/* do we want status messages? */
extern char *sys_errlist[];	/* system errors */
extern int sys_nerr;		/* how many of them there are */
extern int errno;		/* the current error */

/*
 * this structure holds names and functions for all the checks
 */
typedef struct
{
	char	*name;		/* name (for humans) of check */
	int	(*func) ();	/* function implementing check */
}	CHECK;

int	Files(), IdxDir(), IdxIov(), DirDov();

CHECK	Checks[] =
{
	"Files and Permissions", Files,
	"IDX/DIR Consistency", IdxDir,
	"IDX/IOV Consistency", IdxIov,
	"DIR/DOV Consistency", DirDov,
	0, 0
};

/*
 * here we go...
 */
main(argc, argv)
	int	argc;
	char   **argv;
{
	int	result;

	/* when you're strange, no one remembers your name */
	Me = *argv;

	/* process options */
	for (argc--, argv++; argc && **argv == '-'; argc--, argv++)
	{
		char	*equal, **opt;

		(*argv)++;
		switch (**argv)
		{
		    case 'i':
			WantInfo = 1;
			break;
		    case 's':
			WantStatus = 1;
			break;
		    default:
			if (equal = strchr(*argv, '='))
			{
				*equal++ = '\0';
				for (opt = Strings; *opt; opt += 2)
					if (!strcmp(opt[0], *argv))
					{
						opt[1] = equal;
						break;
					}
				if (*opt == '\0')
				{
					fprintf(stderr, "%s: %s: unknown string.\n",
						Me, *argv);
					exit(1);
				}
			} else
			{
				fprintf(stderr, "%s: %s: unknown option.\n",
					Me, *argv);
				exit(1);
			}
		}
	}
	Database = (argc > 0) ? *argv : DATABASE;

	/* process database(s) */
	if (argc == 0)
		result = Nsck();	/* use configured database */
	else
		for (; argc > 0; argc--, argv++)
		{
			Database = *argv;
			result = (result << 1) | Nsck();
		}

	exit(result);
}

/*
 * The real work begins here...
 */
Nsck()
{
	int	trouble = 0;
	CHECK	*ck;

	printf("\n%s:\n", Database);
	for (ck = Checks; ck->name; ck++)
	{
		printf("%s:\n", ck->name);
		switch ((*ck->func) ())
		{
		case FATAL:
			printf("%s: Giving up\n", Database);
			return (1);
			break;

		case ERROR:
			trouble = 1;
			break;

		case FINE:
			break;
		}
	}
	return (trouble);
}

/*
 * Check that all the proper files exist, and that they have all the
 * proper owner, group, and modes
 */
#define RIGHT_MODE	(0660)
Files()
{
	int	 trouble = 0;
	static char *extensions[] =
	{"bdx", "seq", "idx", "iov", "dir", "dov", 0};
	string	 filename;
	char   **x;
	struct stat s;
	static int nsuid = -1;
	static int nsgid = -1;

	/* figure out proper uid and gid */
	if (nsuid < 0)
	{
		struct passwd *pwent;
		struct group *grent;

		if (!(pwent = getpwnam(OWNER)))
		{
			printf("Proper owner \"%s\" does not exist!\n", OWNER);
			trouble = ERROR;
			nsuid = 0;
		} else
			nsuid = pwent->pw_uid;

		if (!(grent = getgrnam(GROUP)))
		{
			printf("Proper group \"%s\" does not exist!\n", GROUP);
			trouble = ERROR;
			nsgid = 0;
		} else
			nsgid = grent->gr_gid;
	}
	/* now check the files */
	for (x = extensions; *x; x++)
	{
		sprintf(filename, "%s.%s", Database, *x);
		if (stat(filename, &s))
		{
			PERR(filename);
			trouble = FATAL;
		} else
		{
			if (s.st_uid != nsuid)
				T("%s: owner %d not %d\n", filename, s.st_uid, nsuid);
			if (s.st_gid != nsgid)
				T("%s: group %d not %d\n", filename, s.st_gid, nsgid);
			if ((s.st_mode & 0777) != RIGHT_MODE)
				T("%s: mode %o not %o\n", filename, s.st_mode, RIGHT_MODE);
		}
	}

	return (trouble);
}

/*
 * make sure all the pointers in the .idx file point to non-deleted,
 * valid entries in the .dir file.
 */
IdxDir()
{
	int	 trouble = FINE;
	int	 dirCount, idxCount;
	int	 i;
	char	*ents = NULL;
	string	 dirName, idxName, iovName;
	string	 buffer;
	int	 dfd = 0, ifd = 0, iod = 0;
	register int *ptr;
	struct stat s;
	int	 aDir;
	register char *d;

	/*
	 * first, we find out how many entries are in the .dir file,
	 * and collect the numbers of all the deleted entries.	Along
	 * the way, we test the size of the .dir file
	 */
	sprintf(dirName, "%s.%s", Database, "dir");
	if ((dfd = open(dirName, O_RDONLY)) == 0)
	{
		PERR(dirName);
		return (FATAL);
	}
	if (read(dfd, buffer, sizeof (int)) < 0)
	{
		PERR(dirName);
		trouble = FATAL;
		goto done;
	}
	dirCount = *((int *) buffer);
	fstat(dfd, &s);
	i = s.st_size / DRECSIZE;
	if (s.st_size % DRECSIZE)
		T("%s: size is not a multiple of %d.\n", dirName, DRECSIZE);

	if (dirCount != i)
	{
		T("%s:number of entries is %d; file size indicates %d.\n",
			dirName, dirCount, i);
		dirCount = i;
	}
	/* now, gather entries */
	maybef(WantStatus, "Gathering entry information.\n");
	if (dirCount)
	{
		ents = malloc(dirCount);
		if (!ents)
		{
			printf("malloc FAILED!	Sheesh!\n");
			trouble = FATAL;
			goto done;
		}
		for (d = ents; d < ents + dirCount; d++)
			*d = 0;

		if (lseek(dfd, 14, L_INCR) < 0)
		{
			PERR(dirName);
			trouble = FATAL;
			goto done;
		}
		for (aDir = 1; aDir < dirCount; aDir++)
		{
			if (aDir % REP_INT == REP_INT - 1)
				maybef(WantStatus, "%-10d\r", aDir);
			if (lseek(dfd, DRECSIZE - sizeof (short), L_INCR) < 0)
			{
				PERR(dirName);
				trouble = FATAL;
				goto done;
			}
			if (read(dfd, buffer, sizeof (short)) < 0)
			{
				PERR(dirName);
				trouble = FATAL;
				goto done;
			}
			if (buffer[0] || buffer[1])
				ents[aDir - 1] = DEAD;
		}
	}
	close(dfd);
	dfd = 0;

	/* entries found.  now, wade through .idx file, checking each ptr */
	maybef(WantStatus, "Checking idx file against .dir list.\n");
	sprintf(idxName, "%s.%s", Database, "idx");
	sprintf(iovName, "%s.%s", Database, "iov");
	if ((ifd = open(idxName, O_RDONLY)) == 0)
	{
		PERR(idxName);
		trouble = FATAL;
		goto done;
	}
	if ((iod = open(iovName, O_RDONLY)) == 0)
	{
		PERR(iovName);
		trouble = FATAL;
		goto done;
	}
	fstat(ifd, &s);
	if (s.st_size % sizeof (struct iindex))
			 T("%s: size is not a multiple of %d.\n", idxName, sizeof (struct iindex));
	idxCount = s.st_size / sizeof (struct iindex);

	for (i = 1; i < idxCount; i++)
	{
		if (i % REP_INT == REP_INT - 1)
			maybef(WantStatus, "%-10d\r", i);
		if (lseek(ifd, i * sizeof (struct iindex), L_SET) < 0)
		{
			PERR(idxName);
			printf("idx %d\n", i);
			trouble = FATAL;
			goto done;
		}
		if (read(ifd, buffer, sizeof (struct iindex)) < 0)
		{
			PERR(idxName);
			printf("idx %d\n", i);
			trouble = FATAL;
			goto done;
		}
		if (!*buffer || *buffer == '\377')
			continue;	/* empty... */
		for (ptr = (int *) buffer; ptr < (int *) (buffer + sizeof (struct iindex)) - 1; ptr++)

			if (HASNULL(*ptr))
			{
				for (ptr++; ptr < (int *) (buffer + sizeof (struct iindex)) - 1; ptr++)

				{
					if (!*ptr)
						break;
					if (ents[*ptr - 1] & DEAD)
						T("Index entry %d references dead entry %d.\n", i, *ptr);
					ents[*ptr - 1] |= REFERENCED;
				}
			}
		for (ptr = (int *) (buffer + sizeof (struct iindex)) - 1; *ptr;)
		{
			if (lseek(iod, *ptr * NOPTRS * PTRSIZE, L_SET) < 0)
			{
				PERR(iovName);
				T("Bad overflow pointer (%d) in idx chain %d.\n", *ptr, i);
				break;
			}
			if (read(iod, buffer, NOPTRS * PTRSIZE) < 0)
			{
				PERR(iovName);
				printf("idx %d\n", i);
				trouble = FATAL;
				goto done;
			}
			for (ptr = (int *) buffer; ptr < (int *) (buffer + NOPTRS * PTRSIZE) - 1; ptr++)
			{
				if (!*ptr)
					break;
				if (ents[*ptr - 1] & DEAD)
					T("Index chain %d references dead entry %d.\n", i, *ptr);
				ents[*ptr - 1] |= REFERENCED;
			}
		}
	}

	/* report on any "funny" dir entries */
	for (d = ents; d < &ents[dirCount]; d++)
		switch (*d)
		{
		case REFERENCED:
			break;
		case DEAD:
			maybef(WantInfo, "INFO: entry %d is dead.\n", d - ents + 1);
			break;
		case DEAD | REFERENCED:
			T("Dead entry %d referenced.\n", d - ents + 1);
			break;
		case 0:
			T("Entry %d unreferenced.\n", d - ents + 1);
			break;
		default:
			printf("Internal error, entry %d (%d).\n", d - ents + 1, *d);
			break;
		}

	/* finished.  clean up and return */
      done:
	if (ifd)
		close(ifd);
	if (iod)
		close(iod);
	if (dfd)
		close(dfd);
	if (ents)
		free(ents);
	return (trouble);
}

/*
 * print a message (maybe)
 */
maybef(yes, fmt, a, b, c, d, e, f, g, h, i, j, l, m, n, o, p)
	int	 yes;
	char	*fmt;
	unsigned a, b, c, d, e, f, g, h, i, j, l, m, n, o, p;
{
	if (yes)
		printf(fmt, a, b, c, d, e, f, g, h, i, j, l, m, n, o, p);
	fflush(stdout);
}

/*
 * Check the consistency of the Idx and Iov files
 * - no unreferenced IOV entries
 * - no doubly-referenced IOV entries
 */
IdxIov()
{
	int	 idx = 0, iov = 0;
	string	 idxName, iovName;
	string	 buffer;
	int	*iovEnts = NULL;
	int	 trouble = FINE;
	int	 idxCount, iovCount;
	struct stat s;
	int	 i;
	register int *ent;

	sprintf(iovName, "%s.iov", Database);
	sprintf(idxName, "%s.idx", Database);
	if ((idx = open(idxName, O_RDONLY)) == 0)
	{
		PERR(idxName);
		trouble = FATAL;
		goto done;
	}
	if ((iov = open(iovName, O_RDONLY)) == 0)
	{
		PERR(iovName);
		trouble = FATAL;
		goto done;
	}
	/* set up and array to keep track of iov entries and references */
	fstat(iov, &s);
	if (s.st_size % (PTRSIZE * NOPTRS))
		T("%s: size not a multiple of %d.\n", iovName, PTRSIZE * NOPTRS);
	iovCount = s.st_size / (PTRSIZE * NOPTRS);
	if (!(iovEnts = (int *) malloc(iovCount * sizeof (int))))
	{
		PERR("malloc iovEnts");
		trouble = FATAL;
		goto done;
	}
	for (ent = iovEnts; ent - iovEnts < iovCount; ent++)
		*ent = 0;

	/* look through idx file for bad things */
	fstat(idx, &s);
	idxCount = s.st_size / sizeof (struct iindex);

	/* put a NULL word at end of iindex buffer as a placemarker */
	*(int *) (buffer + sizeof (struct iindex)) = 0;
	*(int *) (buffer + sizeof (struct iindex) + sizeof (int)) = 0;

	/* read the iindex entries */
	maybef(WantStatus, "Checking idx for bad iov pointers.\n");
	for (i = 0; i < idxCount; i++)
	{
		if (i % REP_INT == REP_INT - 1)
			maybef(WantStatus, "%-10d\r", i);
		if (read(idx, buffer, sizeof (struct iindex)) < 0)
		{
			PERR(idxName);
			trouble = ERROR;
			break;
		}
		if (!*buffer || *buffer == '\377')
			continue;	/* empty... */
		/* look for a NULL byte */
		for (ent = (int *) buffer; !HASNULL(*ent); ent++) ;
		/* look for a NULL word */
		for (ent++; *ent; ent++) ;
		if (ent - (int *) buffer == sizeof (struct iindex) / sizeof (int))
		{
			ent--;
			/* we have an overflow pointer */
			if (*ent < 0 || *ent >= iovCount)
				T("idx %d: overflow pointer out of range (%d).\n", i, *ent);
			else if (iovEnts[*ent])
				T("idx %d: shares overflow %d with %d.\n", i, *ent, iovEnts[*ent]);
			else
				iovEnts[*ent] = i;
		}
	}
	close(idx);
	idx = 0;

	/* now for the iov entries */
	maybef(WantStatus, "Checking iov for bad iov pointers.\n");
	*(int *) (buffer + PTRSIZE * NOPTRS) = 0;
	for (i = 0; i < iovCount; i++)
	{
		if (i % REP_INT == REP_INT - 1)
			maybef(WantStatus, "%-10d\r", i);
		if (read(iov, buffer, PTRSIZE * NOPTRS) < 0)
		{
			PERR(iovName);
			trouble = ERROR;
			break;
		}
		/* look for a NULL word */
		for (ent = (int *) buffer; *ent; ent++) ;
		if (ent - (int *) buffer == PTRSIZE * NOPTRS / sizeof (int))
		{
			ent--;
			/* we have an overflow pointer */
			if (*ent < 0 || *ent >= iovCount)
				T("iov %d: overflow pointer out of range (%d).\n", i, *ent);
			else if (iovEnts[*ent])
				T("iov %d: shares overflow %d with %d.\n", i, *ent, iovEnts[*ent]);
			else
				iovEnts[*ent] = -i;
		}
	}

	/* let's have a look for unused iov entries */
	if (WantInfo)
		for (i = 1; i < iovCount; i++)
			if (iovEnts[i] == 0)
				printf("%s: overflow block %d unused.\n", iovName, i);


      done:
	if (idx)
		close(idx);
	if (iov)
		close(iov);
	if (iovEnts)
		free(iovEnts);
	return (trouble);
}

/*
 * Check the consistency of the dir and dov files
 * - no unreferenced dov entries
 * - no doubly-referenced dov entries
 */
DirDov()
{
	int	 dir = 0, dov = 0;
	string	 dirName, dovName;
	string	 buffer;
	int	*dovEnts = NULL;
	int	 trouble = FINE;
	int	 dirCount, dovCount;
	struct stat s;
	int	 i;
	register int *ent;

	sprintf(dovName, "%s.dov", Database);
	sprintf(dirName, "%s.dir", Database);
	if ((dir = open(dirName, O_RDONLY)) == 0)
	{
		PERR(dirName);
		trouble = FATAL;
		goto done;
	}
	if ((dov = open(dovName, O_RDONLY)) == 0)
	{
		PERR(dovName);
		trouble = FATAL;
		goto done;
	}
	/* set up and array to keep track of dov entries and references */
	fstat(dov, &s);
	if (s.st_size % DOVRSIZE)
		T("%s: size not a multiple of %d.\n", dovName, DOVRSIZE);
	dovCount = s.st_size / (DOVRSIZE);
	if (!(dovEnts = (int *) malloc(dovCount * sizeof (int))))
	{
		PERR("malloc dovEnts");
		trouble = FATAL;
		goto done;
	}
	for (ent = dovEnts; ent - dovEnts < dovCount; ent++)
		*ent = 0;

	/* look through dir file for bad things */
	fstat(dir, &s);
	dirCount = s.st_size / DRECSIZE;

	/* put a NULL word at end of iindex buffer as a placemarker */
	*(int *) (buffer + DRECSIZE) = 0;
	*(int *) (buffer + DRECSIZE + sizeof (int)) = 0;

	/* read the dir entries */
	maybef(WantStatus, "Checking dir for bad dov pointers.\n");
	ent = (int *) buffer;
	for (i = 0; i < dirCount; i++)
	{
		if (i % REP_INT == REP_INT - 1)
			maybef(WantStatus, "%-10d\r", i);
		if (read(dir, buffer, DRECSIZE) < 0)
		{
			PERR(dirName);
			trouble = ERROR;
			break;
		}
		if (buffer[16] || buffer[17])
			continue;	/* dead... */
		/* look for a NULL byte */
		if (*ent)
		{
			/* we have an overflow pointer */
			if (*ent < 0 || *ent >= dovCount)
				T("dir %d: overflow pointer out of range (%d).\n", i, *ent);
			else if (dovEnts[*ent])
				T("dir %d: shares overflow %d with %d.\n", i, *ent, dovEnts[*ent]);
			else
				dovEnts[*ent] = i;
		}
	}
	close(dir);
	dir = 0;

	/* now for the dov entries */
	maybef(WantStatus, "Checking dov for bad dov pointers.\n");
	ent = ((int *) (buffer + DOVRSIZE)) - 1;
	for (i = 0; i < dovCount; i++)
	{
		if (i % REP_INT == REP_INT - 1)
			maybef(WantStatus, "%-10d\r", i);
		if (read(dov, buffer, DOVRSIZE) < 0)
		{
			PERR(dovName);
			trouble = ERROR;
			break;
		}
		/* look for a NULL word */
		if (*ent)
		{
			/* we have an overflow pointer */
			if (*ent < 0 || *ent >= dovCount)
				T("dov %d: overflow pointer out of range (%d).\n", i, *ent);
			else if (dovEnts[*ent])
			{
				if (dovEnts[*ent] > 0)
					T("dov %d: shares overflow %d with %d.\n", i, *ent, dovEnts[*ent]);
				else
					T("dov %d: shares overflow %d with dov %d.\n", i, *ent, -dovEnts[*ent]);
			} else
				dovEnts[*ent] = -i;
		}
	}

	/* let's have a look for unused dov entries */
	if (WantInfo)
		for (i = 1; i < dovCount; i++)
			if (dovEnts[i] == 0)
				printf("%s: overflow block %d unused.\n", dovName, i);


      done:
	if (dir)
		close(dir);
	if (dov)
		close(dov);
	if (dovEnts)
		free(dovEnts);
	return (trouble);
}
