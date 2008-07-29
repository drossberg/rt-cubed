/*              G U I C O M M A N D O V E R L A Y. C X X
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

/** @file GuiCommandOverlay.cxx
 *
 * @author Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *
 * @brief
 *	Implementation of the GuiCommandOverlay class of 3D Geometry
 *	Editor (g3d).
 */

#include "GuiCommandOverlay.h"

#include "Logger.h"
#include "History.h"
#include "CommandInterpreter.h"
#include "GuiWindowManager.h"

#include <OIS/OISKeyboard.h>
#include <OGRE/OgreRenderWindow.h>
#include <RBGui/GuiDefines.h>
#include <RBGui/GuiManager.h>
#include <RBGui/Window.h>
#include <RBGui/Widgets/TextWidget.h>
#include <RBGui/Widgets/TextEntryWidget.h>


/*******************************************************************************
 * GuiCommandOverlay
 ******************************************************************************/
GuiCommandOverlay::GuiCommandOverlay(RBGui::GuiManager& guiMgr) :
  GuiBaseWindow(guiMgr, "Command Overlay", "Command:", false), _prompt(0)
{
  // creating windows and widgets
  _mainWindow->setHideOnClose(true);
  _mainWindow->setCloseable(true);
  _mainWindow->setMovable(false);
  _mainWindow->setResizeable(false);
  // _mainWindow->setBorderVisible(false);
  // _mainWindow->setOpacity(0.8f);
  _mainWindow->setModal(true);
  //show();

  _prompt = static_cast<RBGui::TextEntryWidget*>(_mainWindow->createWidget("TextEntry"));
  _prompt->setName("CommandPrompt");

  // setting callbacks for window/widget events within RBGui.
  // ReturnPressed managed in KeyPressed, since both are called anyway
  _mainWindow->setCallback(&GuiCommandOverlay::callbackFocusReceived, this, "FocusRecieved"); // recieved [sic]
  _prompt->setCallback(&GuiCommandOverlay::callbackPromptKeyPressed, this, "onKeyPressed");

  GuiWindowManager::instance().registerWindow(this);
  History::instance().attach(this);
}

GuiCommandOverlay::~GuiCommandOverlay()
{
  /// \todo mafm: could/should automatically detached by ~Observer()
  History::instance().detach(this);

  delete _prompt; _prompt = 0;
  delete _mainWindow; _mainWindow = 0;
}

void GuiCommandOverlay::resize(float contentLeft, float contentTop, float contentWidth, float contentHeight)
{
  const float promptHeight = 18.0f;

  // main window
  _mainWindow->setPosition(Mocha::Vector2(contentLeft + (contentWidth*0.05f),
					  contentTop + ((contentHeight-promptHeight)/2.0f)));
  _mainWindow->setSize(Mocha::Vector2(contentWidth*0.9f, promptHeight));

  // widgets
  _prompt->setPosition(Mocha::Vector2(0.0f, 0.0f));
  _prompt->setSize(_mainWindow->getClientRectangle().getSize());
}

void GuiCommandOverlay::update(const ObserverEvent& event)
{
  try {
    // history events
    {
      const HistoryObserverEvent* e = dynamic_cast<const HistoryObserverEvent*>(&event);
      if (e) {
	switch (e->_actionId) {
	case HistoryObserverEvent::ADDED_ENTRY:
	  // empty
	  break;
	case HistoryObserverEvent::INDEX_CHANGED:
	  setPromptContent(e->_content);
	  break;
	default:
	  throw "Action not understood by Observer";
	}
	return;
      }
    }

    // event not processed before
    throw "Event type not expected by Observer";
  } catch (const char* error) {
    Logger::logWARNING("GuiCommandOverlay: '%s' event: %s", event._className.c_str(), error);
  }
}

const std::string& GuiCommandOverlay::getPromptContent() const
{
  return _prompt->getText();
}

void GuiCommandOverlay::setPromptContent(const std::string& content)
{
  _prompt->setText(content);
}

void GuiCommandOverlay::callbackPromptKeyPressed(RBGui::GuiElement& /* vElement */, const Mocha::ValueList& vData)
{
  /// \note mafm: should be synchronized with CommandOverlay, to avoid
  /// inconsistencies

  int key = static_cast<int>(vData[0].getAsNumber());
  std::string cmd("");
  switch (key) {
    case OIS::KC_RETURN:
      // return key -- insert in history
      cmd = getPromptContent();
      if (cmd.length() > 0) {
	History::instance().insert(cmd.c_str());
	setPromptContent("");
	CommandOutput output;
	CommandInterpreter::instance().execute(cmd, output);
      } else {
	// return pressed, but empty command
      }
      hide();
      break;
    case OIS::KC_UP:
    case OIS::KC_DOWN:
      // up & down keys -- navigate history
      if (key == OIS::KC_UP) {
	cmd = History::instance().getPrev();
      } else if (key == OIS::KC_DOWN) {
	cmd = History::instance().getNext();
      }
      setPromptContent(cmd);
      break;
    case OIS::KC_TAB:
      // tab -- command autocompletion
      cmd = getPromptContent();
      cmd = CommandInterpreter::instance().getAutocompleteString(cmd);
      setPromptContent(cmd);
      break;
    default:
      // nothing
      ;
  }
}

void GuiCommandOverlay::callbackFocusReceived(RBGui::GuiElement& /* vElement */, const Mocha::ValueList& /* vData */)
{
  _mainWindow->setFocused(_prompt);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
