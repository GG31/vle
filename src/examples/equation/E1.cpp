/**
 * @file examples/equation/E1.cpp
 * @author The VLE Development Team
 */

/*
 * VLE Environment - the multimodeling and simulation environment
 * This file is a part of the VLE environment (http://vle.univ-littoral.fr)
 * Copyright (C) 2003 - 2008 The VLE Development Team
 *
 * This program is free software: you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <examples/equation/E1.hpp>

namespace vle { namespace examples { namespace equation {

void E1::compute(const devs::Time& /* time */)
{
    a = a(-1) + 1;
    b = b(-1) + a(-1) + 1;
    c = c(-1) + b(-1) + 1;
}

void E1::initValue(const devs::Time& /* time */)
{
    init(a, 0.0);
    init(b, a(0) - 3);
    init(c, 0.0);
}

}}} // namespace vle examples equation
