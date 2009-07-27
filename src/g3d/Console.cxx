/*                  M A I N W I N D O W . H
 * BRL-CAD
 *
 * Copyright (c) 2008-2009 United States Government as represented by the
 * U.S. Army Research Laboratory.
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

/** @file OgreGLWidget.h
 *
 * @author Benjamin Saunders <ralith@users.sourceforge.net>
 *
 * @brief 
 *	Implementation for the Console widget
 */

#include "Console.h"

#include "Command.h"
#include "CommandInterpreter.h"

Console::Console(QWidget *parent) : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    
    output = new QLabel();
    output->setText("Console output goes here.");
    output->setAutoFillBackground(true);
    output->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    output->hide();
    installEventFilter(output);
    layout->addWidget(output);

    entry = new QLineEdit();
    installEventFilter(entry);
    layout->addWidget(entry);
    
    installEventFilter(this);

    QObject::connect(entry, SIGNAL(returnPressed),
		     this, SLOT(evalCmd));
}

bool Console::eventFilter(QObject *, QEvent *event) 
{
    switch(event->type())
    {
    case QEvent::Enter:
	output->show();
	break;
	
    case QEvent::Leave:
	output->hide();
	break;

    default:
	break;
    }

    return false;
}

// TODO: Doesn't appear to get signaled.
void Console::evalCmd() 
{
    CommandOutput output;
    CommandInterpreter::instance().execute(entry->text().toStdString(), output);
    entry->clear();
}


/*
 * Local Variables:
 * mode: C++
 * tab-width: 8
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */
