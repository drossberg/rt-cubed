/*                 S O U R C E S V N L I N K . H
 * BRL-CAD
 *
 * Copyright (c) 1997-2008 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */

/** @file SourceSvnLink.h
 *
 *  Description -
 *      
 *
 *  Author - David Loman
 *
 */

#if !defined(_SOURCESVNLINK_H_)
#define _SOURCESVNLINK_H_

#include "iBME/iBMECommon.h"
#include "GeometryEngine/AbstractDbObjectSource.h"
#include "GeometryEngine/DbObject.h"

class SourceSvnLink : public AbstractDbObjectSource
{

public:
	SourceSvnLink();
	virtual ~SourceSvnLink();

	virtual DbObject& getDbObjectByURL(URL& url);
	bool putDbObject(DbObject& dbobj);

};

#endif // !defined(_SOURCESVNLINK_H_)

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
