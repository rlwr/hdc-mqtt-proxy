AC_INIT(hdc-mqtt-proxy, 1.0, remi.lorriaux@windriver.com)
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR([.])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_HEADERS([hdc-mqtt-proxy.h])

AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CC
AC_PROG_GCC_TRADITIONAL
AM_PROG_CC_C_O

dnl Checks for libraries.

dnl Checks for header files.
AC_CHECK_HEADERS([wra.h MQTTAsync.h])

dnl Checks for library functions.
AC_HEADER_STDC
AC_CHECK_FUNCS(strtok_r)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
