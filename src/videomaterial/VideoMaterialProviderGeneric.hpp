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


#ifndef QTGLVIDDEMO_VIDEO_MATERIAL_PROVIDER_GENERIC_HPP
#define QTGLVIDDEMO_VIDEO_MATERIAL_PROVIDER_GENERIC_HPP

#include "VideoMaterial.hpp"


namespace qtglviddemo
{


/**
 * Video material provider subclass which uploads video frames with glTex(Sub)Image2D.
 *
 * This is considered a "generic" provider because all OpenGL implementations support
 * the glTexImage2D() and glTexSubImage2D() functions. So, if the implementation has
 * no specialized video frame upload functions, this can be used as a fallback.
 * However, specialized providers should always be preferred, since (a) glTexImage2D()
 * and glTexSubImage2D() only support RGB formats, forcing pixel format conversions
 * prior to uploading, and (b) they copy the video frame pixels, which requires CPU work.
 *
 * Since, as said, these functions only support RGB(A) data, the list of supported
 * formats contains only one entry, GST_VIDEO_FORMAT_RGBx .
 */
class VideoMaterialProviderGeneric
	: public VideoMaterialProvider
{
public:
	explicit VideoMaterialProviderGeneric(QOpenGLContext *p_glcontext);

private:
	virtual void setVideoInfoChangedFlag(bool const p_flag) override;
	virtual void uploadGstFrame(VideoMaterial &p_videoMaterial, GstVideoFrame &p_vframe) override;

	bool m_videoInfoChanged;
};


} // namespace qtglviddemo end


#endif
