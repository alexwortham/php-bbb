dnl $Id$
dnl config.m4 for extension bbb

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(bbb, for bbb support,
dnl Make sure that the comment is aligned:
dnl [  --with-bbb             Include bbb support])

dnl Otherwise use enable:

 PHP_ARG_ENABLE(bbb, 
	[whether to enable bbb support],
	[ --enable-bbb		Enable bbb support])

if test "$PHP_BBB" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-bbb -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/bbb.h"  # you most likely want to change this
  dnl if test -r $PHP_BBB/$SEARCH_FOR; then # path given as parameter
  dnl   BBB_DIR=$PHP_BBB
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for bbb files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       BBB_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$BBB_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the bbb distribution])
  dnl fi

  dnl # --with-bbb -> add include path
  dnl PHP_ADD_INCLUDE($BBB_DIR/include)

  dnl # --with-bbb -> check for lib and symbol presence
  dnl LIBNAME=bbb # you may want to change this
  dnl LIBSYMBOL=bbb # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $BBB_DIR/$PHP_LIBDIR, BBB_SHARED_LIBADD)
  AC_DEFINE(HAVE_BBBLIB,1,[Whether you have BBB])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong bbb lib version or lib not found])
  dnl ],[
  dnl   -L$BBB_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(BBB_SHARED_LIBADD)

  PHP_NEW_EXTENSION(bbb, bbb.c, $ext_shared)
fi
