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


#ifndef QTGLVIDDEMO_GLRESOURCES_HPP
#define QTGLVIDDEMO_GLRESOURCES_HPP

#include <map>
#include <memory>
#include <QOpenGLVertexArrayObject>
#include "mesh/Mesh.hpp"
#include "videomaterial/VideoMaterial.hpp"


class QOpenGLContext;


namespace qtglviddemo
{


/**
 * Class for common OpenGL resources used by all Qt Quick video object items.
 *
 * There are some OpenGL resources that do not have to be created more than
 * once. In fact, doing so would probably waste resources. For example, the
 * shader for rendering video materials only needs to be instantiated once.
 *
 * This class contains the common resources, which are:
 * - Video material provider
 * - Vertex array object
 * - Map containing Mesh instances (with OpenGL index/vertex buffer objects)
 *
 * Since it is not possible to pass arguments to the constructor of a custom
 * Qt Quick 2 item, this class is set up as a singleton. The instance()
 * function returns the global class instance, and creates it if it does
 * not yet exist.
 *
 * The reason why this singleton class is initialized this way is that there
 * is no designated moment when custom shared OpenGL resources can be initialized
 * in Qt. On desktop machines, QQuickWindow's sceneGraphInitialized() signal
 * can be used for this purpose, but this signal isn't emitted on embedded
 * devices with the eglfs platform. As it turns out, the only reliable way
 * of initializing the common resources is on-demand, that is, whenever the
 * global instance is first accessed with instance().
 *
 * When the global instance is created in instance(), the constructor in
 * turn initializes the common OpenGL resources. For this purpose, it uses
 * the current OpenGL context. It also establishes a connection to said
 * context's aboutToBeDestroyed() signal. The connected slot destroys the
 * global class instance. This way, it is guaranteed that when the
 * destructor runs, the OpenGL context is still valid.
 */
class GLResources
	: public QObject
{
	Q_OBJECT

public:
	/**
	 * Returns the vertex array object (VAO).
	 *
	 * On OpenGL 3.3 and later, having a VAO is a must. On version
	 * 3.2 and older, and on OpenGL ES 2.x, a VAO may not be required,
	 * or VAOs may not even exist. To remain compatible with both
	 * kinds of platforms, GLResources contains a Qt VAO instance.
	 * This instance internally does or doesn't create a VAO,
	 * depending on the OpenGL type. If the instance's isCreated()
	 * function returns true, the VAO was created, otherwise it wasn't.
	 */
	QOpenGLVertexArrayObject & getVAO();

	/**
	 * Returns the video material provider.
	 *
	 * Which provider is used depends on the platform.
	 */
	VideoMaterialProvider & getVideoMaterialProvider();

	/**
	 * Returns a mesh of the given type.
	 *
	 * If such a mesh doesn't exist yet, it is created and stored
	 * in an internal STL map. This way, multiple cube objects can
	 * exist for example, and the mesh has to be created only once.
	 *
	 * The provider's OpenGL context must be valid when this is run.
	 *
	 * @param p_meshType Mesh type string. Valid values are "cube",
	 *        "quad", "teapot".
	 */
	Mesh & getMesh(QString const &p_meshType);

	/**
	 * Returns the singleton instance. If it doesn't exist yet, it
	 * is created.
	 *
	 * The provider's OpenGL context must be valid when this is run.
	 */
	static GLResources& instance();

private slots:
	void teardownSingletonInstance();

private:
	explicit GLResources(QOpenGLContext *p_glcontext);
	~GLResources();

	QOpenGLVertexArrayObject m_vao;

	typedef std::unique_ptr < VideoMaterialProvider > VideoMaterialProviderUPtr;
	VideoMaterialProviderUPtr m_videoMaterialProvider;

	typedef std::map < QString, MeshUPtr > MeshMap;
	MeshMap m_meshMap;
};


} // namespace qtglviddemo end


#endif
