/***************************************************************************
                            ParsedArgument.cpp -  description
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
#include <parsedargument.h>
#include <qstring.h>
#include <qcstring.h>

#include <qtjava/QtSupport.h>
#include <kdejava/ParsedArgument.h>

class ParsedArgumentJBridge : public ParsedArgument
{
public:
	ParsedArgumentJBridge() : ParsedArgument() {}
	~ParsedArgumentJBridge() {}
};


JNIEXPORT void JNICALL
Java_org_kde_koala_ParsedArgument_newParsedArgument(JNIEnv *env, jobject obj)
{
	if (QtSupport::getQt(env, obj) == 0) {
		QtSupport::setQt(env, obj, new ParsedArgumentJBridge());
		QtSupport::setObjectForQtKey(env, obj, QtSupport::getQt(env, obj));
	}
	return;
}

JNIEXPORT void JNICALL
Java_org_kde_koala_ParsedArgument_finalize(JNIEnv *env, jobject obj)
{
	if (QtSupport::allocatedInJavaWorld(env, obj)) {
		delete (ParsedArgument*)QtSupport::getQt(env, obj);
		QtSupport::setQt(env, obj, 0);
	}
	return;
}

JNIEXPORT void JNICALL
Java_org_kde_koala_ParsedArgument_setName(JNIEnv *env, jobject obj, jstring aName)
{
static QString * _qstring_aName = 0;
	((ParsedArgumentJBridge*) QtSupport::getQt(env, obj))->setName((QString&) * (QString *) QtSupport::toQString(env, aName, &_qstring_aName));
	return;
}

JNIEXPORT jstring JNICALL
Java_org_kde_koala_ParsedArgument_name(JNIEnv *env, jobject obj)
{
	QString  _qstring;
	_qstring = ((ParsedArgumentJBridge*) QtSupport::getQt(env, obj))->name();
	return QtSupport::fromQString(env, &_qstring);
}

JNIEXPORT void JNICALL
Java_org_kde_koala_ParsedArgument_setType(JNIEnv *env, jobject obj, jstring aType)
{
static QString * _qstring_aType = 0;
	((ParsedArgumentJBridge*) QtSupport::getQt(env, obj))->setType((QString&) * (QString *) QtSupport::toQString(env, aType, &_qstring_aType));
	return;
}

JNIEXPORT jstring JNICALL
Java_org_kde_koala_ParsedArgument_type(JNIEnv *env, jobject obj)
{
	QString  _qstring;
	_qstring = ((ParsedArgumentJBridge*) QtSupport::getQt(env, obj))->type();
	return QtSupport::fromQString(env, &_qstring);
}

JNIEXPORT void JNICALL
Java_org_kde_koala_ParsedArgument_setNamePos(JNIEnv *env, jobject obj, jint pos)
{
	((ParsedArgumentJBridge*) QtSupport::getQt(env, obj))->setNamePos((int) pos);
	return;
}

JNIEXPORT jint JNICALL
Java_org_kde_koala_ParsedArgument_namePos(JNIEnv *env, jobject obj)
{
	return (jint) ((ParsedArgumentJBridge*) QtSupport::getQt(env, obj))->namePos();
}

JNIEXPORT void JNICALL
Java_org_kde_koala_ParsedArgument_copy(JNIEnv *env, jobject obj, jobject anArgument)
{
	((ParsedArgumentJBridge*) QtSupport::getQt(env, obj))->copy((ParsedArgument*) QtSupport::getQt(env, anArgument));
	return;
}

JNIEXPORT jstring JNICALL
Java_org_kde_koala_ParsedArgument_toString(JNIEnv *env, jobject obj)
{
	QString  _qstring;
	_qstring = ((ParsedArgumentJBridge*) QtSupport::getQt(env, obj))->toString();
	return QtSupport::fromQString(env, &_qstring);
}

//JNIEXPORT jstring JNICALL
//Java_org_kde_koala_ParsedArgument_asPersistantString(JNIEnv *env, jobject obj)
//{
//	QString  _qstring;
//	_qstring = ((ParsedArgumentJBridge*) QtSupport::getQt(env, obj))->asPersistantString();
//	return QtSupport::fromQString(env, &_qstring);
//}

JNIEXPORT void JNICALL
Java_org_kde_koala_ParsedArgument_out(JNIEnv *env, jobject obj)
{
	((ParsedArgumentJBridge*) QtSupport::getQt(env, obj))->out();
	return;
}

