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


#ifndef QTGLVIDDEMO_VIDEO_OBJECT_ITEM_HPP
#define QTGLVIDDEMO_VIDEO_OBJECT_ITEM_HPP

#include <QQuickFramebufferObject>
#include <QRectF>
#include "player/GStreamerPlayer.hpp"
#include "Arcball.hpp"
#include "Camera.hpp"
#include "Transform.hpp"


class QOpenGLContext;


namespace qtglviddemo
{


/**
 * QtQuick 2 item for rendering video objects.
 *
 * A "video object" is a 3D mesh with a video material applied to it. For
 * example, it could be a cube with the video shown on all of its faces.
 *
 * The VideoObjectItem renders video objects using parameters like opacity
 * or mesh type, and produces the video frames using GStreamerPlayer.
 * It is a QtQuick 2 item that can be used as the delegate of a QtQuick 2
 * view.
 *
 * The properties (meshType, rotation etc.) may be set manually in C++
 * or QML, but typically they are defined by using VideoObjectItem as a
 * QtQuick 2 view delegate, and using an instance of VideoObjectModel
 * as the data model for that QtQuick 2 view.
 *
 * For rendering it uses custom OpenGL commands. Internally, the object
 * is actually rendered to an OpenGL framebuffer object (which is why
 * this item inherits from QQuickFramebufferObject and not directly
 * from QQuickItem). This is the recommended way of integrating custom
 * 3D rendering into the QtQuick 2 scenegraph.
 */
class VideoObjectItem
	: public QQuickFramebufferObject
{
	Q_OBJECT

	Q_PROPERTY(qtglviddemo::GStreamerPlayer* player READ getPlayer)

	/// Rotation quaternion to use for rotating the 3D object.
	Q_PROPERTY(QQuaternion rotation READ getRotation WRITE setRotation NOTIFY rotationChanged)
	/// Crop rectangle to use in the video material.
	Q_PROPERTY(QRect cropRectangle READ getCropRectangle WRITE setCropRectangle NOTIFY cropRectangleChanged)
	/// Type of the mesh to render.
	Q_PROPERTY(QString meshType READ getMeshType WRITE setMeshType NOTIFY meshTypeChanged)
	/// Texture rotation angle to use in the video material.
	Q_PROPERTY(int textureRotation READ getTextureRotation WRITE setTextureRotation NOTIFY textureRotationChanged)

	class Renderer;

public:
	/**
	 * Constructor.
	 *
	 * Creates an item with crop rectangle (0,0,100,100), identity rotation
	 * quaternion, and texture rotation angle 0.
	 *
	 * This item will not be rendered until a valid mesh type is set.
	 *
	 * @param p_parent Parent QtQuick 2 item
	 */
	explicit VideoObjectItem(QQuickItem *p_parent = nullptr);
	~VideoObjectItem();

	// QQuickFramebufferObject::createRenderer() override.
	virtual QQuickFramebufferObject::Renderer* createRenderer() const override;


	// Property accessors

	GStreamerPlayer* getPlayer();

	void setRotation(QQuaternion p_rotation);
	QQuaternion const & getRotation() const;

	void setCropRectangle(QRect p_cropRectangle);
	QRect getCropRectangle() const;

	void setMeshType(QString const p_meshType);
	QString getMeshType() const;

	void setTextureRotation(int const p_rotation);
	int getTextureRotation() const;


signals:
	/**
	 * This signal is emitted when it is OK to start playback.
	 *
	 * In a QML script, this is useful for autostarting playback. Calling
	 * the player's setUrl() and play() functions in the Component.onComplete()
	 * signal is not an option, since the renderer might not be set up
	 * at that point yet. So, instead, by listening to this signal, the
	 * setUrl() and play() functions can be done at the right moment.
	 */
	void canStartPlayback();

	/// This signal is emitted when the rotation quaternion changes.
	void rotationChanged();
	/// This signal is emitted when the crop rectangle changes.
	void cropRectangleChanged();
	/// This signal is emitted when the mesh type string changes.
	void meshTypeChanged();
	/// This signal is emitted when the texture rotation angle changes.
	void textureRotationChanged();

	// Internal signal for when the FBO needs to be updated. Typically
	// this is emitted when the player has a new video frame.
	void fboNeedsChange();


protected:
	virtual QSGNode* updatePaintNode(QSGNode *p_oldNode, UpdatePaintNodeData *p_updatePaintNodeData) override;


private:
	virtual void mousePressEvent(QMouseEvent *p_event);
	virtual void mouseMoveEvent(QMouseEvent *p_event);
	virtual void mouseReleaseEvent(QMouseEvent *p_event);

	void onNewFrameAvailable();

	Arcball m_arcball;
	bool m_mouseButtonPressed;
	Camera m_camera;
	Transform m_transform;

	QRect m_cropRectangle;
	QString m_meshType;
	int m_textureRotation;
	
	mutable GStreamerPlayer m_player;
};


} // namespace qtglviddemo end


#endif
