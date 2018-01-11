/**
 * Qt5 OpenGL video demo application
 * Copyright (C) 2018 Carlos Rafael Giani < dv AT pseudoterminal DOT org >
 *
 * qtglviddemo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#ifndef QTGLVIDDEMO_CAMERA_HPP
#define QTGLVIDDEMO_CAMERA_HPP

#include "Transform.hpp"


namespace qtglviddemo
{


/**
 * 3D representation of a camera.
 *
 * This class allows for calculating view and projection 3D 4x4 matrices
 * out of the given parameters. This is useful for modeling the behavior
 * of a virtual camera in 3D rendering.
 *
 * Mathematically, the view area the camera can "see" is called the
 * view frustum. Since we render the 3D scene into a rectangular area,
 * the frustum looks like a pyramid with a cut off tip.
 *
 * Internally, the camera has a Transform instance, which describes the
 * position and orientation of the camera in the 3D world. (Scaling is
 * not done and left at 1.) The view matrix is actually the exact inverse
 * of that transform's matrix. The view matrix transforms vertex coordinates
 * from world space (also called model space) to view space using the view
 * matrix. In other words, this transformation warps the coordinates
 * to make it look as if the camera's position were (0,0,0).
 *
 * After that, perspective transformation is applied by producing the
 * projection matrix.
 *
 * Both view and projection matrix are (re)calculated on-demand if any
 * of their parameters changed.
 */
class Camera
{
public:
	/**
	 * Constructor.
	 *
	 * Creates a camera that is placed at (0,0,0), has a field of
	 * view angle of 90 degrees, an aspect ratio of 1.0 and a Z
	 * range of 1..100.
	 */
	Camera();

	/**
	 * Sets the field of view angle of the view frustum.
	 *
	 * The FOV angle defines how wide the view frustum is.
	 *
	 * @param p_fov Field of view angle to use. Must be positive.
	 *        and nonzero.
	 */
	void setFov(float const p_fov);
	/**
	 * Sets the aspect ratio of the view frustum.
	 *
	 * This is necessary to make sure the output is not stretched
	 * in horizontal or vertical direction in case the rectangular
	 * area we render into has different width and height sizes.
	 *
	 * The aspect ratio is in fact the ratio between the width and
	 * height of that area. So, if the render region is an area
	 * with 320x240 pixels, the aspect ratio is 320/240 ~ 1.333 .
	 *
	 * @param p_aspect Aspect ratio to use. Must be positive
	 *        and nonzero.
	 */
	void setAspect(float const p_aspect);
	/**
	 * Sets the visible range in Z direction.
	 *
	 * Vertices with Z coordinates outside of this range will not
	 * be visible. The range defines the position of the near and
	 * far plane, or the start and end of the view frustum in
	 * Z direction.
	 *
	 * p_znear must always be smaller than p_zfar. Both values
	 * must be positive and nonzero.
	 *
	 * @param p_znear Near plane value; the start of the Z range.
	 * @param p_zfar Far plane value; the end of the Z range.
	 */
	void setZrange(float const p_znear, float const p_zfar);

	/**
	 * Sets the camera's position in world space.
	 *
	 * @param p_position New 3D position vector to use.
	 */
	void setPosition(QVector3D p_position);
	/**
	 * Sets the camera's rotation in world space.
	 *
	 * @param p_rotation New 3D rotation quaternion to use.
	 */
	void setRotation(QQuaternion p_rotation);

	/**
	 * Returns the projection matrix.
	 *
	 * If FOV, aspect ratio, or the Z range were modified,
	 * then this matrix will internally be recalculated
	 * before returning it.
	 */
	QMatrix4x4 const & getProjectionMatrix() const;
	/**
	 * Returns the view matrix.
	 *
	 * This is the exact inverse of the matrix of the
	 * internal transform object.
	 *
	 * If position or rotation were modified,
	 * then this matrix will internally be recalculated
	 * before returning it.
	 */
	QMatrix4x4 const & getViewMatrix() const;


private:
	float m_fov, m_aspect, m_znear, m_zfar;

	Transform m_transform;

	mutable QMatrix4x4 m_projectionMatrix;
	mutable bool m_projectionMatrixValid;

	mutable QMatrix4x4 m_viewMatrix;
	mutable bool m_viewMatrixValid;
};


} // namespace qtglviddemo end


#endif
