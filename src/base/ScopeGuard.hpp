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


#ifndef QTGLVIDDEMO_SCOPE_GUARD_HPP
#define QTGLVIDDEMO_SCOPE_GUARD_HPP

#include <functional>
#include <qglobal.h>


namespace qtglviddemo
{


namespace detail
{


class ScopeGuardImpl
{
public:
	template < typename Func >
	explicit ScopeGuardImpl(Func &&p_func)
		: m_func(std::forward < Func > (p_func))
		, m_dismissed(false)
	{
	}

	~ScopeGuardImpl()
	{
		if (!m_dismissed)
		{
#ifdef QT_NO_EXCEPTIONS
			m_func();
#else
			// Make sure exceptions never exit the destructor, otherwise
			// undefined behavior occurs. For details about this, see
			// http://bin-login.name/ftp/pub/docs/programming_languages/cpp/cffective_cpp/MEC/MI11_FR.HTM
			try
			{
				m_func();
			}
			catch (...)
			{
			}
#endif
		}
	}

	ScopeGuardImpl(ScopeGuardImpl &&p_other)
		: m_func(std::move(p_other.m_func))
		, m_dismissed(p_other.m_dismissed)
	{
		p_other.m_dismissed = true;
	}

	/// Dismisses the scope guard, which will do nothing after this was called.
	void dismiss() const throw()
	{
		m_dismissed = true;
	}


private:
	ScopeGuardImpl(ScopeGuardImpl const &) = delete;
	ScopeGuardImpl& operator = (ScopeGuardImpl const &) = delete;

	std::function < void() > m_func;
	mutable bool m_dismissed;
};


} // namespace detail end


template < typename Func >
detail::ScopeGuardImpl makeScopeGuard(Func &&p_func)
{
	return detail::ScopeGuardImpl(std::forward < Func > (p_func));
}


} // namespace qtglviddemo end


#endif
