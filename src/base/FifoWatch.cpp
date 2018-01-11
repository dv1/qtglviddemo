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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <QDebug>
#include <QLoggingCategory>
#include "FifoWatch.hpp"


Q_DECLARE_LOGGING_CATEGORY(lcQtGLVidDemo)


namespace qtglviddemo
{


FifoWatch::FifoWatch(QObject *p_parent)
	: QObject(p_parent)
	, m_fifoNotifier(nullptr)
	, m_fifoFd(-1)
{
}


FifoWatch::~FifoWatch()
{
	stop();
}


QString FifoWatch::getPath() const
{
	return m_fifoPath;
}


void FifoWatch::start(QString p_fifoPath, bool p_unlinkAtStop)
{
	// Stop first to not collide with any ongoing watch.
	stop();

	// Get the FIFO path as UTF-8 string that we can pass to mkfifo().
	m_fifoPathAsUtf8 = p_fifoPath.toUtf8();

	// Attempt to create and open the FIFO. If this fails,
	// the FIFO watch remains in the stopped state.
	if (mkfifo(m_fifoPathAsUtf8.constData(), S_IRUSR | S_IWUSR) == 0)
	{
		// Creating the FIFO succeeded.
		qCDebug(lcQtGLVidDemo) << "Successfully created FIFO" << p_fifoPath;

		m_unlinkAtStop = p_unlinkAtStop;

		// Open the newly created FIFO.
		// NOTE: We use the O_RDWR mode, not O_RDONLY. For the reason
		// why, see https://stackoverflow.com/a/580057/560774
		// We open the FIFO in nonblocking mode, which is necessary
		// for being able to watch the FIFO's file descriptor.
		m_fifoFd = open(m_fifoPathAsUtf8.constData(), O_RDWR | O_NONBLOCK);
		if (m_fifoFd > 0)
		{
			qCDebug(lcQtGLVidDemo) << "Creating socket notifier to listen to incoming data from FIFO" << p_fifoPath;
			m_fifoNotifier = new QSocketNotifier(m_fifoFd, QSocketNotifier::Read);

			// Store the FIFO path. Do this here, _after_ the watch
			// has been set up successfully, since a non-empty path
			// member is OK only if a watch is currently ongoing.
			// (See the getPath() documentation.)
			m_fifoPath = p_fifoPath;

			connect(m_fifoNotifier, &QSocketNotifier::activated, this, &FifoWatch::readFromFifo);
		}
		else
		{
			qCWarning(lcQtGLVidDemo) << "Could not open FIFO" << p_fifoPath << ":" << std::strerror(errno) << "- removing FIFO";
			// Unlink the FIFO since it proved to be unusable.
			unlinkFifo();
		}
	}
	else
	{
		// Creating the FIFO failed. This can happen for example
		// if a file already exist at the given location.
		qCWarning(lcQtGLVidDemo) << "Could not create FIFO" << p_fifoPath << ":" << std::strerror(errno);
	}
}


void FifoWatch::stop()
{
	if (m_fifoNotifier != nullptr)
	{
		delete m_fifoNotifier;
		m_fifoNotifier = nullptr;
	}

	if (m_fifoFd > 0)
	{
		::close(m_fifoFd);
		unlinkFifo();
		m_fifoFd = -1;
	}

	m_fifoPath = "";
}


void FifoWatch::readFromFifo(int)
{
	assert(m_fifoFd > 0);

	char buf[1024];
	ssize_t numBytesRead;

	QString line = "";

	// Read out all currently available bytes from the FIFO.
	while (true)
	{
		numBytesRead = read(m_fifoFd, buf, sizeof(buf));
		if (numBytesRead < 0)
		{
			// EAGAIN signals the end of the available data and is
			// therefore not considered an error.
			// (Standard behavior with non-blocking file descriptors.)
			if (errno != EAGAIN)
			{
				qCWarning(lcQtGLVidDemo) << "Could not read from FIFO:" << std::strerror(errno);
				return;
			}
			else
				break;
		}
		else if (numBytesRead > 0)
			line += QString::fromUtf8(buf, numBytesRead);
	}

	// Remove whitespace from the start and end of the received line. Here,
	// whitespace includes CR/LF line delimiters. We do not want to pass
	// these on. CR/LF in the middle of the line is OK, just not not at
	// the ends, because often, such a delimiter is added when pushing data
	// into the FIFO via a shell. Example:
	//
	//   echo Hello > /tmp/my-fifo
	//
	// This would produce "Hello\n" without trimming.
	line = line.trimmed();

	qCDebug(lcQtGLVidDemo) << "New line from FIFO:" << line;

	emit newFifoLine(line);
}


void FifoWatch::unlinkFifo()
{
	if (m_unlinkAtStop)
		unlink(m_fifoPathAsUtf8.constData());
}


} // namespace qtglviddemo end
