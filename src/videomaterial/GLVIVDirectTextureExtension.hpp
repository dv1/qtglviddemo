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


#ifndef IMX_GL_VIV_DIRECT_TEXTURE_H
#define IMX_GL_VIV_DIRECT_TEXTURE_H

// Use our own definitions to make sure they are the
// same no matter what Vivante driver version is used
#define GL_VIV_direct_texture 1

#include <KHR/khrplatform.h>
#include <qopengl.h>


#define GL_VIV_YV12  0x8FC0
#define GL_VIV_NV12  0x8FC1
#define GL_VIV_YUY2  0x8FC2
#define GL_VIV_UYVY  0x8FC3
#define GL_VIV_NV21  0x8FC4
#define GL_VIV_I420  0x8FC5


#ifndef GL_APICALL
#define GL_APICALL  KHRONOS_APICALL
#endif

#ifndef GL_APIENTRY
#define GL_APIENTRY KHRONOS_APIENTRY
#endif

#ifndef GL_APIENTRYP
#define GL_APIENTRYP GL_APIENTRY*
#endif


typedef void (GL_APIENTRYP PFNGLTEXDIRECTVIVPROC)           (GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Pixels);
typedef void (GL_APIENTRYP PFNGLTEXDIRECTVIVMAPPROC)        (GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Logical, const GLuint * Physical);
typedef void (GL_APIENTRYP PFNGLTEXDIRECTTILEDMAPVIVPROC)   (GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Logical, const GLuint * Physical);
typedef void (GL_APIENTRYP PFNGLTEXDIRECTINVALIDATEVIVPROC) (GLenum Target);


class QOpenGLContext;


namespace qtglviddemo
{


/**
 * Vivante direct texture extension functions.
 *
 * Do not attempt to create an instance of this class if isVivDirectTextureSupported()
 * returns false. Otherwise, the function pointer values are undefined.
 */
struct VivDirectTextureFuncs
{
	PFNGLTEXDIRECTVIVPROC           glTexDirectVIV;
	PFNGLTEXDIRECTVIVMAPPROC        glTexDirectVIVMap;
	PFNGLTEXDIRECTTILEDMAPVIVPROC   glTexDirectTiledMapVIV;
	PFNGLTEXDIRECTINVALIDATEVIVPROC glTexDirectInvalidateVIV;

	/**
	 * Constructor.
	 *
	 * Sets the pointers to the Vivante direct texture extension functions.
	 * The specified OpenGL context must be valid when this constructor runs.
	 *
	 * @param p_context OpenGL context to use for getting the functions.
	 *        Must not be null.
	 */
	explicit VivDirectTextureFuncs(QOpenGLContext *p_context);
};


/**
 * Checks if the Vivante direct texture extension is supported.
 *
 * This returns true if the extension is supported, false otherwise.
 *
 * The specified OpenGL context must be valid when this function is called.
 *
 * @param p_context OpenGL context to use for checking. Must not be null.
 */
bool isVivDirectTextureSupported(QOpenGLContext *p_context);


} // namespace qtglviddemo end


#endif
