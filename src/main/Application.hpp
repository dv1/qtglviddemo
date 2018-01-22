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


#ifndef QTGLVIDDEMO_APPLICATION_HPP
#define QTGLVIDDEMO_APPLICATION_HPP

#include <utility>
#include <memory>
#include <QUrl>
#include <QApplication>
#include <QQuickWindow>
#include <QQmlApplicationEngine>
#include "base/FifoWatch.hpp"
#include "base/VideoInputDevicesModel.hpp"
#include "scene/VideoObjectModel.hpp"


namespace qtglviddemo
{


/**
 * Main application class.
 *
 * This inherits from QApplication instead of QCoreApplication or
 * QGuiApplication because in this demo we make use of QtQuick Controls 2
 * (which require QGuiApplication) and standard dialog windows such as the
 * QFileDialog (which require QApplication).
 *
 * In this class, QML context properties are installed, command line
 * arguments are parsed, configuration files are loaded/saved, the
 * FIFO watch is set up, and the data model that listens for V4L2
 * capture devices is created.
 */
class Application
	: public QApplication
{
public:
	/**
	 * Constructor.
	 *
	 * Sets up the FIFO watch, the video input devices model, the video
	 * object model, the QML engine, and the main window is shown.
	 *
	 * The configuration is NOT loaded here. Neither is the QML UI
	 * loaded. These steps are done in prepare().
	 */
	Application(int &argc, char **argv);
	/**
	 * Destructor.
	 *
	 * If the FIFO watch is running, it is stopped here.
	 * Also, if the command line arguments specified that the
	 * configuration file needs to be updated when the program ends,
	 * the current configuration is saved to file here.
	 */
	virtual ~Application();

	/**
	 * Prepares resources, states, data structures, and QML UI.
	 *
	 * This is called right before exec().
	 *
	 * The configuration is loaded here if a config file is specified
	 * in the arguments. This may also start the FIFO watch if a FIFO
	 * path is specified in the configuration.
	 *
	 * The QML user interface is also loaded here, _after_ the
	 * configuration file is loaded. This is important, because the
	 * QML UI code may want to access settings that were specified
	 * in the command line (splashcreen filename for example):
	 *
	 * Returns true if preparation finished successfully, false
	 * otherwise. If this returns false, the program should exit.
	 */
	bool prepare();

	/**
	 * Parses the command line arguments that were forwarded to the constructor.
	 *
	 * The return value is an STL pair. The first value is true if
	 * parsing finished successfully, false otherwise. The second
	 * value is set to the return code that main() should return in
	 * case parsing fails.
	 * (The second value is not used if parsing succeeded.)
	 */
	std::pair < bool, int > parseCommandLineArgs();

	/// Retrieve a reference to the main application window.
	QQuickWindow & getMainWindow();


private:
	void loadConfiguration();
	void saveConfiguration();

	QString m_configFilename;
	bool m_saveConfigAtEnd;

	QString m_splashScreenFilename;
	bool m_keepSplashscreen;

	QQmlApplicationEngine m_engine;
	QQuickWindow *m_mainWindow;

	// Keeping the FIFO path separately because the FifoWatch getPath()
	// function returns an empty string when the FIFO watch is stopped.
	QString m_fifoPath;
	FifoWatch m_fifoWatch;

	VideoObjectModel m_videoObjectModel;
	VideoInputDevicesModel m_videoInputDevicesModel;
};


} // namespace qtglviddemo end


#endif
