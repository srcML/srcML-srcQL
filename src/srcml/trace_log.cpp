/**
 * @file trace_log.cpp
 *
 * @copyright Copyright (C) 2014 SDML (www.srcML.org)
 *
 * This file is part of the srcml command-line client.
 *
 * The srcML Toolkit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The srcML Toolkit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the srcml command-line client; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <trace_log.hpp>
#include <srcml_cli.hpp>
#include <iomanip>
 
TraceLog::TraceLog(std::ostream& out, int options)
    : out(out), count(0), overallcount(0), num_skipped(), num_error(0) {

    enabled = options & SRCML_COMMAND_VERBOSE;
}

void TraceLog::header() {

    if (!enabled)
        return;

    out << "Source encoding:  (null)\nXML encoding:  UTF-8\n";
}

void TraceLog::report() {

    if (!enabled)
        return;

    out << "\nTranslated: " << count << "\tSkipped: " << num_skipped << "\tError: " << num_error << "\tTotal: " << (count + num_skipped + num_error) << '\n';
}

TraceLog& operator<<(TraceLog& tlog, char c) {

    if (!tlog.enabled)
        return tlog;

    tlog.out << std::setw(5);
    if (c != '-')
        tlog.out << ++tlog.count;
    else {
        tlog.out << '-';
        ++tlog.num_skipped;
    }

    return tlog;
}

TraceLog& operator<<(TraceLog& tlog, const std::string& s) {

    if (!tlog.enabled)
        return tlog;

    tlog.out << ' ' << s << '\n';

    return tlog;
}
