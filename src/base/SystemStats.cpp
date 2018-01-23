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


#include <unistd.h>
#include <fstream>
#include <sstream>
#include <string>
#include "SystemStats.hpp"


namespace qtglviddemo
{


SystemStats::SystemStats()
	: m_normCpuUsage(0)
	, m_normMemoryUsage(0)
	, m_memoryUsage(0)
	, m_lastStatIdle(0)
	, m_lastStatTotal(0)
{
}


void SystemStats::update()
{
	{
		// Get CPU usage from /proc/stat. For understanding the fields,
		// see https://www.idnt.net/en-GB/kb/941772

		std::ifstream statFile("/proc/stat");

		std::string line;
		std::getline(statFile, line);
		std::istringstream sstr(line);

		std::string label;
		int statUser, statNice, statSystem, statIdle, statIoWait, statIrq, statSoftIrq;
		sstr >> label >> statUser >> statNice >> statSystem >> statIdle >> statIoWait >> statIrq >> statSoftIrq;

		int total = statUser + statNice + statSystem + statIdle + statIoWait + statIrq + statSoftIrq;
		int totalDelta = total - m_lastStatTotal;

		int idleDelta = statIdle - m_lastStatIdle;

		m_lastStatTotal = total;
		m_lastStatIdle = statIdle;

		m_normCpuUsage = 1.0f - float(idleDelta) / float(totalDelta);
	}

	{
		long pagesize = sysconf(_SC_PAGESIZE);
		long totalMemory = sysconf(_SC_PHYS_PAGES) * pagesize;
		long freeMemory = sysconf(_SC_AVPHYS_PAGES) * pagesize;
		long usedMemory = totalMemory - freeMemory;

		m_memoryUsage = usedMemory;
		m_normMemoryUsage = double(usedMemory) / double(totalMemory);
	}
}


float SystemStats::getNormalizedCpuUsage() const
{
	return m_normCpuUsage;
}


float SystemStats::getNormalizedMemoryUsage() const
{
	return m_normMemoryUsage;
}


std::uint64_t SystemStats::getMemoryUsageInBytes() const
{
	return m_memoryUsage;
}


} // namespace qtglviddemo end
