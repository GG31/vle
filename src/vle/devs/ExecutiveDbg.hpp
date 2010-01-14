/**
 * @file vle/devs/ExecutiveDbg.hpp
 * @author The VLE Development Team
 * See the AUTHORS or Authors.txt file
 */

/*
 * VLE Environment - the multimodeling and simulation environment
 * This file is a part of the VLE environment
 * http://www.vle-project.org
 *
 * Copyright (C) 2007-2009 INRA http://www.inra.fr
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


#ifndef VLE_DEVS_EXECUTIVEDBG_HPP
#define VLE_DEVS_EXECUTIVEDBG_HPP

#include <vle/devs/DllDefines.hpp>
#include <vle/devs/DynamicsDbg.hpp>
#include <vle/devs/Executive.hpp>

namespace vle { namespace devs {

/**
 * @brief An Executive wrapper for the Barros DEVS extension. Provides a
 * mecanism to debug an Executive class.
 */
template < typename UserModel > class ExecutiveDbg : public UserModel
{
public:
    ExecutiveDbg(const ExecutiveInit& init, const InitEventList& events)
        : UserModel(init, events), mCurrentTime()
    {
        TraceExtension(fmt(_("                     %1% [DEVS] constructor"))
                       % UserModel::getModelName());
    }

    virtual ~ExecutiveDbg()
    {}

    Time init(const Time& time)
    {
        mCurrentTime = time;

        TraceDevs(fmt(_("%1$20.10g %2% [DEVS] init")) % time %
                  UserModel::getModelName());

        return UserModel::init(time);
    }

    void output(const Time& time, ExternalEventList& output) const
    {
        TraceDevs(fmt(_("%1$20.10g %2% [DEVS] output")) % time %
                  UserModel::getModelName());

        UserModel::output(time, output);

        if (output.empty()) {
            TraceDevs(fmt(
                    _("                .... %1% [DEVS] output returns empty "
                      "output")) % UserModel::getModelName());
        } else {
            TraceDevs(fmt(
                    _("                .... %1% [DEVS] output returns %2%")) %
                UserModel::getModelName() % output);
        }
    }

    Time timeAdvance() const
    {
        TraceDevs(fmt(_("                     %1% [DEVS] ta")) %
                  UserModel::getModelName());

        Time time(UserModel::timeAdvance());

        TraceDevs(fmt(_("                .... %1% [DEVS] ta returns %2%")) %
                  UserModel::getModelName() % time);

        return time;
    }

    void internalTransition(const Time& time)
    {
        mCurrentTime = time;

        TraceDevs(fmt(_("%1$20.10g %2% [DEVS] internal transition")) % time %
                  UserModel::getModelName());

        UserModel::internalTransition(time);
    }

    void externalTransition(const ExternalEventList& event, const Time& time)
    {
        mCurrentTime = time;

        TraceDevs(fmt(_("%1$20.10g %2% [DEVS] external transition: [%3%]")) %
                  time % UserModel::getModelName() % event);

        UserModel::externalTransition(event, time);
    }

    Event::EventType confluentTransitions(
        const Time& time,
        const ExternalEventList& extEventlist) const
    {
        TraceDevs(fmt(
                _("%1$20.10g %2% [DEVS] confluent transition: [%3%]")) % time %
            UserModel::getModelName() % extEventlist);

        return UserModel::confluentTransitions(time, extEventlist);
    }

    void request(const RequestEvent& event,
                 const Time& time,
                 ExternalEventList& output) const
    {
        TraceDevs(fmt(_("%1$20.10g %2% [DEVS] request: [%3%]")) % time %
                  UserModel::getModelName() % event);

        UserModel::request(event, time, output);

        if (output.empty()) {
            TraceDevs(fmt(
                    _("                .... %1% [DEVS] request returns empty "
                      "output")) % UserModel::getModelName());
        } else {
            TraceDevs(fmt(
                    _("                .... %1% [DEVS] request returns %2%")) %
                UserModel::getModelName() % output);
        }
    }

    vle::value::Value* observation(
        const ObservationEvent& event) const
    {
        TraceDevs(fmt(_("%1$20.10g %2% [DEVS] observation: [%3%]")) %
                  event.getTime() % UserModel::getModelName() % event);

        return UserModel::observation(event);
    }

    void finish()
    {
        TraceDevs(fmt(_("                     %1% [DEVS] finish")) %
                  UserModel::getModelName());

        UserModel::finish();
    }

    /**
     * @brief Build a new devs::Simulator from the dynamics library. Attach
     * to this model information of dynamics, condition and observable.
     * @param model the graph::AtomicModel reference source of
     * devs::Simulator.
     * @param dynamics the name of the dynamics to attach.
     * @param condition the name of the condition to attach.
     * @param observable the name of the observable to attach.
     * @throw utils::InternalError if dynamics not exist.
     */
    virtual const graph::AtomicModel* createModel(
        const std::string& name,
        const std::vector < std::string >& inputs,
        const std::vector < std::string >& outputs,
        const std::string& dynamics,
        const vpz::Strings& conditions,
        const std::string& observable)
    {
        std::string inputsString, outputsString, conditionsString;

        {
            std::vector < std::string >::const_iterator it;
            for (it = inputs.begin(); it != inputs.end(); ++it) {
                inputsString += *it;
                if (it + 1 != inputs.end()) {
                    inputsString += ",";
                }
            }
        }
        {
            std::vector < std::string >::const_iterator it;
            for (it = outputs.begin(); it != outputs.end(); ++it) {
                outputsString += *it;
                if (it + 1 != outputs.end()) {
                    outputsString += ",";
                }
            }
        }
        {
            vpz::Strings::const_iterator it = conditions.begin();
            while (it != conditions.begin()) {
                conditionsString += *it;
                ++it;
                if (it != conditions.end()) {
                    conditionsString += ",";
                }
            }
        }

        TraceExtension(fmt(
                _("%1$20.10g %2% [EXE] createModel "
                  "name: %3%, inputs: (%4%), output: (%5%), "
                  "dynamics %6%, conditions: (%7%), observable (%8%)")) %
            mCurrentTime % UserModel::getModelName() % name % inputsString %
            outputsString % dynamics % conditionsString % observable);

        return UserModel::createModel(name, inputs, outputs, dynamics,
                                      conditions, observable);
    }

    /**
     * @brief Build a new devs::Simulator from the vpz::Classes information.
     * @param classname the name of the class to clone.
     * @param parent the parent of the model.
     * @param modelname the new name of the model.
     * @throw utils::badArg if modelname already exist.
     */
    virtual const graph::Model* createModelFromClass(
        const std::string& classname,
        const std::string& modelname)
    {
        TraceExtension(fmt(
                _("%1$20.10g %2% [EXE] createModelFromClass "
                  "class: %3%, modelname: %4%")) % mCurrentTime %
            UserModel::getModelName() % classname % modelname);

        return UserModel::createModelFromClass(classname, modelname);
    }

    /**
     * @brief Delete the specified model from coupled model. All
     * connection are deleted, Simulator are deleted and all events are
     * deleted from event table.
     * @param modelname the name of model to delete.
     */
    virtual void delModel(const std::string& modelname)
    {
        TraceExtension(fmt(
                _("%1$20.10g %2% [EXE] delModel "
                  "model: %3%")) % mCurrentTime % UserModel::getModelName() %
            modelname);

        UserModel::delModel(modelname);
    }

    virtual void addConnection(const std::string& modelsource,
                               const std::string& outputport,
                               const std::string& modeldestination,
                               const std::string& inputport)
    {
        TraceExtension(fmt(
                _("%1$20.10g %2% [EXE] addConnection "
                  "from model: %3% port: %4% "
                  "to model %5% port: %6%")) % mCurrentTime %
            UserModel::getModelName() % modelsource % outputport %
            modeldestination % inputport);

        UserModel::addConnection(modelsource, outputport, modeldestination,
                                 inputport);
    }

    virtual void removeConnection(const std::string& modelsource,
                                  const std::string& outputport,
                                  const std::string& modeldestination,
                                  const std::string& inputport)
    {
        TraceExtension(fmt(
                _("%1$20.10g %2% [EXE] removeConnection "
                  "from model: %3% port: %4% "
                  "to model %5% port: %6%")) % mCurrentTime %
            UserModel::getModelName() % modelsource % outputport %
            modeldestination % inputport);

        UserModel::removeConnection(modelsource, outputport, modeldestination,
                                    inputport);
    }

    virtual void addInputPort(const std::string& modelName,
                              const std::string& portName)
    {
        TraceExtension(fmt(
                _("%1$20.10g %2% [EXE] addInputPort "
                  "model: %3%, port: %4%")) % mCurrentTime %
            UserModel::getModelName() % modelName % portName);

        UserModel::addInputPort(modelName, portName);
    }

    virtual void addOutputPort(const std::string& modelName,
                               const std::string& portName)
    {
        TraceExtension(fmt(
                _("%1$20.10g %2% [EXE] addOutputPort "
                  "model: %3%, port: %4%")) % mCurrentTime %
            UserModel::getModelName() % modelName % portName);

        UserModel::addOutputPort(modelName, portName);
    }

    virtual void removeInputPort(const std::string& modelName,
                                 const std::string& portName)
    {
        TraceExtension(fmt(
                _("%1$20.10g %2% [EXE] removeInputPort "
                  "model: %3%, port: %4%")) % mCurrentTime %
            UserModel::getModelName() % modelName % portName);

        UserModel::removeInputPort(modelName, portName);
    }

    virtual void removeOutputPort(const std::string& modelName,
                                  const std::string& portName)
    {
        TraceExtension(fmt(
                _("%1$20.10g %2% [EXE] removeOutputPort "
                  "model: %3%, port: %4%")) % mCurrentTime %
            UserModel::getModelName() % modelName % portName);

        UserModel::removeOutputPort(modelName, portName);
    }

private:
    devs::Time mCurrentTime;
};

}} // namespace vle devs

#define DECLARE_EXECUTIVE_DBG(mdl) \
    extern "C" { \
        vle::devs::Dynamics* \
        makeNewExecutive(const vle::devs::ExecutiveInit& init, \
                         const vle::devs::InitEventList& events) \
        { \
            return new vle::devs::ExecutiveDbg < mdl >(init, events); \
        } \
    }

#define DECLARE_NAMED_EXECUTIVE_DBG(name, mdl) \
    extern "C" { \
        vle::devs::Dynamics* \
        makeNewExecutive##name(const vle::devs::ExecutiveInit& init, \
                               const vle::devs::InitEventList& events) \
        { \
            return new vle::devs::ExecutiveDbg < mdl >(init, events); \
        } \
    }

#endif
