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


#include <assert.h>
#include <utility>
#include "Camera.hpp"


namespace qtglviddemo
{


Camera::Camera()
	: m_fov(90.0f)
	, m_aspect(1.0f)
	, m_znear(1.0f)
	, m_zfar(100.0f)
	, m_projectionMatrixValid(false)
	, m_viewMatrixValid(false)
{
}


void Camera::setFov(float const p_fov)
{
	assert(p_fov > 0.0f);

	m_fov = p_fov;
	m_projectionMatrixValid = false;
}


void Camera::setAspect(float const p_aspect)
{
	assert(p_aspect > 0.0f);

	m_aspect = p_aspect;
	m_projectionMatrixValid = false;
}


void Camera::setZrange(float const p_znear, float const p_zfar)
{
	assert(p_znear > 0.0f);
	assert(p_zfar > 0.0f);
	assert(p_znear < p_zfar);

	m_znear = p_znear;
	m_zfar = p_zfar;
	m_projectionMatrixValid = false;
}


void Camera::setPosition(QVector3D p_position)
{
	m_transform.setPosition(std::move(p_position));
	m_viewMatrixValid = false;
}


void Camera::setRotation(QQuaternion p_rotation)
{
	m_transform.setRotation(std::move(p_rotation));
	m_viewMatrixValid = false;
}


QMatrix4x4 const & Camera::getProjectionMatrix() const
{
	if (!m_projectionMatrixValid)
	{
		m_projectionMatrix = QMatrix4x4();
		m_projectionMatrix.perspective(m_fov, m_aspect, m_znear, m_zfar);
		m_projectionMatrixValid = true;
	}

	return m_projectionMatrix;
}


QMatrix4x4 const & Camera::getViewMatrix() const
{
	if (!m_viewMatrixValid)
	{
		m_viewMatrix = m_transform.getMatrix().inverted();
		m_viewMatrixValid = true;
	}

	return m_viewMatrix;
}


} // namespace qtglviddemo end
