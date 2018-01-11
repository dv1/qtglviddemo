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


#include <cstring>
#include <utility>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include "GStreamerVideoRenderer.hpp"


struct GStreamerVideoRenderer
{
	GObject parent;
	GstElement *videoBin;
	GstElement *videoAppsink;
	qtglviddemo::NewVideoFrameAvailableCB newVideoFrameAvailableCB;
};


struct GStreamerVideoRendererClass
{
	GObjectClass parent_class;
};


namespace
{

GstElement* createVideoSink(GstPlayerVideoRenderer *p_iface, GstPlayer *p_gstplayer);
void initVideoRenderInterface(GstPlayerVideoRendererInterface *p_iface);
void disposeVideoRenderer(GObject *p_object);

} // unnamed namespace end


// The GstPlayer video renderer is not a GLib class, it is a GLib interface.
// We implement the renderer by subclassing GObject and implementing the
// renderer interface.

G_DEFINE_TYPE_WITH_CODE(
	GStreamerVideoRenderer, gstreamer_video_renderer, G_TYPE_OBJECT,
	G_IMPLEMENT_INTERFACE(GST_TYPE_PLAYER_VIDEO_RENDERER, initVideoRenderInterface)
)


// These _class_init and _init functions are declared by the
// G_DEFINE_TYPE_WITH_CODE() boilerplate.
static void gstreamer_video_renderer_class_init(GStreamerVideoRendererClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->dispose = GST_DEBUG_FUNCPTR(disposeVideoRenderer);
}


static void gstreamer_video_renderer_init(GStreamerVideoRenderer *renderer)
{
	// Create a bin containing all elements necessary for the video output.
	// We output frames to an appsink. Format conversions may be necessary,
	// so we insert some conversion elements in front of the appsink. Since
	// the GstPlayer video renderer interface is supposed to return only
	// one element, we put all of these converter elements and the appsink
	// into one bin, so that from the outside, they look like one element.
	renderer->videoBin = gst_bin_new("videoBin");
	GstElement *videoconvert = gst_element_factory_make("videoconvert", nullptr);
	renderer->videoAppsink = gst_element_factory_make("appsink", "videoAppsink");

	// Configure the video appsink to drop the current frame is a new frame
	// is produced and the application didn't pull the current frame yet.
	// This is essential to make sure the appsink doesn't block if its
	// queue is full.
	g_object_set(G_OBJECT(renderer->videoAppsink), "sync", gboolean(TRUE), "max-buffers", guint(1), nullptr);
	gst_app_sink_set_drop(GST_APP_SINK(renderer->videoAppsink), TRUE);

	// Add the converter and appsink elements to the bin and link them.
	gst_bin_add_many(GST_BIN(renderer->videoBin), videoconvert, renderer->videoAppsink, nullptr);
	gst_element_link(videoconvert, renderer->videoAppsink);

	// Set up a ghost pad. This ghost pad is added to the bin. Its job is
	// to forward incoming data to the inner elements.
	GstPad *pad = gst_element_get_static_pad(videoconvert, "sink");
	gst_element_add_pad(renderer->videoBin, gst_ghost_pad_new("sink", pad));
	gst_object_unref(GST_OBJECT(pad));

	// Sink-ref the element. The video renderer's create_video_sink function
	// is used by GstPlayer to get the video renderer element and pass it
	// to the internal playbin. This playbin takes ownership over the video
	// output element, which means it sink-refs it.
	//
	// GStreamer elements are GObjects which implement the GInitiallyUnowned
	// interface. This means that newly created elements are flagged as a
	// "floating" reference. This affects the behavior of gst_object_ref_sink().
	// If it is called on an element with a floating reference, it does _not_
	// increase the refcount; instead, it just clears the floating flag. If
	// however the reference isn't floating, it _does_ increase the refcount.
	//
	// Details can be found at:
	// https://developer.gnome.org/gobject/stable/gobject-The-Base-Object-Type.html#gobject-The-Base-Object-Type.description
	//
	// Floating references are useful if GObjects are passed to some container
	// that can take ownership over these GObjects. However, in our case,
	// the GStreamerVideoRenderer subclass is the one that "owns" the video bin.
	// So, call gst_object_ref_sink() to make sure the floating flag is cleared.
	// (We do explicitely unref the video bin in the dispose function.)
	gst_object_ref_sink(GST_OBJECT(renderer->videoBin));
}




namespace
{


GstElement* createVideoSink(GstPlayerVideoRenderer *p_iface, GstPlayer *)
{
	GStreamerVideoRenderer *self = reinterpret_cast < GStreamerVideoRenderer* > (p_iface);
	return self->videoBin;
}


void initVideoRenderInterface(GstPlayerVideoRendererInterface *p_iface)
{
	p_iface->create_video_sink = GST_DEBUG_FUNCPTR(createVideoSink);
}


void disposeVideoRenderer(GObject *p_object)
{
	GStreamerVideoRenderer *self = reinterpret_cast < GStreamerVideoRenderer* > (p_object);
	gst_object_unref(GST_OBJECT(self->videoBin));
	G_OBJECT_CLASS(gstreamer_video_renderer_parent_class)->dispose(p_object);
}


void setNewVideoFrameAvailableCB(GStreamerVideoRenderer &p_videoRenderer, qtglviddemo::NewVideoFrameAvailableCB p_newVideoFrameAvailableCB)
{
	p_videoRenderer.newVideoFrameAvailableCB = std::move(p_newVideoFrameAvailableCB);

	if (p_videoRenderer.newVideoFrameAvailableCB)
	{
		// Install new_sample callback function that invokes the
		// newVideoFrameAvailableCB function object when a new
		// frame is available.
		GstFlowReturn (*newSampleCB)(GstAppSink *, gpointer) = [](GstAppSink *, gpointer p_user_data) -> GstFlowReturn {
			GStreamerVideoRenderer *renderer = reinterpret_cast < GStreamerVideoRenderer* > (p_user_data);
			renderer->newVideoFrameAvailableCB();
			return GST_FLOW_OK;
		};

		GstAppSinkCallbacks callbacks;
		std::memset(&callbacks, 0, sizeof(callbacks));
		callbacks.new_sample = newSampleCB;
		gst_app_sink_set_callbacks(GST_APP_SINK_CAST(p_videoRenderer.videoAppsink), &callbacks, gpointer(&p_videoRenderer), nullptr);
	}
}


} // unnamed namespace end


namespace qtglviddemo
{


GstPlayerVideoRenderer* createGStreamerVideoRenderer(NewVideoFrameAvailableCB newVideoFrameAvailableCB)
{
	// Create the video renderer instance.
	gpointer renderer = g_object_new(gstreamer_video_renderer_get_type(), nullptr);
	// Pass the frame available callback to the new renderer.
	setNewVideoFrameAvailableCB(*(static_cast < GStreamerVideoRenderer* > (renderer)), std::move(newVideoFrameAvailableCB));
	// We are done, return the new renderer.
	return static_cast < GstPlayerVideoRenderer* > (renderer);
}


GstElement* getGStreamerVideoRendererVideoAppsink(GstPlayerVideoRenderer *renderer)
{
	GStreamerVideoRenderer *self = (GStreamerVideoRenderer *)renderer;
	return self->videoAppsink;
}


void setGStreamerVideoRendererSinkCaps(GstPlayerVideoRenderer *renderer, GstCaps *sinkCaps)
{
	GStreamerVideoRenderer *self = (GStreamerVideoRenderer *)renderer;
	gst_app_sink_set_caps(GST_APP_SINK_CAST(self->videoAppsink), sinkCaps);
}


} // namespace qtglviddemo end
