#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <err.h>
#include <config.h>
#include "udsc.h"
#include "utils.h"
#include "server.h"
#include "client.h"

static void usage(void)
{
	const char *usage = "\n"
		"-m, --mode\t\tSet mode to SERVER or CLIENT\n"
		"-t, --type\t\tSet socket type to "
		"STREAM, DGRAM, RAW, RDM, SEQPACKET\n"
		"-p, --path\t\tSet socket path\n"
		"-e, --eol\t\tSet end of line type to DOS or UNIX\n"
		"-v, --version\t\tPrint version information and exit\n";
		"-h, --help\t\tShow this help message and exit\n";

	print("Usage: %s [OPTION...]\n", PACKAGE);
	print("%s", usage);
}

int main(int argc, char **argv)
{
	int c, index, error = 0;
	int type = SOCK_STREAM, mode = SERVER_MODE;
	const char *path = {0};
	const char *eol = "\n";
	struct sockinfo sockinfo;
	const struct option options[] = {
		{"type",	required_argument,	0,	't'},
		{"path",	required_argument,	0,	'p'},
		{"mode",	required_argument,	0,	'm'},
		{"eol",		required_argument,	0,	'e'},
		{"version",	no_argument,		0,	'v'},
		{"help",	no_argument,		0,	'h'},
		{0,		0,			0,	0}
	};

	if (argc < 2)
		return 1;

	while (1) {
		c = getopt_long(argc, argv, "t:p:m:e:vh", options, &index);
		if (c == -1)
			break;

		switch (c) {
		case 't':
			type = strt(optarg);
			if (type == -1) {
				warnx("unsupported type: \"%s\"", optarg);
				error = 1;
			}
			break;
		case 'p':
			path = optarg;
			break;
		case 'm':
			mode = strm(optarg);
			if (mode == -1) {
				warnx("unknown mode: \"%s\"", optarg);
				error = 1;
			}
			break;
		case 'e':
			eol = streol(optarg);
			if (!streol) {
				warnx("unknown end of line: \"%s\"", optarg);
				error = 1;
			}
			break;
		case 'v':
			print("%s version %s\n", PACKAGE, VERSION);
			return 0;
		case 'h':
		case '?':
		default:
			usage();
			return 0;
		}

		if (error)
			return 1;
	}

	if (optind < argc) {
		usage();
		return 1;
	}

	sockinfo.path = path;
	sockinfo.type = type;
	sockinfo.eol = eol;

	switch (mode) {
	case SERVER_MODE:
		server(&sockinfo);
		break;
	case CLIENT_MODE:
		client(&sockinfo);
		break;
	default:
		break;
	}

	return 0;
}
