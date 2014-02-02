/*
 * Copyright (C) 2008 Tommi Maekitalo
 * 
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

#include <cxxtools/timespan.h>
#include <cxxtools/serializationinfo.h>
#include <sys/time.h>
#include <time.h>
#include <iostream>

namespace cxxtools
{
    Timespan Timespan::gettimeofday()
    {
        timeval tv;
        ::gettimeofday(&tv, 0);
        return Timespan(tv.tv_sec, tv.tv_usec);
    }

    std::ostream& operator<< (std::ostream& out, const Timespan& ts)
    {
        out << static_cast<double>(ts.totalUSecs()) / 1e6;
        return out;
    }

    std::istream& operator>> (std::istream& in, Timespan& ts)
    {
        uint64_t usecs;
        in >> usecs;
        if (in)
            ts = Timespan(usecs);
        return in;
    }

    namespace tshelper
    {
        void get(std::istream& in, Timespan& ts, uint64_t res)
        {
            double usecs;
            in >> usecs;
            if (in)
                ts = Timespan(usecs * res);
        }
    }

    void operator >>=(const SerializationInfo& si, Timespan& timespan)
    {
        double s;
        si >>= s;
        if (s >= 0)
            timespan = Timespan(static_cast<int64_t>(s * 1e6 + .5));
        else
            timespan = Timespan(static_cast<int64_t>(s * 1e6 - .5));
    }

    void operator <<=(SerializationInfo& si, const Timespan& timespan)
    {
        si.setTypeName("seconds");
        si <<= (timespan.totalUSecs() / 1e6);
    }

}
