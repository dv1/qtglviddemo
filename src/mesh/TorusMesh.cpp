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
#include "TorusMesh.hpp"


namespace qtglviddemo
{


Mesh::MeshData calculateTorusMeshData(float const p_majorRadius, float const p_minorRadius, unsigned int const p_majorTesselation, unsigned int const p_minorTesselation)
{
	/* Calculate a torus. p_majorRadius is the overall radius of the torus,
	 * while p_minorRadius is the radius of the torus' tube. Likewise,
	 * p_majorTesselation and p_minorTesselation specify the level of
	 * tesselation across the torus and the torus tube, respectively.
	 *
	 * The torus mesh is made of ring segments that make up the tube
	 * sections.
	 *
	 * The vertices of the very first section are duplicated and used
	 * for the last section. This is because at the first section,
	 * the first and last triangles of the sphere mesh meet, and while
	 * these vertices share the same position and normal vector, they
	 * have different UV coordinates.
	 *
	 * Three indices per triangle are calculated. Triangles are pairwise
	 * grouped to form a quad. So, for each quad, there are 2*3 = 6 indices.
	 * Quads are inserted between torus sections.
	 */

	// A torus mesh with less tesselation makes no sense.
	assert(p_majorTesselation >= 4);
	assert(p_minorTesselation >= 3);

	Mesh::MeshData meshData;
	meshData.m_vertices.resize((p_majorTesselation + 1) * p_minorTesselation);
	meshData.m_indices.resize(p_majorTesselation * p_minorTesselation * 2 * 3);

	// Calculate vertices by generating sections
	for (unsigned int majorI = 0; majorI < (p_majorTesselation + 1); ++majorI)
	{
		unsigned baseOfs = majorI * p_minorTesselation;

		float majorF = float(majorI) / float(p_majorTesselation);
		float majorAngle = majorF * 2.0f * M_PI;
		float majorX = std::cos(majorAngle);
		float majorZ = std::sin(majorAngle);

		// Calculate the sections of the current section
		for (unsigned int minorI = 0; minorI < p_minorTesselation; ++minorI)
		{
			Mesh::Vertex & vtx = meshData.m_vertices[baseOfs + minorI];

			float minorF = float(minorI) / float(p_minorTesselation);
			float minorAngle = minorF * 2.0f * M_PI;

			// Reverse X direction for correct backface culling
			// and V texture coordinate direction.
			float minorX = -std::cos(minorAngle);
			float minorY =  std::sin(minorAngle);

			// The torus tube section ring is oriented along the
			// normal vector. Compute the X and Z position coordinates
			// by applying this "section radius" to the overall torus
			// radius.
			vtx.position[0] = majorX * (p_majorRadius + minorX * p_minorRadius);
			vtx.position[1] = minorY * p_minorRadius;
			vtx.position[2] = majorZ * (p_majorRadius + minorX * p_minorRadius);
			vtx.normal[0] = majorX * minorX;
			vtx.normal[1] = minorY;
			vtx.normal[2] = majorZ * minorX;
			// Make the texture repeat itself 4 times, otherwise it
			// looks too "stretched". Also flip the coordinate
			// direction, otherwise the texture looks flipped in
			// the X direction.
			vtx.uv[0] = (1.0f - majorF) * 4.0f;
			vtx.uv[1] = minorF;
		}
	}

	for (unsigned int majorI = 0; majorI < p_majorTesselation; ++majorI)
	{
		unsigned int ringAVtxOfs = p_minorTesselation * majorI;
		unsigned int ringBVtxOfs = p_minorTesselation * (majorI + 1);

		for (unsigned int minorI = 0; minorI < p_minorTesselation; ++minorI)
		{
			Mesh::Index * idx = &(meshData.m_indices[(majorI * p_minorTesselation + minorI) * 2 * 3]);
			unsigned int minorI2 = (minorI + 1) % p_minorTesselation;

			idx[0] = ringAVtxOfs + minorI;
			idx[1] = ringBVtxOfs + minorI;
			idx[2] = ringAVtxOfs + minorI2;

			idx[3] = ringAVtxOfs + minorI2;
			idx[4] = ringBVtxOfs + minorI;
			idx[5] = ringBVtxOfs + minorI2;
		}
	}

	return meshData;
}


} // namespace qtglviddemo end
