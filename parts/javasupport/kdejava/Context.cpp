/***************************************************************************
                            Context.cpp -  description
                             -------------------
    begin                : Mon May 28 10:58:39 2001
    copyright            : (C) 2000-2001 Lost Highway Ltd.
    email                : Richard_Dale@tipitina.demon.co.uk
    generated by         : duke@tipitina on Mon May 28 10:58:39 2001, using kdoc parser/custom Java conversion backend.
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
#include <kdevcore.h>
#include <qstring.h>
#include <qcstring.h>

#include <qtjava/QtSupport.h>
#include <kdejava/Context.h>

class ContextJBridge : public Context
{
public:
	ContextJBridge(const QCString& type) : Context(type) {}
	~ContextJBridge() {}
};


JNIEXPORT void JNICALL
Java_org_kde_koala_Context_newContext(JNIEnv *env, jobject obj, jstring type)
{
static QCString * _qcstring_type = 0;
	if (QtSupport::getQt(env, obj) == 0) {
		QtSupport::setQt(env, obj, new ContextJBridge((QCString&) * (QCString *) QtSupport::toQCString(env, type, &_qcstring_type)));
		QtSupport::setObjectForQtKey(env, obj, QtSupport::getQt(env, obj));
	}
	return;
}

JNIEXPORT void JNICALL
Java_org_kde_koala_Context_finalize(JNIEnv *env, jobject obj)
{
	if (QtSupport::allocatedInJavaWorld(env, obj)) {
		delete (Context*)QtSupport::getQt(env, obj);
		QtSupport::setQt(env, obj, 0);
	}
	return;
}

JNIEXPORT jboolean JNICALL
Java_org_kde_koala_Context_hasType(JNIEnv *env, jobject obj, jstring type)
{
static QCString * _qcstring_type = 0;
	return (jboolean) ((ContextJBridge*) QtSupport::getQt(env, obj))->hasType((QCString&) * (QCString *) QtSupport::toQCString(env, type, &_qcstring_type));
}

