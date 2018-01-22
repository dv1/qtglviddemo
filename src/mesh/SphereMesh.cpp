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
#include <cmath>
#include "SphereMesh.hpp"


namespace qtglviddemo
{


Mesh::MeshData calculateSphereMeshData(float const p_radius, unsigned int const p_latitudeTesselation, unsigned int const p_longitudeTesselation)
{
	/* Calculate a UV sphere. p_longitudeTesselation specifies how many
	 * vertical ring segments shall be calculated. p_latitudeTesselation
	 * does the same for the horizontal segments.
	 *
	 * The vertices of the very first vertical segment are duplicated
	 * and used for the last segment. This is because at the first longitude,
	 * the first and last triangles of the sphere mesh meet, and while
	 * these vertices share the same position and normal vector, they
	 * have different UV coordinates.
	 *
	 * Three indices per triangle are calculated. Triangles are pairwise
	 * grouped to form a quad. So, for each quad, there are 2*3 = 6 indices.
	 * Also, the number of quads in horizontal and vertical direction is
	 * one less than the number of horizontal and vertical ring segments.
	 * This is because a triangle is bounded by segments, like this:
	 *
	 * [vertex] [triangle] [vertex] [triangle] [vertex]
	 */

	// A sphere mesh with less tesselation makes no sense.
	assert(p_latitudeTesselation >= 3);
	assert(p_longitudeTesselation >= 3);

	// Calculate number of vertices and indices.
	// Use p_longitudeTesselation+1 instead of p_longitudeTesselation to
	// make room for the extra vertical segment.
	// And, as said about the indices above, the number of quads in
	// horizontal and vertical direction would be p_latitudeTesselation-1
	// and p_longitudeTesselation-1 , but since we add an extra segment,
	// we use p_longitudeTesselation-1+1 -> p_longitudeTesselation instead.
	Mesh::MeshData meshData;
	meshData.m_vertices.resize(p_latitudeTesselation * (p_longitudeTesselation + 1));
	meshData.m_indices.resize((p_latitudeTesselation - 1) * p_longitudeTesselation * 2 * 3);

	// Calculate vertices by generating latitude rings
	for (unsigned int latitude = 0; latitude < p_latitudeTesselation; ++latitude)
	{
		float latitudeF = float(latitude) / float(p_latitudeTesselation - 1);
		float latAngle = latitudeF * M_PI;
		float y = std::cos(latAngle);
		float latitudeRadius = std::sin(latAngle);

		// Go through all the longitudes, producing one vertex per each.
		// These make up the latitude ring segment.
		for (unsigned int longitude = 0; longitude < (p_longitudeTesselation + 1); ++longitude)
		{
			Mesh::Vertex & vtx = meshData.m_vertices[latitude * (p_longitudeTesselation + 1) + longitude];
			
			float longitudeF = float(longitude) / float(p_longitudeTesselation);
			float longAngle = longitudeF * 2.0 * M_PI;
			float x = std::cos(longAngle);
			float z = std::sin(longAngle);
			vtx.position[0] = x * latitudeRadius;
			vtx.position[1] = y * p_radius;
			vtx.position[2] = z * latitudeRadius;
			vtx.normal[0] = x;
			vtx.normal[1] = y;
			vtx.normal[2] = z;
			vtx.uv[0] = 1.0f - longitudeF; // Necessary because otherwise the texture is flipped in X direction. 
			vtx.uv[1] = latitudeF;
		}
	}

	for (unsigned int latitude = 0; latitude < (p_latitudeTesselation - 1); ++latitude)
	{
		unsigned int latAVtxOfs = (p_longitudeTesselation + 1) * latitude;
		unsigned int latBVtxOfs = (p_longitudeTesselation + 1) * (latitude + 1);

		for (unsigned int longitude = 0; longitude < p_longitudeTesselation; ++longitude)
		{
			Mesh::Index * idx = &(meshData.m_indices[(latitude * p_longitudeTesselation + longitude) * 2 * 3]);

			idx[0] = latAVtxOfs + longitude;
			idx[1] = latAVtxOfs + longitude + 1;
			idx[2] = latBVtxOfs + longitude;

			idx[3] = latBVtxOfs + longitude;
			idx[4] = latAVtxOfs + longitude + 1;
			idx[5] = latBVtxOfs + longitude + 1;
		}
	}

	return meshData;
}


} // namespace qtglviddemo end
