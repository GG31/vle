/**
 * @file examples/equation/B1.cpp
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


#include <examples/equation/B1.hpp>

namespace vle { namespace examples { namespace equation {

B1::B1(const graph::AtomicModel& model,
       const devs::InitEventList& events) :
    extension::DifferenceEquation::Simple(model, events)
{
    b = createVar("b");
    a = createSync("a");
}

double B1::compute(const devs::Time& /* time */)
{
    return b(-1) + a(-1) + 1;
}

double B1::initValue(const devs::Time& /* time */)
{
    return a(0) - 3;
}

}}} // namespace vle examples equation

