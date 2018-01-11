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


#ifndef QTGLVIDDEMO_GSTREAMER_PLAYER_HPP
#define QTGLVIDDEMO_GSTREAMER_PLAYER_HPP

#include <memory>
#include <vector>
#include <QUrl>
#include <QObject>
#include <gst/gst.h>
#include <gst/player/player.h>
#include <gst/video/video.h>
#include "GStreamerCommon.hpp"
#include "GStreamerMediaSample.hpp"


namespace qtglviddemo
{


/**
 * Main GStreamer based media player class.
 *
 * This implements a media player using GStreamer and the GstPlayer library.
 * GstPlayer takes care of several nontrivial features such as seeking or
 * buffering. This reduces code complexity and potential for errors.
 *
 * Decoded video frames are sent to an appsink element. This element makes
 * it possible for the main application to pull video frames from it. We
 * use this in Qt Quick item code to pull decoded video frames during
 * rendering. If a video frame isn't pulled fast enough, and a new frame
 * is produced, then the current frame is discarded. In other words,
 * frame dropping is done if necessary.
 *
 * For subtitles, an appsink is also used, except that in this case,
 * the subtitles aren't pulled - instead, the subtitleChanged signal
 * is emitted.
 *
 * The player class is designed to be usable in QML.
 *
 * Playback is started by first setting the url property and then calling
 * play(). Calling play() during playback does nothing unless playback is
 * currently paused, in which cause it is resumed.
 *
 * Note that typically, whatever outputs decoded frames supports only a
 * certain subset of formats. For this reason, make sure setSinkCaps()
 * or setSinkCapsFromVideoFormats() is called before starting playback.
 *
 * GstPlayer requires two other components to be implemented and instantiated:
 * a signal dispatcher and a video renderer. See the corresponding source
 * files for details.
 */
class GStreamerPlayer
	: public QObject
{
	Q_OBJECT
	/**
	 * The next url to play.
	 *
	 * If this is changed, it will take effect only after playback is
	 * restarted by calling stop() and play() again.
	 */
	Q_PROPERTY(QUrl url READ getUrl WRITE setUrl NOTIFY urlChanged)
	/// Current playback state.
	Q_PROPERTY(State state READ getState NOTIFY stateChanged)
	/**
	 * Current playback position, in milliseconds.
	 *
	 * If the position cannot be currently determined, the position is -1.
	 */
	Q_PROPERTY(int position READ getPosition)
	/**
	 * Current playback duration, in milliseconds.
	 *
	 * If the duration cannot be currently determined, the position is -1.
	 */
	Q_PROPERTY(int duration READ getDuration NOTIFY durationChanged)
	/**
	 * If this is true, then seek() is supported.
	 *
	 * Note that there can be valid position values even if this
	 * is set to false.
	 */
	Q_PROPERTY(bool isSeekable READ isSeekable NOTIFY isSeekableChanged)
	/// The current subtitle.
	Q_PROPERTY(QString subtitle READ getSubtitle NOTIFY subtitleChanged)


public:
	enum class State
	{
		/// Player is currently stopped (= idle).
		Stopped = GST_PLAYER_STATE_STOPPED,
		/// Player is currently buffering data. Playback is paused.
		Buffering = GST_PLAYER_STATE_BUFFERING,
		/// Player is paused because the user requested it to be paused.
		Paused = GST_PLAYER_STATE_PAUSED,
		/// Player is playing.
		Playing = GST_PLAYER_STATE_PLAYING
	};
	Q_ENUM(State)

	/**
	 * Constructor.
	 *
	 * This sets up the GstPlayer, the appsinks, etc. but does not start
	 * playback. Use the url property and play() for this purpose.
	 *
	 * @param newVideoFrameAvailableCB Callback function object that shall
	 *        be invoked whenever a new video frame is available. If this
	 *        is not a valid function object, no notification is done.
	 *        Note that this is called from a GStreamer streaming thread.
	 * @param p_parent Parent QObject
	 */
	explicit GStreamerPlayer(NewVideoFrameAvailableCB p_newVideoFrameAvailableCB = NewVideoFrameAvailableCB(), QObject *p_parent = nullptr);
	~GStreamerPlayer();


	// Property accessors

	void setUrl(QUrl p_url);
	QUrl getUrl() const;

	State getState() const;

	int getPosition() const;
	int getDuration() const;

	bool isSeekable() const;

	QString getSubtitle() const;


	/**
	 * Sets the allowed video caps.
	 *
	 * This limits the possible formatting of the frames the player
	 * can produce. Internally, if necessary, frames are converted
	 * prior to passing them to the video appsink.
	 *
	 * Make sure this is called before playback is started, otherwise
	 * frames are produced with incorrect formats.
	 *
	 * @param p_sinkCaps Sink caps to use. This does not take
	 *        ownership over the caps.
	 */
	void setSinkCaps(GstCaps *p_sinkCaps);
	/**
	 * Sets the list of allowed video formats.
	 *
	 * This is a variant of setSinkCaps() that limits only the
	 * set of pixel formats frames can use. Other capabilities such
	 * as width, height, framerate remain unrestricted.
	 *
	 * @param p_videoFormats The set of allowed video formats.
	 *        Must not be empty.
	 */
	void setSinkCapsFromVideoFormats(std::vector < GstVideoFormat > const &p_videoFormats);

	/**
	 * Starts playback if not playing yet, or resumes if paused.
	 *
	 * This function is used in two cases:
	 *
	 * * If the current playback state is Stopped, this starts
	 *   playback. The uri property must be set prior to the
	 *   playback start. If the allowed video formats need to
	 *   be restricted (for example to make sure only frames that
	 *   can be consumed by a GPU are produced), also make sure
	 *   that setSinkCaps() or setSinkCapsFromVideoFormats()
	 *   is called prior to playback start.
	 *
	 * * If the current playback state is Paused, this resumes
	 *   (= unpauses) playback.
	 *
	 * Either way, the state change to Playing happens
	 * asynchronously, so do not expect the state to be Playing
	 * when this function ends. Observe the stateChanged signal
	 * instead.
	 *
	 * This function can be called from QML.
	 */
	Q_INVOKABLE void play();
	/**
	 * Pauses playing playback.
	 *
	 * If the current state is Playing, this initiates a
	 * state change to Paused. Otherwise it does nothing.
	 *
	 * This function can be called from QML.
	 */
	Q_INVOKABLE void pause();
	/**
	 * Stops playback.
	 *
	 * This changes the state to Stopped. If the current
	 * state is already Stopped, this does nothing.
	 *
	 * Unlike other calls, this blocks until the Stopped
	 * state is reached.
	 *
	 * This function can be called from QML.
	 */
	Q_INVOKABLE void stop();
	/**
	 *
	 * This function can be called from QML.
	 */
	Q_INVOKABLE void seek(int p_position);

	/**
	 * Pulls the current video sample from the video appsink.
	 *
	 * If the appsink currently has no sample, the media sample's
	 * getSample() function will return a null pointer.
	 *
	 * Note that the returned media sample holds a reference to
	 * the underlying GstSample, so make sure the media sample
	 * is discarded once it is no longer needed.
	 */
	GStreamerMediaSample pullVideoSample();


signals:
	/**
	 * This signal is emitted during buffering, for example when
	 * playing from a HTTP source and the HTTP network buffer is
	 * being filled. When buffering starts, the playback state is
	 * switched to Buffering, which is similar to Paused, except
	 * that it can't be "resumed".
	 *
	 * p_percent is the percentage of the buffer fill level. Once
	 * the value reaches 100, the playback state is switched back
	 * to Paused or Playing (depending on whatever the state was
	 * before buffering started).
	 *
	 * For applications, this signal is mainly useful for displaying
	 * some sort of progress indicator in the user interface. Do
	 * not rely on the values to decide if the indicator shall be
	 * shown; observe the stateChanged signal for this purpose.
	 *
	 * @param p_percent Current buffer fill level percentage. Valid
	 *        range is 0-100.
	 */
	void buffering(int p_percent);
	/**
	 * This signal is emitted when the end of the playback stream
	 * is reached. The playback state is set to Stopped when this
	 * happens.
	 *
	 * One use for this signal is looping. When it is emitted,
	 * calling play() will restart playback.
	 */
	void endOfStream();
	/// This signal is emitted when the URL changed.
	void urlChanged();
	/// This signal is emitted when the current playback state changed.
	void stateChanged();
	/**
	 * This signal is emitted when the current playback duration changed.
	 *
	 * The current duration is passed as an argument here, because
	 * it is supplied by GstPlayer, and calling getDuration() would
	 * be an alternative with slightly more overhead (because it calls
	 * gst_player_get_duration()).
	 *
	 * @param newDuration New playback duration, in milliseconds.
	 *        If no duration is known, this is set to -1.
	 */
	void durationChanged(int newDuration);
	/**
	 * This signal is emitted when the current playback position changed.
	 *
	 * The current position is passed as an argument here, because
	 * it is supplied by GstPlayer, and calling getPosition() would
	 * be an alternative with slightly more overhead (because it calls
	 * gst_player_get_position()).
	 *
	 * @param newPosition New playback position, in milliseconds
	 *        If no position is known, this is set to -1.
	 */
	void positionUpdated(int newPosition);
	/// This signal is emitted whenever the seekable property changes.
	void isSeekableChanged();
	/**
	 * This signal is emitted when a new subtitle is available.
	 *
	 * Note that if a slot is connected to this with a direct connection,
	 * said slot takes a long time time to finish, and in the meantime,
	 * new subtitles are available, then these subtitles will be dropped.
	 */
	void subtitleChanged();


private:
	GstFlowReturn onNewSubtitleSample();

	static void staticOnGstPlayerEndOfStream(GStreamerPlayer *self);
	static void staticOnGstPlayerStateChanged(GStreamerPlayer *self, GstPlayerState p_state);
	static void staticOnGstPlayerDurationChanged(GStreamerPlayer *self, guint64 p_duration);
	static void staticOnGstPlayerPositionUpdated(GStreamerPlayer *self, guint64 p_position);
	static void staticOnGstPlayerBufferingChanged(GStreamerPlayer *self, gint p_percentage);
	static void staticOnGstPlayerMediaInfoUpdated(GStreamerPlayer *self, GstPlayerMediaInfo *p_mediaInfo);

	GstPlayer *m_gstplayer;
	GstPlayerSignalDispatcher *m_gstdispatcher;
	GstPlayerVideoRenderer *m_gstvidrenderer;
	GstElement *m_subtitleAppsink;

	QUrl m_url;
	State m_state;

	QString m_subtitle;

	GstCaps *m_lastSampleCaps;
};

typedef std::unique_ptr < GStreamerPlayer > GStreamerPlayerUPtr;


} // namespace qtglviddemo end


#endif
