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


#ifndef QTGLVIDDEMO_UTILITY_HPP
#define QTGLVIDDEMO_UTILITY_HPP

class QWindow;
class QSocketNotifier;


namespace qtglviddemo
{


/**
 * Class for RAII-based GStreamer deinitialization.
 *
 * This is useful for making sure gst_deinit() is called even if an
 * exception is thrown for some reason.
 */
struct ScopedGstDeinit
{
	/// Destructor. Calls gst_deinit().
	~ScopedGstDeinit();
};


/**
 * Sets up an unnamed pipe for the scoped signal handlers below.
 *
 * This is used together with ScopedSighandler. First, a ScopedSignalPipe
 * instance is created. Then, ScopedSighandler instances are set up. This
 * way, RAII-based Unix signal handler setup is possible.
 */
class ScopedSignalPipe
{
public:
	/**
	 * Constructor.
	 *
	 * Creates an unnamed pipe and calls p_window's close() function
	 * if the pipe receives a message from a signal handler.
	 *
	 * @param p_window Window to close in case of a Unix signal
	 *        emission. Must not be null.
	 */
	explicit ScopedSignalPipe(QWindow *p_window);
	/// Destructor. Cleans up the unnamed pipe.
	~ScopedSignalPipe();

private:
	int m_pipeFds[2];
	QSocketNotifier *m_notifier;
};


/**
 * Sets up a signal handler that emits a message through the ScopedSignalPipe.
 */
class ScopedSighandler
{
public:
	/**
	 * Constructor. Sets up a Unix signal handler for the given
	 * signal. This handler emits a message through the ScopedSignalPipe
	 * if it is triggered.
	 *
	 * If the given signal was previously marked as to be ignored
	 * (via SIG_IGN), then neither the constructor nor the destructor
	 * do anything.
	 *
	 * @param p_signal Signal to install. One example is SIGINT.
	 */
	explicit ScopedSighandler(int p_signal);
	/// Destructor. Removes the signal handler.
	~ScopedSighandler();


private:
	struct Priv;
	Priv *m_priv;
};


} // namespace qtglviddemo end


#endif
