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


#include "GStreamerMediaSample.hpp"


namespace qtglviddemo
{


GStreamerMediaSample::GStreamerMediaSample(GstSample *p_sample, bool p_sampleHasNewCaps)
	: m_sample(p_sample)
	, m_sampleHasNewCaps(p_sampleHasNewCaps)
{
}


GStreamerMediaSample::GStreamerMediaSample(GStreamerMediaSample && p_other)
	: m_sample(p_other.m_sample)
	, m_sampleHasNewCaps(p_other.m_sampleHasNewCaps)
{
	// Mark the other instance as moved
	p_other.m_sample = nullptr;
}


GStreamerMediaSample::~GStreamerMediaSample()
{
	if (m_sample != nullptr)
		gst_sample_unref(m_sample);
}


GStreamerMediaSample& GStreamerMediaSample::operator = (GStreamerMediaSample && p_other)
{
	// If there is currently a media sample present, get rid of it first
	if (m_sample != nullptr)
		gst_sample_unref(m_sample);

	m_sample = p_other.m_sample;
	m_sampleHasNewCaps = p_other.m_sampleHasNewCaps;

	// Mark the other instance as moved
	p_other.m_sample = nullptr;

	return *this;
}


GstSample* GStreamerMediaSample::getSample()
{
	return m_sample;
}


bool GStreamerMediaSample::sampleHasNewCaps() const
{
	return m_sampleHasNewCaps;
}


} // namespace qtglviddemo end
