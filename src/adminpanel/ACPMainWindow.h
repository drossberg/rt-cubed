/*             A C P M A I N W I N D O W . H
 * BRL-CAD
 *
 * Copyright (c) 2009 United States Government as represented by
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
/** @file ACPMainWindow.h
 *
 * Main window widget.
 *
 */

#ifndef __ACPMAINWINDOW_H__
#define __ACPMAINWINDOW_H__

#include <QWidget>

class ACPMainWindow :  public QWidget
{
public:
	ACPMainWindow();
	virtual ~ACPMainWindow();
};

#endif /* __ACPMAINWINDOW_H__ */
