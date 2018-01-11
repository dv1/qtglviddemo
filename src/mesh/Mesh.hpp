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


#ifndef QTGLVIDDEMO_MESH_HPP
#define QTGLVIDDEMO_MESH_HPP

#include <memory>
#include <cstdint>
#include <vector>
#include <QString>
#include <QOpenGLBuffer>


namespace qtglviddemo
{


/**
 * Class for 3D mesh data stored in OpenGL buffers.
 *
 * This is a 3D mesh that is used for OpenGL rendering. It is meant for modern
 * OpenGL, and therefore contains only triangles (no quads, polygons etc.).
 * One triangle = 3 indices.
 *
 * Meshes have a "type". This is a string that is used to identify the type
 * of the mesh contents. This type must be associated with any contents that
 * are used for this mesh. For example, of cube mesh data is provided in the
 * setContents() call, the mesh type must be "cube". The mesh type is used
 * for looking up the correct mesh data when creating meshes from config files.
 */
class Mesh
{
public:
	struct Vertex
	{
		float position[3];
		float normal[3];
		float uv[2];
	};

	typedef std::uint16_t Index;

	typedef std::vector < Vertex > Vertices;
	typedef std::vector < Index > Indices;

	struct MeshData
	{
		Vertices m_vertices;
		Indices m_indices;
	};

	/**
	 * Constructor.
	 *
	 * This creates the OpenGL buffers, but does not fill them with data.
	 *
	 * Note that a valid OpenGL context must be present when this is called.
	 *
	 * @param p_type String denoting the type of the mesh.
	 */
	explicit Mesh(QString p_type);
	/**
	 * Destructor.
	 *
	 * Destroys the OpenGL buffer objects.
	 *
	 * Note that a valid OpenGL context must be present when this is called.
	 */
	~Mesh();

	// Mesh instances may not be copied.
	Mesh(Mesh const &) = delete;
	Mesh& operator = (Mesh const &) = delete;

	/// Returns the mesh type string.
	QString const & getType() const;

	/**
	 * Sets the contents of this mesh.
	 *
	 * This fills the OpenGL vertex and index buffers that were created by
	 * the constructor. Neither p_vertices nor p_indices may be empty.
	 * If the OpenGL buffers are already filled, their old content is
	 * discarded, and the new one filled in.
	 *
	 * The number of indices must be an integer multiple of 3, since three
	 * indices make up one triangle.
	 *
	 * Note that a valid OpenGL context must be present when this is called.
	 *
	 * @param p_vertices STL vector containing vertex data.
	 * @param p_indices STL vector containing index data.
	 */
	void setContents(Vertices const &p_vertices, Indices const &p_indices);
	/**
	 * setContents() convenience overload.
	 *
	 * This sets vertex and index data from a MeshData instance.
	 *
	 * Note that a valid OpenGL context must be present when this is called.
	 *
	 * @param p_meshData MeshData instance containg the vertex and index data.
	 */
	void setContents(MeshData const &p_meshData);
	/**
	 * Clears the contents of the OpenGL buffer objects.
	 *
	 * Note that a valid OpenGL context must be present when this is called.
	 */
	void clearContents();
	/// Returns true if the OpenGL buffers are filled with vertex/index data.
	bool hasContents() const;

	/**
	 * Binds the OpenGL vertex and index buffers to the current OpenGL context.
	 *
	 * This must be called prior to issuing drawing calls so the GPU reads
	 * vertex data from the right place.
	 *
	 * Note that a valid OpenGL context must be present when this is called.
	 */
	void bindBuffers();
	/**
	 * Releases (= unbinds) the OpenGL vertex and index buffers from the current
	 * OpenGL context.
	 *
	 * If the buffers weren't bound earlier, this does nothing.
	 */
	void releaseBuffers();

	/// Return the number of vertices that were set in the setContents() call.
	std::size_t getNumVertices() const;
	/// Return the number of indices that were set in the setContents() call.
	std::size_t getNumIndices() const;


private:
	QString const m_type;
	QOpenGLBuffer m_vertexBuffer, m_indexBuffer;
	std::size_t m_numVertices, m_numIndices;
};

// Define a unique_ptr for the mesh for ownership management.
typedef std::unique_ptr < Mesh > MeshUPtr;


} // namespace qtglviddemo end


#endif

