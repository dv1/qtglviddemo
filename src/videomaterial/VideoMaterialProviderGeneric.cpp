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


#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include "VideoMaterialProviderGeneric.hpp"


namespace qtglviddemo
{


VideoMaterialProviderGeneric::VideoMaterialProviderGeneric(QOpenGLContext *p_glcontext)
	: VideoMaterialProvider(p_glcontext, { GST_VIDEO_FORMAT_RGBx })
	, m_videoInfoChanged(true)
{
}


void VideoMaterialProviderGeneric::setVideoInfoChangedFlag(bool const p_flag)
{
	m_videoInfoChanged = p_flag;
}


void VideoMaterialProviderGeneric::uploadGstFrame(VideoMaterial &p_videoMaterial, GstVideoFrame &p_vframe)
{
	// Call glTexImage2D() if the video info changed or if this the
	// first upload call. Otherwise, call glTexSubImage2D(); which
	// is faster because it does not have to reallocate the texture.

	if (m_videoInfoChanged)
	{
		m_glcontext->functions()->glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGBA,
			p_videoMaterial.getTotalWidth(), p_videoMaterial.getTotalHeight(),
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			p_vframe.data[0]
		);

		m_videoInfoChanged = false;
	}
	else
	{
		m_glcontext->functions()->glTexSubImage2D(
			GL_TEXTURE_2D,
			0,
			0, 0,
			p_videoMaterial.getTotalWidth(), p_videoMaterial.getTotalHeight(),
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			p_vframe.data[0]
		);
	}
}


} // namespace qtglviddemo end
