AC_PREREQ([2.64])
AC_INIT([libamg], [0.1], [tgrande@pd3.com.br])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

# Checks for programs.
AC_PROG_CC
AM_PROG_AS
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_LIB(pthread, pthread_join,,AC_MSG_ERROR([unable to find the pthread_join() function]))
AC_CHECK_LIB(gtlcsme, TETH_Init,,AC_MSG_ERROR([unable to find the TETH_Init() function]), -lgtlcommon)
AC_CHECK_LIB(gtlcommon, GTL_Open,,AC_MSG_ERROR([unable to find the GTL_Open() function]), -lgtlcsme)
AC_CHECK_LIB(vapi, VAPI_Init,,AC_MSG_ERROR([unable to find the VAPI_Init() function]))

# Checks for header files.
AC_CHECK_HEADERS([stdio.h stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_MMAP

AM_PROG_LIBTOOL

SHAVE_INIT([m4], [enable]) # dnl Make the output pretty

AC_CONFIG_FILES([m4/shave
		 m4/shave-libtool
		 Makefile
		 src/Makefile
		 src/libnetlink/Makefile])

AC_OUTPUT
