/*                      I B M E C O M M O N . H
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

/** @file iBMECommon.h
 *
 *  Description -
 *      
 *
 *  Author - David Loman
 *
 */

#if !defined(_IBMECOMMON_H_)
#define _IBMECOMMON_H_

#include <iostream>

  // These are only temporary
  typedef std::string     UUID;
  typedef std::string     URL;
  typedef std::string     String;



  // These are Perminent
  typedef uint8_t    uChar;
  typedef int8_t     Char;

  typedef uint8_t    uByte;
  typedef int8_t     Byte;

  typedef uint16_t   uShort;
  typedef int16_t    Short;

  typedef uint32_t   uInt;
  typedef int32_t    Int;

  typedef uint64_t   uLong;
  typedef int64_t    Long;

#endif // !defined(_IBMECOMMON_H_)

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
