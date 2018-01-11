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
#include <gst/app/gstappsink.h>
#include <QTextDocumentFragment>
#include <QDebug>
#include <QLoggingCategory>
#include <QThread>
#include <QAbstractEventDispatcher>
#include "base/ScopeGuard.hpp"
#include "GStreamerPlayer.hpp"
#include "GStreamerVideoRenderer.hpp"
#include "GStreamerSignalDispatcher.hpp"


Q_DECLARE_LOGGING_CATEGORY(lcQtGLVidDemo)


namespace qtglviddemo
{


GStreamerPlayer::GStreamerPlayer(NewVideoFrameAvailableCB p_newVideoFrameAvailableCB, QObject *p_parent)
	: QObject(p_parent)
	, m_gstplayer(nullptr)
	, m_gstdispatcher(nullptr)
	, m_gstvidrenderer(nullptr)
	, m_subtitleAppsink(nullptr)
	, m_state(State::Stopped)
	, m_lastSampleCaps(nullptr)
{
	// Set up the core GstPlayer instance. Create the associated signal
	// dispatcher and video renderer and pass them to the GstPlayer.
	m_gstdispatcher = createGStreamerSignalDispatcher(this);
	m_gstvidrenderer = createGStreamerVideoRenderer(std::move(p_newVideoFrameAvailableCB));
	m_gstplayer = gst_player_new(m_gstvidrenderer, m_gstdispatcher);

	// Set up the subtitle appsink.
	m_subtitleAppsink = gst_element_factory_make("appsink", "subtitleAppsink");
	// Create and connect the GLib signal callback for new subtitles.
	GstFlowReturn (*newSubtitleSampleCB)(GstElement *, gpointer) = [](GstElement *, gpointer p_userData) -> GstFlowReturn {
		return reinterpret_cast < GStreamerPlayer* > (p_userData)->onNewSubtitleSample();
	};
	g_signal_connect(G_OBJECT(m_subtitleAppsink), "new-sample", G_CALLBACK(newSubtitleSampleCB), this);
	// appsink does not emit signals by default, so we need to enable it.
	gst_app_sink_set_emit_signals(GST_APP_SINK(m_subtitleAppsink), TRUE);

	// There is currently no GstPlayer API to set the subtitle sink, so we
	// have to manually do that by acquiring a reference to the GstPlayer's
	// playbin and setting its text-sink property. playbin takes ownership
	// over the subtitle appsink; we don't have to worry about unref'ing it.
	GstElement *playbin = gst_player_get_pipeline(m_gstplayer);
	g_object_set(G_OBJECT(playbin), "text-sink", m_subtitleAppsink, "flags", gint(0x55), nullptr);
	gst_object_unref(GST_OBJECT(playbin));

	// Connect the GstPlayer signals. These are emitted from the main Qt
	// thread (the signal dispatcher takes care of that).
	g_object_connect(
		m_gstplayer,
		"swapped-signal::end-of-stream", G_CALLBACK(GStreamerPlayer::staticOnGstPlayerEndOfStream), this,
		"swapped-signal::state-changed", G_CALLBACK(GStreamerPlayer::staticOnGstPlayerStateChanged), this,
		"swapped-signal::duration-changed", G_CALLBACK(GStreamerPlayer::staticOnGstPlayerDurationChanged), this,
		"swapped-signal::position-updated", G_CALLBACK(GStreamerPlayer::staticOnGstPlayerPositionUpdated), this,
		"swapped-signal::buffering", G_CALLBACK(GStreamerPlayer::staticOnGstPlayerBufferingChanged), this,
		"swapped-signal::media-info-updated", G_CALLBACK(GStreamerPlayer::staticOnGstPlayerMediaInfoUpdated), this,
		nullptr
	);

	// Enable video and subtitle tracks, but disable audio, since at this
	// moment we do not care for audio output.
	gst_player_set_video_track_enabled(m_gstplayer, true);
	gst_player_set_audio_track_enabled(m_gstplayer, false);
	gst_player_set_subtitle_track_enabled(m_gstplayer, true);
}


GStreamerPlayer::~GStreamerPlayer()
{
	// Stop the GstPlayer to make sure no new GLib signal emissions
	// are dispatched. Also disconnect all of its signals to make sure
	// they don't try to invoke callbacks related to this GStreamerPlayer
	// instance.
	// Stopping the GstPlayer also sets an internal flag that makes sure
	// any signals that were queued by the dispatcher and might still be
	// in the Qt event loop won't do anything.
	if (m_gstplayer != nullptr)
	{
		qCDebug(lcQtGLVidDemo) << "Stopping gstplayer and disconnecting GLib signals";
		gst_player_stop(m_gstplayer);
		g_signal_handlers_disconnect_by_data(m_gstplayer, this);
	}

	// Unref the GstPlayer.
	// Note that this may not always destroy the gstplayer instance right
	// away. If there is an unemitted signal in the event loop remaining,
	// then the gstplayer is destroyed once this signal is torn down,
	// because these signals hold references to the gstplayer. (The signals
	// are marshaled into the event loop by GStreamerSignalDispatcher.)
	// But this is okay, since there are no adverse effects of a lingering
	// gstplayer instance.
	if (m_gstplayer != nullptr)
	{
		qCDebug(lcQtGLVidDemo) << "Unref'ing gstplayer";
		gst_object_unref(GST_OBJECT(m_gstplayer));
	}

	// Unref any lingering last sample caps.
	if (m_lastSampleCaps != nullptr)
		gst_caps_unref(m_lastSampleCaps);
}


void GStreamerPlayer::setUrl(QUrl p_url)
{
	if (m_url != p_url)
	{
		m_url = std::move(p_url);

		QByteArray urlCStr = m_url.toString().toUtf8();
		gst_player_set_uri(m_gstplayer, urlCStr.data());

		emit urlChanged();
	}
}


QUrl GStreamerPlayer::getUrl() const
{
	return m_url;
}


GStreamerPlayer::State GStreamerPlayer::getState() const
{
	return m_state;
}


int GStreamerPlayer::getPosition() const
{
	GstClockTime pos = gst_player_get_position(m_gstplayer);
	return GST_CLOCK_TIME_IS_VALID(pos) ? int(pos / GST_MSECOND) : int(-1);
}


int GStreamerPlayer::getDuration() const
{
	GstClockTime dur = gst_player_get_duration(m_gstplayer);
	// Use std::max() to avoid fringe cases where a duration of than 1 ms length is reported.
	return GST_CLOCK_TIME_IS_VALID(dur) ? std::max(int(dur / GST_MSECOND), 1) : int(-1);
}


bool GStreamerPlayer::isSeekable() const
{
	if (m_gstplayer == nullptr)
		return false;

	GstPlayerMediaInfo *mediaInfo = gst_player_get_media_info(m_gstplayer);
	if (mediaInfo == nullptr)
		return false;

	bool seekable = gst_player_media_info_is_seekable(mediaInfo);
	g_object_unref(G_OBJECT(mediaInfo));

	return seekable;
}


QString GStreamerPlayer::getSubtitle() const
{
	return m_subtitle;
}


void GStreamerPlayer::setSinkCaps(GstCaps *p_sinkCaps)
{
	setGStreamerVideoRendererSinkCaps(m_gstvidrenderer, p_sinkCaps);
}


void GStreamerPlayer::setSinkCapsFromVideoFormats(std::vector < GstVideoFormat > const &p_videoFormats)
{
	// Produce caps with unrestricted width/height/framerate and a list of format strings.
	// Example: if p_videoFormats contains GST_VIDEO_FORMAT_RGBA and GST_VIDEO_FORMAT_I420,
	// this produces: "video/x-raw; width: [ 1, 2147483647 ], height: [ 1, 2147483647 ],
	// framerate: [ 0/1, 2147483647/1 ], format: { RGBA, I420 }".

	assert(!p_videoFormats.empty());

	GstCaps *caps = gst_caps_new_simple(
		"video/x-raw",
		"width", GST_TYPE_INT_RANGE, 1, G_MAXINT,
		"height", GST_TYPE_INT_RANGE, 1, G_MAXINT,
		"framerate", GST_TYPE_FRACTION_RANGE, 0, 1, G_MAXINT, 1,
		nullptr
	);

	GValue format = G_VALUE_INIT;
	GValue formats = G_VALUE_INIT;
	g_value_init(&format, G_TYPE_STRING);
	g_value_init(&formats, GST_TYPE_LIST);
	for (GstVideoFormat fmt : p_videoFormats)
	{
		g_value_set_static_string(&format, gst_video_format_to_string(fmt));
		gst_value_list_append_value(&formats, &format);
	}
	gst_caps_set_value(caps, "format", &formats);
	g_value_unset(&format);
	g_value_unset(&formats);

	setSinkCaps(caps);

	gst_caps_unref(caps);
}


void GStreamerPlayer::play()
{
	gst_player_play(m_gstplayer);
}


void GStreamerPlayer::pause()
{
	gst_player_pause(m_gstplayer);
}


void GStreamerPlayer::stop()
{
	gst_player_stop(m_gstplayer);
}


void GStreamerPlayer::seek(int p_position)
{
	gst_player_seek(m_gstplayer, GstClockTime(p_position) * GST_MSECOND);
}


GStreamerMediaSample GStreamerPlayer::pullVideoSample()
{
	GstSample *sample;
	bool hasNewCaps = false;

	sample = gst_app_sink_try_pull_sample(GST_APP_SINK_CAST(getGStreamerVideoRendererVideoAppsink(m_gstvidrenderer)), 0);

	if (sample != nullptr)
	{
		// Check if the caps changed, and if so, record it. This
		// information is then passed to the new media sample below.
		GstCaps *caps = gst_sample_get_caps(sample);
		hasNewCaps = (m_lastSampleCaps == nullptr) || !gst_caps_is_equal(m_lastSampleCaps, caps);
		// Remember the current caps so we can compare them against
		// future caps to detect caps changes.
		gst_caps_replace(&m_lastSampleCaps, caps);
	}

	return GStreamerMediaSample(sample, hasNewCaps);
}


GstFlowReturn GStreamerPlayer::onNewSubtitleSample()
{
	GstSample *subtitleSample = gst_app_sink_pull_sample(GST_APP_SINK(m_subtitleAppsink));
	if (subtitleSample == nullptr)
		return GST_FLOW_OK;

	auto cleanup = makeScopeGuard([&]() { gst_sample_unref(subtitleSample); });

	GstBuffer *buffer = gst_sample_get_buffer(subtitleSample);
	if (buffer == nullptr)
		return GST_FLOW_OK;

	gsize data_size = gst_buffer_get_size(buffer);
	if (data_size == 0)
		return GST_FLOW_OK;

	GstMapInfo map_info;
	gst_buffer_map(buffer, &map_info, GST_MAP_READ);

	// Subtitle data is provided as UTF-8 text.
	QString htmlSubtitle = QString::fromUtf8(reinterpret_cast < char const * > (map_info.data), map_info.size);

	gst_buffer_unmap(buffer, &map_info);

	// The incoming data is provided typically in the Pango text attribute
	// markup format. This format contains a subset of HTML, including
	// HTML entities like &auml; . For more details about the markup, go to:
	// https://developer.gnome.org/pango/stable/PangoMarkupFormat.html)
	//
	// Qt Quick 2 items such as Text do have a "StyledText" format support,
	// but this does not cover the Pango markup properly. In particular, it
	// does not support HTML entities.
	//
	// To fix this, we convert newline characters to the HTML <br> tag, and
	// then pass the subtitle string to the fromHtml() function. This decodes
	// HTML entities and converts <br> back to newline.
	//
	// However, StyledText does not support newline characters, so we have
	// to convert newline to <br> again afterwards.
	//
	// Note that this is only a minimal subtitle format support. There are
	// other subtitle formats such as WebVTT, TTML etc. that would need
	// extra consideration. To keep things simple, we do not handle these.
	htmlSubtitle.replace("\r\n", "<br>");
	htmlSubtitle.replace("\n", "<br>");
	m_subtitle = QTextDocumentFragment::fromHtml(htmlSubtitle).toPlainText();
	m_subtitle.replace("\n", "<br>");

	// We have a new subtitle, inform the slots.
	emit subtitleChanged();

	return GST_FLOW_OK;
}


void GStreamerPlayer::staticOnGstPlayerEndOfStream(GStreamerPlayer *self)
{
	emit self->endOfStream();
}


void GStreamerPlayer::staticOnGstPlayerStateChanged(GStreamerPlayer *self, GstPlayerState p_state)
{
	State newState = static_cast < State > (p_state);
	self->m_state = newState;

	emit self->stateChanged();
}


void GStreamerPlayer::staticOnGstPlayerDurationChanged(GStreamerPlayer *self, guint64 p_duration)
{
	// Use std::max() to avoid fringe cases where a duration of than 1 ms length is reported.
	emit self->durationChanged(std::max(int(p_duration / GST_MSECOND), 1));
}


void GStreamerPlayer::staticOnGstPlayerPositionUpdated(GStreamerPlayer *self, guint64 p_position)
{
	emit self->positionUpdated(p_position / GST_MSECOND);
}


void GStreamerPlayer::staticOnGstPlayerBufferingChanged(GStreamerPlayer *self, gint p_percentage)
{
	emit self->buffering(p_percentage);
}


void GStreamerPlayer::staticOnGstPlayerMediaInfoUpdated(GStreamerPlayer *self, GstPlayerMediaInfo *)
{
	emit self->isSeekableChanged();
}


} // namespace qtglviddemo end
