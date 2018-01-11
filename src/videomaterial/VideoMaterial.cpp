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
#include <QOpenGLFunctions>
#include "VideoMaterial.hpp"


Q_DECLARE_LOGGING_CATEGORY(lcQtGLVidDemo)


#define LONG_STRING_CONST(...) #__VA_ARGS__


namespace qtglviddemo
{


namespace
{


QString const defaultVertexShaderSource = LONG_STRING_CONST(

attribute highp vec3 vertexPosition;
attribute highp vec3 vertexNormal;
attribute highp vec2 vertexTexcoords;

varying highp vec2 texcoordsVariant;
varying highp vec3 normalVariant;

uniform highp mat3 modelviewMatrix;
uniform highp mat4 modelviewprojMatrix;

uniform highp vec4 cropRectangle;
uniform highp mat2 textureRotationMatrix;

void main(void)
{
	gl_Position = modelviewprojMatrix * vec4(vertexPosition, 1.0);
	vec2 uvRotCenter = cropRectangle.zw * 0.5;
	vec2 uv = vertexTexcoords * cropRectangle.zw;
	uv = textureRotationMatrix * (uv - uvRotCenter) + uvRotCenter;
	texcoordsVariant = uv + cropRectangle.xy;
	normalVariant = modelviewMatrix * vertexNormal;
}

); // LONG_STRING_CONST end


QString const defaultFragmentShaderSource = LONG_STRING_CONST(

const vec3 lightVector = vec3(0.0, 0.0, 1.0);

varying highp vec2 texcoordsVariant;
varying highp vec3 normalVariant;

uniform sampler2D videoTexture;

void main(void)
{
	float lighting = clamp(dot(lightVector, normalize(normalVariant)), 0.0, 1.0);
	vec4 texel = texture2D(videoTexture, texcoordsVariant);
	float mask = float(texcoordsVariant.x >= 0.0)
	           * float(texcoordsVariant.y >= 0.0)
	           * float(texcoordsVariant.x <= 1.0)
	           * float(texcoordsVariant.y <= 1.0)
	           ;
	gl_FragColor = vec4(mask * lighting * texel.rgb, 1.0);
}

); // LONG_STRING_CONST end


} // unnamed namespace end




VideoMaterialPrivIFace::~VideoMaterialPrivIFace()
{
}




VideoMaterial::VideoMaterial()
	: m_privIFace(nullptr)
	, m_textureId(0)
{
}


VideoMaterial::VideoMaterial(VideoMaterialPrivIFace &p_privIFace, QOpenGLContext *p_glcontext)
	: m_privIFace(&p_privIFace)
	, m_glcontext(p_glcontext)
	, m_textureId(0)
	, m_curBuffer(nullptr)
	, m_cropRectangle(0.0f, 0.0f, 1.0f, 1.0f)
	, m_textureRotation(0)
{
	assert(p_glcontext != nullptr);
	m_glcontext->functions()->glGenTextures(1, &m_textureId);

	m_glcontext->functions()->glBindTexture(GL_TEXTURE_2D, m_textureId);

	// Set min/mag filter to GL_LINEAR to make sure OpenGL
	// does not attempt to use any mipmapping.
	m_glcontext->functions()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_glcontext->functions()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Set wrap values to GL_CLAMP_TO_EDGE to force the GPU
	// to use the texture's border pixel values for texture
	// coordinates outside of the 0.0-1.0 range.
	m_glcontext->functions()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_glcontext->functions()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	m_glcontext->functions()->glBindTexture(GL_TEXTURE_2D, 0);
}


VideoMaterial::VideoMaterial(VideoMaterial && p_other)
	: m_privIFace(p_other.m_privIFace)
	, m_glcontext(p_other.m_glcontext)
	, m_textureId(p_other.m_textureId)
	, m_curBuffer(p_other.m_curBuffer)
	, m_videoInfo(std::move(p_other.m_videoInfo))
	, m_frameWidth(p_other.m_frameWidth)
	, m_frameHeight(p_other.m_frameHeight)
	, m_totalWidth(p_other.m_totalWidth)
	, m_totalHeight(p_other.m_totalHeight)
	, m_cropRectangle(std::move(p_other.m_cropRectangle))
	, m_textureRotation(p_other.m_textureRotation)
{
	// Mark the other instance as empty for its destructor.
	p_other.m_privIFace = nullptr;
}


VideoMaterial::~VideoMaterial()
{
	if (m_privIFace == nullptr)
		return;

	m_glcontext->functions()->glBindTexture(GL_TEXTURE_2D, 0);
	m_glcontext->functions()->glDeleteTextures(1, &m_textureId);

	if (m_curBuffer != nullptr)
		gst_buffer_unref(m_curBuffer);
}


VideoMaterial& VideoMaterial::operator = (VideoMaterial && p_other)
{
	m_privIFace = p_other.m_privIFace;
	m_glcontext = p_other.m_glcontext;
	m_textureId = p_other.m_textureId;
	m_curBuffer = p_other.m_curBuffer;
	m_videoInfo = std::move(p_other.m_videoInfo);
	m_frameWidth = p_other.m_frameWidth;
	m_frameHeight = p_other.m_frameHeight;
	m_totalWidth = p_other.m_totalWidth;
	m_totalHeight = p_other.m_totalHeight;
	m_cropRectangle = std::move(p_other.m_cropRectangle);
	m_textureRotation = p_other.m_textureRotation;

	// Mark the other instance as empty for its destructor.
	p_other.m_privIFace = nullptr;

	return *this;
}


void VideoMaterial::setShaderUniformValues()
{
	assert(m_privIFace != nullptr);
	m_privIFace->setShaderUniformValues(*this);
}


void VideoMaterial::bind()
{
	assert(m_privIFace != nullptr);
	m_privIFace->bindMaterial(*this);
}


void VideoMaterial::unbind()
{
	assert(m_privIFace != nullptr);
	m_privIFace->unbindTexture();
}


void VideoMaterial::setVideoInfo(GstVideoInfo p_videoInfo)
{
	m_videoInfo = std::move(p_videoInfo);
	m_privIFace->setVideoInfoChangedFlag(true);
}


void VideoMaterial::setVideoGstbuffer(GstBuffer *p_buffer)
{
	assert(m_privIFace != nullptr);
	assert(p_buffer != nullptr);

	// Set the GstBuffer. If a buffer was set previously, m_curBuffer
	gst_buffer_replace(&m_curBuffer, p_buffer);

	// Map the frame. This provides access to a pointer to the frame's pixels
	// and also to frame metadata. gst_video_frame_map() copies the provided
	// video info. If the GstBuffer contains a GstVideoMeta, it then updates
	// its copy with the information from the meta.
	GstVideoFrame vframe;
	gst_video_frame_map(&vframe, &m_videoInfo, m_curBuffer, GST_MAP_READ);

	// Bind the material's texture. Also make sure that texture unit #0 is
	// the one that OpenGL calls here will use.
	m_glcontext->functions()->glActiveTexture(GL_TEXTURE0);
	m_glcontext->functions()->glBindTexture(GL_TEXTURE_2D, m_textureId);

	// Get the frame sizes - the sizes of the subregion of the frame
	// that contains the actual pixels, excluding any padding pixels.
	m_frameWidth = GST_VIDEO_INFO_WIDTH(&(vframe.info));
	m_frameHeight = GST_VIDEO_INFO_HEIGHT(&(vframe.info));

	// Get the total sizes - the sizes of the frame including the padding
	// pixel rows / columns. Provider subclasses typically skip padding
	// pixels by adjusting the texture coordinates to include only the
	// actual frame pixels, because OpenGL texture upload functions usually
	// do not allow for specifying row stride or plane offset values.

	// Calculate the total width by dividing the stride (which is given in
	// bytes) by the number of bytes per pixel in the frame's first plane.
	// For example, with an RGB format, the number of bytes per pixel is 3.
	// With I420, the number is 1.
	m_totalWidth = GST_VIDEO_INFO_PLANE_STRIDE(&(vframe.info), 0) / GST_VIDEO_INFO_COMP_PSTRIDE(&(vframe.info), 0);
	// Calculate the total height by checking how far apart the first and
	// second plane are inside the frame. Any padding rows will be in
	// between them along with the actual frame rows, so, dividing the
	// distance by the stride yields the combined number of frame and
	// padding rows.
	// If there is just one plane though, there is no way to determine
	// the number of padding rows. However, this is irrelevant for formats
	// with a single plane, since the padding row count is necessary to
	// skip padding rows between planes. If there's just one plane, there
	// is no need to skip anything.
	if (GST_VIDEO_INFO_N_PLANES(&(vframe.info)) > 1)
		m_totalHeight = (GST_VIDEO_INFO_PLANE_OFFSET(&(vframe.info), 1) - GST_VIDEO_INFO_PLANE_OFFSET(&(vframe.info), 0)) / GST_VIDEO_INFO_PLANE_STRIDE(&(vframe.info), 0);
	else
		m_totalHeight = GST_VIDEO_INFO_HEIGHT(&(vframe.info));

	// Let the subclass do the actual uploading.
	m_privIFace->uploadGstFrame(*this, vframe);

	// We are done with the frame pixels, unmap.
	gst_video_frame_unmap(&vframe);

	// We are done with the texture, unbind it now.
	m_glcontext->functions()->glBindTexture(GL_TEXTURE_2D, 0);
}


bool VideoMaterial::hasVideoGstbuffer() const
{
	return (m_curBuffer != nullptr);
}


void VideoMaterial::setCropRectangle(QRect p_cropRectangle)
{
	m_cropRectangle = std::move(p_cropRectangle);
}


QRect const & VideoMaterial::getCropRectangle() const
{
	return m_cropRectangle;
}


void VideoMaterial::setTextureRotation(int const p_rotation)
{
	m_textureRotation = p_rotation;

	// Qt has no rotation implementation for its 2x2 matrix datatype,
	// so instead, we calculate the Z rotation in a 4x4 matrix, and
	// copy the top left 2x2 values.
	QMatrix4x4 rotMat;
	rotMat.rotate(float(m_textureRotation), 0.0f, 0.0f, 1.0f);
	m_textureRotationMatrix(0, 0) = rotMat(0, 0);
	m_textureRotationMatrix(0, 1) = rotMat(0, 1);
	m_textureRotationMatrix(1, 0) = rotMat(1, 0);
	m_textureRotationMatrix(1, 1) = rotMat(1, 1);
}


int VideoMaterial::getTextureRotation() const
{
	return m_textureRotation;
}


QMatrix2x2 const & VideoMaterial::getTextureRotationMatrix() const
{
	return m_textureRotationMatrix;
}


guint VideoMaterial::getFrameWidth() const
{
	return m_frameWidth;
}


guint VideoMaterial::getFrameHeight() const
{
	return m_frameHeight;
}


guint VideoMaterial::getTotalWidth() const
{
	return m_totalWidth;
}


guint VideoMaterial::getTotalHeight() const
{
	return m_totalHeight;
}


GLuint VideoMaterial::getTextureId() const
{
	return m_textureId;
}




VideoMaterialProvider::VideoMaterialProvider(QOpenGLContext *p_glcontext, SupportedVideoFormats p_formats, QString const &p_vertexShaderSource, QString const &p_fragmentShaderSource)
	: m_glcontext(p_glcontext)
	, m_formats(std::move(p_formats))
{
	// Set up the shaders.
	m_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, p_vertexShaderSource.isEmpty() ? defaultVertexShaderSource : p_vertexShaderSource);
	m_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, p_fragmentShaderSource.isEmpty() ? defaultFragmentShaderSource : p_fragmentShaderSource);
	m_shaderProgram.link();
	qCDebug(lcQtGLVidDemo) << "Shader program link result:" << m_shaderProgram.log();

	// Bind the program to get the uniform and attribute IDs.

	m_shaderProgram.bind();

	m_cropRectangleUniform = m_shaderProgram.uniformLocation("cropRectangle");
	m_textureRotationMatrixUniform = m_shaderProgram.uniformLocation("textureRotationMatrix");

	m_modelviewMatrixUniform = m_shaderProgram.uniformLocation("modelviewMatrix");
	m_modelviewprojMatrixUniform = m_shaderProgram.uniformLocation("modelviewprojMatrix");
	m_vertexPositionAttrib = m_shaderProgram.attributeLocation("vertexPosition");
	m_vertexNormalAttrib = m_shaderProgram.attributeLocation("vertexNormal");
	m_vertexTexcoordsAttrib = m_shaderProgram.attributeLocation("vertexTexcoords");

	// Instruct the shader to fetch texels from texture unit #0. This is
	// where the video material texture will be bound to.
	m_shaderProgram.setUniformValue("videoTexture", GLint(0));

	m_shaderProgram.release();
}


void VideoMaterialProvider::unbindTexture()
{
	m_glcontext->functions()->glBindTexture(GL_TEXTURE_2D, 0);
}


VideoMaterial VideoMaterialProvider::createVideoMaterial()
{
	return VideoMaterial(*this, m_glcontext);
}


QOpenGLShaderProgram& VideoMaterialProvider::getShaderProgram()
{
	return m_shaderProgram;
}


VideoMaterialProvider::SupportedVideoFormats const & VideoMaterialProvider::getSupportedVideoFormats() const
{
	return m_formats;
}


int VideoMaterialProvider::getModelviewMatrixUniform() const
{
	return m_modelviewMatrixUniform;
}


int VideoMaterialProvider::getModelviewprojMatrixUniform() const
{
	return m_modelviewprojMatrixUniform;
}


int VideoMaterialProvider::getVertexPositionAttrib() const
{
	return m_vertexPositionAttrib;
}


int VideoMaterialProvider::getVertexNormalAttrib() const
{
	return m_vertexNormalAttrib;
}


int VideoMaterialProvider::getVertexTexcoordsAttrib() const
{
	return m_vertexTexcoordsAttrib;
}


void VideoMaterialProvider::bindMaterial(VideoMaterial &p_videoMaterial)
{
	m_glcontext->functions()->glActiveTexture(GL_TEXTURE0);
	m_glcontext->functions()->glBindTexture(GL_TEXTURE_2D, p_videoMaterial.getTextureId());
}


void VideoMaterialProvider::setVideoInfoChangedFlag(bool const)
{
}


void VideoMaterialProvider::setShaderUniformValues(VideoMaterial &p_videoMaterial)
{
	// Calculate crop rectangle values for the shader based on the specified
	// crop rectangle and the ratio between frame and total sizes.
	//
	// We need to skip the padding frame pixels and also make sure only
	// the pixels in the crop rectangle are used. To that end, the crop
	// rectangle's coordinates have to be transformed from the 0-100 to
	// the 0.0-1.0 scale. Then it has to be shrunk by the frame/total
	// size ratio, because the crop rectangle is specified in a coordinate
	// space that does not include the padding pixels, but the texture
	// coordinates are in a space that does include them.

	QRect const & cropRectangle = p_videoMaterial.getCropRectangle();

	// Figure out the frame/total width/height ratio. These values are always
	// less than or equal to 1.0.
	float scaleW = float(p_videoMaterial.getFrameWidth()) / float(p_videoMaterial.getTotalWidth());
	float scaleH = float(p_videoMaterial.getFrameHeight()) / float(p_videoMaterial.getTotalHeight());

	// Transform the rectangle coordinates from the 0-100 to the 0.0-1.0 range.
	float cw = std::min(cropRectangle.width() / 100.0f, 1.0f - cropRectangle.x() / 100.0f);
	float ch = std::min(cropRectangle.height() / 100.0f, 1.0f - cropRectangle.y() / 100.0f);

	// Calculate scaled rectangle coordinates to exclude padding pixels.
	float x = (cropRectangle.x() / 100.0f) * scaleW;
	float y = (cropRectangle.y() / 100.0f) * scaleW;
	float w = cw * scaleW;
	float h = ch * scaleH;

	// Pass on the scaled coordinates to the crop rectangle shader uniform.
	m_shaderProgram.setUniformValue(
		m_cropRectangleUniform,
		x, y, w, h
	);

	// Pass on the texture rotation matrix to the rotation uniform.
	m_shaderProgram.setUniformValue(m_textureRotationMatrixUniform, p_videoMaterial.getTextureRotationMatrix());
}


} // namespace qtglviddemo end
