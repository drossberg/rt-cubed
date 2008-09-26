/*            C O M M A N D . H
 * BRL-CAD
 *
 * Copyright (c) 2008 United States Government as represented by
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

/** @file Command.h
 *
 * @author Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *
 * @brief
 *	Command base class.
 */

#ifndef __G3D_COMMAND_H__
#define __G3D_COMMAND_H__


#include <string>
#include <vector>


/**
 * @brief Output for a command
 *
 * @author Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 */
class CommandOutput
{
public:
  /** Append given string to the output of a command */
  void appendLine(const std::string& line);
  /** Get the output of a command */
  const std::string& getOutput() const;

private:
  /** The complete output */
  std::string _output;
};


/**
 * @brief Command abstract class
 *
 * @author Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *
 * A base class of a command, derived classes need to define the
 * execute() method, and also create a constructor to call the
 * constructor of the base class with the appropriate parameters
 * (constructors themselves cannot be virtual).
 */
class Command
{
public:
  /** Constructor with some basics needed when creating any
   * command. */
  Command(const std::string& name,
	  const std::string& shortDescr,
	  const std::string& extraDescr);
  /** Default destructor */
  virtual ~Command() { }

  /** Execute the command (to be implemented by the real commands) */
  virtual void execute(std::vector<std::string>& args, CommandOutput& output) = 0;

  /** The name of the command */
  const std::string& getName() const;
  /** Get a one-line description of the command */
  const std::string& getShortDescription() const;
  /** Get a extra description of the command */
  const std::string& getExtraDescription() const;
  /** Get the name of the arguments */
  const std::vector<std::string>& getArgNames() const;
  /** Get the syntax */
  std::string getSyntax() const;

protected:
  /** The name of the command */
  std::string _name;
  /** Description, one line only if possible */
  std::string _shortDescription;
  /** Extra description */
  std::string _extraDescription;
  /** The names of the arguments */
  std::vector<std::string> _argNames;
};

#endif


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8