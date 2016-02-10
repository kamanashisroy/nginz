# please refer to https://wiki.gnome.org/Projects/Vala/GameDevelopmentSeries/Setup

m4_define([nginz_major], [0])
m4_define([nginz_minor], [0])
m4_define([nginz_micro], [1])

m4_define([nginz_version], [nginz_major.nginz_minor.nginz_micro])
m4_define([nginz_suffix], [-nginz_version])
m4_define([nginz_api_version], [0.0])

m4_define([nginz_name], [nginz])

AC_PREREQ([2.64])
AC_INIT([NginZ], [nginz_version], [kamanashisroy@github.com])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([dist-bzip2 subdir-objects])
AM_MAINTAINER_MODE

NGINZ_MAJOR_VERSION=nginz_major
NGINZ_MINOR_VERSION=nginz_minor
NGINZ_MICRO_VERSION=nginz_micro
NGINZ_VERSION=nginz_version
NGINZ_API_VERSION=nginz_api_version
PACKAGE_SUFFIX=nginz_suffix
AC_SUBST(PACKAGE_SUFFIX)
AC_SUBST(NGINZ_MAJOR_VERSION)
AC_SUBST(NGINZ_MINOR_VERSION)
AC_SUBST(NGINZ_MICRO_VERSION)
AC_SUBST(NGINZ_VERSION)
AC_SUBST(NGINZ_API_VERSION)

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])


# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])


# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_RANLIB


AROOP_REQUIRED=0.2
PKG_CHECK_MODULES(AROOP, aroop_core-0.2.0 >= $AROOP_REQUIRED,
	[AROOP_DEBUG_LIBS=`$PKG_CONFIG --define-variable=variant=debug --static --libs aroop_core-0.2.0`],
	[AROOP_DEBUG_LIBS=;])
AC_SUBST(AROOP_CFLAGS)
AC_SUBST(AROOP_LIBS)
AC_SUBST(AROOP_DEBUG_LIBS)

AC_CONFIG_FILES([Makefile
           libnginz.pc
           ])

AC_OUTPUT
