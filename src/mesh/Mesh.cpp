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


#include "Mesh.hpp"


namespace qtglviddemo
{


Mesh::Mesh(QString p_type)
	: m_type(std::move(p_type))
	, m_vertexBuffer(QOpenGLBuffer::VertexBuffer)
	, m_indexBuffer(QOpenGLBuffer::IndexBuffer)
{
}


Mesh::~Mesh()
{
}


QString const & Mesh::getType() const
{
	return m_type;
}


void Mesh::setContents(Vertices const &p_vertices, Indices const &p_indices)
{
	m_numVertices = p_vertices.size();
	m_numIndices = p_indices.size();

	// Fill the OpenGL buffer objects. Set their usage pattern to StaticDraw,
	// since we'll fill them rarely (usually only once), but will use them
	// for rendering very often.

	m_vertexBuffer.create();
	m_vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
	m_vertexBuffer.bind();
	m_vertexBuffer.allocate(&(p_vertices[0]), p_vertices.size() * sizeof(Vertices::value_type));
	m_vertexBuffer.release();

	m_indexBuffer.create();
	m_indexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
	m_indexBuffer.bind();
	m_indexBuffer.allocate(&(p_indices[0]), p_indices.size() * sizeof(Indices::value_type));
	m_indexBuffer.release();
}


void Mesh::setContents(MeshData const &p_meshData)
{
	setContents(p_meshData.m_vertices, p_meshData.m_indices);
}


void Mesh::clearContents()
{
	m_vertexBuffer.destroy();
	m_indexBuffer.destroy();
}


bool Mesh::hasContents() const
{
	return m_vertexBuffer.isCreated() && m_indexBuffer.isCreated();
}


void Mesh::bindBuffers()
{
	m_vertexBuffer.bind();
	m_indexBuffer.bind();
}


void Mesh::releaseBuffers()
{
	m_vertexBuffer.release();
	m_indexBuffer.release();
}


std::size_t Mesh::getNumVertices() const
{
	return m_numVertices;
}


std::size_t Mesh::getNumIndices() const
{
	return m_numIndices;
}


} // namespace qtglviddemo end
