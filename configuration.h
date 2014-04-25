
#ifndef BUFSIZ
#define BUFSIZ	(1<<16)
#endif

/* Backtrace length on error() */
#define BACKTRACE_LENGTH (1<<8)

#define CONFIG_PATHS 	{ ".clsyncmgr.conf", "/etc/clsync/clsyncmgr.conf",  ".clsync.conf", "/etc/clsync/clsync.conf", NULL }
#define DEFAULT_CONFIG_BLOCK "default"

#define USER_LEN	(1<<8)
#define GROUP_LEN	USER_LEN

