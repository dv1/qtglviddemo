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


#include <utility>
#include "Transform.hpp"


namespace qtglviddemo
{


Transform::Transform()
	: m_matrixValid(false)
	, m_position(0, 0, 0)
	, m_scale(1)
	, m_rotation(1, 0, 0, 0)
{
}


QMatrix4x4 const & Transform::getMatrix() const
{
	// Lazy evaluation: recalculate the matrix on-demand,
	// that is, if current matrix is marked as invalid.
	if (!m_matrixValid)
	{
		m_matrix = QMatrix4x4();
		m_matrix.translate(m_position);
		m_matrix.rotate(m_rotation);
		m_matrix.scale(m_scale);

		m_matrixValid = true;
	}

	return m_matrix;
}


void Transform::setPosition(QVector3D p_position)
{
	m_position = std::move(p_position);
	m_matrixValid = false;
}


QVector3D const & Transform::getPosition() const
{
	return m_position;
}


void Transform::setScale(float const p_scale)
{
	m_scale = p_scale;
	m_matrixValid = false;
}


float Transform::getScale() const
{
	return m_scale;
}


void Transform::setRotation(QQuaternion p_rotation)
{
	m_rotation = std::move(p_rotation);
	m_matrixValid = false;
}


QQuaternion const & Transform::getRotation() const
{
	return m_rotation;
}


} // namespace qtglviddemo end
