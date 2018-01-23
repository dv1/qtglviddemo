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


#ifndef QTGLVIDDEMO_ARCBALL_HPP
#define QTGLVIDDEMO_ARCBALL_HPP

#include <QVector3D>
#include "Transform.hpp"


namespace qtglviddemo
{


/**
 * Class for arcball-based rotation with a mouse pointer or touch event.
 *
 * This class allows for producing rotation quaternions from user interface
 * interactions, typically a mouse pointer or a touch event. The user presses
 * on a unit sphere, and rotates the sphere by dragging that point.
 * Rotation is implemented by projecting the 2D event coordinates on this
 * sphere when the user presses on it. When the user drags, the 2D drag event
 * coordinates are also projected on the sphere. Using these projected
 * coordinates, an axis and an angle are calculated, and with these, a
 * rotation quaternion is produced.
 *
 * Using this class requires associating it with a Transform object. The
 * transform object's rotation quaternion is automatically adjusted when
 * the user drags the arcball.
 *
 * Also, before using the arcball, make sure the viewport is set by calling
 * setViewport. This defines the valid area for 2D event coordinates.
 *
 * There is no release event or anything like that, since the rotation
 * is always calculated using the point defined in the press event as
 * the starting point. So, once the user lets go of the touchscreen or
 * mouse button, the last rotation calculated during the drag event is
 * simply retained.
 */
class Arcball
{
public:
	/**
	 * Constructor.
	 *
	 * @param p_transform Transform to associate this arcball with.
	 *        If null, no arcball calculations will be done.
	 */
	explicit Arcball(Transform *p_transform = nullptr);

	/**
	 * Associates the given transform object with this arcball.
	 *
	 * If p_transform is null, then no transform object is associated,
	 * and no arcball calculations are done, meaning that press()
	 * and drag() will do nothing.
	 *
	 * @param p_transform Transform to associate this arcball with.
	 */
	void setTransform(Transform *p_transform);

	/**
	 * Sets the viewport (the valid area) for 2D event coordinates.
	 *
	 * Using press() and drag() before this was called results in
	 * undefined behavior.
	 *
	 * @param p_width Width of the viewport.
	 * @param p_height Height of the viewport.
	 */
	void setViewport(unsigned int const p_width, unsigned int const p_height);

	/**
	 * Press event that starts the arcball rotation.
	 *
	 * The rotation begins when the user "presses" on the arcball.
	 * The mouse button is held down.
	 *
	 * On touchscreens, "pressing" is actually be the first touch
	 * event report. The user still has the finger on the touchscreen
	 * at this point.
	 *
	 * This does not rotate the arcball. It just defines a
	 * starting point for rotation calculations.
	 *
	 * @param p_x X coordinate of the event. Must be within the
	 *        viewport defined by setViewport().
	 * @param p_y Y coordinate of the event. Must be within the
	 *        viewport defined by setViewport().
	 */
	void press(unsigned int const p_x, unsigned int const p_y);
	/**
	 * Drag event that actually rotates the arcball.
	 *
	 * The rotation is calculated when the user moves the mouse
	 * while still keeping the mouse button pressed.
	 *
	 * On touchscreens, the user is still pressing the finger on
	 * the touchscreen and moving it across the screen.
	 *
	 * @param p_x X coordinate of the event. Must be within the
	 *        viewport defined by setViewport().
	 * @param p_y Y coordinate of the event. Must be within the
	 *        viewport defined by setViewport().
	 */
	void drag(unsigned int const p_x, unsigned int const p_y);

	/**
	 * Returns the last rotation axis that was computed in the drag() function.
	 */
	QVector3D getLastRotationAxis() const;
	/**
	 * Returns the last rotation angle that was computed in the drag() function.
	 */
	float getLastRotationAngle() const;


private:
	QVector3D projectOnSphere(unsigned int const p_x, unsigned int const p_y) const;

	Transform *m_transform;

	QVector3D m_lastRotationAxis;
	float m_lastRotationAngle;

	QQuaternion m_startRotation;
	QVector3D m_startVector;
	unsigned int m_viewport[2];
};


} // namespace qtglviddemo end


#endif
