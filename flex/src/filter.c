/* filter - postprocessing of flex output through filters */

/*  This file is part of flex. */

/*  Redistribution and use in source and binary forms, with or without */
/*  modification, are permitted provided that the following conditions */
/*  are met: */

/*  1. Redistributions of source code must retain the above copyright */
/*     notice, this list of conditions and the following disclaimer. */
/*  2. Redistributions in binary form must reproduce the above copyright */
/*     notice, this list of conditions and the following disclaimer in the */
/*     documentation and/or other materials provided with the distribution. */

/*  Neither the name of the University nor the names of its contributors */
/*  may be used to endorse or promote products derived from this software */
/*  without specific prior written permission. */

/*  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR */
/*  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED */
/*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR */
/*  PURPOSE. */

#include "flexdef.h"
static const char * check_4_gnu_m4 =
    "m4_dnl ifdef(`__gnu__', ,"
    "`errprint(Flex requires GNU M4. Set the PATH or set the M4 environment variable to its path name.)"
    " m4exit(2)')\n";


/** global chain. */
struct filter *output_chain = NULL;

static const char letters[] =
"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

/* Adds temporary dir if environment variable FLEX_TMP_DIR is set */
char* add_tmp_dir(const char* tmp_file_name)
{
	char* new_tmp_file_name = 0;
	size_t tmp_dir_len = 0;
	size_t len = 0;
	char const *tmp_dir = getenv ("FLEX_TMP_DIR");

	if (!tmp_dir)
		return _strdup(tmp_file_name);

	tmp_dir_len = strlen(tmp_dir);
	len = tmp_dir_len + strlen(tmp_file_name) + 2; // two extra chars: '\' between dir and file name and '\0' at the end
	new_tmp_file_name = (char*)malloc(len*sizeof(char));
	if (tmp_dir[tmp_dir_len-1] == '\\' || tmp_dir[tmp_dir_len-1] == '/')
		sprintf(new_tmp_file_name, "%s%s", tmp_dir, tmp_file_name);
	else
		sprintf(new_tmp_file_name, "%s\\%s", tmp_dir, tmp_file_name);

	return new_tmp_file_name;
}

int max_temp_file_names = 100;
int num_temp_file_names = 0;
char* temp_file_names[100];
/* Generate a temporary file name based on TMPL.  TMPL must match the
   rules for mk[s]temp (i.e. end in "XXXXXX").  The name constructed
   does not exist at the time of the call to mkstemp.  TMPL is
   overwritten with the result.  */
FILE* mkstempFILE (char *tmpl, const char *mode)
{
	int len;
	char *XXXXXX;
	static unsigned long long value;
	unsigned long long random_time_bits;
	unsigned int count;
	FILE* fd = NULL;
	int r;

	/* A lower bound on the number of temporary files to attempt to
	generate.  The maximum total number of temporary file names that
	can exist for a given template is 62**6.  It should never be
	necessary to try all these combinations.  Instead if a reasonable
	number of names is tried (we define reasonable as 62**3) fail to
	give the system administrator the chance to remove the problems.  */
#define ATTEMPTS_MIN (62 * 62 * 62)

	/* The number of times to attempt to generate a temporary file.  To
	conform to POSIX, this must be no smaller than TMP_MAX.  */
#if ATTEMPTS_MIN < TMP_MAX
	unsigned int attempts = TMP_MAX;
#else
	unsigned int attempts = ATTEMPTS_MIN;
#endif

	if (num_temp_file_names >= max_temp_file_names)
		return NULL;

	len = strlen (tmpl);
	if (len < 6 || strcmp (&tmpl[len - 6], "XXXXXX"))
	{
		return NULL;
	}

	/* This is where the Xs start.  */
	XXXXXX = &tmpl[len - 6];

	/* Get some more or less random data but unique per process */
	{
		static unsigned long long g_value;
		g_value = _getpid();
 		g_value += 100;
		random_time_bits = (((unsigned long long)234546 << 32)
			| (unsigned long long)g_value);
	}
	value += random_time_bits ^ (unsigned long long)122434;

	for (count = 0; count < attempts; value += 7777, ++count)
	{
		char* tmp_file_name = 0;
		unsigned long long v = value;

		/* Fill in the random bits.  */
		XXXXXX[0] = letters[v % 62];
		v /= 62;
		XXXXXX[1] = letters[v % 62];
		v /= 62;
		XXXXXX[2] = letters[v % 62];
		v /= 62;
		XXXXXX[3] = letters[v % 62];
		v /= 62;
		XXXXXX[4] = letters[v % 62];
		v /= 62;
		XXXXXX[5] = letters[v % 62];

		tmp_file_name = add_tmp_dir(tmpl);
		/* file doesn't exist */
		if (r = _access(tmp_file_name, 0) == -1)
		{
			fd = fopen (tmp_file_name, mode);
			if (fd)
			{
				temp_file_names[num_temp_file_names] = tmp_file_name;
				++num_temp_file_names;
				return fd;
			}
		}

		free(tmp_file_name);
		tmp_file_name = 0;
	}

	/* We got out of the loop because we ran out of combinations to try.  */
	return NULL;
}

/* delete all temp files */
void unlinktemp()
{
	while (num_temp_file_names)
	{
		--num_temp_file_names;

		if (_unlink(temp_file_names[num_temp_file_names]))
			fprintf(stderr, _("error delete file %s"), temp_file_names[num_temp_file_names]);

		free(temp_file_names[num_temp_file_names]);
		temp_file_names[num_temp_file_names] = NULL;
	}
}


#if 0
/* Allocate and initialize an external filter.
 * @param chain the current chain or NULL for new chain
 * @param cmd the command to execute.
 * @param ... a NULL terminated list of (const char*) arguments to command,
 *            not including argv[0].
 * @return newest filter in chain
 */
struct filter *filter_create_ext (struct filter *chain, const char *cmd,
				  ...)
{
	struct filter *f;
	int     max_args;
	const char *s;
	va_list ap;

	/* allocate and initialize new filter */
	f = (struct filter *) flex_alloc (sizeof (struct filter));
	if (!f)
		flexerror (_("flex_alloc failed (f) in filter_create_ext"));
	memset (f, 0, sizeof (*f));
	f->filter_func = NULL;
	f->extra = NULL;
	f->next = NULL;
	f->argc = 0;

	if (chain != NULL) {
		/* append f to end of chain */
		while (chain->next)
			chain = chain->next;
		chain->next = f;
	}


	/* allocate argv, and populate it with the argument list. */
	max_args = 8;
	f->argv =
		(const char **) flex_alloc (sizeof (char *) *
					    (max_args + 1));
	if (!f->argv)
		flexerror (_("flex_alloc failed (f->argv) in filter_create_ext"));
	f->argv[f->argc++] = cmd;

	va_start (ap, cmd);
	while ((s = va_arg (ap, const char *)) != NULL) {
		if (f->argc >= max_args) {
			max_args += 8;
			f->argv =
				(const char **) flex_realloc (f->argv,
							      sizeof (char
								      *) *
							      (max_args +
							       1));
		}
		f->argv[f->argc++] = s;
	}
	f->argv[f->argc] = NULL;

	va_end (ap);
	return f;
}
#endif
/* Allocate and initialize an internal filter.
 * @param chain the current chain or NULL for new chain
 * @param filter_func The function that will perform the filtering.
 *        filter_func should return 0 if successful, and -1
 *        if an error occurs -- or it can simply exit().
 * @param extra optional user-defined data to pass to the filter.
 * @return newest filter in chain
 */
struct filter *filter_create_int (struct filter *chain,
				  int (*filter_func) (struct filter *),
				  void *extra)
{
	struct filter *f;

	/* allocate and initialize new filter */
	f = (struct filter *) flex_alloc (sizeof (struct filter));
	if (!f)
		flexerror (_("flex_alloc failed in filter_create_int"));
	memset (f, 0, sizeof (*f));
	f->next = NULL;
	f->in_file = NULL;
	f->out_file = NULL;

	f->filter_func = filter_func;
	f->extra = extra;

	if (chain != NULL) {
		/* append f to end of chain */
		while (chain->next)
			chain = chain->next;
		chain->next = f;
	}

	return f;
}

/** Fork and exec entire filter chain.
 *  @param chain The head of the chain.
 *  @return true on success.
 */
bool filter_apply_chain (struct filter * chain, FILE* in_file, FILE* out_file)
{
	char out_file_name[256] = "~X_flex_temp_XXXXXX";
	static char file_num = '0';
	FILE* mid_out_file = NULL;
	int r;
	bool result;

	++file_num;
	out_file_name[1] = file_num;

	/* nothing to do */
	if (!chain)
		return true;

	/* setup in/out files*/
	chain->in_file = in_file;
	if (chain->next)
	{
		mid_out_file = mkstempFILE(out_file_name, "w+");
		if (!mid_out_file)
			flexerror (_("fail create temp file"));

		chain->out_file = mid_out_file;
	}
	else
	{
		chain->out_file = out_file;
	}

	/* run current filter */
	if (!chain->filter_func)
		flexerror (_("fail call filter"));

	r = chain->filter_func (chain);

	if (r == -1)
		flexfatal (_("filter_func failed"));

	if (chain->next)
	{
		if (fseek(chain->out_file, 0, SEEK_SET) != 0)
			flexerror(_("fseek failed"));

		/* go to next filter */
		result = filter_apply_chain(chain->next, chain->out_file, out_file);
	}

	/* close temp file */
	if (mid_out_file)
	{
		if (fclose(mid_out_file))
			lerrsf(_("error close file %s"), out_file_name);
	}

	return result;
#if 0		
	int pipes[2];
	intptr_t pid;

	/* Tricky recursion, since we want to begin the chain
	 * at the END. Why? Because we need all the forked processes
	 * to be children of the main flex process.
	 */
	if (chain)
		filter_apply_chain (chain->next);
	else
		return true;

	/* Now we are the right-most unprocessed link in the chain.
	 */

	fflush (stdout);
	fflush (stderr);

	if (_pipe (pipes, 512, _O_TEXT) == -1)
		flexerror (_("pipe failed"));

	if ((pid = _spawnl (_P_NOWAIT, "win_flex.exe", "win_flex.exe", NULL)) == -1)
		flexerror (_("fork failed"));

	if (pid == 0) {
		/* child */

        /* We need stdin (the FILE* stdin) to connect to this new pipe.
         * There is no portable way to set stdin to a new file descriptor,
         * as stdin is not an lvalue on some systems (BSD).
         * So we dup the new pipe onto the stdin descriptor and use a no-op fseek
         * to sync the stream. This is a Hail Mary situation. It seems to work.
         */
		_close (pipes[1]);
		if (_dup2 (pipes[0], _fileno (stdin)) == -1)
			flexfatal (_("dup2(pipes[0],0)"));
		_close (pipes[0]);
        fseek (stdin, 0, SEEK_CUR);

		/* run as a filter, either internally or by exec */
		if (chain->filter_func) {
			int     r;

			if ((r = chain->filter_func (chain)) == -1)
				flexfatal (_("filter_func failed"));
			exit (0);
		}
		else {
			_execvp (chain->argv[0],
				(char **const) (chain->argv));
			flexfatal (_("exec failed"));
		}

		exit (1);
	}

	/* Parent */
	_close (pipes[0]);
	if (_dup2 (pipes[1], _fileno (stdout)) == -1)
		flexfatal (_("dup2(pipes[1],1)"));
	_close (pipes[1]);
    fseek (stdout, 0, SEEK_CUR);

	return true;
#endif
}

/** Truncate the chain to max_len number of filters.
 * @param chain the current chain.
 * @param max_len the maximum length of the chain.
 * @return the resulting length of the chain.
 */
int filter_truncate (struct filter *chain, int max_len)
{
	int len = 1;

	if (!chain)
		return 0;

	while (chain->next && len < max_len) {
		chain = chain->next;
		++len;
	}

	chain->next = NULL;
	return len;
}

/** Splits the chain in order to write to a header file.
 *  Similar in spirit to the 'tee' program.
 *  The header file name is in extra.
 *  @return 0 (zero) on success, and -1 on failure.
 */
int filter_tee_header (struct filter *chain)
{
	/* This function reads from stdin and writes to both the C file and the
	* header file at the same time.
	*/

	const int readsz = 512;
	char   *buf;
	int     to_cfd = -1;
	FILE   *to_c = NULL, *to_h = NULL;
	bool    write_header;
	FILE* header_out_file = NULL;

	write_header = (chain->extra != NULL);

	/* Store a copy of the stdout pipe, which is already piped to C file
	* through the running chain. Then create a new pipe to the H file as
	* stdout, and fork the rest of the chain again.
	*/

	if (!chain->out_file)
		flexfatal (_("out_file failed"));

	if (!chain->in_file)
		flexfatal (_("in_file failed"));

	to_c = chain->out_file;

	if (write_header) {
		char tmp_file_name[256] = "~temp_header_file_XXXXXX";
		to_h = mkstempFILE(tmp_file_name, "w+");
		if (!to_h)
			flexfatal (_("fopen(headerfilename) failed"));
	}

	/* Now to_c is a pipe to the C branch, and to_h is a pipe to the H branch.
	*/

	if (write_header) {
		fputs (check_4_gnu_m4, to_h);
		fputs ("m4_changecom`'m4_dnl\n", to_h);
		fputs ("m4_changequote`'m4_dnl\n", to_h);
		fputs ("m4_changequote([[,]])[[]]m4_dnl\n", to_h);
		fputs ("m4_define([[M4_YY_NOOP]])[[]]m4_dnl\n", to_h);
		fputs ("m4_define( [[M4_YY_IN_HEADER]],[[]])m4_dnl\n",
			to_h);
		fprintf (to_h, "#ifndef %sHEADER_H\n", prefix);
		fprintf (to_h, "#define %sHEADER_H 1\n", prefix);
		fprintf (to_h, "#define %sIN_HEADER 1\n\n", prefix);
		fprintf (to_h,
			"m4_define( [[M4_YY_OUTFILE_NAME]],[[%s]])m4_dnl\n",
			headerfilename ? headerfilename : "<stdout>");

	}

	fputs (check_4_gnu_m4, to_c);
	fputs ("m4_changecom`'m4_dnl\n", to_c);
	fputs ("m4_changequote`'m4_dnl\n", to_c);
	fputs ("m4_changequote([[,]])[[]]m4_dnl\n", to_c);
	fputs ("m4_define([[M4_YY_NOOP]])[[]]m4_dnl\n", to_c);
	fprintf (to_c, "m4_define( [[M4_YY_OUTFILE_NAME]],[[%s]])m4_dnl\n",
		outfilename ? outfilename : "<stdout>");

	buf = (char *) flex_alloc (readsz);
	if (!buf)
		flexerror (_("flex_alloc failed in filter_tee_header"));
	while (fgets (buf, readsz, chain->in_file)) {
		fputs (buf, to_c);
		if (write_header)
			fputs (buf, to_h);
	}

	if (write_header) {
		fprintf (to_h, "\n");

		/* write a fake line number. It will get fixed by the linedir filter. */
		fprintf (to_h, "#line 4000 \"M4_YY_OUTFILE_NAME\"\n");

		fprintf (to_h, "#undef %sIN_HEADER\n", prefix);
		fprintf (to_h, "#endif /* %sHEADER_H */\n", prefix);
		fputs ("m4_undefine( [[M4_YY_IN_HEADER]])m4_dnl\n", to_h);

		fflush (to_h);
		if (ferror (to_h))
			lerrsf (_("error writing output file %s"),
			(char *) chain->extra);
		else
		{
			if (fseek(to_h, 0, SEEK_SET))
				flexerror(_("fseek failed"));

			header_out_file = fopen ((char *) chain->extra, "w");
			if (!header_out_file)
				flexfatal (_("fopen(headerfilename) failed"));

			/* make branch for header file */
			filter_apply_chain(chain->next, to_h, header_out_file);

			/* cleanup */
			if (fclose (to_h))
				lerrsf (_("error closing output file 1 %s"),
				(char *) chain->extra);

			if (fclose (header_out_file))
				lerrsf (_("error closing output file 2 %s"),
				(char *) chain->extra);
		}
	}

	fflush (to_c);
	if (ferror (to_c))
		lerrsf (_("error writing output file %s"),
		outfilename ? outfilename : "<stdout>");
/*
	else if (fclose (to_c))
		lerrsf (_("error closing output file %s"),
		outfilename ? outfilename : "<stdout>");

	while (_cwait (NULL, ) > 0) ;
	assert(false);
	exit (0);
	return 0;
	*/

	return 0;
}

extern int main_m4 (int argc, char *const *argv, FILE* in, FILE* out);

int filter_m4_p (struct filter *chain)
{
    char const *argv[10];
    int i = 0;

    argv[i++] = "M4";
    argv[i++] = "-P";
    argv[i++] = NULL;

	return main_m4 (i-1, argv, chain->in_file, chain->out_file);
}

/** Adjust the line numbers in the #line directives of the generated scanner.
 * After the m4 expansion, the line numbers are incorrect since the m4 macros
 * can add or remove lines.  This only adjusts line numbers for generated code,
 * not user code. This also happens to be a good place to squeeze multiple
 * blank lines into a single blank line.
 */
int filter_fix_linedirs (struct filter *chain)
{
	char   *buf;
	const int readsz = 512;
	int     lineno = 1;
	bool    in_gen = true;	/* in generated code */
	bool    last_was_blank = false;

	if (!chain)
		return 0;

	buf = (char *) flex_alloc (readsz);
	if (!buf)
		flexerror (_("flex_alloc failed in filter_fix_linedirs"));

	while (fgets (buf, readsz, chain->in_file)) {

		regmatch_t m[10];

		/* Check for #line directive. */
		if (buf[0] == '#'
		    && regexec (&regex_linedir, buf, 3, m, 0) == 0) {

			int     num;
			char   *fname;

			/* extract the line number and filename */
			num = regmatch_strtol (&m[1], buf, NULL, 0);
			fname = regmatch_dup (&m[2], buf);

			if (strcmp (fname,
				outfilename ? outfilename : "<stdout>")
					== 0
			 || strcmp (fname,
			 	headerfilename ? headerfilename : "<stdout>")
					== 0) {

				char    *s1, *s2;
				char	filename[MAXLINE];

				s1 = fname;
				s2 = filename;

				while ((s2 - filename) < (MAXLINE - 1) && *s1) {
					/* Escape the backslash */
					if (*s1 == '\\')
						*s2++ = '\\';
					/* Escape the double quote */
					if (*s1 == '\"')
						*s2++ = '\\';
					/* Copy the character as usual */
					*s2++ = *s1++;
				}

				*s2 = '\0';

				/* Adjust the line directives. */
				in_gen = true;
				snprintf (buf, readsz, "#line %d \"%s\"\n",
					  lineno + 1, filename);
			}
			else {
				/* it's a #line directive for code we didn't write */
				in_gen = false;
			}

			free (fname);
			last_was_blank = false;
		}

		/* squeeze blank lines from generated code */
		else if (in_gen
			 && regexec (&regex_blank_line, buf, 0, NULL,
				     0) == 0) {
			if (last_was_blank)
				continue;
			else
				last_was_blank = true;
		}

		else {
			/* it's a line of normal, non-empty code. */
			last_was_blank = false;
		}

		fputs (buf, chain->out_file);
		lineno++;
	}
	fflush (chain->out_file);
	if (ferror (chain->out_file))
		lerrsf (_("error writing output file %s"),
			outfilename ? outfilename : "<stdout>");

	return 0;
}

/* vim:set expandtab cindent tabstop=4 softtabstop=4 shiftwidth=4 textwidth=0: */
