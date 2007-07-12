/**
 * @file devs/EventView.cpp
 * @author The VLE Development Team.
 * @brief Define a Event View base on devs::View class. This class
 * build state event when event are push.
 */

/*
 * Copyright (c) 2004, 2005 The VLE Development Team.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <vle/devs/EventView.hpp>
#include <vle/value/Double.hpp>
#include <iostream>



namespace vle { namespace devs {

EventView::EventView(const std::string& name, Stream* p_stream) :
    View(name, p_stream)
{
}

StateEvent* EventView::processStateEvent(StateEvent* event)
{
    value::Value val = event->getAttributeValue(event->getPortName());

    if (val.get()) {
        if (m_lastTime < event->getTime()) {
            m_stream->writeValues(event->getTime(),
                                  m_valueList,
                                  getObservableList());
            m_lastTime = event->getTime();
        }

        StreamModelPortValue::iterator it =
            m_valueList.find(StreamModelPort(event->getModel(),
                                             event->getPortName()));

        m_valueList[StreamModelPort(event->getModel(),
                                    event->getPortName())] = val;
    }
    return 0;
}

}} // namespace vle devs