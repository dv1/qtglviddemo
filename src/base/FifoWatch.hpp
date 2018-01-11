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


#ifndef QTGLVIDDEMO_FIFO_WATCH_HPP
#define QTGLVIDDEMO_FIFO_WATCH_HPP

#include <QObject>
#include <QByteArray>
#include <QSocketNotifier>


namespace qtglviddemo
{


/**
 * FIFO input watcher class.
 *
 * This class creates a named pipe (a FIFO) and observes it,
 * looking for any incoming text lines. If a line is received,
 * newFifoLine is emitted.
 *
 * If the FIFO already exists, this class does nothing.
 */
class FifoWatch
	: public QObject
{
	Q_OBJECT
public:
	/**
	 * Constructor.
	 *
	 * This does not start the FIFO watch. It just sets
	 * up internal states. Use start() for starting the
	 * observations.
	 *
	 * @param p_parent Parent QObject
	 */
	FifoWatch(QObject *p_parent = nullptr);
	/**
	 * Destructor
	 *
	 * Internally calls stop() automatically.
	 */
	~FifoWatch();

	/***
	 * Returns the path of the currently watched FIFO.
	 *
	 * If no FIFO watch is currently ongoing, this
	 * returns an empty string.
	 */
	QString getPath() const;


public slots:
	/**
	 * Creates a FIFO at the given path and begins watching it for I/O activity.
	 *
	 * Internally calls stop() first, so any ongoing watch
	 * will be ceased (and if in the prior start() call,
	 * p_unlinkAtStop was set to true, the watched FIFO
	 * will be deleted).
	 *
	 * Typically, FIFOs are created in the temporary
	 * directory /tmp/ . So one valid path would be
	 * /tmp/myfifo for example.
	 *
	 * If the given path already exists, stop() is called,
	 * but nothing else happens.
	 *
	 * @param p_fifoPath Path of the new FIFO to create
	 *        and watch.
	 * @param p_unlinkAtStop If true, the created FIFO is
	 *        unlinked (= deleted).
	 */
	void start(QString p_fifoPath, bool p_unlinkAtStop);
	/**
	 * Stops any ongoing FIFO watch.
	 *
	 * If in the prior start() call, p_unlinkAtStop was set to
	 * true, the watched FIFO will be deleted by stop().
	 *
	 * If no FIFO watch is currently active, this function
	 * does nothing.
	 */
	void stop();


signals:
	/**
	 * This signal is emitted whenever a new line has been
	 * received through the FIFO. (This implies that this
	 * signal is only ever emitted if a FIFO watch is
	 * currently active.)
	 *
	 * @param line Newly received line.
	 */
	void newFifoLine(QString line);


private slots:
	void readFromFifo(int p_socket);

private:
	void unlinkFifo();

	QSocketNotifier *m_fifoNotifier;
	QString m_fifoPath;
	QByteArray m_fifoPathAsUtf8;
	int m_fifoFd;
	bool m_unlinkAtStop;
};


} // namespace qtglviddemo end


#endif
