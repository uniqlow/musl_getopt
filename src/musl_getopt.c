#include <unistd.h>
#include <wchar.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

char *musl_optarg;
int musl_optind=1, musl_opterr=1, musl_optopt, musl__optpos, musl_optreset=0;

#define optpos musl__optpos

void __getopt_msg(const char *a, const char *b, const char *c, size_t l)
{
	FILE *f = stderr;
	fputs(a, f)>=0
	&& fwrite(b, strlen(b), 1, f)
	&& fwrite(c, 1, l, f)==l
	&& putc('\n', f);
}

int getopt(int argc, char * const argv[], const char *optstring)
{
	int i;
	wchar_t c, d;
	int k, l;
	char *optchar;

	if (!musl_optind || musl_optreset) {
		musl_optreset = 0;
		musl__optpos = 0;
		musl_optind = 1;
	}

	if (musl_optind >= argc || !argv[musl_optind])
		return -1;

	if (argv[musl_optind][0] != '-') {
		if (optstring[0] == '-') {
			musl_optarg = argv[musl_optind++];
			return 1;
		}
		return -1;
	}

	if (!argv[musl_optind][1])
		return -1;

	if (argv[musl_optind][1] == '-' && !argv[musl_optind][2])
		return musl_optind++, -1;

	if (!optpos) optpos++;
	if ((k = mbtowc(&c, argv[musl_optind]+optpos, MB_LEN_MAX)) < 0) {
		k = 1;
		c = 0xfffd; /* replacement char */
	}
	optchar = argv[musl_optind]+optpos;
	optpos += k;

	if (!argv[musl_optind][optpos]) {
		musl_optind++;
		optpos = 0;
	}

	if (optstring[0] == '-' || optstring[0] == '+')
		optstring++;

	i = 0;
	d = 0;
	do {
		l = mbtowc(&d, optstring+i, MB_LEN_MAX);
		if (l>0) i+=l; else i++;
	} while (l && d != c);

	if (d != c || c == ':') {
		musl_optopt = c;
		if (optstring[0] != ':' && musl_opterr)
			__getopt_msg(argv[0], ": unrecognized option: ", optchar, k);
		return '?';
	}
	if (optstring[i] == ':') {
		musl_optarg = 0;
		if (optstring[i+1] != ':' || optpos) {
			musl_optarg = argv[musl_optind++] + optpos;
			optpos = 0;
		}
		if (musl_optind > argc) {
			musl_optopt = c;
			if (optstring[0] == ':') return ':';
			if (musl_opterr) __getopt_msg(argv[0],
				": option requires an argument: ",
				optchar, k);
			return '?';
		}
	}
	return c;
}

/*weak_alias(getopt, __posix_getopt); */
