dnl SPDX-License-Identifer: GPL-2.0-or-later
dnl
dnl Copyright (C) 2014  Vyacheslav Trushkin
dnl Copyright (C) 2020  Boian Bonev
dnl
dnl This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
dnl

dnl Process this file with autoconf to produce a configure script.

dnl The minimum version of autoconf required.
AC_PREREQ(2.61)

dnl Initialise automake with the package name, version and
dnl bug-reporting address.
AC_INIT([libiotop], [1.15], [perry.werneck@gmail.com])

dnl Place auxilliary scripts here.
AC_CONFIG_AUX_DIR([scripts])

dnl Compute the canonical host-system type
AC_CANONICAL_HOST

dnl Put macro definitions here (though they aren't used).
AC_CONFIG_HEADER([src/include/config.h])

dnl Initialise automake stuff.
AM_INIT_AUTOMAKE

dnl Check for iconv
AM_ICONV

dnl Checks for programs.
AC_PROG_CC

AC_LANG([C])

PKG_CHECK_EXISTS

app_vrs_major=$(echo $VERSION | cut -d. -f1)
app_vrs_minor=$(echo $VERSION | cut -d. -f2)

AC_DEFINE_UNQUOTED(PACKAGE_MAJOR_VERSION, $app_vrs_major)
AC_DEFINE_UNQUOTED(PACKAGE_MINOR_VERSION, $app_vrs_minor)

AC_SUBST(PACKAGE_MAJOR_VERSION,$app_vrs_major)
AC_SUBST(PACKAGE_MINOR_VERSION,$app_vrs_minor)

dnl ---------------------------------------------------------------------------
dnl Check for ELF symbol visibility support
dnl ---------------------------------------------------------------------------
m4_define([GLIB_CHECK_COMPILE_WARNINGS],
[m4_ifvaln([$1], [AC_LANG_CONFTEST([$1])])dnl
rm -f conftest.$ac_objext
glib_ac_compile_save="$ac_compile"
ac_compile='$CC -c $CFLAGS $CPPFLAGS conftest.$ac_ext'
AS_IF([_AC_EVAL_STDERR($ac_compile) &&
         AC_TRY_COMMAND([(if test -s conftest.err; then false ; else true; fi)])],
      [$2],
      [echo "$as_me: failed program was:" >&AS_MESSAGE_LOG_FD
cat conftest.$ac_ext >&AS_MESSAGE_LOG_FD
m4_ifvaln([$3],[$3])dnl])
ac_compile="$glib_ac_compile_save"
rm -f conftest.$ac_objext conftest.err m4_ifval([$1], [conftest.$ac_ext])[]dnl
])# GLIB_CHECK_COMPILE_WARNINGS

AC_MSG_CHECKING(for GNUC visibility attribute)
GLIB_CHECK_COMPILE_WARNINGS([AC_LANG_SOURCE([[
void
__attribute__ ((visibility ("hidden")))
     f_hidden (void)
{
}
void
__attribute__ ((visibility ("internal")))
     f_internal (void)
{
}
void
__attribute__ ((visibility ("protected")))
     f_protected (void)
{
}
void
__attribute__ ((visibility ("default")))
     f_default (void)
{
}
int main (int argc, char **argv)
{
        f_hidden();
        f_internal();
        f_protected();
        f_default();
        return 0;
}
]])],g_have_gnuc_visibility=yes,g_have_gnuc_visibility=no)
AC_MSG_RESULT($g_have_gnuc_visibility)
if test x$g_have_gnuc_visibility = xyes; then
  AC_DEFINE(HAVE_GNUC_VISIBILITY, 1, [supports GCC visibility attributes])
fi

dnl ---------------------------------------------------------------------------
dnl Check for compiler visibility flag
dnl ---------------------------------------------------------------------------
AC_MSG_CHECKING(whether $CC accepts -fvisibility=hidden)
AC_CACHE_VAL(app_cv_fvisibility_ok,
[ac_save_cc="$CC"
CC="$CC -fvisibility=hidden"
AC_TRY_RUN([int main() { return 0; }],
  app_cv_fvisibility_ok=yes,
  app_cv_fvisibility_ok=no,
  app_cv_fvisibility_ok=no)
CC="$ac_save_cc"])
AC_MSG_RESULT($app_cv_fvisibility_ok)
if test $app_cv_fvisibility_ok = yes; then
  CFLAGS="$CFLAGS -fvisibility=hidden"
  CXXFLAGS="$CXXFLAGS -fvisibility=hidden"
fi

dnl ---------------------------------------------------------------------------
dnl Check for pic
dnl ---------------------------------------------------------------------------
AC_ARG_ENABLE([pic],
	[AS_HELP_STRING([--disable-pic], [disable support for PIC libraries (required to compile on 64bits)])],
[
	app_cv_pic="$enableval"
],[

	case "$host_cpu" in
	x86_64)
		app_cv_pic="yes"
		;;

	aarch64)
		app_cv_pic="yes"
		;;

	*)
		app_cv_pic="no"
	esac

])

if test "$app_cv_pic" == "yes"; then
	CFLAGS="$CFLAGS -fPIC"
fi

dnl ---------------------------------------------------------------------------
dnl Check for headers
dnl ---------------------------------------------------------------------------

AC_CHECK_HEADER(malloc.h, AC_DEFINE(HAVE_MALLOC_H,,[do we have malloc.h?]))

dnl ---------------------------------------------------------------------------
dnl Directory config
dnl ---------------------------------------------------------------------------

AC_SUBST(BASEDIR,$ac_pwd)

dnl ---------------------------------------------------------------------------
dnl Configure which files to generate.
dnl ---------------------------------------------------------------------------

AC_CONFIG_FILES(src/libiotop/Makefile)
dnl AC_CONFIG_FILES(src/libiotop/libiotop.pc)

dnl ---------------------------------------------------------------------------
dnl Output the generated config.status script.
dnl ---------------------------------------------------------------------------
AC_OUTPUT


