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
#include <QDebug>
#include <QLoggingCategory>
#include <QOpenGLContext>
#include "GLResources.hpp"
#include "videomaterial/VideoMaterialProviderGeneric.hpp"
#include "videomaterial/VideoMaterialProviderVivante.hpp"
#include "videomaterial/GLVIVDirectTextureExtension.hpp"
#include "mesh/CubeMesh.hpp"
#include "mesh/QuadMesh.hpp"
#include "mesh/TeapotMesh.hpp"


Q_DECLARE_LOGGING_CATEGORY(lcQtGLVidDemo)


namespace qtglviddemo
{


static GLResources *inst = nullptr;


GLResources::GLResources(QOpenGLContext *p_glcontext)
{
	// This constructor is called when the singleton instance is created.

	// Create the video material provider.
#ifdef WITH_VIV_GPU
	// On platforms with a Vivante GPU, try to create a Vivante video
	// material provider first.
	if (isVivDirectTextureSupported(p_glcontext))
	{
		qCDebug(lcQtGLVidDemo) << "Vivante direct textures supported - using Vivante video material provider";
		m_videoMaterialProvider = VideoMaterialProviderUPtr(new VideoMaterialProviderVivante(p_glcontext));
	}
	else
#endif
	{
		qCDebug(lcQtGLVidDemo) << "using generic video material provider";
		m_videoMaterialProvider = VideoMaterialProviderUPtr(new VideoMaterialProviderGeneric(p_glcontext));
	}
}


GLResources::~GLResources()
{
	// Destroy all allocated meshes by clearing the map.
	m_meshMap.clear();
	// Destroy the video material provider.
	m_videoMaterialProvider.reset();
}


QOpenGLVertexArrayObject & GLResources::getVAO()
{
	return m_vao;
}


VideoMaterialProvider & GLResources::getVideoMaterialProvider()
{
	assert(m_videoMaterialProvider);
	return *m_videoMaterialProvider;
}


Mesh & GLResources::getMesh(QString const &p_meshType)
{
	auto iter = m_meshMap.find(p_meshType);
	if (iter == m_meshMap.end())
	{
		MeshUPtr newMesh(new Mesh(p_meshType));

		if (p_meshType == "quad")
			newMesh->setContents(getQuadMeshData());
		else if (p_meshType == "cube")
			newMesh->setContents(getCubeMeshData());
		else if (p_meshType == "teapot")
			newMesh->setContents(getTeapotMeshData());

		auto retval = m_meshMap.emplace(p_meshType, std::move(newMesh));
		iter = retval.first;
	}

	return *(iter->second);
}


GLResources& GLResources::instance()
{
	if (inst == nullptr)
	{
		QOpenGLContext *glctx = QOpenGLContext::currentContext();

		qCDebug(lcQtGLVidDemo) << "Setting up shared OpenGL resources";
		inst = new GLResources(glctx);

		// Connect the aboutToBeDestroyed() signal. This is the only
		// reliable way to detect when to destroy resources, since
		// the QQuickWindow's sceneGraphInvalidated() may not be
		// emitted on all platforms.
		connect(glctx, &QOpenGLContext::aboutToBeDestroyed, inst, &GLResources::teardownSingletonInstance, Qt::DirectConnection);

	}
	return *inst;
}


void GLResources::teardownSingletonInstance()
{
	if (inst != nullptr)
	{
		qCDebug(lcQtGLVidDemo) << "Tearing down shared OpenGL resources";
		delete inst;
		inst = nullptr;
	}
}


} // namespace qtglviddemo end
