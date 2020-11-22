#include "musl_getopt.h"
#include <stddef.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

extern int musl__optpos, musl_optreset;

static void permute(char *const *argv, int dest, int src)
{
	char **av = (char **)argv;
	char *tmp = av[src];
	int i;
	for (i=src; i>dest; i--)
		av[i] = av[i-1];
	av[dest] = tmp;
}

static int __getopt_long_core(int argc, char *const *argv, const char *optstring, const struct musl_option *longopts, int *idx, int longonly);

static int __getopt_long(int argc, char *const *argv, const char *optstring, const struct musl_option *longopts, int *idx, int longonly)
{
	int ret, skipped, resumed;
	if (!musl_optind || musl_optreset) {
		musl_optreset = 0;
		musl__optpos = 0;
		musl_optind = 1;
	}
	if (musl_optind >= argc || !argv[musl_optind]) return -1;
	skipped = musl_optind;
	if (optstring[0] != '+' && optstring[0] != '-') {
		int i;
		for (i=musl_optind; ; i++) {
			if (i >= argc || !argv[i]) return -1;
			if (argv[i][0] == '-' && argv[i][1]) break;
		}
		musl_optind = i;
	}
	resumed = musl_optind;
	ret = __getopt_long_core(argc, argv, optstring, longopts, idx, longonly);
	if (resumed > skipped) {
		int i, cnt = musl_optind-resumed;
		for (i=0; i<cnt; i++)
			permute(argv, skipped, musl_optind-1);
		musl_optind = skipped + cnt;
	}
	return ret;
}

static int __getopt_long_core(int argc, char *const *argv, const char *optstring, const struct musl_option *longopts, int *idx, int longonly)
{
	musl_optarg = 0;
	if (longopts && argv[musl_optind][0] == '-' &&
		((longonly && argv[musl_optind][1] && argv[musl_optind][1] != '-') ||
		 (argv[musl_optind][1] == '-' && argv[musl_optind][2])))
	{
		int colon = optstring[optstring[0]=='+'||optstring[0]=='-']==':';
		int i, cnt, match;
		char *arg, *opt, *start = argv[musl_optind]+1;
		for (cnt=i=0; longopts[i].name; i++) {
			const char *name = longopts[i].name;
			opt = start;
			if (*opt == '-') opt++;
			while (*opt && *opt != '=' && *opt == *name)
				name++, opt++;
			if (*opt && *opt != '=') continue;
			arg = opt;
			match = i;
			if (!*name) {
				cnt = 1;
				break;
			}
			cnt++;
		}
		if (cnt==1 && longonly && arg-start == mblen(start, MB_LEN_MAX)) {
			int l = arg-start;
			for (i=0; optstring[i]; i++) {
				int j;
				for (j=0; j<l && start[j]==optstring[i+j]; j++);
				if (j==l) {
					cnt++;
					break;
				}
			}
		}
		if (cnt==1) {
			i = match;
			opt = arg;
			musl_optind++;
			if (*opt == '=') {
				if (!longopts[i].has_arg) {
					musl_optopt = longopts[i].val;
					if (colon || !musl_opterr)
						return '?';
					__getopt_msg(argv[0],
						": option does not take an argument: ",
						longopts[i].name,
						strlen(longopts[i].name));
					return '?';
				}
				musl_optarg = opt+1;
			} else if (longopts[i].has_arg == musl_required_argument) {
				if (!(musl_optarg = argv[musl_optind])) {
					musl_optopt = longopts[i].val;
					if (colon) return ':';
					if (!musl_opterr) return '?';
					__getopt_msg(argv[0],
						": option requires an argument: ",
						longopts[i].name,
						strlen(longopts[i].name));
					return '?';
				}
				musl_optind++;
			}
			if (idx) *idx = i;
			if (longopts[i].flag) {
				*longopts[i].flag = longopts[i].val;
				return 0;
			}
			return longopts[i].val;
		}
		if (argv[musl_optind][1] == '-') {
			musl_optopt = 0;
			if (!colon && musl_opterr)
				__getopt_msg(argv[0], cnt ?
					": option is ambiguous: " :
					": unrecognized option: ",
					argv[musl_optind]+2,
					strlen(argv[musl_optind]+2));
			musl_optind++;
			return '?';
		}
	}
	return getopt(argc, argv, optstring);
}

int musl_getopt_long(int argc, char *const *argv, const char *optstring, const struct musl_option *longopts, int *idx)
{
	return __getopt_long(argc, argv, optstring, longopts, idx, 0);
}

int musl_getopt_long_only(int argc, char *const *argv, const char *optstring, const struct musl_option *longopts, int *idx)
{
	return __getopt_long(argc, argv, optstring, longopts, idx, 1);
}
