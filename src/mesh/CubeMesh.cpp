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


#include "CubeMesh.hpp"


namespace qtglviddemo
{


namespace
{

Mesh::Vertex const cubeVertices[4*6] = {
	{ {  1, -1, -1 }, {  0,  0, -1 }, {  0,  1 } },
	{ { -1, -1, -1 }, {  0,  0, -1 }, {  1,  1 } },
	{ {  1,  1, -1 }, {  0,  0, -1 }, {  0,  0 } },
	{ { -1,  1, -1 }, {  0,  0, -1 }, {  1,  0 } },

	{ { -1, -1,  1 }, {  0,  0,  1 }, {  0,  1 } },
	{ {  1, -1,  1 }, {  0,  0,  1 }, {  1,  1 } },
	{ { -1,  1,  1 }, {  0,  0,  1 }, {  0,  0 } },
	{ {  1,  1,  1 }, {  0,  0,  1 }, {  1,  0 } },

	{ { -1, -1, -1 }, {  0, -1,  0 }, {  1,  0 } },
	{ {  1, -1, -1 }, {  0, -1,  0 }, {  0,  0 } },
	{ { -1, -1,  1 }, {  0, -1,  0 }, {  1,  1 } },
	{ {  1, -1,  1 }, {  0, -1,  0 }, {  0,  1 } },

	{ {  1,  1, -1 }, {  0,  1,  0 }, {  1,  0 } },
	{ { -1,  1, -1 }, {  0,  1,  0 }, {  0,  0 } },
	{ {  1,  1,  1 }, {  0,  1,  0 }, {  1,  1 } },
	{ { -1,  1,  1 }, {  0,  1,  0 }, {  0,  1 } },

	{ { -1, -1,  1 }, { -1,  0,  0 }, {  1,  1 } },
	{ { -1,  1,  1 }, { -1,  0,  0 }, {  1,  0 } },
	{ { -1, -1, -1 }, { -1,  0,  0 }, {  0,  1 } },
	{ { -1,  1, -1 }, { -1,  0,  0 }, {  0,  0 } },

	{ {  1, -1, -1 }, {  1,  0,  0 }, {  1,  1 } },
	{ {  1,  1, -1 }, {  1,  0,  0 }, {  1,  0 } },
	{ {  1, -1,  1 }, {  1,  0,  0 }, {  0,  1 } },
	{ {  1,  1,  1 }, {  1,  0,  0 }, {  0,  0 } }
};

Mesh::Index const cubeIndices[3*2*6] = {
	 0,  1,  2,     2,  1,  3,
	 4,  5,  6,     6,  5,  7,
	 8,  9, 10,    10,  9, 11,
	12, 13, 14,    14, 13, 15,
	16, 17, 18,    18, 17, 19,
	20, 21, 22,    22, 21, 23
};

Mesh::MeshData const cubeData = {
	{ cubeVertices, cubeVertices + sizeof(cubeVertices)/sizeof(Mesh::Vertex) },
	{ cubeIndices, cubeIndices + sizeof(cubeIndices)/sizeof(Mesh::Index) }
};

} // unnamed namespace end


Mesh::MeshData const & getCubeMeshData()
{
	return cubeData;
}


} // namespace qtglviddemo end
