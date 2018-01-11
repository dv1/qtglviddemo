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


#ifndef QTGLVIDDEMO_VIDEO_MATERIAL_HPP
#define QTGLVIDDEMO_VIDEO_MATERIAL_HPP

#include <vector>
#include <gst/gst.h>
#include <gst/video/video.h>
#include <qopengl.h>
#include <QOpenGLShaderProgram>
#include <QRectF>
#include <QMatrix4x4>


namespace qtglviddemo
{


class VideoMaterial;


// Private interface between video material and video material provider.
struct VideoMaterialPrivIFace
{
public:
	virtual ~VideoMaterialPrivIFace();
	virtual void bindMaterial(VideoMaterial &p_videoMaterial) = 0;
	virtual void unbindTexture() = 0;
	virtual void setVideoInfoChangedFlag(bool const p_flag) = 0;
	virtual void setShaderUniformValues(VideoMaterial &p_videoMaterial) = 0;
	virtual void uploadGstFrame(VideoMaterial &p_videoMaterial, GstVideoFrame &p_vframe) = 0;
};


/**
 * Class containing states and a texture for video frames.
 *
 * A "video material" in this demo is a set of OpenGL resources and states that,
 * combined, can be used to render video frames as textures with the GPU, with
 * some states such as rotation applied.
 *
 * VideoMaterial is designed to accept video frames stored in GStreamer GstBuffers.
 * Width, height, and pixel format are taken from a GstVideoInfo instance. GStreamer
 * dataflow will first produce caps, then data. The caps can be converted to
 * a GstVideoInfo, and this info can then be passed to the VideoMaterial instance
 * using setVideoInfo().
 *
 * The actual data follows. GstBuffers with new frames are pushed into VideoMaterial
 * by using the setVideoGstbuffer() function. This function increases the buffer's
 * reference count. If a buffer was set previously already, this old buffer is
 * unreferenced. setVideoGstbuffer() also (re)creates the internal texture to conform
 * to the information in the GstVideoInfo instance that was provided to setVideoInfo().
 * If the GstBuffer has metadata (such as the GstVideoMeta), it will also be used.
 * (If GstVideoMeta is present, and has different values than GstVideoInfo, the former
 * overrides the latter.) If the texture was recreated because something about the
 * format changed, an internal flag is set, and hasVideoInfoChanged() will return true.
 *
 * VideoMaterial holds a reference to the latest passed GstBuffer because some texture
 * upload mechanisms may not actually upload the pixels, but instead set up the texture
 * to use the GstBuffer's memory block as the backing store for the texels. So, with
 * these mechanism, every time the GPU needs to fetch a texel, it does so from this
 * GstBuffer's memory block. By ref'ing the GstBuffer, it is made sure that the memory
 * block stays valid and tha the data isn't written over while the GPU reads from it.
 *
 * The shader associated with this material supports cropping and texture rotation.
 * The crop rectangle's coordinate ranges are 0-100 in both x and y direction. (We
 * can't use floating point coordinates in the 0.0-1.0 range, because Qt Quick UI
 * spinbox controls use integer values, and converting between the two scales can
 * cause problems when trying to compare rectangles to check for modifications.)
 * The texture rotation is defined with a degree as integer value in the 0-359 range.
 *
 * Default crop rectangle is x 0 y 0 width 100 height 100. Default texture rotation
 * angle is 0.
 *
 * The video material has "frame" width/height and "total" width/height. The difference
 * is that the latter include padding colums/rows, the former doesn't. Padding rows
 * and columsn are typically added to video frames when video codecs require width
 * and height sizes that are an integer multiple of a certain value. For example,
 * often, they have to be a multiple of 16. If however the frame width is, say, 509
 * pixels, then this is a problem, because 509 isn't an integer multiple of 16. 512,
 * however, is. So, encoders add 3 extra padding columns to reach a width of 512 pixels.
 * The shader needs to know about the number of padding rows/columns so that it
 * restricts the texture coordinates to exclude these padding pixels.
 *
 * This class is typically not instantiated outside of the VideoMaterialProvider.
 * Use VideoMaterialProvider's createVideoMaterial() for creating an instance.
 *
 * The specifics about texture pixel uploads aren't defined by this class, but
 * by VideoMaterialProvider subclasses.
 */
class VideoMaterial
{
public:
	/**
	 * Default constructor.
	 *
	 * Sets up an empty video material instance with no connection to any video
	 * material provider. This one is mostly useful as a target for a move
	 * assignment. This constructor also does not allocate and OpenGL resources,
	 * so it can be used for creating an instance before any OpenGL context
	 * exists.
	 */
	VideoMaterial();
	/**
	 * Constructor.
	 *
	 * Sets up an empty video material connected to a video material provider.
	 * The video material can access the provider through the given interface
	 * reference. It also expects a Qt OpenGL context object for allocating
	 * OpenGL resources.
	 *
	 * @param p_privIFace Interface to a video material provider.
	 * @param p_glcontext Qt OpenGL context object pointer. Must not be null.
	 */
	explicit VideoMaterial(VideoMaterialPrivIFace &p_privIFace, QOpenGLContext *p_glcontext);
	/// Move constructor.
	VideoMaterial(VideoMaterial && p_other);
	/**
	 * Destructor.
	 *
	 * Destroys any allocated OpenGL resources, so make sure the OpenGL context
	 * object that was passed to the constructor is still valid when this
	 * destructor is executed.
	 */
	~VideoMaterial();

	/// Move assignment operator.
	VideoMaterial& operator = (VideoMaterial && p_other);

	/**
	 * Sets the value of the shader uniforms associated with this material.
	 *
	 * These are the uniform values containing crop rectangle coordinates,
	 * texture rotation information, etc. This is called after bind() and
	 * before rendering.
	 */
	void setShaderUniformValues();
	/**
	 * Binds this material to the current OpenGL context.
	 *
	 * This must be called before rendering, otherwise this material isn't
	 * used during the rendering process.
	 *
	 * The provider's OpenGL context must be valid when this is called.
	 */
	void bind();
	/**
	 * Unbind this material from the current OpenGL context.
	 *
	 * If this material wasn't bound, this function does nothing.
	 *
	 * The provider's OpenGL context must be valid when this is called.
	 */
	void unbind();

	/**
	 * Defines the format of the video material's texture.
	 *
	 * The given GStreamer GstVideoInfo structure contains the width, height,
	 * row stride, plane offsets, and pixel format the texture must use. The
	 * Other GstVideoInfo fields are ignored.
	 *
	 * Note that the values in the video info might be overridden if a
	 * GstBuffer passed to setVideoGstbuffer() contains GstVideoMeta metadata.
	 * Essentially, the video info contains the "default" video information,
	 * and GstVideoMeta metadata can contain individual exceptions to these
	 * default values. Frames of different size, different padding row/column
	 * count etc.
	 *
	 * @param p_videoInfo GstVideoInfo containing the format information.
	 */
	void setVideoInfo(GstVideoInfo p_videoInfo);

	/**
	 * Set the GstBuffer containing the video frame to be rendered.
	 *
	 * If previously, a GstBuffer was already set, that old buffer is
	 * unref'd first.
	 *
	 * You must call setVideoInfo() prior to calling this, otherwise there
	 * won't be an allocated texture to fill pixels into (or associate with).
	 * This also means that the provider's OpenGL context must be valid
	 * when this is called.
	 *
	 * @param p_buffer Pointer to the GstBuffer. Must not be null.
	 */
	void setVideoGstbuffer(GstBuffer *p_buffer);
	/**
	 * Returns true if a GstBuffer has been previously set, false otherwise.
	 *
	 * If this returns false, this material cannot be used for rendering.
	 */
	bool hasVideoGstbuffer() const;

	/**
	 * Sets the crop rectangle.
	 *
	 * The coordinates will be copied to the corresponding shader uniform
	 * by the setShaderUniformValues() call. The valid range of the
	 * integer coordinates is 0-100.
	 *
	 * @param p_cropRectangle Crop rectangle to use.
	 */
	void setCropRectangle(QRect p_cropRectangle);
	/// Returns the currently used crop rectangle.
	QRect const & getCropRectangle() const;

	/**
	 * Sets the texture rotation angle.
	 *
	 * This also recalculates an internal 2x2 rotation matrix that is
	 * used by the shader to rotate the texture coordinates.
	 *
	 * @param Rotation angle. Valid range is 0-359.
	 */
	void setTextureRotation(int const p_rotation);
	/// Returns the texture rotation angle.
	int getTextureRotation() const;
	/**
	 * Returns the texture rotation matrix.
	 *
	 * This 2x2 matrix is calculated in the setTextureRotation() function.
	 * Default value is the identity matrix.
	 */
	QMatrix2x2 const & getTextureRotationMatrix() const;

	/**
	 * Returns the current frame width.
	 *
	 * This value is set by setVideoInfo(), and if a GstBuffer contains
	 * GstVideoMeta metadata, by setVideoGstbuffer().
	 */
	guint getFrameWidth() const;
	/**
	 * Returns the current frame height.
	 *
	 * This value is set by setVideoInfo(), and if a GstBuffer contains
	 * GstVideoMeta metadata, by setVideoGstbuffer().
	 */
	guint getFrameHeight() const;

	/**
	 * Returns the current total width.
	 *
	 * This value is set by setVideoInfo(), and if a GstBuffer contains
	 * GstVideoMeta metadata, by setVideoGstbuffer().
	 */
	guint getTotalWidth() const;
	/**
	 * Returns the current total height.
	 *
	 * This value is set by setVideoInfo(), and if a GstBuffer contains
	 * GstVideoMeta metadata, by setVideoGstbuffer().
	 */
	guint getTotalHeight() const;

	/**
	 * Get the ID (or "name" in OpenGL jargon) of the allocated OpenGL
	 * texture.
	 *
	 * This ID is 0 if the video material was created by the default
	 * constructor, nonzero otherwise.
	 */
	GLuint getTextureId() const;


	/// VideoMaterial is movable but not copyable.
	VideoMaterial(VideoMaterial const & p_other) = delete;
	VideoMaterial& operator = (VideoMaterial const & p_other) = delete;


private:
	VideoMaterialPrivIFace *m_privIFace;
	QOpenGLContext *m_glcontext;

	GLuint m_textureId;
	GstBuffer *m_curBuffer;

	GstVideoInfo m_videoInfo;
	guint m_frameWidth, m_frameHeight;
	guint m_totalWidth, m_totalHeight;

	QRect m_cropRectangle;
	int m_textureRotation;

	QMatrix2x2 m_textureRotationMatrix;
};


/**
 * Class for providing video material instances and accompanying shaders and states.
 *
 * This is the class to use for creating video material instances. It also contains
 * the OpenGL shaders that are used for rendering, and provides shader uniform IDs
 * for setting shader uniform values.
 *
 * Only one instance of the video material provider is necessary per OpenGL context.
 * To support different video streams rendered as OpenGL texture, call
 * createVideoMaterial() for each video stream.
 *
 * Subclasses have to at least define the uploadGstFrame() function (declared in
 * VideoMaterialPrivIFace).
 */
class VideoMaterialProvider
	: protected VideoMaterialPrivIFace
{
public:
	typedef std::vector < GstVideoFormat > SupportedVideoFormats;

	/**
	 * Unbinds the texture currently bound to the OpenGL context.
	 *
	 * Don't call this directly. Use VideoMaterial::bind() instead.
	 *
	 * Default implementation calls glBindTexture() with texture ID 0.
	 */
	virtual void unbindTexture() override;
	/**
	 * Creates a video material instance, associated with this provider.
	 *
	 * This creates the internal video material texture, but does not
	 * fill it. So, the provider's OpenGL context must be valid when
	 * this is called.
	 */
	virtual VideoMaterial createVideoMaterial();

	/**
	 * Returns the OpenGL shader program to use for rendering with
	 * video materials associated with this provider.
	 */
	QOpenGLShaderProgram& getShaderProgram();
	/**
	 * Returns a list of GStreamer video formats that can be used
	 * for uploading to video material textures.
	 *
	 * Callers can use this list to set up format restrictions in
	 * media players. The player's pipeline then chooses one of
	 * the formats from this list for the video frames that are
	 * passed to the video matetial's setVideoGstbuffer() call.
	 *
	 * This list must not be empty.
	 */
	SupportedVideoFormats const & getSupportedVideoFormats() const;

	// Shader uniform IDs for matrix uniforms.
	int getModelviewMatrixUniform() const;
	int getModelviewprojMatrixUniform() const;

	// IDs for vertex attributes.
	int getVertexPositionAttrib() const;
	int getVertexNormalAttrib() const;
	int getVertexTexcoordsAttrib() const;


protected:
	VideoMaterialProvider(QOpenGLContext *p_glcontext, SupportedVideoFormats p_formats, QString const &p_vertexShaderSource = "", QString const &p_fragmentShaderSource = "");

	virtual void bindMaterial(VideoMaterial &p_videoMaterial) override;
	virtual void setVideoInfoChangedFlag(bool const p_flag) override;
	virtual void setShaderUniformValues(VideoMaterial &p_videoMaterial) override;

	QOpenGLContext *m_glcontext;
	QOpenGLShaderProgram m_shaderProgram;
	SupportedVideoFormats m_formats;

	int m_cropRectangleUniform;
	int m_textureRotationMatrixUniform;

	int m_modelviewMatrixUniform;
	int m_modelviewprojMatrixUniform;
	int m_vertexPositionAttrib;
	int m_vertexNormalAttrib;
	int m_vertexTexcoordsAttrib;
};


} // namespace qtglviddemo end


#endif
