/*
    clsyncmgr - intermediate daemon to aggregate clsync's sockets

    Copyright (C) 2014  Dmitry Yu Okunev <dyokunev@ut.mephi.ru> 0x8E30679C

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This file is only for initial processes [parse arguments, config and run 
 * clsyncmgr(ctx_p) from clsyncmgr.c]
 */

#include "common.h"

#include <getopt.h>	/* getopt_long()	  */
#include <stdlib.h>	/* getsubopt()		  */
#include <errno.h>	/* errno		  */
#include <stdio.h>	/* sscanf()		  */
#include <string.h>	/* strchr()		  */
#include <sys/types.h>	/* getpwnam()		  */
#include <pwd.h>	/* getpwnam()		  */
#include <grp.h>	/* getgrnam()		  */
#include <glib.h>	/* g_key_file_get_value() */

#include "malloc.h"
#include "error.h"
#include "clsyncmgr.h"

#include <libclsync.h>

enum paramsource_enum {
	PS_UNKNOWN       = 0,
	PS_ARGUMENT,
	PS_CONFIG
};
typedef enum paramsource_enum paramsource_t;

static const struct option long_options[] =
{
	{"help",		optional_argument,	NULL,	FL_HELP},
	{"version",		optional_argument,	NULL,	FL_SHOW_VERSION},

/*	{"socket",		required_argument,	NULL,   FL_SOCKETPATH},
	{"socket-auth",		required_argument,	NULL,	FL_SOCKETAUTH},
	{"socket-mod",		required_argument,	NULL,	FL_SOCKETMOD},
	{"socket-own",		required_argument,	NULL,	FL_SOCKETOWN},*/

	{"execute",		required_argument,	NULL,	FL_EXECUTE},

	{"background",		optional_argument,	NULL,	FL_BACKGROUND},
	{"config-file",		required_argument,	NULL,	FL_CONFIGFILE},
	{"config-block",	required_argument,	NULL,	FL_CONFIGBLOCK},
	{"pid-file",		required_argument,	NULL,	FL_PIDFILE},
	{"uid",			required_argument,	NULL,	FL_UID},
	{"gid",			required_argument,	NULL,	FL_GID},

	{"syslog",		optional_argument,	NULL,	FL_SYSLOG},
	{"debug",		optional_argument,	NULL,	FL_DEBUG},
	{"quiet",		optional_argument,	NULL,	FL_QUIET},
	{"connect",		required_argument,	NULL,	FL_CONNECT},
	{"watch-dirs",		required_argument,	NULL,	FL_WATCHDIR},

	{NULL,			0,			NULL,	0}
};

static char *const socketauth[] = {
	[SOCKAUTH_UNSET]	= "",
	[SOCKAUTH_NULL]		= "null",
	NULL
};

#define syntax() _syntax(ctx_p)
void _syntax(clsyncmgr_t *ctx_p) {
	info_short("possible options:");

	int i=0;
	while (long_options[i].name != NULL) {
		if (!(long_options[i].val & FLM_CONFIGONLY))
			info_short("\t--%-24s%c%c%s", long_options[i].name, 
				long_options[i].val & FLM_LONGOPTONLY ? ' ' : '-', 
				long_options[i].val & FLM_LONGOPTONLY ? ' ' : long_options[i].val, 
				(long_options[i].has_arg == required_argument ? " argument" : ""));
		i++;
	}
	exit(errno);

	return;
}

#define version() _version(ctx_p)
int _version(clsyncmgr_t *ctx_p) {
	info(PROGRAM" v%i.%i\n\t"AUTHOR"", VERSION_MAJ, VERSION_MIN);
	exit(0);
}

int parse_parameter(clsyncmgr_t *ctx_p, uint16_t param_id, char *arg, paramsource_t paramsource) {
	switch (paramsource) {
		case PS_ARGUMENT:
			if (param_id & FLM_CONFIGONLY) {
				errno = EINVAL;
				error("Parameter \"%p\" is valid only in configuration file", param_id);
				syntax();
				return 0;
			}
			ctx_p->flags_set[param_id] = 1;
			break;
		case PS_CONFIG:
			if (ctx_p->flags_set[param_id])
				return 0;
			break;
		default:
			warning("Unknown parameter #%i source (value \"%s\").", param_id, arg!=NULL ? arg : "");
			break;
	}

	switch (param_id) {
		case '?':
		case FL_HELP:
			syntax();
			break;
		case FL_CONFIGFILE:
			ctx_p->config_path  = arg;
			break;
		case FL_CONFIGBLOCK:
			ctx_p->config_block = arg;
			break;
		case FL_GID:
			ctx_p->gid = (unsigned int)atol(arg);
			ctx_p->flags[param_id]++;
			break;
		case FL_UID:
			ctx_p->uid = (unsigned int)atol(arg);
			ctx_p->flags[param_id]++;
			break;
		case FL_PIDFILE:
			ctx_p->pidfile		= arg;
			break;
		case FL_SHOW_VERSION:
			version();
			break;
		case FL_SOCKETPATH:
			ctx_p->socketpath	= arg;
			break;
		case FL_SOCKETAUTH: {
			char *value;

			ctx_p->flags[FL_SOCKETAUTH] = getsubopt(&arg, socketauth, &value);
			if (ctx_p->flags[FL_SOCKETAUTH] == -1)
				errno = EINVAL,
				critical("Wrong socket auth mech entered: \"%s\"", arg);

			break;
		}
		case FL_SOCKETMOD: {
			if (!sscanf(arg, "%o", &ctx_p->socketmod))
				errno = EINVAL,
				critical("Non octal value passed to --socket-mod: \"%s\"", arg);

			ctx_p->flags[param_id]++;
			break;
		}
		case FL_SOCKETOWN: {
			char *colon = strchr(arg, ':');
			uid_t uid;
			gid_t gid;

			if (colon == NULL) {
				struct passwd *pwent = getpwnam(arg);

				if(pwent == NULL) {
					errno = EINVAL;
					critical("Cannot find username \"%s\" (case #0)", 
						arg);
				}

				uid = pwent->pw_uid;
				gid = pwent->pw_gid;

			} else {

				char user[USER_LEN+2], group[GROUP_LEN+2];

				memcpy(user, arg, MIN(USER_LEN, colon-arg));
				user[colon-arg] = 0;

				strncpy(group, &colon[1], GROUP_LEN);

				errno=0;
				struct passwd *pwent = getpwnam(user);
				if (pwent == NULL) {
					errno = EINVAL;
					critical("Cannot find username \"%s\" (case #1)",
						user);
				}

				errno=0;
				struct group  *grent = getgrnam(group);
				if(grent == NULL) {
					errno = EINVAL;
					critical("Cannot find group \"%s\"", group);
				}

				uid = pwent->pw_uid;
				gid = grent->gr_gid;
			}

			ctx_p->socketuid = uid;
			ctx_p->socketgid = gid;
			ctx_p->flags[param_id]++;

			break;
		}
		case FL_EXECUTE:
			ctx_p->execute		= arg;
			ctx_p->flags[param_id]++;

			break;
		case FL_CONNECT:
		case FL_WATCHDIR:
		{
			char *ptr = arg, *start = arg;
			typeof(clsyncmgr_watchdir_add)        *funct_add	= NULL;
			typeof(clsyncmgr_watchdir_remove_all) *funct_remove_all = NULL;
			switch (param_id) {
				case FL_CONNECT:
					funct_add        = clsyncmgr_watchdir_add;
					funct_remove_all = clsyncmgr_watchdir_remove_all;
					break;
				case FL_WATCHDIR:
					funct_add        = clsyncmgr_socketpath_add;
					funct_remove_all = clsyncmgr_socketpath_remove_all;
					break;
				default:
					break;
			}
			do {
				switch(*ptr) {
					case 0:
					case ':': {
						if (ptr == start) {
							start = ptr+1;
							funct_remove_all(ctx_p);
							continue;
						} 

						char *path = start;
						path[ptr-start] = 0;
						funct_add(ctx_p, path);
						start = ptr+1;
						break;
					}
				}
			} while(*(ptr++));
			break;
		}

		default:
			if(arg == NULL)
				ctx_p->flags[param_id]++;
			else
				ctx_p->flags[param_id] = atoi(arg);
			break;
	}
	return 0;
}

int parse_arguments(int argc, char *argv[], clsyncmgr_t *ctx_p) {
	int c;
	int option_index = 0;

	// Generating "optstring" (man 3 getopt_long) with using information from struct array "long_options"
	char *optstring     = alloca((('z'-'a'+1)*3 + '9'-'0'+1)*3 + 1);
	char *optstring_ptr = optstring;

	const struct option *lo_ptr = long_options;
	while (lo_ptr->name != NULL) {
		if (!(lo_ptr->val & (FLM_CONFIGONLY|FLM_LONGOPTONLY))) {
			*(optstring_ptr++) = lo_ptr->val & 0xff;

			if (lo_ptr->has_arg == required_argument)
				*(optstring_ptr++) = ':';

			if (lo_ptr->has_arg == optional_argument) {
				*(optstring_ptr++) = ':';
				*(optstring_ptr++) = ':';
			}
		}
		lo_ptr++;
	}
	*optstring_ptr = 0;

	// Parsing arguments
	while (1) {
		c = getopt_long(argc, argv, optstring, long_options, &option_index);
	
		if (c == -1) break;
		int ret = parse_parameter(ctx_p, c, optarg, PS_ARGUMENT);
		if(ret) return ret;
	}
	if (optind+1 < argc) {
		errno = EINVAL;
		error("Not enough arguments");
		syntax();
	}

	return 0;
}

char *configs_parse_str[1<<10] = {0};

void gkf_parse(clsyncmgr_t *ctx_p, GKeyFile *gkf) {
	const struct option *lo_ptr = long_options;
	while (lo_ptr->name != NULL) {
		gchar *value = g_key_file_get_value(gkf, ctx_p->config_block, lo_ptr->name, NULL);
		if (value != NULL) {
			unsigned char val_char = lo_ptr->val&0xff;

			if (configs_parse_str[val_char])
				free(configs_parse_str[val_char]);

			configs_parse_str[val_char] = value;
			int ret = parse_parameter(ctx_p, lo_ptr->val, value, PS_CONFIG);
			if (ret) exit(ret);
		}
		lo_ptr++;
	}

	return;
}

int parse_config(clsyncmgr_t *ctx_p) {
	GKeyFile *gkf;

	gkf = g_key_file_new();

	if (ctx_p->config_path) {
		debug(1, "Trying config-file \"%s\"", ctx_p->config_path);
		if (!g_key_file_load_from_file(gkf, ctx_p->config_path, G_KEY_FILE_NONE, NULL)) {
			error("Cannot open/parse file \"%s\"", ctx_p->config_path);
			g_key_file_free(gkf);
			return -1;
		} else
			gkf_parse(ctx_p, gkf);

	} else {
		char *config_paths[] = CONFIG_PATHS;
		char **config_path_p = config_paths, *config_path_real = xmalloc(PATH_MAX);
		size_t config_path_real_size=PATH_MAX;

		char *homedir = getenv("HOME");
		size_t homedir_len = strlen(homedir);

		while (*config_path_p != NULL) {
			size_t config_path_len = strlen(*config_path_p);

			if (config_path_len+homedir_len+3 > config_path_real_size) {
				config_path_real_size = config_path_len+homedir_len+3;
				config_path_real      = xmalloc(config_path_real_size);
			}

			if (*config_path_p[0] != '/') {
				memcpy(config_path_real, homedir, homedir_len);
				config_path_real[homedir_len] = '/';
				memcpy(&config_path_real[homedir_len+1], *config_path_p, config_path_len+1);
			} else 
				memcpy(config_path_real, *config_path_p, config_path_len+1);

			debug(1, "Trying config-file \"%s\"", config_path_real);
			if (!g_key_file_load_from_file(gkf, config_path_real, G_KEY_FILE_NONE, NULL)) {
				debug(1, "Cannot open/parse file \"%s\"", config_path_real);
				config_path_p++;
				continue;
			}

			gkf_parse(ctx_p, gkf);

			break;
		}
		free(config_path_real);
	}

	g_key_file_free(gkf);

	return 0;
}

int ctx_check(clsyncmgr_t *ctx_p)
{
	errno = 0;

/*	if (ctx_p->socketpath == NULL) {
		errno = EINVAL;
		error("--socket is not set");
	}*/

	if (dynamic_count(&ctx_p->watchdirs) == 0) {
		errno = EINVAL;
		error("--watch-dirs and --connect are not set");
	}

	return errno;
}

int main(int argc, char *argv[])
{
	static clsyncmgr_t ctx={{0}};
	clsyncmgr_t *ctx_p = &ctx;
	int ret;

	ctx_p->config_block = DEFAULT_CONFIG_BLOCK;


	ret = parse_arguments(argc, argv, ctx_p);
	if (ret)
		error("Cannot parse arguments");

	ret = parse_config(ctx_p);
	if (ret)
		error("Cannot parse config file");

	ret = ctx_check(ctx_p);
	if (ret)
		error("Got error while checking parameters values");

	if (!ret)
		ret = clsyncmgr(ctx_p);

	return ret;
}

