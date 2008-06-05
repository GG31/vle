/**
 * @file src/vle/eov/NetStreamReader.cpp
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


#include <vle/eov/NetStreamReader.hpp>
#include <vle/utils/Debug.hpp>
#include <vle/utils/Path.hpp>
#include <gtkmm/main.h>
#include <gdkmm/drawable.h>
#include <gdkmm/gc.h>


namespace vle { namespace eov {

NetStreamReader::NetStreamReader(int port, Gtk::Main& app) :
    oov::NetStreamReader(port),
    m_thread(0),
    m_app(app),
    m_window(0)
{
}

NetStreamReader::~NetStreamReader()
{
    if (m_thread) {
        m_thread->join();
    }
}

void NetStreamReader::onParameter(const vpz::ParameterTrame& trame)
{
    Glib::Mutex::Lock lock(m_mutex);

    oov::NetStreamReader::onParameter(trame);

    if (m_thread) {
        if (m_window) {
            m_window->hide();
        }
        m_thread->join();
        m_thread = 0;
    }

    oov::PluginPtr poov = plugin();

    Assert(utils::InternalError, poov->isCairo(), boost::format(
            "Plugin '%1%' is not a oov::CairoPlugin") % trame.plugin());

    m_thread = Glib::Thread::create(
        sigc::mem_fun(this, &NetStreamReader::runWindow), true);

    m_newpluginname = trame.plugin();
}

void NetStreamReader::onNewObservable(const vpz::NewObservableTrame& trame)
{
    Glib::Mutex::Lock lock(m_mutex);

    oov::NetStreamReader::onNewObservable(trame);
}

void NetStreamReader::onDelObservable(const vpz::DelObservableTrame& trame)
{
    Glib::Mutex::Lock lock(m_mutex);

    oov::NetStreamReader::onDelObservable(trame);
}

void NetStreamReader::onValue(const vpz::ValueTrame& trame)
{
    Glib::Mutex::Lock lock(m_mutex);

    oov::NetStreamReader::onValue(trame);
}
        
void NetStreamReader::onClose(const vpz::EndTrame& trame)
{
    Glib::Mutex::Lock lock(m_mutex);
    oov::NetStreamReader::onClose(trame);
}

void NetStreamReader::getGtkPlugin(const std::string& pluginname)
{
    utils::Path::PathList lst(utils::Path::path().getStreamDirs());
    utils::Path::PathList::const_iterator it;

    std::string error((boost::format(
                "Error opening eov plugin '%1%' in:") % pluginname).str());
 
    std::string newfilename(pluginname);

    Assert(utils::InternalError, pluginname.size() > 6 and
           pluginname.compare(0, 5, "cairo") == 0, boost::format(
               "oov plugin must begin by string 'cairo': '%1%'") % pluginname);

    newfilename.replace(0, 5, "gtk");

    for (it = lst.begin(); it != lst.end(); ++it) {
        try {
            PluginFactory pf(newfilename, *it);
            m_plugin = pf.build(plugin(), this);
            return;
        } catch (const std::exception& e) {
            error += e.what();
        }
    }

    Throw(utils::InternalError, error);
}

void NetStreamReader::getDefaultPlugin()
{
    utils::Path::PathList lst(utils::Path::path().getStreamDirs());
    utils::Path::PathList::const_iterator it;
    
    std::string error((boost::format(
                "Error opening eov default plugin:")).str());

    for (it = lst.begin(); it != lst.end(); ++it) {
        try {
            PluginFactory pf("gtkdefault", *it);
            m_plugin = pf.build(plugin(), this);
            return;
        } catch (const std::exception& e) {
            error += e.what();
        }
    }

    Throw(utils::InternalError, error);
}

void NetStreamReader::runWindow()
{
    oov::CairoPluginPtr coov = oov::toCairoPlugin(plugin());

    std::string error;
    
    try {
        getGtkPlugin(m_newpluginname);
    } catch (const std::exception& e) {
        error.assign(e.what());
    }

    try {
        getDefaultPlugin();
    } catch (const std::exception& e) {
        error += '\n' + e.what();
        Throw(utils::InternalError, error);
    }

    Window mainwindow(m_plugin);
    m_window = &mainwindow;
    m_app.run(mainwindow);
    m_window = 0;
}

NetStreamReader::PluginFactory::PluginFactory(const std::string& plugin,
                                              const std::string& pathname) :
    m_module(0),
    m_plugin(plugin)
{
    std::string file(Glib::Module::build_path(pathname, plugin));
    m_module = new Glib::Module(file);
    if (not (*m_module)) {
        delete m_module;
        m_module = 0;
        Throw(utils::InternalError, boost::format(
                "\n[%1%]: %2%") % pathname % Glib::Module::get_last_error());
    }
    m_module->make_resident();
}   

NetStreamReader::PluginFactory::~PluginFactory()
{
    delete m_module;
}

PluginPtr NetStreamReader::PluginFactory::build(oov::PluginPtr oovplugin,
                                                NetStreamReader* net)
{
    Plugin* call = 0;
    void*   makeNewEovPlugin = 0;

    if (not m_module->get_symbol("makeNewEovPlugin", makeNewEovPlugin)) {
        Throw(utils::InternalError, boost::format(
                "Error when searching makeNewEovPlugin function in plugin %1%")
            % m_plugin);
    }

    call = ((Plugin*(*)(oov::PluginPtr&, NetStreamReader*))(makeNewEovPlugin))(oovplugin, net);
    if (not call) {
        Throw(utils::InternalError, boost::format(
                "Error when calling makeNewEovPlugin function in plugin %1%")
            % m_plugin);
    }

    PluginPtr plugin(call);
    return plugin;
}

}} // namespace vle eov
