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


#ifndef QTGLVIDDEMO_VIDEO_MATERIAL_PROVIDER_VIVANTE_HPP
#define QTGLVIDDEMO_VIDEO_MATERIAL_PROVIDER_VIVANTE_HPP

#include "VideoMaterial.hpp"


namespace qtglviddemo
{


class VivDirectTextureFuncs;


/**
 * Video material provider subclass which uploads video frames with Vivante direct textures.
 *
 * This provider can only be used if the isVivDirectTextureSupported() function
 * (declared in GLVIVDirectTextureExtension.hpp) returns true. Otherwise, the
 * Vivante direct texture extension is not present, and this provider won't work.
 *
 * This provider does not actually upload frame pixels into the texture. Instead,
 * it associates the video frame pixels stored in the GstBuffer with the texture,
 * meaning that during rendering, texels are directly fetched from the GstBuffer's
 * memory block. If the GstBuffer's memory block is physically contiguous, this
 * mapping will cause the GPU to fetch the texels via DMA. In addition, the
 * Vivante direct textures support transparent YUV->RGB conversions. In sum, they
 * are an efficient way to display videos as textures, since there is little CPU
 * work done (no pixels are copied, no color space conversion is done by the CPU).
 * If the hardware has a Vivante GPU, and the closed-source Vivante drivers are
 * installed, use this provider. Do NOT use this provider if the system uses the
 * open-source etnaviv driver instead of the closed-source Vivante ones, since
 * etnaviv does not expose the Vivante direct texture extension.
 */
class VideoMaterialProviderVivante
	: public VideoMaterialProvider
{
public:
	explicit VideoMaterialProviderVivante(QOpenGLContext *p_glcontext);

private:
	virtual void uploadGstFrame(VideoMaterial &p_videoMaterial, GstVideoFrame &p_vframe) override;

	VivDirectTextureFuncs *m_vivFuncs;
};


} // namespace qtglviddemo end


#endif
