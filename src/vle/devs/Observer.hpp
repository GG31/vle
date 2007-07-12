/**
 * @file devs/Observer.hpp
 * @author The VLE Development Team.
 * @brief Represent an observer on a devs::Simulator and a port name.
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

#ifndef DEVS_OBSERVER_HPP
#define DEVS_OBSERVER_HPP

#include <vle/devs/DevsTypes.hpp>
#include <vle/devs/StateEvent.hpp>
#include <vle/devs/Stream.hpp>
#include <vle/graph/AtomicModel.hpp>
#include <vle/utils/Tools.hpp>
#include <string>
#include <vector>

namespace vle { namespace devs {

    class Stream;

    //////////////////////////////////////////////////////////////////////////

    class Observable
    {
    public:
        Observable(Simulator* model, const std::string& portname) :
            m_model(model),
            m_portname(portname)
        { }

        inline Simulator* simulator() const
        { return m_model; }

        inline const std::string& portname() const
        { return m_portname; }

    private:
        Observable() :
            m_model(0)
        { }
        
        Simulator*      m_model;
	std::string     m_portname;
    };

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief Represent an observer on a devs::Simulator and a port name.
     *
     */
    class Observer
    {
    public:
        typedef std::vector < Observable > ObservableList;

        Observer(const std::string& name, Stream* stream);

        virtual ~Observer();

        StateEvent* addObservable(Simulator* model,
                                  const std::string& portName,
                                  const Time& currenttime);

        void finish();

	/**
	 * Return the model of the first element of observable list.
	 *
	 * @return a reference to the first observable.
	 */
        inline Simulator* getFirstModel() const
        { return m_observableList.front().simulator(); }

        const std::string& getFirstPortName() const;

        inline const std::string& getName() const
        { return m_name; }

        inline const ObservableList& getObservableList() const
        { return m_observableList; }

        inline unsigned int getSize() const
        { return m_size; }

        inline vle::devs::Stream * getStream() const
        { return m_stream; }

        virtual bool isEvent() const =0;

        virtual bool isTimed() const =0;

        virtual devs::StateEvent* processStateEvent(
            devs::StateEvent* p_event) =0;

	/**
	 * Delete an observable for a specified Simulator. Linear work.
	 *
	 * @param model delete observable attached to the specified
	 * Simulator.
	 */
	void removeObservable(Simulator* model);

	/**
	 * Delete an observable for a specified AtomicModel. Linear work.
	 *
	 * @param model delete observable attached to the specified
	 * AtomicModel.
	 */
        void removeObservable(graph::AtomicModel* model);

        /** 
         * @brief Test if a simulator is already connected with the same port
         * to the Observer.
         * 
         * @param simulator the simulator to observe.
         * @param portname the port of the simulator to observe.
         * 
         * @return true if simulator is already connected with the same port.
         */
        bool exist(Simulator* simulator, const std::string& portname) const;

    protected:
        ObservableList      m_observableList;
        std::string         m_name;
        Stream*             m_stream;
        size_t              m_size;
    };

}} // namespace vle devs

#endif
