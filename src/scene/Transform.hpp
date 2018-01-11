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


#ifndef QTGLVIDDEMO_TRANSFORM_HPP
#define QTGLVIDDEMO_TRANSFORM_HPP

#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector3D>


namespace qtglviddemo
{


/**
 * 3D transformation class.
 *
 * This allows for calculating 3D transformations and producing 4x4 matrices
 * containing these transformations.
 *
 * Supported transformations are rotation, translation, and scaling.
 * The scaling is uniform, meaning that scaling in X,Y,Z direction is done
 * with equal magnitude.
 *
 * The advantage of using this class over using matrix multiplications is
 * that the position vector, scale factor, and rotation quaternion can be
 * adjusted independently, and the order of the individual transformations
 * is maintained.
 *
 * Rotation is performed using the position vector as the origin. Scaling
 * also uses the position vector as the origin. First, scaling is done.
 * Then, the scaled version is rotated. Finally, the scaled and rotated
 * version is moved from (0,0,0) to the position vector.
 */
class Transform
{
public:
	/**
	 * Constructor.
	 *
	 * Sets up an identity transform: position vector (0,0,0),
	 * scale factor 1, identity quaternion as rotation.
	 */
	Transform();

	/**
	 * Returns the transform in matrix form.
	 *
	 * This matrix is calculated on-demand. If for example the position
	 * is changed, then an internal flag is set to denote that the
	 * matrix needs to be updated. Later, when getMatrix() is called,
	 * the matrix is recalculated.
	 */
	QMatrix4x4 const & getMatrix() const;

	/**
	 * Sets the position vector.
	 *
	 * The position vector is also used as the origin for the
	 * rotation and scale transformations.
	 *
	 * @param p_position New position vector to use.
	 */
	void setPosition(QVector3D p_position);
	/// Returns the current position vector.
	QVector3D const & getPosition() const;

	/**
	 * Sets the scale factor.
	 *
	 * While a factor of 0 may work, it is untested.
	 *
	 * The position vector is used as the origin for this transformation.
	 *
	 * @param p_scale New scale factor to use.
	 */
	void setScale(float const p_scale);
	/// Returns the current scale factor.
	float getScale() const;

	/**
	 * Sets the rotation quaternion.
	 *
	 * The position vector is used as the origin for this transformation.
	 *
	 * @param p_scale New rotation quaternion to use.
	 */
	void setRotation(QQuaternion p_rotation);
	/// Returns the current rotation quaternion.
	QQuaternion const & getRotation() const;


private:
	// These are defined as mutable to be able to perform lazy evaluation
	// in getMatrix().
	mutable bool m_matrixValid;
	mutable QMatrix4x4 m_matrix;

	QVector3D m_position;
	float m_scale;
	QQuaternion m_rotation;
};


} // namespace qtglviddemo end


#endif
