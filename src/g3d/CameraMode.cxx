/*                C A M E R A M O D E . C X X
 * BRL-CAD
 *
 * Copyright (c) 2008 United States Government as represented by the
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

/** @file CameraMode.cxx
 *
 * @author Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *
 * @brief
 *	Implementation of the base class for Camera modes of 3D
 *	Geometry Editor (g3d).
 */

#include "CameraMode.h"

#include "Logger.h"

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreViewport.h>


/*******************************************************************************
 * CameraMode
 ******************************************************************************/
const float CameraMode::ROTATION_DEFAULT_SPEED = (2.0f*PI_NUMBER)/4.0f; // 4s for full revolution
const float CameraMode::ZOOM_DEFAULT_SPEED_RATIO = 4.0f; // 4 times per second
const float CameraMode::RADIUS_MAX_DISTANCE = 10000.0f; // m
const float CameraMode::RADIUS_MIN_DISTANCE = 0.1f; // m
const float CameraMode::RADIUS_DEFAULT_DISTANCE = 500.0f; // m

CameraMode::CameraMode(const char* name) :
  _name(name), _windowWidth(0), _windowHeight(0),
  _actionRotateX(NEUTRAL), _actionRotateY(NEUTRAL), _actionRotateZ(NEUTRAL),
  _actionZoom(NEUTRAL),
  _actionResetToCenter(false),
  _rotationSpeed(ROTATION_DEFAULT_SPEED),
  _zoomSpeedRatio(ZOOM_DEFAULT_SPEED_RATIO),
  _radius(RADIUS_DEFAULT_DISTANCE), _horizontalRot(0.0f), _verticalRot(0.0f),
  _center(0, 0, 0)
{
}

void CameraMode::updateCamera(Ogre::Camera* camera, double elapsedSeconds)
{
  _windowWidth = camera->getViewport()->getActualWidth();
  _windowHeight = camera->getViewport()->getActualHeight();

  // apply rotations
  if (_actionResetToCenter) {
    // center (reset rotation) when requested
    _horizontalRot = 0.0f;
    _verticalRot = 0.0f;
    _center = Vector3(0.0f, 0.0f, 0.0f);
    _radius = RADIUS_DEFAULT_DISTANCE;
    _actionResetToCenter = false;
  } else {
    // vertical rotation
    if (_actionRotateX == POSITIVE) {
      increaseVarWithLimit(_verticalRot,
			   _rotationSpeed * elapsedSeconds,
			   VERTICAL_ROTATION_MAX_LIMIT);
    } else if (_actionRotateX == NEGATIVE) {
      decreaseVarWithLimit(_verticalRot,
			   _rotationSpeed * elapsedSeconds,
			   VERTICAL_ROTATION_MIN_LIMIT);
    }

    // horizontal rotation
    if (_actionRotateY == POSITIVE) {
      _horizontalRot += _rotationSpeed * elapsedSeconds;
    } else if (_actionRotateY == NEGATIVE) {
      _horizontalRot -= _rotationSpeed * elapsedSeconds;
    }
  }

  // radius
  if (_actionZoom == POSITIVE) {
    divideVarWithLimit(_radius,
		       1.0f + (ZOOM_DEFAULT_SPEED_RATIO*elapsedSeconds),
		       RADIUS_MIN_DISTANCE);
  } else if (_actionZoom == NEGATIVE) {
    multiplyVarWithLimit(_radius,
			 1.0f + (ZOOM_DEFAULT_SPEED_RATIO*elapsedSeconds),
			 RADIUS_MAX_DISTANCE);
  }

  Ogre::SceneNode tmpNode(0);

  // set initial center
  Ogre::Vector3 centerTranslation(_center.x, _center.y, _center.z);
  tmpNode.translate(centerTranslation, Ogre::SceneNode::TS_LOCAL);

  // rotations
  tmpNode.yaw(Ogre::Radian(_horizontalRot));
  tmpNode.pitch(Ogre::Radian(_verticalRot));

  // position -- push back given radius
  Ogre::Vector3 radiusDistance(0, 0, _radius);
  tmpNode.translate(radiusDistance, Ogre::SceneNode::TS_LOCAL);

  // set the resulting position to the camera
  Ogre::Vector3 pos(camera->getPosition());
  if (pos != tmpNode.getPosition()) {
    //Logger::logDEBUG("Camera position (%0.1f, %0.1f, %0.1f)", pos.x, pos.y, pos.z);

    camera->setPosition(tmpNode.getPosition());
    camera->lookAt(_center.x, _center.y, _center.z);
  }
}

const char* CameraMode::getName() const
{
  return _name;
}

void CameraMode::setResetToCenter(bool b)
{
  _actionResetToCenter = b;
}

void CameraMode::setZoom(Direction direction)
{
  _actionZoom = direction;
}

void CameraMode::setRotateX(Direction direction)
{
  _actionRotateX = direction;
}

void CameraMode::setRotateY(Direction direction)
{
  _actionRotateY = direction;
}

void CameraMode::setRotateZ(Direction direction)
{
  _actionRotateZ = direction;
}

void CameraMode::setRotationSpeed(float speed)
{
  _rotationSpeed = speed;
}

float CameraMode::getRotationSpeed() const
{
  return _rotationSpeed;
}

void CameraMode::setZoomSpeedRatio(float ratio)
{
  _zoomSpeedRatio = ratio;
}

float CameraMode::getZoomSpeedRatio() const
{
  return _zoomSpeedRatio;
}

void CameraMode::stop()
{
  _actionRotateX = NEUTRAL;
  _actionRotateY = NEUTRAL;
  _actionRotateZ = NEUTRAL;
  _actionZoom = NEUTRAL;
}

float CameraMode::degreesToRadians(float degrees)
{
  return (degrees*2.0f*PI_NUMBER)/360.0f;
}

void CameraMode::increaseVarWithLimit(float& rotation, float incrValue, float limit)
{
  rotation += incrValue;
  if (rotation > limit) {
    rotation = limit;
  }
}

void CameraMode::decreaseVarWithLimit(float& rotation, float incrValue, float limit)
{
  rotation -= incrValue;
  if (rotation < limit) {
    rotation = limit;
  }
}

void CameraMode::multiplyVarWithLimit(float& rotation, float incrValue, float limit)
{
  rotation *= incrValue;
  if (rotation > limit) {
    rotation = limit;
  }
}

void CameraMode::divideVarWithLimit(float& rotation, float incrValue, float limit)
{
  rotation /= incrValue;
  if (rotation < limit) {
    rotation = limit;
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8