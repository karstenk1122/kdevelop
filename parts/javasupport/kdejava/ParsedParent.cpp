/***************************************************************************
                            ParsedParent.cpp -  description
                             -------------------
    begin                : Tue Jul  3 08:02:55 2001
    copyright            : (C) 2000-2001 Lost Highway Ltd.
    email                : Richard_Dale@tipitina.demon.co.uk
    generated by         : duke@tipitina on Tue Jul  3 08:02:55 2001, using kdoc parser/custom Java conversion backend.
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
***************************************************************************/

#define _BOOL_H_
#include <parsedparent.h>
#include <qstring.h>
#include <qcstring.h>

#include <qtjava/QtSupport.h>
#include <kdejava/ParsedParent.h>

class ParsedParentJBridge : public ParsedParent
{
public:
	ParsedParentJBridge() : ParsedParent() {}
	~ParsedParentJBridge() {}
};


JNIEXPORT void JNICALL
Java_org_kde_koala_ParsedParent_newParsedParent(JNIEnv *env, jobject obj)
{
	if (QtSupport::getQt(env, obj) == 0) {
		QtSupport::setQt(env, obj, new ParsedParentJBridge());
		QtSupport::setObjectForQtKey(env, obj, QtSupport::getQt(env, obj));
	}
	return;
}

JNIEXPORT void JNICALL
Java_org_kde_koala_ParsedParent_finalize(JNIEnv *env, jobject obj)
{
	if (QtSupport::allocatedInJavaWorld(env, obj)) {
		delete (ParsedParent*)QtSupport::getQt(env, obj);
		QtSupport::setQt(env, obj, 0);
	}
	return;
}

JNIEXPORT void JNICALL
Java_org_kde_koala_ParsedParent_setName(JNIEnv *env, jobject obj, jstring aName)
{
static QString * _qstring_aName = 0;
	((ParsedParentJBridge*) QtSupport::getQt(env, obj))->setName((QString&) * (QString *) QtSupport::toQString(env, aName, &_qstring_aName));
	return;
}

JNIEXPORT jstring JNICALL
Java_org_kde_koala_ParsedParent_name(JNIEnv *env, jobject obj)
{
	QString  _qstring;
	_qstring = ((ParsedParentJBridge*) QtSupport::getQt(env, obj))->name();
	return QtSupport::fromQString(env, &_qstring);
}

JNIEXPORT void JNICALL
Java_org_kde_koala_ParsedParent_setAccess(JNIEnv *env, jobject obj, jint aAccess)
{
	((ParsedParentJBridge*) QtSupport::getQt(env, obj))->setAccess((PIAccess) aAccess);
	return;
}

JNIEXPORT jint JNICALL
Java_org_kde_koala_ParsedParent_access(JNIEnv *env, jobject obj)
{
	return (jint) ((ParsedParentJBridge*) QtSupport::getQt(env, obj))->access();
}

JNIEXPORT jboolean JNICALL
Java_org_kde_koala_ParsedParent_isPublic(JNIEnv *env, jobject obj)
{
	return (jboolean) ((ParsedParentJBridge*) QtSupport::getQt(env, obj))->isPublic();
}

JNIEXPORT jboolean JNICALL
Java_org_kde_koala_ParsedParent_isProtected(JNIEnv *env, jobject obj)
{
	return (jboolean) ((ParsedParentJBridge*) QtSupport::getQt(env, obj))->isProtected();
}

JNIEXPORT jboolean JNICALL
Java_org_kde_koala_ParsedParent_isPrivate(JNIEnv *env, jobject obj)
{
	return (jboolean) ((ParsedParentJBridge*) QtSupport::getQt(env, obj))->isPrivate();
}

JNIEXPORT void JNICALL
Java_org_kde_koala_ParsedParent_out(JNIEnv *env, jobject obj)
{
	((ParsedParentJBridge*) QtSupport::getQt(env, obj))->out();
	return;
}

