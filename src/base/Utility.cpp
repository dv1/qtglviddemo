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
#include <gst/gst.h>
#include <signal.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>
#include <iostream>

#include <QDebug>
#include <QLoggingCategory>
#include <QQuickView>
#include <QSocketNotifier>
#include "Utility.hpp"


Q_DECLARE_LOGGING_CATEGORY(lcQtGLVidDemo)


namespace qtglviddemo
{


namespace
{

	
volatile sig_atomic_t signalFd = -1;

void sigHandler(int)
{
	if (signalFd != -1)
	{
		auto ret = write(signalFd, "1", 1);
		assert(ret >= 1);
	}
}


} // unnamed namespace end




ScopedGstDeinit::~ScopedGstDeinit()
{
	gst_deinit();
}




ScopedSignalPipe::ScopedSignalPipe(QWindow *p_window)
	: m_notifier(nullptr)
{
	assert(p_window != nullptr);

	m_pipeFds[0] = m_pipeFds[1] = -1;
	if (pipe(m_pipeFds) == -1)
	{
		qCCritical(lcQtGLVidDemo) << "Could not create signal pipe: " << std::strerror(errno);
		return;
	}
	signalFd = m_pipeFds[1];

	m_notifier = new QSocketNotifier(m_pipeFds[0], QSocketNotifier::Read, nullptr);
	QObject::connect(m_notifier, &QSocketNotifier::activated, [this, p_window]() {
		if (signalFd < 0)
			return;

		char c;
		auto ret = read(m_pipeFds[0], &c, 1);
		if (ret >= 1)
		{
			qCDebug(lcQtGLVidDemo) << "Signal caught, quitting";
			p_window->close();
		}
		else if (ret < 0)
		{
			qCCritical(lcQtGLVidDemo) << "Error reading from signal pipe:" << std::strerror(errno) << " " << signalFd;
			p_window->close();
		}
	});
}


ScopedSignalPipe::~ScopedSignalPipe()
{
	delete m_notifier;
	if (m_pipeFds[0] != -1)
		close(m_pipeFds[0]);
	if (m_pipeFds[1] != -1)
		close(m_pipeFds[1]);
	signalFd = -1;
}




struct ScopedSighandler::Priv
{
	int m_signal;
	struct sigaction m_oldSigaction;
	bool m_initialized;

	Priv()
		: m_initialized(false)
	{
	}
};


ScopedSighandler::ScopedSighandler(int p_signal)
	: m_priv(new Priv)
{
	m_priv->m_signal = p_signal;

	// Retain the old signal handler so we can restore
	// it in the destructor.
	sigaction(p_signal, nullptr, &(m_priv->m_oldSigaction));

	// Only set up a signal handler if this signal wasn't
	// marked as to be ignored.
	if (m_priv->m_oldSigaction.sa_handler != SIG_IGN)
	{
		struct sigaction newSigaction;
		sigemptyset(&newSigaction.sa_mask);
		newSigaction.sa_handler = sigHandler;
		newSigaction.sa_flags = SA_RESTART;
		sigfillset(&(newSigaction.sa_mask));

		if (sigaction(p_signal, &newSigaction, nullptr) < 0)
		{
			qCCritical(lcQtGLVidDemo) << "Could not set up signal handler:" << std::strerror(errno);
		}
		else
			m_priv->m_initialized = true;
	}
}


ScopedSighandler::~ScopedSighandler()
{
	// Restore the previous signal handler.
	if (m_priv->m_initialized)
		sigaction(m_priv->m_signal, &(m_priv->m_oldSigaction), nullptr);

	delete m_priv;
}



} // namespace qtglviddemo end
