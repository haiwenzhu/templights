dnl $Id$
dnl config.m4 for extension templights

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(templights, for templights support,
dnl Make sure that the comment is aligned:
dnl [  --with-templights             Include templights support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(templights, whether to enable templights support,
dnl Make sure that the comment is aligned:
[  --enable-templights           Enable templights support])

if test "$PHP_TEMPLIGHTS" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-templights -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/templights.h"  # you most likely want to change this
  dnl if test -r $PHP_TEMPLIGHTS/$SEARCH_FOR; then # path given as parameter
  dnl   TEMPLIGHTS_DIR=$PHP_TEMPLIGHTS
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for templights files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       TEMPLIGHTS_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$TEMPLIGHTS_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the templights distribution])
  dnl fi

  dnl # --with-templights -> add include path
  dnl PHP_ADD_INCLUDE($TEMPLIGHTS_DIR/include)

  dnl # --with-templights -> check for lib and symbol presence
  dnl LIBNAME=templights # you may want to change this
  dnl LIBSYMBOL=templights # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $TEMPLIGHTS_DIR/lib, TEMPLIGHTS_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_TEMPLIGHTSLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong templights lib version or lib not found])
  dnl ],[
  dnl   -L$TEMPLIGHTS_DIR/lib -lm
  dnl ])
  dnl
  PHP_SUBST(TEMPLIGHTS_SHARED_LIBADD)

  PHP_NEW_EXTENSION(templights, templights.c, $ext_shared)
fi
