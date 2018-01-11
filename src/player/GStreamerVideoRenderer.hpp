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


#ifndef QTGLVIDDEMO_GSTREAMER_VIDEO_RENDERER_HPP
#define QTGLVIDDEMO_GSTREAMER_VIDEO_RENDERER_HPP

#include <functional>
#include <gst/gst.h>
#include <gst/player/player.h>
#include "GStreamerCommon.hpp"


namespace qtglviddemo
{


/**
 * Creates an implementation of GstPlayerVideoRenderer.
 *
 * GstPlayer uses the GstPlayerVideoRenderer interface to create a video
 * sink element. This function instantiates an implementation of this interface
 * with an appsink as the video sink element. This appsink is capable of
 * housing the current video frame so the application can pull it. In addition,
 * the appsink can notify about a newly received frame if newVideoFrameAvailableCB
 * is a valid function object.
 *
 * The GStreamerPlayer pullVideoSample() function pulls video samples from
 * this renderer's appsink.
 *
 * @param newVideoFrameAvailableCB Callback function object that shall be
 *        invoked whenever a new video frame is available in the appsink.
 *        If this is not a valid function object, no notification is done.
 *        Note that this is called from a GStreamer streaming thread.
 */
GstPlayerVideoRenderer* createGStreamerVideoRenderer(NewVideoFrameAvailableCB newVideoFrameAvailableCB = NewVideoFrameAvailableCB());
/**
 * Retrieves the video renderer's appsink.
 *
 * The returned element reference is valid for as long as the renderer exists.
 *
 * @param renderer Video renderer instance to get the appsink from.
 */
GstElement* getGStreamerVideoRendererVideoAppsink(GstPlayerVideoRenderer *renderer);
/**
 * Sets the allowed output sink caps.
 *
 * This is necessary if only certain subset of output video formats are allowed.
 * If for example the output only supports the I420 pixel format, then this must
 * be called with the format caps set to I420.
 *
 * If the sink caps are null, then the formats are unrestricted.
 *
 * @param renderer Video renderer instance whose sink caps shall be set.
 * @param sinkCaps Sink caps to set.
 */
void setGStreamerVideoRendererSinkCaps(GstPlayerVideoRenderer *renderer, GstCaps *sinkCaps);


} // namespace qtglviddemo end


#endif
