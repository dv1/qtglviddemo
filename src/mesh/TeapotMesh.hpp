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


#ifndef QTGLVIDDEMO_TEAPOT_MESH_HPP
#define QTGLVIDDEMO_TEAPOT_MESH_HPP

#include "Mesh.hpp"


namespace qtglviddemo
{


// Mesh data for the "Utah teapot" 3D object.
Mesh::MeshData const & getTeapotMeshData();


} // namespace qtglviddemo end


#endif
