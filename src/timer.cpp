/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * As a special exception, you may use this file as part of a free
 * software library without restriction. Specifically, if other files
 * instantiate templates or use macros or inline functions from this
 * file, or you compile this file and link it with other files to
 * produce an executable, this file does not by itself cause the
 * resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other
 * reasons why the executable file might be covered by the GNU Library
 * General Public License.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "cxxtools/timer.h"
#include "cxxtools/clock.h"
#include "cxxtools/selector.h"
#include "cxxtools/datetime.h"
#include <limits>
#include <stdint.h>

namespace cxxtools
{

class Timer::Sentry
{
    public:
        Sentry(Sentry*& sentry)
        : _deleted(false)
        , _sentry(sentry)
        {
           sentry = this;
        }

        ~Sentry()
        {
            if( ! _deleted )
                this->detach();
        }

        bool operator!() const
        { return _deleted; }

        void detach()
        {
            _sentry = 0;
            _deleted = true;
        }

    bool _deleted;
    Sentry*& _sentry;
};


Timer::Timer(SelectorBase* selector)
: _sentry(0)
, _selector(0)
, _active(false)
, _finished(0)
{
    if (selector)
        setSelector(selector);
}


Timer::~Timer()
{
    try
    {
        if(_selector)
            _selector->remove(*this);
    }
    catch(...) {}

    if(_sentry)
        _sentry->detach();
}


bool Timer::active() const
{
    return _active;
}


const Timespan& Timer::interval() const
{
    return _interval;
}


void Timer::start(const Milliseconds& interval)
{
    if (_active)
        stop();
    
    _active = true;
    _interval = interval;
    _once = false;

    _finished = Clock::getSystemTicks() + _interval;

    if (_selector)
        _selector->onTimerChanged(*this);
}


void Timer::start(const DateTime& startTime, const Milliseconds& interval)
{
    if (_active)
        stop();
    
    _active = true;
    _interval = interval;
    _once = false;

    DateTime now = Clock::getLocalTime();
    if (startTime > now)
    {
        _finished = Clock::getSystemTicks() + (startTime - now);
    }
    else
    {
        // startTime =< now
        Timespan elapsed = now - startTime;
        unsigned ticksElapsed = elapsed.totalMSecs() / interval.totalMSecs();
        DateTime tickTime = startTime + (ticksElapsed + 1) * Timespan(interval);
        _finished = Clock::getSystemTicks() + (tickTime - now);
    }

    if (_selector)
        _selector->onTimerChanged(*this);
}

void Timer::after(const Milliseconds& interval)
{
    start(interval);
    _once = true;
}


void Timer::at(const DateTime& tickTime)
{
    if (_active)
        stop();
    
    _once = true;

    DateTime now = Clock::getLocalTime();
    if (tickTime >= now)
    {
        _active = true;
        _finished = Clock::getSystemTicks() + (tickTime - now);

        if (_selector)
            _selector->onTimerChanged(*this);
    }
}


void Timer::stop()
{
    _active = false;
    _finished = Timespan(0);

    if(_selector)
        _selector->onTimerChanged(*this);
}


bool Timer::update()
{
    if(_active == false)
        return false;

    Timespan now = Clock::getSystemTicks();
    return this->update(now);
}


bool Timer::update(const Milliseconds& now)
{
    if(_active == false)
        return false;

    bool hasElapsed = now >= _finished;

    Timer::Sentry sentry(_sentry);

    while( _active && now >= _finished )
    {
        _finished += _interval;

        if( ! sentry )
            return hasElapsed;

        timeout.send();

        if (_once)
            stop();
    }

    return hasElapsed;
}


void Timer::setSelector(SelectorBase* selector)
{
    if(_selector == selector)
        return;

    if(_selector)
    {
        _selector->onRemoveTimer(*this);
    }

    if(selector)
    {
        selector->onAddTimer(*this);
    }

    _selector = selector;
}

}
