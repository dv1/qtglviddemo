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
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include "GLVIVDirectTextureExtension.hpp"
#include "VideoMaterialProviderVivante.hpp"


namespace qtglviddemo
{


namespace
{


GLenum toVivPixelFormat(GstVideoFormat p_gstformat)
{
	switch (p_gstformat)
	{
		case GST_VIDEO_FORMAT_I420:  return GL_VIV_I420;
		case GST_VIDEO_FORMAT_YV12:  return GL_VIV_YV12;
		case GST_VIDEO_FORMAT_NV12:  return GL_VIV_NV12;
		case GST_VIDEO_FORMAT_NV21:  return GL_VIV_NV21;
		case GST_VIDEO_FORMAT_YUY2:  return GL_VIV_YUY2;
		case GST_VIDEO_FORMAT_UYVY:  return GL_VIV_UYVY;
		case GST_VIDEO_FORMAT_RGB16: return GL_RGB565;
		case GST_VIDEO_FORMAT_RGBA:  return GL_RGBA;
		case GST_VIDEO_FORMAT_BGRA:  return GL_BGRA_EXT;
		case GST_VIDEO_FORMAT_RGBx:  return GL_RGBA;
		case GST_VIDEO_FORMAT_BGRx:  return GL_BGRA_EXT;
		default: assert(false);
	}
}


} // unnamed namespace end


VideoMaterialProviderVivante::VideoMaterialProviderVivante(QOpenGLContext *p_glcontext)
	: VideoMaterialProvider(p_glcontext, {
		GST_VIDEO_FORMAT_I420,
		GST_VIDEO_FORMAT_YV12,
		GST_VIDEO_FORMAT_NV12,
		GST_VIDEO_FORMAT_NV21,
		GST_VIDEO_FORMAT_YUY2,
		GST_VIDEO_FORMAT_UYVY,
		GST_VIDEO_FORMAT_RGB16,
		GST_VIDEO_FORMAT_RGBA,
		GST_VIDEO_FORMAT_BGRA,
		GST_VIDEO_FORMAT_RGBx,
		GST_VIDEO_FORMAT_BGRx
	})
{
	m_vivFuncs = new VivDirectTextureFuncs(p_glcontext);
}


void VideoMaterialProviderVivante::uploadGstFrame(VideoMaterial &p_videoMaterial, GstVideoFrame &p_vframe)
{
	// Pass on the virtual address, and 0 as the physical address. If
	// we could get the address to the video frame's physically contiguous
	// memory block, we'd pass it on, but the new GstPhysMemory structure
	// has been introduced in 1.12, and it was in -bad there, so it is
	// unstable in 1.12. 1.14 is not out yet. So we do not use it for now.
	// Plus, we do not really need it. It would slightly improve performance
	// if the physical address were set, but the extension is capable of
	// figuring it out from the virtual address.

	GLvoid *virtualAddr = p_vframe.data[0];
	GLuint physicalAddr = ~0U;

	// Map the GstBuffer memory to the texture.
	m_vivFuncs->glTexDirectVIVMap(
		GL_TEXTURE_2D,
		p_videoMaterial.getTotalWidth(), p_videoMaterial.getTotalHeight(),
		toVivPixelFormat(GST_VIDEO_INFO_FORMAT(&(p_vframe.info))),
		&virtualAddr,
		&physicalAddr
	);
	// Invalidate the texture. This is necessary to flush any GPU or CPU
	// cache lines filled with texture data that is now invalid since
	// we changed/created the mapping above.
	m_vivFuncs->glTexDirectInvalidateVIV(GL_TEXTURE_2D);
}


} // namespace qtglviddemo end
