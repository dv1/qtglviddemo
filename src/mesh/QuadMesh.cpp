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


#include "QuadMesh.hpp"


namespace qtglviddemo
{


namespace
{

Mesh::Vertex const quadVertices[4*2] = {
	{ { -1, -1, 0 }, { 0, 0,  1 }, { 0, 1 } },
	{ {  1, -1, 0 }, { 0, 0,  1 }, { 1, 1 } },
	{ { -1,  1, 0 }, { 0, 0,  1 }, { 0, 0 } },
	{ {  1,  1, 0 }, { 0, 0,  1 }, { 1, 0 } },
	{ { -1, -1, 0 }, { 0, 0, -1 }, { 1, 1 } },
	{ {  1, -1, 0 }, { 0, 0, -1 }, { 0, 1 } },
	{ { -1,  1, 0 }, { 0, 0, -1 }, { 1, 0 } },
	{ {  1,  1, 0 }, { 0, 0, -1 }, { 0, 0 } }
};

Mesh::Index const quadIndices[6*2] = {
	0, 1, 2,
	2, 1, 3,
	4, 6, 5,
	5, 6, 7
};

Mesh::MeshData const quadData = {
	{ quadVertices, quadVertices + sizeof(quadVertices)/sizeof(Mesh::Vertex) },
	{ quadIndices, quadIndices + sizeof(quadIndices)/sizeof(Mesh::Index) }
};

} // unnamed namespace end


Mesh::MeshData const & getQuadMeshData()
{
	return quadData;
}


} // namespace qtglviddemo end
