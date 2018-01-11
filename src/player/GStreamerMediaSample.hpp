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


#ifndef QTGLVIDDEMO_GSTREAMER_MEDIA_SAMPLE_HPP
#define QTGLVIDDEMO_GSTREAMER_MEDIA_SAMPLE_HPP

#include <gst/gst.h>
#include <gst/video/video.h>


namespace qtglviddemo
{


/**
 * Class containing a GstSample.
 *
 * A GstSample is returned by GStreamer appsinks. It contains a GstBuffer and
 * extra metadata such as caps and segment information. For this demo program,
 * the GstBuffer and the caps are of interest. The caps describe the format of
 * the data in the GstBuffer.
 *
 * The reason why this class exists is to encapsulate GstSamples in a lightweight
 * object that unrefs the GstSample in the destructor, adhering to the RAII
 * principle. It also has a flag that denotes if this sample's caps are new, or
 * if these are the same caps a previous GstSample had. This is useful to check
 * if something has to be reconfigured (OpenGL textures for example if the
 * GstSample contains a video frame and the width and height changed).
 */
class GStreamerMediaSample
{
public:
	/**
	 * Constructor.
	 *
	 * @param p_sample Sample pointer to store in this object.
	 * @param p_sampleHasNewCaps If true, then the sample's caps are new
	 *        (= they are different compared to a previous sample's caps).
	 */
	explicit GStreamerMediaSample(GstSample *p_sample, bool p_sampleHasNewCaps);
	/// Move constructor.
	GStreamerMediaSample(GStreamerMediaSample && p_other);
	/**
	 * Destructor.
	 *
	 * Unrefs the GstSample set by the constructor (if the pointer wasn't null).
	 */
	~GStreamerMediaSample();

	/// Move assignment operator.
	GStreamerMediaSample& operator = (GStreamerMediaSample && p_other);

	// This class is movable, but not copyable.
	GStreamerMediaSample(GStreamerMediaSample const & p_other) = delete;
	GStreamerMediaSample& operator = (GStreamerMediaSample const & p_other) = delete;

	/**
	 * Returns the GstSample pointer from this object.
	 *
	 * The return value may be null. This can happen if an attempt was made
	 * to pull a sample and there was none.
	 *
	 * This function does not call gst_sample_ref().
	 */
	GstSample* getSample();
	/**
	 * Returns true if the sample's caps are new (= they are different
	 * compared to a previous sample's caps).
	 */
	bool sampleHasNewCaps() const;

private:
	GstSample *m_sample;
	bool m_sampleHasNewCaps;
};


} // namespace qtglviddemo end


#endif
