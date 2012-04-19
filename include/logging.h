#ifndef __LOGGING_H__
#  define __LOGGING_H__

//# define DEBUG

# include <errno.h>


# ifdef __USE_STDIO__
#  include <stdio.h>

#  define start_logging(ident) ((void)0)
#  define stop_logging() ((void)0)

    
#  ifndef DEBUG
#   define l_err(fmt,...)  fprintf(stderr, "+ERR:"fmt"\n", ##__VA_ARGS__)
#   define l_perr(str)     fprintf(stderr, "+ERR:%s: %s\n", str, strerror(errno))
#   define l_warn(fmt,...) fprintf(stderr, "+WARN:"fmt"\n", ##__VA_ARGS__)
#   define l_info(fmt,...) fprintf(stdout, "+INFO:"fmt"\n", ##__VA_ARGS__)
#  else /* DEBUG */
#   define l_err(fmt,...)  fprintf(stderr, "+ERR:"__FILE__":%s:%d:"fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#   define l_perr(str)     fprintf(stderr, "+ERR:"__FILE__":%s:%d: %s: %s\n",__FUNCTION__, __LINE__, str, strerror(errno))
#   define l_warn(fmt,...) fprintf(stderr, "+WARN:"__FILE__":%s:%d:"fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#   define l_info(fmt,...) fprintf(stdout, "+INFO:" __FILE__":%s:%d:"fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#  endif /* DEBUG */
    
#  ifdef DEBUG
#   define l_debug(fmt,...) fprintf(stdout, "+DEBUG:", __FILE__":%s:%d:"fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#  else 
#   define l_debug(fmt,...) ((void)0)
#  endif
    
# else /* __USE_STDIO__ */

/* default using syslog */

#  include <syslog.h>
    
#  define start_logging(ident) openlog(ident, LOG_PID, LOG_DAEMON)
#  define stop_logging() closelog()
    
#  ifndef DEBUG
#   define l_err(fmt,...) syslog(LOG_ERR, fmt, ##__VA_ARGS__)
#   define l_perr(str) syslog(LOG_ERR, "%s: %s", str, strerror(errno))
#   define l_warn(fmt,...) syslog(LOG_WARNING, fmt, ##__VA_ARGS__)
#   define l_info(fmt,...) syslog(LOG_INFO, fmt, ##__VA_ARGS__)
#  else /* DEBUG */
#   define l_err(fmt,...) syslog(LOG_ERR, __FILE__":%s:%d:"fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#   define l_perr(str) syslog(LOG_ERR, __FILE__":%s:%d: %s: %s",__FUNCTION__, __LINE__, str, strerror(errno))
#   define l_warn(fmt,...) syslog(LOG_WARNING, __FILE__":%s:%d:"fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#   define l_info(fmt,...) syslog(LOG_INFO, __FILE__":%s:%d:"fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#  endif /* DEBUG */
    
#  ifdef DEBUG
#   define l_debug(fmt,...) syslog(LOG_DEBUG, __FILE__":%s:%d:"fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#  else 
#   define l_debug(fmt,...) ((void)0)
#  endif

# endif /* __USE_STDIO__ */

#endif /*__LOGGING_H__*/
