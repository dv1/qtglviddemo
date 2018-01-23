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


#ifndef QTGLVIDDEMO_SYSTEMSTATS_HPP
#define QTGLVIDDEMO_SYSTEMSTATS_HPP

#include <cstdint>


namespace qtglviddemo
{


/**
 * System stats measurement class.
 *
 * This class is used for getting system stats (CPU usage etc.).
 */
class SystemStats
{
public:
	/**
	 * Constructor.
	 *
	 * Sets initial usage values to 0.
	 */
	SystemStats();

	/**
	 * Update measurements.
	 *
	 * Call this regularly to get the current measurements.
	 */
	void update();

	/// Returns the current CPU usage in the 0..1 range (1 = CPU fully used).
	float getNormalizedCpuUsage() const;
	/// Returns the current memory usage in the 0..1 range (1 = memory full).
	float getNormalizedMemoryUsage() const;
	/// Returns the current memory usage in bytes.
	std::uint64_t getMemoryUsageInBytes() const;

private:
	float m_normCpuUsage, m_normMemoryUsage;
	std::uint64_t m_memoryUsage;
	int m_lastStatIdle, m_lastStatTotal;
};


} // namespace qtglviddemo end


#endif
