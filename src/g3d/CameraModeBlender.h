/*                C A M E R A M O D E B L E N D E R . H
 * BRL-CAD
 *
 * Copyright (c) 2008-2011 United States Government as represented by the
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

/** @file CameraModeBlender.h
 *
 * @author Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *
 * @brief
 *	Declaration of the Camera mode "Blender" of 3D Geometry Editor
 *	(g3d).
 */

#ifndef __G3D_CAMERAMODEBLENDER_H__
#define __G3D_CAMERAMODEBLENDER_H__

#include "CameraMode.h"

/** @brief Blender camera mode
 *
 * @author Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *
 * The behavior of this camera tries to mimic the behaviour of
 * Blender modeling program.
 */
class CameraModeBlender : public CameraMode
{
public:
  /** Default constructor */
  CameraModeBlender();

  /** @see CameraMode::injectKeyPressed */
  virtual bool injectKeyPressed(QKeyEvent *e);
  /** @see CameraMode::injectKeyReleased */
  virtual bool injectKeyReleased(QKeyEvent *e);
  /** @see CameraMode::injectMouseMotion */
  virtual bool injectMouseMotion(QMouseEvent *e);
  /** @see CameraMode::injectMousePressed */
  virtual bool injectMousePressed(QMouseEvent *e);
  /** @see CameraMode::injectMouseReleased */
  virtual bool injectMouseReleased(QMouseEvent *e);
  /** @see CameraMode::injectMouseScrolled */
  virtual bool injectMouseScrolled(Direction direction);

  /** @see CameraMode::cameraControlUpPressed */
  virtual void cameraControlUpPressed();
  /** @see CameraMode::cameraControlDownPressed */
  virtual void cameraControlDownPressed();
  /** @see CameraMode::cameraControlLeftPressed */
  virtual void cameraControlLeftPressed();
  /** @see CameraMode::cameraControlRightPressed */
  virtual void cameraControlRightPressed();
  /** @see CameraMode::cameraControlZoomInPressed */
  virtual void cameraControlZoomInPressed();
  /** @see CameraMode::cameraControlZoomOutPressed */
  virtual void cameraControlZoomOutPressed();
  /** @see CameraMode::cameraControlCenterPressed */
  virtual void cameraControlCenterPressed();
  /** @see CameraMode::cameraControlUpReleased */
  virtual void cameraControlUpReleased();
  /** @see CameraMode::cameraControlDownReleased */
  virtual void cameraControlDownReleased();
  /** @see CameraMode::cameraControlLeftReleased */
  virtual void cameraControlLeftReleased();
  /** @see CameraMode::cameraControlRightReleased */
  virtual void cameraControlRightReleased();
  /** @see CameraMode::cameraControlZoomInReleased */
  virtual void cameraControlZoomInReleased();
  /** @see CameraMode::cameraControlZoomOutReleased */
  virtual void cameraControlZoomOutReleased();
  /** @see CameraMode::cameraControlCenterReleased */
  virtual void cameraControlCenterReleased();

private:
  /** Default rotation step */
  static const float ROTATION_STEP; // radians
  /** Default pan fraction */
  static const float PAN_FRACTION; // ratio
  /** Default zoom step ratio */
  static const float ZOOM_STEP; // ratio

  /** Mode */
  bool _dragModeEnabled;
  /** Mode helper */
  int _dragModeOriginX;
  /** Mode helper */
  int _dragModeOriginY;
  /** Mode helper */
  float _dragOriginalHorizontalRotation;
  /** Mode helper */
  float _dragOriginalVerticalRotation;

  /** Mode */
  bool _panModeEnabled;

  /** Common functionality for all places needing it */
  void panUp();
  /** Common functionality for all places needing it */
  void panDown();
  /** Common functionality for all places needing it */
  void panLeft();
  /** Common functionality for all places needing it */
  void panRight();
  /** Common functionality for all places needing it */
  void doZoomIn();
  /** Common functionality for all places needing it */
  void doZoomOut();
};

#endif

/*
 * Local Variables:
 * tab-width: 8
 * mode: C
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */
