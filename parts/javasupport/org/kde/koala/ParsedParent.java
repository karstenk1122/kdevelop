/***************************************************************************
                            ParsedParent.java -  description
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

package org.kde.koala;

import org.kde.qt.*;
import java.util.*;
import java.lang.Error;

/**
 Represents the parent to a class. 
 @short  Represents the parent to a class.
*/
public class  ParsedParent implements QtSupport {
	private long _qt;
	private boolean _allocatedInJavaWorld = true;

	protected ParsedParent(Class dummy){}

	private native void newParsedParent();
	public ParsedParent() {
		newParsedParent();
	}
	protected native void finalize() throws InternalError;

/**  Sets parent the parent name 
*/
	public native void setName( String aName);

/**  Sets parent the parent name 
*/
	public native String name();

/**  Sets the export status 
*/
	public native void setAccess( int aAccess);

/**  Gets the export status 
*/
	public native int access();
	public native boolean isPublic();
	public native boolean isProtected();
	public native boolean isPrivate();

/**  Outputs the class as text on stdout 
*/
	public native void out();
}
