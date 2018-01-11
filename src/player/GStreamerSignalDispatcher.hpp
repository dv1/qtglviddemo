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


#ifndef QTGLVIDDEMO_GSTREAMER_SIGNAL_DISPATCHER_HPP
#define QTGLVIDDEMO_GSTREAMER_SIGNAL_DISPATCHER_HPP

#include <gst/player/player.h>


namespace qtglviddemo
{


class GStreamerPlayer;


/**
 * Creates an implementation of GstPlayerSignalDispatcherInterface.
 *
 * GstPlayer runs its own GLib mainloop in a separate thread. Its GLib signals
 * would be emitted from this separate thread. GLib signal connections are more
 * limited compared to Qt ones; they essentially behave like Qt connections of the
 * DirectConnection type. This means that signals that are emitted in a separate
 * thread also invoke the connected callbacks in that thread. Therefore, any
 * application that uses GstPlayer would have to use mutexes etc. to make sure
 * no race conditions occur.
 *
 * To avoid this, GstPlayer has a feature called a "signal dispatcher". It allows
 * for integrating signal emissions into existing mainloops so the emissions
 * happen in the correct thread. In the Qt case, it means that a function pointer
 * and a data pointer are passed to the signal dispatcher, and the dispatcher
 * wraps these two in a QEvent and pushes it to the Qt event queue. This way,
 * that function pointer is invoked in the main Qt thread, the GstPlayer GLib
 * signals are emitted from the main Qt thread, and no race conditions can occur.
 *
 * Signal emissions hold a reference to the gstplayer that owns the dispatcher.
 * If the gstplayer is stopped, it raises an internal flag that makes sure any
 * associated signal emission doesn't actually do anything other than releasing
 * its reference to the gstplayer. This is important during shutdown, since
 * signal emissions may still linger in the Qt event queue. So, during shutdown,
 * gst_player_stop() is called, which raises this flag. If the lingering emissions
 * are dispatched later when the Qt event loop processes them, this dispatch only
 * releases the gstplayer reference and does nothing else.
 */
GstPlayerSignalDispatcher* createGStreamerSignalDispatcher(GStreamerPlayer *player);


} // namespace qtglviddemo end


#endif
