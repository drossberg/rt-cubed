
/*           D A T A O U T P U T S T R E A M . C X X
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

/** @file DataOutputStream.cxx
 *
 *  Description -
 *      
 *
 *  Author - David Loman
 *
 */



#include "io/DataOutputStream.h"

#include <iostream>

using ibme::io::DataOutputStream;

DataOutputStream::DataOutputStream(OutputStream& out) : FilterOutputStream(out)
{
	written = 0;
}

DataOutputStream::~DataOutputStream()
{
}

size_t DataOutputStream::size() const throw ()
{
	return written;
}

void DataOutputStream::write(uByte b) throw (IOException)
{
	out.write(b);
	written++;
}

void DataOutputStream::write(const uByte* data, size_t offset, size_t length) throw (IOException)
{
	if (length)
	{
		out.write(data, offset, length);
		written += length;
	}
}

void DataOutputStream::write(const array<uByte>& b) throw (IOException)
{
	write(b.data(), 0, b.size());
}

void DataOutputStream::writeBoolean(bool b) throw (IOException)
{
	out.write(b ? 1 : 0);
	written++;
}

void DataOutputStream::writeByte(Byte b) throw (IOException)
{
  this->writeUByte(static_cast<uByte>(b));
}
void DataOutputStream::writeUByte(uByte b) throw (IOException)
{
	out.write(b);
	written++;
}


void DataOutputStream::writeShort(Short s) throw (IOException)
{
  this->writeUShort(static_cast<uShort>(s));
}
void DataOutputStream::writeUShort(uShort s) throw (IOException)
{
	out.write((s >>  8)       );
	out.write((s      ) & 0xff);
	written += 2;
}

void DataOutputStream::writeInt(Int i) throw (IOException)
{
  this->writeUInt(static_cast<uInt>(i));
}
void DataOutputStream::writeUInt(uInt i) throw (IOException)
{
	out.write((i >> 24)       );
	out.write((i >> 16) & 0xff);
	out.write((i >>  8) & 0xff);
	out.write((i      ) & 0xff);
	written += 4;
}

void DataOutputStream::writeLong(Long l) throw (IOException)
{
  this->writeULong(static_cast<uLong>(l));
}
void DataOutputStream::writeULong(uLong l) throw (IOException)
{
	out.write((l >> 56)       );
	out.write((l >> 48) & 0xff);
	out.write((l >> 40) & 0xff);
	out.write((l >> 32) & 0xff);
	out.write((l >> 24) & 0xff);
	out.write((l >> 16) & 0xff);
	out.write((l >>  8) & 0xff);
	out.write((l      ) & 0xff);
	written += 8;
}


void DataOutputStream::writeChar(uShort v) throw (IOException)
{
  this->writeUShort(v);
}

void DataOutputStream::writeString(const String& str) throw (IOException)
{
	for (size_t i = 0; i < str.length(); i++)
	{
	  this->writeChar(str[i]);
	}
}


