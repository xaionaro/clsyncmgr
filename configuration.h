
#ifndef BUFSIZ
#define BUFSIZ			(1<<16)
#endif

/* Backtrace length on error() */
#define BACKTRACE_LENGTH	(1<<8)

#define CONFIG_PATHS 		{ ".clsyncmgr.conf", "/etc/clsync/clsyncmgr.conf",  ".clsync.conf", "/etc/clsync/clsync.conf", NULL }
#define DEFAULT_CONFIG_BLOCK	"default"

#define USER_LEN		(1<<8)
#define GROUP_LEN		USER_LEN

#define ALLOC_PORTION		16

#define SYSLOG_BUFSIZ		BUFSIZ

#define MAXARGUMENTS		(1<<8)
#define MAXCHILDREN		(1<<8)
#define MAXRULES		(1<<8)

#define PROC_RBUF		(1<<24)

#define	MAXLABELLENGTH		(1<<8)

#define CLSYNCSOCK_WINDOW	(1<<8)
