dnl
dnl Check location of Qt documentation
dnl
AC_DEFUN(KDEV_PATH_QTDOC,
[
AC_MSG_CHECKING([for Qt documentation])

ac_qt_docdirs=""
AC_ARG_WITH(qtdoc_dir, 
[  --with-qtdoc-dir=DIR     where the Qt documentation is installed ],
ac_qt_docdirs=""
qt_docdirs=""
[
  ac_qt_docdirs=$withval
],
)

qt_docdirs="/usr/doc/qt-doc/html /usr/local/qt/html /usr/local/lib/qt/html /usr/lib/qt/doc/html /usr/X11/lib/qt/html /usr/X11/lib/qt/doc/html" 
qt_docdirs="$ac_qt_docdirs $qt_docdirs"
AC_FIND_FILE(classes.html, $qt_docdirs, qt_docdir)
AC_MSG_RESULT($qt_docdir)
if test "$qt_docdir" = NO; then
  qt_docdir=""
fi
AC_DEFINE_UNQUOTED(QT_DOCDIR, "$qt_docdir")
]) 


dnl
dnl Check location of kdelibs documentation
dnl
AC_DEFUN(KDEV_PATH_KDELIBSDOC,
[
AC_MSG_CHECKING([for kdelibs documentation])

ac_kdelibs_docdirs=""
AC_ARG_WITH(kdelibsdoc_dir, 
[  --with-kdelibsdoc-dir=DIR   where the kdelibs documentation is installed ],
ac_kdelibs_docdirs=""
kdelibs_docdirs=""
[
  ac_kdelibs_docdirs=$withval
],
)

kdelibs_docdirs="/usr/doc/kdelibs-doc/html /opt/kde/share/doc/HTML/default/kdelibs"
kdelibs_docdirs="$ac_kdelibs_docdirs $kdelibs_docdirs"
AC_FIND_FILE(kdecore/index.html, $kdelibs_docdirs, kdelibs_docdir)
AC_MSG_RESULT($kdelibs_docdir)
if test "$kdelibs_docdir" = NO; then
  kdelibs_docdir=""
fi
AC_DEFINE_UNQUOTED(KDELIBS_DOCDIR, "$kdelibs_docdir")
]) 


dnl
dnl Check location of kdoc index files
dnl
AC_DEFUN(KDEV_PATH_KDOCINDEX,
[
AC_MSG_CHECKING([for kdoc index])

ac_kdoc_indexdirs=""
AC_ARG_WITH(kdocindex_dir, 
[  --with-kdocindex-dir=DIR    where the kdoc index files are ],
ac_kdoc_indexdirs=""
kdoc_indexdirs=""
[
  ac_kdoc_indexdirs=$withval
],
)

kdoc_indexdirs="/usr/share/kdoc/index"
if test "$kdelibs_docdir" != ""; then 
  kdoc_indexdirs="$kdoc_indexdirs $kdelibs_docdir/kdoc-reference"
fi
kdoc_indexdirs="$ac_qtdocdirs $kdoc_indexdirs"
AC_FIND_FILE(kdecore.kdoc kdecore.kdoc.gz, $kdoc_indexdirs, kdoc_indexdir)
AC_MSG_RESULT($kdoc_indexdir)
if test "$kdoc_indexdir" = NO; then
  kdoc_indexdir=""
fi
AC_DEFINE_UNQUOTED(KDOC_INDEXDIR, "$kdoc_indexdir")
]) 


dnl
dnl Check whether we support Debian docbase
dnl
AC_DEFUN(KDEV_CHECK_DOCBASE,
[
AC_MSG_CHECKING(for docbase)
AC_ARG_ENABLE(docbase,
[  --enable-docbase        enable Debian docbase support],
[if test "$enableval" = yes; then
  enable_docbase=yes
fi],
enable_docbase=no)
AC_MSG_RESULT($enable_docbase)

if test "$enable_docbase" = "yes"; then
  AC_DEFINE_UNQUOTED(WITH_DOCBASE)
fi
])


dnl
dnl Check whether we use kdoc2
dnl
AC_DEFUN(KDEV_CHECK_KDOC2,
[
AC_MSG_CHECKING(for kdoc2)
AC_ARG_ENABLE(kdoc2,
[  --enable-kdoc2        enable kdoc2 support],
[if test "$enableval" = yes; then
  enable_kdoc2=yes
fi],
enable_kdoc2=no)
AC_MSG_RESULT($enable_kdoc2)

if test "$enable_kdoc2" = "yes"; then
  AC_DEFINE_UNQUOTED(WITH_KDOC2)
fi
])
