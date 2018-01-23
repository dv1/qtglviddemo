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
#include <cmath>
#include <limits>
#include <utility>
#include <QQuaternion>
#include <QDebug>
#include "Arcball.hpp"


namespace qtglviddemo
{


Arcball::Arcball(Transform *p_transform)
	: m_transform(p_transform)
{
}


void Arcball::setTransform(Transform *p_transform)
{
	m_transform = p_transform;
}


void Arcball::setViewport(unsigned int const p_width, unsigned int const p_height)
{
	m_viewport[0] = p_width;
	m_viewport[1] = p_height;
}


void Arcball::press(unsigned int const p_x, unsigned int const p_y)
{
	if (m_transform == nullptr)
		return;

	m_lastRotationAngle = 0.0f;

	// Project a ray starting at the 2D coordinates
	// from the screen on the sphere.
	m_startVector = projectOnSphere(p_x, p_y);
	// Use the existing transform rotation as the base.
	m_startRotation = m_transform->getRotation();
}


void Arcball::drag(unsigned int const p_x, unsigned int const p_y)
{
	if (m_transform == nullptr)
		return;

	QQuaternion newRot;

	// Project a ray starting at the 2D coordinates
	// from the screen on the sphere.
	QVector3D endVector = projectOnSphere(int(p_x), int(p_y));

	// Calculate the axis out of the start and end vector.
	// Also get the axis length to catch fringe cases where
	// the vector is so short that it would cause numerical
	// problems.
	QVector3D axis = QVector3D::crossProduct(m_startVector, endVector);
	float axisLength = std::sqrt(QVector3D::dotProduct(axis, axis));

	// Calculate the angle using the dot product between start
	// and end vector. Since both start and end vector are
	// of unit length, the dot product is directly the cosine
	// of the angle between them.
	float angle = std::acos(QVector3D::dotProduct(m_startVector, endVector));
	// Convert the angle from radians to degrees, since this is
	// what QQuaternion expects.
	angle = angle * 180.0f / M_PI;

	if (axisLength > std::numeric_limits < float > ::epsilon())
	{
		// Produce the rotation quaternion.
		newRot = QQuaternion::fromAxisAndAngle(axis, angle);
	}
	else
	{
		// We cannot produce a rotation quaternion out of
		// the calculations above for numerical reasons.
		// In this case, use the unit quaternion instead.
		newRot = QQuaternion();
	}

	m_lastRotationAxis = axis;
	m_lastRotationAngle = angle;

	// Combine the new rotation quaternion with the base rotation
	// that was saved in press().
	newRot = newRot * m_startRotation;
	newRot.normalize();

	// Update the rotation quaternion of the associated transform.
	m_transform->setRotation(std::move(newRot));
}


QVector3D Arcball::getLastRotationAxis() const
{
	return m_lastRotationAxis;
}


float Arcball::getLastRotationAngle() const
{
	return m_lastRotationAngle;
}


QVector3D Arcball::projectOnSphere(unsigned int const p_x, unsigned int const p_y) const
{
	// Translate the coordinates from the 0..viewport scales to
	// -1..+1. Also flip the Y coordinate, since the Y axis
	// of the screen and the Y axis in the 3D scene are reversed.
	float fx = +(float(p_x) / float(m_viewport[0]) * 2.0f - 1.0f);
	float fy = -(float(p_y) / float(m_viewport[1]) * 2.0f - 1.0f);

	float length = fx*fx + fy*fy;

	if (length > 1.0f)
	{
		// The projected ray will miss the sphere, because the
		// user didn't actually click on the sphere. In this
		// case, use the 2D coordinates to perform a rotation
		// around the Z axis instead by projecting them on
		// the unit circle on the XY plane. Also normalize
		// the length to make sure the produced vector has
		// a length of 1.

		float norm = 1.0f / std::sqrt(length);
		return QVector3D(fx * norm, fy * norm, 0.0f);
	}
	else
	{
		// The projected ray will hit the sphere. Calculate
		// the hit point on the hemisphere that is facing us.
		// The produced vector has a length of 1.

		return QVector3D(fx, fy, std::sqrt(1.0f - length));
	}
}


} // namespace qtglviddemo end
