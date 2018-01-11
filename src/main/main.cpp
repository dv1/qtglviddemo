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


#include <gst/gst.h>
#include <signal.h>

// NOTE: Even though the qmlRegisterType() documentation lists QQmlEngine
// as the header, it alone is not enough, since the QML registration
// functions are defined elsewhere. Including QtQml makes sure the necessary
// heades are included.
#include <QtQml>

#include <QFontDatabase>
#include <QLoggingCategory>
#include <QQuickStyle>
#include "base/Utility.hpp"
#include "player/GStreamerPlayer.hpp"
#include "scene/VideoObjectItem.hpp"
#include "scene/VideoObjectModel.hpp"
#include "Application.hpp"


Q_LOGGING_CATEGORY(lcQtGLVidDemo, "qtglviddemo", QtInfoMsg)




int main(int argc, char *argv[])
{
	// Initialize GStreamer.
	if (!gst_init_check(&argc, &argv, nullptr))
		return -1;

	// Set up scoped GStreamer deinitialization. We do this because
	// some GStreamer functionality such as its tracing subsystem
	// rely on the gst_deinit() function being called at the end
	// of the program's execution.
	qtglviddemo::ScopedGstDeinit gstdeinit;

	// Enforce the Material style for the user interface controls.
	QQuickStyle::setStyle("Material");

	// Register some of our data types with QML so they can be used
	// in QML scripts.
	qmlRegisterUncreatableType < qtglviddemo::GStreamerPlayer > ("qtglviddemo", 1, 0, "GStreamerPlayer", "cannot create player object");
	qmlRegisterUncreatableType < qtglviddemo::VideoObjectModel > ("qtglviddemo", 1, 0, "VideoObjectModel", "cannot create video object model");
	qmlRegisterType < qtglviddemo::VideoObjectItem > ("qtglviddemo", 1, 0, "VideoObject");

	// Set up Qt application object.
	qtglviddemo::Application app(argc, argv);

	// Add a font from the resources so we can use it in Qt Quick 2.
	QFontDatabase::addApplicationFont(":/Dosis-SemiBold.ttf");

	// Parse command line arguments. If parsing failed (determined
	// by the boolean in retval.first), exit here with the given
	// return code (stored in retval.second).
	auto retval = app.parseCommandLineArgs();
	if (!retval.first)
		return retval.second;

	// Prepare the application. This means: the loading configuration
	// file, loading the QML UI script etc.
	if (!app.prepare())
		return -1;

	// Setup signal handlers and the corresponding unnamed pipe
	// so we can catch signals and gracefully exit.
	// (The signal handlers cause the main application window
	// to be closed, which in turn causes the application's
	// mainloop to stop and exit.)

	qtglviddemo::ScopedSignalPipe signalPipe(&(app.getMainWindow()));

	qtglviddemo::ScopedSighandler sigintHandler(SIGINT);
	qtglviddemo::ScopedSighandler sigtermHandler(SIGTERM);
	qtglviddemo::ScopedSighandler sigquitHandler(SIGQUIT);
	qtglviddemo::ScopedSighandler sighupHandler(SIGHUP);

	// Start the application's mainloop.
	return app.exec();
}
