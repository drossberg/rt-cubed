/*                C A M E R A M O D E . H
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

/** @file CameraMode.h
 *
 * @author Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *
 * @brief
 *	Declaration of the base class for Camera modes of 3D Geometry
 *	Editor (g3d).
 */

#ifndef __G3D_CAMERAMODE_H__
#define __G3D_CAMERAMODE_H__


#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>


namespace Ogre {
  class Camera;
}


/** Pi constant */
#define PI_NUMBER 3.14159265358979323846

/** Limit for vertical rotations */
const float VERTICAL_ROTATION_MAX_LIMIT = (PI_NUMBER/2.0f)-0.01f;
/** Limit for vertical rotations */
const float VERTICAL_ROTATION_MIN_LIMIT = -(PI_NUMBER/2.0f)+0.01f;


/** @brief Base class for camera mode
 *
 * @author Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *
 * Implements the actions asked to the camera, each derived class
 * behaves different according to its own nature (assigns different
 * key/mouse bindings for the actions of translation, rotation, etc).
 *
 * We keep common functionality implemented here.
 */
class CameraMode
{
public:
  /** Direction value: Negative/Neutral/Positive */
  enum Direction {
    NEGATIVE = -1,
    NEUTRAL = 0,
    POSITIVE = 1
  };

  /** Default constructor */
  CameraMode(const char* name);
  /** Destructor */
  virtual ~CameraMode() { }

  /** Called every frame via camera manager, with the time elapsed
   * since last update, so we move the camera of the engine, and thus
   * control how do we view the scene. */
  void updateCamera(Ogre::Camera* camera, double elapsedSeconds);

  /** Get the name */
  const char* getName() const;

  /** Set flag for this camera action */
  void setZoom(Direction direction);
  /** Set flag for this camera action */
  void setRotateX(Direction direction);
  /** Set flag for this camera action */
  void setRotateY(Direction direction);
  /** Set flag for this camera action */
  void setRotateZ(Direction direction);

  /** Set/get value for this camera value */
  void setRotationSpeed(float speed);
  /** Set/get value for this camera value */
  float getRotationSpeed() const;
  /** Set/get value for this camera value */
  void setZoomSpeedRatio(float ratio);
  /** Set/get value for this camera value */
  float getZoomSpeedRatio() const;

  /** Set flag for this camera action */
  void setResetToCenter(bool b);

  /** Stop all movements and rotations */
  void stop();

  /** Convert from degrees to radians */
  static float degreesToRadians(float degrees);
  /** Increase the variable by given value, but result not more than
   * given limit */
  static void increaseVarWithLimit(float& rotation, float incrValue, float limit);
  /** Decrease the variable by given value, but result not less than
   * given limit */
  static void decreaseVarWithLimit(float& rotation, float incrValue, float limit);
  /** Multiply the variable by given value, but result not more than
   * given limit */
  static void multiplyVarWithLimit(float& rotation, float incrValue, float limit);
  /** Divide the variable by given value, but result not less than
   * given limit */
  static void divideVarWithLimit(float& rotation, float incrValue, float limit);

  /** Inject input */
  virtual bool injectKeyPressed(OIS::KeyCode /* keyCode */)
    { return false; }
  /** Inject input */
  virtual bool injectKeyReleased(OIS::KeyCode /* keyCode */)
    { return false; }
  /** Inject input */
  virtual bool injectMouseMotion(int /* x */, int /* y */)
    { return false; }
  /** Inject input */
  virtual bool injectMousePressed(OIS::MouseButtonID /* buttonId */,
				  int /* x */, int /* y */)
    { return false; }
  /** Inject input */
  virtual bool injectMouseReleased(OIS::MouseButtonID /* buttonId */,
				   int /* x */, int /* y */)
    { return false; }
  /** Inject input */
  virtual bool injectMouseScrolled(Direction /* direction */)
    { return false; }


protected:
  /** Speed of the rotation */
  static const float ROTATION_DEFAULT_SPEED; // cycles/second
  /** Speed ratio of the zoom */
  static const float ZOOM_DEFAULT_SPEED_RATIO; // times per second
  /** Maximum radius distance */
  static const float RADIUS_MAX_DISTANCE; // m
  /** Minimum radius distance */
  static const float RADIUS_MIN_DISTANCE; // m
  /** Default radius distance */
  static const float RADIUS_DEFAULT_DISTANCE; // m

  /** Name of the mode */
  const char* _name;

  /** Windows parameters */
  int _windowWidth;
  /** Windows parameters */
  int _windowHeight;

  /** Flag for camera action */
  Direction _actionRotateX;
  /** Flag for camera action */
  Direction _actionRotateY;
  /** Flag for camera action */
  Direction _actionRotateZ;
  /** Flag for camera action */
  Direction _actionZoom;

  /** Flag for camera action */
  bool _actionResetToCenter;

  /** Basic value for calculation */
  float _rotationSpeed; /// cycles/second
  /** Basic value for calculation */
  float _zoomSpeedRatio; /// times per second
  /** Current radius */
  float _radius;
  /** Current horizontal rotation */
  float _horizontalRot;
  /** Current vertical rotation */
  float _verticalRot;
  /** Coordinates to take as center
   *
   * \todo mafm: probably should use somethinf similar from other
   * BRL-CAD sources
   */
  class Vector3 {
  public:
    float x, y, z;
    Vector3(float xx, float yy, float zz) : x(xx), y(yy), z(zz) { }
  } _center;
};


#endif


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8