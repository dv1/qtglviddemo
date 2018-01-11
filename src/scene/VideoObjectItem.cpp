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
#include <memory>
#include <QOpenGLFunctions>
#include <QOpenGLFramebufferObject>
#include <QQuickWindow>
#include <QLoggingCategory>
#include "GLResources.hpp"
#include "VideoObjectItem.hpp"


Q_DECLARE_LOGGING_CATEGORY(lcQtGLVidDemo)


namespace qtglviddemo
{


/**
 * Renderer for the VideoObjectItem.
 *
 * VideoObjectItem is derived from QQuickFramebufferObject, which expects
 * a renderer class to be defined.
 *
 * In this class, the actual rendering is performed.
 */
class VideoObjectItem::Renderer
	: public QQuickFramebufferObject::Renderer
{
public:
	explicit Renderer(VideoObjectItem &p_item, QOpenGLContext *p_glcontext)
		: m_glcontext(p_glcontext)
		, m_item(p_item)
		, m_mesh(nullptr)
		, m_mustRender(true)
		, m_firstRender(true)
	{
		// Set the formats the player is allowed to use for the video
		// frames. This makes sure that the player only produces frames
		// that are compatible with the video material.
		m_item.m_player.setSinkCapsFromVideoFormats(GLResources::instance().getVideoMaterialProvider().getSupportedVideoFormats());

		// Create the video material.
		m_videoMaterial = GLResources::instance().getVideoMaterialProvider().createVideoMaterial();

		qCDebug(lcQtGLVidDemo) << "Created FBO renderer";
	}

	~Renderer()
	{
		qCDebug(lcQtGLVidDemo) << "Destroyed FBO renderer";
	}

	QOpenGLFramebufferObject *createFramebufferObject(QSize const & p_size)
	{
		// We want an FBO that has a depth buffer.
		QOpenGLFramebufferObjectFormat format;
		format.setAttachment(QOpenGLFramebufferObject::Depth);

		// Force re-rendering, since we recreated the FBO,
		// and the old FBO contents are lost.
		m_mustRender = true;

		// Create the FBO.
		return new QOpenGLFramebufferObject(p_size, format);
	}


	virtual void render() override
	{
		// In here, the FBO contents are (re)rendered only if it is
		// really necessary. If there is no mesh, or no mesh contents,
		// or no new video frame, and nothing set m_mustRender to
		// true, then no rendering is done.

		bool notYetCleared = true;

		// If this is the very first render() call, make sure the FBO
		// is cleared even if there is no mesh, no video frame etc.
		if (m_firstRender)
		{
			clearFBO();

			notYetCleared = false;
			m_firstRender = false;
		}

		// Exit if there is no mesh set at the moment. No need to
		// call update(), since changes in the mesh type will trigger
		// synchronize() and render() calls anyway.
		if (m_mesh == nullptr)
			return;

		// Mesh is set, but has no contents. This should not happen,
		// but check anyway to be safe. We do need to call update()
		// to force the base class to schedule another call to render()
		// because the frame isn't automatically redrawn once contents
		// are set.
		if (!m_mesh->hasContents())
		{
			update();
			return;
		}

		// Try to get a new video frame to render.
		GStreamerMediaSample videoSample = m_item.m_player.pullVideoSample();
		GstSample *sample = videoSample.getSample();
		if (sample != nullptr)
		{
			// This media sample contains a video frame with new caps.
			// One example of why this can happen is that the width and
			// height of video frames changed.
			if (videoSample.sampleHasNewCaps())
			{
				// If caps changed, convert them to GstVideoInfo and
				// pass this new video info to the video material to
				// make sure the texture has the right format and size.

				GstVideoInfo videoInfo;
				gst_video_info_from_caps(&videoInfo, gst_sample_get_caps(sample));
				m_videoMaterial.setVideoInfo(std::move(videoInfo));
			}

			// Pass on the GstBuffer the new video frame is contained in to
			// the video material. It refs the GstBuffer (and unrefs it when
			// it is done with it), so we can safely discard the media sample
			// afterwards.
			m_videoMaterial.setVideoGstbuffer(gst_sample_get_buffer(sample));

			// We have a new video frame, so we must re-render the FBO contents.
			m_mustRender = true;
		}

		// Only render something if something else declared it necessary
		// (meaning, m_mustRender is set to true) and if there is actually
		// a video frame to render (otherwise we can't render anything).
		if (!m_mustRender || !m_videoMaterial.hasVideoGstbuffer())
		{
			// Rendering currently is not necessary, or there's no
			// video frame at the moment.
			return;
		}

		qCDebug(lcQtGLVidDemo) << "Rendering video object item FBO frame";

		GLResources & glresources = GLResources::instance();
		VideoMaterialProvider &vidmatProvider = glresources.getVideoMaterialProvider();
		QOpenGLShaderProgram &prog = vidmatProvider.getShaderProgram();

		QOpenGLFunctions *glfuncs = m_glcontext->functions();

		// Clear the FBO for the new rendering.
		if (notYetCleared)
			clearFBO();

		// Set necessary OpenGL states. We want depth buffer tests,
		// backface culling, but no blending. (We don't do blending. The
		// Qt Quick 2 scenegraph does that by appling blending to the
		// QQuickFramebufferObject item.)
		glfuncs->glEnable(GL_DEPTH_TEST);
		glfuncs->glEnable(GL_CULL_FACE);
		glfuncs->glDisable(GL_BLEND);

		// Bind the video material shader.
		vidmatProvider.getShaderProgram().bind();

		// Bind the VAO if one is present.
		if (glresources.getVAO().isCreated())
			glresources.getVAO().bind();

		// Bind the video material and set the shader uniform values
		// associated with the material.
		m_videoMaterial.bind();
		m_videoMaterial.setShaderUniformValues();

		// Set the shader uniform values associated with transformation
		// matrices to make sure the mesh is rendered with rotation,
		// perspective etc. applied.
		vidmatProvider.getShaderProgram().setUniformValue(vidmatProvider.getModelviewMatrixUniform(), m_modelviewMatrix.normalMatrix());
		vidmatProvider.getShaderProgram().setUniformValue(vidmatProvider.getModelviewprojMatrixUniform(), m_modelviewprojMatrix);

		// Bind the mesh vertex and index buffers.
		m_mesh->bindBuffers();

		// Enable and configure the attribute arrays. These define how
		// the vertex shader gets access to the vertex data. Our meshes
		// have position, normal vector, and texture coordinate attributes,
		// so we need three attribute arrays and the relative offsets
		// of each one of these vertex attributes.
		prog.enableAttributeArray(vidmatProvider.getVertexPositionAttrib());
		prog.setAttributeBuffer(vidmatProvider.getVertexPositionAttrib(), GL_FLOAT, 0, 3, sizeof(Mesh::Vertices::value_type));
		prog.enableAttributeArray(vidmatProvider.getVertexNormalAttrib());
		prog.setAttributeBuffer(vidmatProvider.getVertexNormalAttrib(), GL_FLOAT, sizeof(float)*3, 3, sizeof(Mesh::Vertices::value_type));
		prog.enableAttributeArray(vidmatProvider.getVertexTexcoordsAttrib());
		prog.setAttributeBuffer(vidmatProvider.getVertexTexcoordsAttrib(), GL_FLOAT, sizeof(float)*6, 2, sizeof(Mesh::Vertices::value_type));

		// Everything is ready, we can now render the mesh.
		glfuncs->glDrawElements(GL_TRIANGLES, m_mesh->getNumIndices(), GL_UNSIGNED_SHORT, nullptr);

		// We rendered the mesh. Cleanup.

		prog.disableAttributeArray(vidmatProvider.getVertexPositionAttrib());
		prog.disableAttributeArray(vidmatProvider.getVertexNormalAttrib());
		prog.disableAttributeArray(vidmatProvider.getVertexTexcoordsAttrib());

		m_mesh->releaseBuffers();

		m_videoMaterial.unbind();

		if (glresources.getVAO().isCreated())
			glresources.getVAO().release();

		vidmatProvider.getShaderProgram().release();

		// We just rendered into the FBO, so we do not
		// _have_ render again at the moment.
		m_mustRender = false;

		// Reset any modified OpenGL state.
		m_window->resetOpenGLState();
	}

	virtual void synchronize(QQuickFramebufferObject *) override
	{
		// In here, check if any states changed that affect
		// the mesh rendering. If so, set m_mustRender to
		// true so that render() re-renders the FBO contents.

		m_window = m_item.window();

		// Get current transformation matrices and combine
		// them into modelview and modelviewprojection ones.
		QMatrix4x4 modelMatrix = m_item.m_transform.getMatrix();
		m_modelviewMatrix = m_item.m_camera.getViewMatrix() * modelMatrix;
		QMatrix4x4 newModelviewprojMatrix = m_item.m_camera.getProjectionMatrix() * m_modelviewMatrix;
		// Check that either the camera or the mesh transform
		// changed, and if so, force a re-rendering.
		// (We do not check for changes in m_modelviewMatrix,
		// just m_modelviewprojMatrix, since changes in the
		// former also affect the latter.)
		if (m_modelviewprojMatrix != newModelviewprojMatrix)
		{
			qCDebug(lcQtGLVidDemo) << "New ModelViewProjection matrix";
			m_modelviewprojMatrix = newModelviewprojMatrix;
			m_mustRender = true;
		}

		// If the mesh type changed, we must re-render.
		if (m_meshType != m_item.m_meshType)
		{
			m_meshType = m_item.m_meshType;
			qCDebug(lcQtGLVidDemo) << "New mesh type:" << m_meshType;
			m_mesh = &(GLResources::instance().getMesh(m_meshType));
			m_mustRender = true;
		}

		// If the crop rectangle changed, we must re-render.
		if (m_videoMaterial.getCropRectangle() != m_item.m_cropRectangle)
		{
			qCDebug(lcQtGLVidDemo) << "New crop rectangle:" << m_item.m_cropRectangle;
			m_videoMaterial.setCropRectangle(m_item.m_cropRectangle);
			m_mustRender = true;
		}

		// If the texture rotation changed, we must re-render.
		if (m_videoMaterial.getTextureRotation() != m_item.m_textureRotation)
		{
			qCDebug(lcQtGLVidDemo) << "New texture rotation angle:" << m_item.m_textureRotation;
			m_videoMaterial.setTextureRotation(m_item.m_textureRotation);
			m_mustRender = true;
		}
	}


private:
	void clearFBO()
	{
		// Clear the FBO. Make sure the alpha channel values are set to 0
		// so the FBO produces an image where the background pixels are
		// 100% translucent.

		QOpenGLFunctions *glfuncs = m_glcontext->functions();
		glfuncs->glClearColor(0, 0, 0, 0);
		glfuncs->glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	}

	QOpenGLContext *m_glcontext;
	QQuickWindow *m_window;
	VideoObjectItem &m_item;
	Mesh *m_mesh;
	VideoMaterial m_videoMaterial;

	QString m_meshType;
	QMatrix4x4 m_modelviewMatrix;
	QMatrix4x4 m_modelviewprojMatrix;
	bool m_mustRender;
	bool m_firstRender;
};




VideoObjectItem::VideoObjectItem(QQuickItem *p_parent)
	: QQuickFramebufferObject(p_parent)
	, m_mouseButtonPressed(false)
	, m_cropRectangle(0, 0, 100, 100)
	, m_textureRotation(0)
	, m_player([this]() { onNewFrameAvailable(); })
{
	// Connect the forceFBOUpdate signal to update(). We cannot
	// call update() directly in the GStreamerPlayer new frame
	// callback because it is called from a different thread.
	// So, instead, we use signals to make sure update() is
	// called in the right thread.
	connect(this, &VideoObjectItem::fboNeedsChange, this, &VideoObjectItem::update);

	// This item accepts mouse and touch events.
	setAcceptHoverEvents(true);
	setAcceptedMouseButtons(Qt::AllButtons);
	setFlag(ItemAcceptsInputMethod, true);
	// This item has contents and should be rendered.
	setFlag(ItemHasContents);

	// Camera setup. 60 degree field of view, valid depth range from
	// 0.1 to 100, and the camera moved to the back by 3.5 units
	// so we can see the mesh in the center.
	m_camera.setFov(60.0f);
	m_camera.setZrange(0.1f, 100.0f);
	m_camera.setPosition(QVector3D(0, 0, 3.5f));

	// Tie the arcball to the item's transform object.This way, if the
	// arcball is moved by a mouse or touch event, the transform object
	// is automatically updated to contain the arcball's rotation.
	m_arcball.setTransform(&m_transform);

	qCDebug(lcQtGLVidDemo) << "Created video object item" << this;
}


VideoObjectItem::~VideoObjectItem()
{
	qCDebug(lcQtGLVidDemo) << "Destroyed video object item" << this;
}


QQuickFramebufferObject::Renderer* VideoObjectItem::createRenderer() const
{
	qCDebug(lcQtGLVidDemo) << "Creating new FBO renderer";

	// Problem: The renderer needs non-const access to the player.
	// But, the "this" pointer is const, because this whole function
	// is const.
	// Also, we need to emit canStartPlayback(), but emissions aren't
	// possible from const objects.
	// For this reason, we have to use const_cast.
	// XXX: This is a hack, and there is most likely a better solution.
	// The main problem is that createRenderer() is const.
	VideoObjectItem &self = *(const_cast < VideoObjectItem* > (this));

	std::unique_ptr < Renderer > renderer(new Renderer(self, QOpenGLContext::currentContext()));

	// Inform listeners that they can start playback now.
	emit self.canStartPlayback();

	// And return the new renderer.
	return renderer.release();
}


GStreamerPlayer* VideoObjectItem::getPlayer()
{
	return &m_player;
}


void VideoObjectItem::setRotation(QQuaternion p_rotation)
{
	m_transform.setRotation(std::move(p_rotation));
	update();
	emit rotationChanged();
}


QQuaternion const & VideoObjectItem::getRotation() const
{
	return m_transform.getRotation();
}


void VideoObjectItem::setCropRectangle(QRect p_cropRectangle)
{
	m_cropRectangle = std::move(p_cropRectangle);
	update();
	emit cropRectangleChanged();
}


QRect VideoObjectItem::getCropRectangle() const
{
	return m_cropRectangle;
}


void VideoObjectItem::setMeshType(QString const p_meshType)
{
	m_meshType = p_meshType;
	update();
	emit meshTypeChanged();
}


QString VideoObjectItem::getMeshType() const
{
	return m_meshType;
}


void VideoObjectItem::setTextureRotation(int const p_rotation)
{
	m_textureRotation = p_rotation;
	update();
	emit textureRotationChanged();
}


int VideoObjectItem::getTextureRotation() const
{
	return m_textureRotation;
}


QSGNode* VideoObjectItem::updatePaintNode(QSGNode *p_oldNode, UpdatePaintNodeData *p_updatePaintNodeData)
{
	QQuickWindow *win = window();

	if (win != nullptr)
	{
		// Calculate the pixel sizes and take the device pixel ratio into account.
		float devPixelRatio = win->effectiveDevicePixelRatio();
		QSizeF itemSize = QSizeF(width(), height()) * devPixelRatio;

		if (!itemSize.isEmpty())
		{
			// Next, use the pixel sizes to update the arcball's viewport
			// and the camera's aspect ratio. This way, they are up to
			// date, even if the item is resized.
			m_arcball.setViewport(itemSize.width(), itemSize.height());
			m_camera.setAspect(float(itemSize.width()) / float(itemSize.height()));
		}
	}

	return QQuickFramebufferObject::updatePaintNode(p_oldNode, p_updatePaintNodeData);
}


void VideoObjectItem::mousePressEvent(QMouseEvent *p_event)
{
	QQuickItem::mousePressEvent(p_event);

	m_arcball.press(p_event->x(), p_event->y());
	m_mouseButtonPressed = true;

	// We must accept the mouse press event, otherwise
	// mouse move and mouse release events won't be
	// reported to us.
	p_event->accept();
}


void VideoObjectItem::mouseMoveEvent(QMouseEvent *p_event)
{
	QQuickItem::mouseMoveEvent(p_event);

	if (m_mouseButtonPressed)
	{
		m_arcball.drag(p_event->x(), p_event->y());
		update();
		emit rotationChanged();
	}

	// We handled the event, so accept it.
	p_event->accept();
}


void VideoObjectItem::mouseReleaseEvent(QMouseEvent *p_event)
{
	QQuickItem::mouseReleaseEvent(p_event);

	m_mouseButtonPressed = false;

	// We handled the event, so accept it.
	p_event->accept();
}


void VideoObjectItem::onNewFrameAvailable()
{
	// Don't call update() directly here. Instead, for thread safety
	// reasons, emit fboNeedsChange().
	emit fboNeedsChange();
}


} // namespace qtglviddemo end
