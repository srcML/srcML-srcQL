/**
 * @file dassert.hpp
 *
 * @copyright Copyright (C) 2013-2014 srcML, LLC. (www.srcML.org)
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
 * along with the srcML Toolkit; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef INCLUDED_DASSERT_HPP
#define INCLUDED_DASSERT_HPP

#include <iostream>
#include <assert.h>

/** Wrap C assertion to print out difference @todo use auto keyword so only run left/righthand side once and avoid side affects. */
#define dassert(LEFT, RIGHT) if(!((LEFT) == (RIGHT))) { \
        std::cerr << "'" << LEFT << "'" << "\n";        \
        std::cerr << "'" << RIGHT << "'" << "\n";       \
        std::cerr << argv[0] << ": " __FILE__ << ":" << __LINE__ << ": " << __FUNCTION__ << " Check '" << #LEFT << "' == '" << #RIGHT << "' failed.\n"; \
        exit(1); } else { std::cerr << __LINE__ << '\n'; }

#endif
