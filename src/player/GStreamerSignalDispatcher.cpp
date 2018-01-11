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


#include <utility>
#include <gst/gst.h>
#include <QDebug>
#include <QLoggingCategory>
#include <QCoreApplication>
#include <QEvent>
#include "GStreamerSignalDispatcher.hpp"
#include "GStreamerPlayer.hpp"


Q_DECLARE_LOGGING_CATEGORY(lcQtGLVidDemo)


namespace
{


// Adapted from http://stackoverflow.com/a/21653558 . This is utility code
// to make sure a given function object is executed in the Qt event loop.
template < typename F >
void postFunctionToThread(QObject *p_receiver, F && p_function)
{
	class FuncEvent
		: public QEvent
	{
	public:
		using Func = typename std::decay < F > ::type;

		explicit FuncEvent(Func && p_func)
			: QEvent(QEvent::None)
			, m_func(std::move(p_func))
		{
		}

		explicit FuncEvent(Func const &p_func)
			: QEvent(QEvent::None)
			, m_func(p_func)
		{
		}

		~FuncEvent()
		{
#ifdef QT_NO_EXCEPTIONS
			m_func();
#else
			try
			{
				m_func();
			}
			catch (...)
			{
				// Can't let exceptions out in the destructor
			}
#endif
		}

	private:
		Func m_func;
	};

	QCoreApplication::postEvent(p_receiver, new FuncEvent(std::forward < F > (p_function)));
}


} // unnamed namespace end




struct GStreamerSignalDispatcher
{
	GObject parent;
	qtglviddemo::GStreamerPlayer *player;
};


struct GStreamerSignalDispatcherClass
{
	GObjectClass parent_class;
};


namespace
{


void dispatch(GstPlayerSignalDispatcher *p_iface, GstPlayer *p_gstplayer, void (*p_emitter)(gpointer data), gpointer p_emitter_data, GDestroyNotify p_emitter_destroy);
void initDispatcherInterface(GstPlayerSignalDispatcherInterface *p_iface);


} // unnamed namespace end


// The GstPlayer signal dispatcher is not a GLib class, it is a GLib interface.
// We implement the dispatcher by subclassing GObject and implementing the
// dispatcher interface.

G_DEFINE_TYPE_WITH_CODE(
	GStreamerSignalDispatcher, gstreamer_signal_dispatcher, G_TYPE_OBJECT, 
	G_IMPLEMENT_INTERFACE(GST_TYPE_PLAYER_SIGNAL_DISPATCHER, initDispatcherInterface)
)


// Init function definitions necessary for the GObject boilerplate. We do not
// need to initialize anything, so they are empty, but they must still exist.
// (See G_DEFINE_TYPE_WITH_CODE documentation for details.)

static void gstreamer_signal_dispatcher_class_init(GStreamerSignalDispatcherClass *)
{
}

static void gstreamer_signal_dispatcher_init(GStreamerSignalDispatcher *)
{
}


namespace
{


void dispatch(GstPlayerSignalDispatcher *p_iface, GstPlayer *, void (*p_emitter)(gpointer data), gpointer p_emitter_data, GDestroyNotify p_emitter_destroy)
{
	GStreamerSignalDispatcher *self = (GStreamerSignalDispatcher *)p_iface;

	QObject *receiver = static_cast < QObject* > (self->player);

	qCDebug(lcQtGLVidDemo) << "Dispatching GstPlayer signal; emitter data" << p_emitter_data;

	// Make sure the signal emission is handled in the main Qt thread.
	postFunctionToThread(receiver, [=]() {
		qCDebug(lcQtGLVidDemo) << "Handling dispatched GstPlayer signal; emitter data" << p_emitter_data;
		p_emitter(p_emitter_data);
		if (p_emitter_destroy != nullptr)
			p_emitter_destroy(p_emitter_data);
	});
}


void initDispatcherInterface(GstPlayerSignalDispatcherInterface *p_iface)
{
	p_iface->dispatch = GST_DEBUG_FUNCPTR(dispatch);
}


} // unnamed namespace end


namespace qtglviddemo
{


GstPlayerSignalDispatcher* createGStreamerSignalDispatcher(GStreamerPlayer *player)
{
	gpointer dispatcher = g_object_new(gstreamer_signal_dispatcher_get_type(), nullptr);
	static_cast < GStreamerSignalDispatcher* > (dispatcher)->player = player;
	return static_cast < GstPlayerSignalDispatcher* > (dispatcher);
}


} // namespace qtglviddemo end
