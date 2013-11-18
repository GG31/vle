/*
 * @file vle/devs/EventTable.cpp
 *
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems
 * http://www.vle-project.org
 *
 * Copyright (c) 2003-2007 Gauthier Quesnel <quesnel@users.sourceforge.net>
 * Copyright (c) 2003-2012 ULCO http://www.univ-littoral.fr
 * Copyright (c) 2007-2012 INRA http://www.inra.fr
 *
 * See the AUTHORS or Authors.txt file for copyright owners and contributors
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


#include <vle/devs/EventTable.hpp>
#include <vle/devs/InternalEvent.hpp>
#include <vle/devs/ExternalEvent.hpp>
#include <vle/devs/ExternalEventList.hpp>
#include <vle/devs/RequestEventList.hpp>
#include <vle/devs/ObservationEvent.hpp>
#include <vle/utils/Debug.hpp>



namespace vle { namespace devs {

std::map < Simulator*, EventBagModel >::value_type&
    CompleteEventBagModel::topBag()
{
    while (_itbags != _bags.end()) {
        if ((*_itbags).first->dynamics()->isExecutive()) {
            _exec.push_back(&(*_itbags));
            _itexec = _exec.begin();
            _itbags++;
        } else {
            std::map < Simulator*, EventBagModel >::iterator r = _itbags;
            ++_itbags;
            return *r;
        }
    }

    while (_itexec != _exec.end()) {
        std::list < std::map < Simulator*, EventBagModel >::value_type*
            >::iterator r = _itexec;
        ++_itexec;
        return **r;
    }

    throw utils::InternalError(_("Top bag problem"));
}

void CompleteEventBagModel::invalidateModel(Simulator* mdl)
{
    (void)mdl;

    throw utils::InternalError(
        _("Do not use CompleteEventBagModel::invalidateModel, consider using "
          "CompleteEventBagModel::delModel"));
}

void CompleteEventBagModel::delModel(Simulator* mdl)
{
    assert(_itbags == _bags.end()); // normal all mdl are destroyed
                                    // after internal or external
                                    // transition.
    _states.remove(mdl);
}

EventTable::EventTable(size_t sz)
{
    mInternalEventList.reserve(sz);
}

EventTable::~EventTable()
{
    {
	InternalEventList::iterator it = mInternalEventList.begin();
	while (it != mInternalEventList.end()) {
	    delete (*it);
	    ++it;
	}
    }

    {
        ViewEventList::iterator it = mObservationEventList.begin();
	while (it != mObservationEventList.end()) {
	    delete (*it);
	    ++it;
	}
    }

    {
	for (ExternalEventModel::iterator it = mExternalEventModel.begin();
	     it != mExternalEventModel.end(); ++it) {
	    (*it).second.first.deleteAndClear();
	    (*it).second.second.deleteAndClear();
	}
    }
}

size_t EventTable::getEventNumber() const
{
    size_t sum = mObservationEventList.size() + mInternalEventList.size();

    for (ExternalEventModel::const_iterator it = mExternalEventModel.begin();
	     it != mExternalEventModel.end(); ++it) {
	sum += (*it).second.first.size() + (*it).second.second.size();
    }

    return sum;
}

void EventTable::cleanInternalEventList()
{
    while (not mInternalEventList.empty() and
	   not mInternalEventList[0]->isValid()) {
	delete mInternalEventList[0];
        std::pop_heap(mInternalEventList.begin(), mInternalEventList.end(),
                      internalLessThan);
        mInternalEventList.pop_back();
    }

}

const Time& EventTable::topEvent()
{
    if (not mExternalEventModel.empty()) {
        return mCurrentTime;
    } else {
        cleanInternalEventList();
        if (not mInternalEventList.empty()) {
            if (not mObservationEventList.empty()) {
                if (mInternalEventList[0]->getTime() <=
                    mObservationEventList.front()->getTime()) {
                    return mInternalEventList[0]->getTime();
                } else {
                    return mObservationEventList.front()->getTime();
                }
            } else {
                return mInternalEventList[0]->getTime();
            }
        } else {
            if (not mObservationEventList.empty()) {
                return mObservationEventList.front()->getTime();
            } else {
                return Time::infinity;
            }
        }
    }
}

CompleteEventBagModel& EventTable::popEvent()
{
    mCurrentTime = topEvent();

    if (mCurrentTime != Time::infinity) {
	while (not mInternalEventList.empty() and
               mInternalEventList[0]->getTime() == mCurrentTime) {
            if (mInternalEventList[0]->isValid()) {
                Simulator* mdl = mInternalEventList[0]->getModel();
                EventBagModel& bagmodel = mCompleteEventBagModel.getBag(mdl);
                bagmodel.addInternal(mInternalEventList[0]);
            }
            popInternalEvent();
	}

        while (not mExternalEventModel.empty()) {
            Simulator* mdl = (*mExternalEventModel.begin()).first;
            EventBagModel& bagmodel = mCompleteEventBagModel.getBag(mdl);
            bagmodel.addExternal((*mExternalEventModel.begin()).second.first);
            bagmodel.addRequest((*mExternalEventModel.begin()).second.second);
	    mExternalEventModel.erase(mExternalEventModel.begin());
	}

	if (mCompleteEventBagModel.emptyBag())
	  while (not mObservationEventList.empty() and
		 mObservationEventList.front()->getTime() == mCurrentTime) {
	    mCompleteEventBagModel.addState(mObservationEventList.front());
	    popObservationEvent();
	  }
    }
    mCompleteEventBagModel.init();
    return mCompleteEventBagModel;
}

bool EventTable::putInternalEvent(InternalEvent* event)
{
    mInternalEventList.push_back(event);
    std::push_heap(mInternalEventList.begin(), mInternalEventList.end(),
                   internalLessThan);

    assert(event->getModel());

    if (mInternalEventModel[event->getModel()])
      mInternalEventModel[event->getModel()]->invalidate();

    mInternalEventModel[event->getModel()] = event;
    return true;
}

bool EventTable::putExternalEvent(ExternalEvent* event)
{
    Simulator* mdl = event->getTarget();
    assert(mdl);

    mExternalEventModel[mdl].first.addEvent(event);
    InternalEventModel::iterator it = mInternalEventModel.find(mdl);
    if (it != mInternalEventModel.end() and (*it).second and
        (*it).second->getTime() > getCurrentTime()) {
	(*it).second->invalidate();
	(*it).second = 0;
    }
    return true;
}

bool EventTable::putRequestEvent(RequestEvent* event)
{
    Simulator* mdl = event->getTarget();
    assert(mdl);

    mExternalEventModel[mdl].second.addEvent(event);
    return true;
}

bool EventTable::putObservationEvent(ViewEvent* event)
{
    mObservationEventList.add(event);
    std::push_heap(mObservationEventList.begin(), mObservationEventList.end(),
                   viewEventLessThan);

    return true;
}

void EventTable::popInternalEvent()
{
    if (not mInternalEventList.empty()) {
        InternalEvent* evt = mInternalEventList[0];
        std::pop_heap(mInternalEventList.begin(), mInternalEventList.end(),
                      internalLessThan);
        mInternalEventList.pop_back();
	if (evt->isValid()) {
	    mInternalEventModel[evt->getModel()] = 0;
	} else {
	    delete evt;
	}
    }
}

void EventTable::popObservationEvent()
{
    if (not mObservationEventList.empty()) {
        std::pop_heap(mObservationEventList.begin(),
                      mObservationEventList.end(),
                      viewEventLessThan);
        mObservationEventList.pop();
    }
}

void EventTable::invalidateModel(Simulator* mdl)
{
    (void)mdl;

    throw utils::InternalError(
        _("Do not use EventTable::invalidateModel, consider using "
          "EventTable::delModelEvents"));
}

struct SetDeleter : std::unary_function < Event *, void >
{
    SetDeleter(std::list < Event * > *lst)
        : m_lst(lst)
    {}

    inline void operator()(Event *evt)
    {
        if (evt && evt->needDelete()) {
            evt->notDeleter();
            m_lst->push_front(new Event(*evt));
            m_lst->front()->deleter();
        }
    }

private:
    std::list < Event* > *m_lst;
};

void EventTable::delModelEvents(Simulator* mdl)
{
    {
        InternalEventModel::iterator it = mInternalEventModel.find(mdl);
        if (it != mInternalEventModel.end()) {
            if ((*it).second)
                (*it).second->invalidate();

            mInternalEventModel.erase(it);
        }
    }

    {
        ExternalEventModel::iterator it = mExternalEventModel.find(mdl);
        if (it != mExternalEventModel.end()) {

            std::for_each((*it).second.first.begin(),
                          (*it).second.first.end(),
                          SetDeleter(&m_toDeleteEvents));

            (*it).second.first.deleteAndClear();

            std::for_each((*it).second.second.begin(),
                          (*it).second.second.end(),
                          SetDeleter(&m_toDeleteEvents));

            (*it).second.second.deleteAndClear();

            mExternalEventModel.erase(it);
        }
    }

    mObservationEventList.remove(mdl);
    mCompleteEventBagModel.delModel(mdl);
}

}} // namespace vle devs
