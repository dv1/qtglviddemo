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
#include <QByteArray>
#include <QDebug>
#include <QLoggingCategory>
#include <QOpenGLContext>
#include "GLVIVDirectTextureExtension.hpp"


Q_DECLARE_LOGGING_CATEGORY(lcQtGLVidDemo)


namespace qtglviddemo
{


VivDirectTextureFuncs::VivDirectTextureFuncs(QOpenGLContext *p_context)
{
	assert(p_context != nullptr);

	glTexDirectVIV           = reinterpret_cast < PFNGLTEXDIRECTVIVPROC >           (p_context->getProcAddress(QByteArray("glTexDirectVIV")));
	glTexDirectVIVMap        = reinterpret_cast < PFNGLTEXDIRECTVIVMAPPROC >        (p_context->getProcAddress(QByteArray("glTexDirectVIVMap")));
	glTexDirectTiledMapVIV   = reinterpret_cast < PFNGLTEXDIRECTTILEDMAPVIVPROC >   (p_context->getProcAddress(QByteArray("glTexDirectTiledMapVIV")));
	glTexDirectInvalidateVIV = reinterpret_cast < PFNGLTEXDIRECTINVALIDATEVIVPROC > (p_context->getProcAddress(QByteArray("glTexDirectInvalidateVIV")));
}


bool isVivDirectTextureSupported(QOpenGLContext *p_context)
{
	assert(p_context != nullptr);

	// Newer Vivante drivers call the extension GL_VIV_tex_direct instead
	// of GL_VIV_direct_texture, even though it is the same extension.
	if (p_context->hasExtension(QByteArray("GL_VIV_direct_texture")))
	{
		qCDebug(lcQtGLVidDemo) << "GL_VIV_direct_texture supported";
		return true;
	}
	else if (p_context->hasExtension(QByteArray("GL_VIV_tex_direct")))
	{
		qCDebug(lcQtGLVidDemo) << "GL_VIV_tex_direct supported";
		return true;
	}
	else
	{
		qCDebug(lcQtGLVidDemo) << "Neither GL_VIV_direct_texture nor GL_VIV_tex_direct supported";
		return false;
	}
}


} // namespace qtglviddemo end
