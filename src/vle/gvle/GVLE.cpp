/**
 * @file vle/gvle/GVLE.cpp
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


#include <vle/gvle/GVLE.hpp>
#include <vle/gvle/About.hpp>
#include <vle/gvle/Message.hpp>
#include <vle/gvle/Editor.hpp>
#include <vle/gvle/ExecutionBox.hpp>
#include <vle/gvle/ExperimentBox.hpp>
#include <vle/gvle/Modeling.hpp>
#include <vle/gvle/ExperimentBox.hpp>
#include <vle/gvle/HostsBox.hpp>
#include <vle/gvle/GVLEMenuAndToolbar.hpp>
#include <vle/gvle/PreferencesBox.hpp>
#include <vle/gvle/ViewOutputBox.hpp>
#include <vle/gvle/View.hpp>
#include <vle/utils/Exception.hpp>
#include <vle/utils/Trace.hpp>
#include <vle/utils/Debug.hpp>
#include <vle/utils/Package.hpp>
#include <vle/utils/Path.hpp>
#include <vle/vpz/Vpz.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <boost/filesystem.hpp>
#include <gtkmm/filechooserdialog.h>
#include <glibmm/miscutils.h>
#include <gtkmm/stock.h>

namespace vle { namespace gvle {

const std::string GVLE::WINDOW_TITLE =
    "GVLE  " +
    std::string(VLE_VERSION) +
    std::string(VLE_EXTRA_VERSION);

GVLE::FileTreeView::FileTreeView(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gnome::Glade::Xml>& /*refGlade*/) :
    Gtk::TreeView(cobject)
{

    mRefTreeModel = Gtk::TreeStore::create(mColumns);
    set_model(mRefTreeModel);
    append_column(_("Files"), mColumns.m_col_name);
    mRefTreeSelection = get_selection();
    mIgnoredFilesList.push_front("build");
}

GVLE::FileTreeView::~FileTreeView()
{
}

void GVLE::FileTreeView::buildHierarchyDirectory(
    const Gtk::TreeModel::Row& parent, const std::string& dirname)
{
    Glib::Dir dir(dirname);
    std::list<std::string> entries (dir.begin(), dir.end());
    entries.sort();
    std::list <std::string> ::iterator it;
    for (it = entries.begin(); it != entries.end(); ++it) {
	if (((*it)[0] != '.') //Don't show hidden files
	    and (std::find(mIgnoredFilesList.begin(), mIgnoredFilesList.end(), *it)
	         == mIgnoredFilesList.end())) {
	    std::string nextpath = Glib::build_filename(dirname, *it);
	    if (isDirectory(nextpath)) {
		Gtk::TreeModel::Row row = *(mRefTreeModel->append(parent.children()));
		row[mColumns.m_col_name] = *it;
		buildHierarchy(*row, nextpath);
	    }
	}
    }
}

void GVLE::FileTreeView::buildHierarchyFile(
    const Gtk::TreeModel::Row& parent, const std::string& dirname)
{
    Glib::Dir dir(dirname);
    std::list<std::string> entries (dir.begin(), dir.end());
    entries.sort();
    std::list<std::string>::iterator it;
    for (it = entries.begin(); it != entries.end(); ++it) {
	if (((*it)[0] != '.') //Don't show hidden files
	    and (std::find(mIgnoredFilesList.begin(), mIgnoredFilesList.end(), *it)
	         == mIgnoredFilesList.end())) {
	    std::string nextpath = Glib::build_filename(dirname, *it);
	    if (not isDirectory(nextpath)) {
		Gtk::TreeModel::Row row = *(mRefTreeModel->append(parent.children()));
		row[mColumns.m_col_name] = *it;
	    }
	}
    }
}


void GVLE::FileTreeView::buildHierarchy(
    const Gtk::TreeModel::Row& parent, const std::string& dirname)
{
    buildHierarchyDirectory(parent, dirname);
    buildHierarchyFile(parent, dirname);
}

void GVLE::FileTreeView::clear()
{
    mRefTreeModel->clear();
}

void GVLE::FileTreeView::build()
{
    if (not mPackage.empty()) {
	Gtk::TreeIter iter = mRefTreeModel->append();
	Gtk::TreeModel::Row row = *iter;
	row[mColumns.m_col_name] = boost::filesystem::basename(mPackage);
	buildHierarchy(*row, mPackage);
	expand_row(Gtk::TreePath(iter), false);
    }
}

bool GVLE::FileTreeView::isDirectory(const std::string& dirname)
{
    return Glib::file_test(dirname, Glib::FILE_TEST_IS_DIR);
}

void GVLE::FileTreeView::on_row_activated(
    const Gtk::TreeModel::Path&,
    Gtk::TreeViewColumn*)
{
    Glib::RefPtr<Gtk::TreeView::Selection> refSelection	= get_selection();
    Gtk::TreeModel::const_iterator it = refSelection->get_selected();
    const Gtk::TreeModel::Row row = *it;
    const std::list<std::string>* lstpath = projectFilePath(row);

    std::string absolute_path =
	Glib::build_filename(mPackage, Glib::build_filename(*lstpath));
    if (not isDirectory(absolute_path)) {
	if (mParent->getEditor()->existTab(absolute_path)) {
	    mParent->getEditor()->focusTab(absolute_path);
	} else {
	    if (boost::filesystem::extension(absolute_path) == ".vpz") {
		mParent->getEditor()->closeVpzTab();
		if (not mParent->getEditor()->existVpzTab())
		    mParent->getEditor()->openTab(absolute_path);
	    } else {
		mParent->getEditor()->openTab(absolute_path);
	    }
	}
    }
    else {
	if (not row_expanded(Gtk::TreePath(it)))
	    expand_row(Gtk::TreePath(it), false);
	else
	    collapse_row(Gtk::TreePath(it));
    }
}

std::list<std::string>* GVLE::FileTreeView::projectFilePath(
    const Gtk::TreeRow& row)
{
    if (row.parent()) {
	std::list<std::string>* lst =
	    projectFilePath(*row.parent());
	lst->push_back(std::string(row.get_value(mColumns.m_col_name)));
	return lst;
    } else {
	return new std::list<std::string>();
    }
}

GVLE::GVLE(BaseObjectType* cobject,
	   const Glib::RefPtr<Gnome::Glade::Xml> xml):
    Gtk::Window(cobject),
    m_modeling(new Modeling(this)),
    m_currentButton(POINTER),
    m_helpbox(0)
{
    mRefXML = xml;
    m_modeling->setGlade(mRefXML);

    mGlobalVpzPrevDirPath = "";

    mConditionsBox = new ConditionsBox(mRefXML, m_modeling);
    mSimulationBox = new LaunchSimulationBox(mRefXML, m_modeling);
    mPreferencesBox = new PreferencesBox(mRefXML, m_modeling);
    mOpenPackageBox = new OpenPackageBox(mRefXML, m_modeling);
    mOpenVpzBox = new OpenVpzBox(mRefXML, m_modeling);
    mNewProjectBox = new NewProjectBox(mRefXML, m_modeling);
    mSaveVpzBox = new SaveVpzBox(mRefXML, m_modeling);

    mRefXML->get_widget("MenuAndToolbarVbox", mMenuAndToolbarVbox);
    mRefXML->get_widget("StatusBarPackageBrowser", mStatusbar);
    mRefXML->get_widget("TextViewLogPackageBrowser", mLog);
    mRefXML->get_widget_derived("FileTreeViewPackageBrowser", mFileTreeView);
    mFileTreeView->setParent(this);
    mRefXML->get_widget_derived("NotebookPackageBrowser", mEditor);
    mEditor->setParent(this);
    mRefXML->get_widget_derived("TreeViewModel", mModelTreeBox);
    mModelTreeBox->setModeling(m_modeling);
    mRefXML->get_widget_derived("TreeViewClass", mModelClassBox);
    mModelClassBox->createNewModelBox(m_modeling);

    mMenuAndToolbar = new GVLEMenuAndToolbar(this);
    mMenuAndToolbarVbox->pack_start(*mMenuAndToolbar->getMenuBar());
    mMenuAndToolbarVbox->pack_start(*mMenuAndToolbar->getToolbar());
    mMenuAndToolbar->getToolbar()->set_toolbar_style(Gtk::TOOLBAR_BOTH);

    m_modeling->setModified(false);
    set_title(WINDOW_TITLE);
    resize(900, 550);
    show();
}

GVLE::~GVLE()
{
    delete m_modeling;

    delete mConditionsBox;
    delete mSimulationBox;
    delete mPreferencesBox;
    delete mOpenPackageBox;
    delete mOpenVpzBox;
    delete mNewProjectBox;
    delete mSaveVpzBox;
}

void GVLE::show()
{
    buildPackageHierarchy();
    show_all();
}

void GVLE::setModifiedTitle(const std::string& name)
{
    if (not name.empty() and
	boost::filesystem::extension(name) == ".vpz") {
	Editor::Documents::iterator it =
	    mEditor->getDocumentsList().find(name);
	if (it != mEditor->getDocumentsList().end())
	    it->second->setTitle(Glib::path_get_basename(name),
				 getModeling()->getTopModel(),
				 true);
    }
}

void GVLE::buildPackageHierarchy()
{
    mModelTreeBox->clear();
    mModelClassBox->clear();
    mPackage = vle::utils::Path::path().getPackageDir();
    setTitle();
    mFileTreeView->clear();
    mFileTreeView->setPackage(mPackage);
    mFileTreeView->build();
}

void GVLE::setFileName(std::string name)
{
    if (not name.empty() and
	boost::filesystem::extension(name) == ".vpz") {
	m_modeling->parseXML(name);
	mMenuAndToolbar->onViewMode();
    }
    m_modeling->setModified(false);
}

void GVLE::insertLog(const std::string& text)
{
    mLog->get_buffer()->insert(
	mLog->get_buffer()->end(), text);
}

void GVLE::redrawModelTreeBox()
{
    assert(m_modeling->getTopModel());
    mModelTreeBox->parseModel(m_modeling->getTopModel());
}

void GVLE::redrawModelClassBox()
{
    mModelClassBox->parseClass();
}

void GVLE::clearModelTreeBox()
{
    mModelTreeBox->clear();
}

void GVLE::clearModelClassBox()
{
    mModelClassBox->clear();
}

void GVLE::showRowTreeBox(const std::string& name)
{
    mModelTreeBox->showRow(name);
}

void GVLE::showRowModelClassBox(const std::string& name)
{
    mModelClassBox->showRow(name);
}

bool GVLE::on_delete_event(GdkEventAny* event)
{
    if (event->type == GDK_DELETE) {
	onMenuQuit();
	return true;
    }
    return false;
}

void GVLE::onArrow()
{
    m_currentButton = POINTER;
    m_status.push(_("Selection"));
}

void GVLE::onAddModels()
{
    m_currentButton = ADDMODEL;
    m_status.push(_("Add models"));
}

void GVLE::onAddLinks()
{
    m_currentButton = ADDLINK;
    m_status.push(_("Add links"));
}

void GVLE::onDelete()
{
    m_currentButton = DELETE;
    m_status.push(_("Delete object"));
}

void GVLE::onAddCoupled()
{
    m_currentButton = ADDCOUPLED;
    m_status.push(_("Coupled Model"));
}

void GVLE::onZoom()
{
    m_currentButton = ZOOM;
    m_status.push(_("Zoom"));
}

void GVLE::onQuestion()
{
    m_currentButton = QUESTION;
    m_status.push(_("Question"));
}

void GVLE::newFile()
{
    mEditor->createBlankNewFile();
}

void GVLE::onMenuNew()
{
    if (m_modeling->isModified() == false) {
        m_modeling->delNames();
        m_modeling->start();
        redrawModelTreeBox();
    } else if (gvle::Question(_("Do you really want destroy model ?"))) {
        m_modeling->delNames();
        m_modeling->start();
        redrawModelTreeBox();
    }
}

void GVLE::onMenuNewProject()
{
    mNewProjectBox->show();
}


void GVLE::openFile()
{
    Gtk::FileChooserDialog file(_("Choose a file"), Gtk::FILE_CHOOSER_ACTION_OPEN);
    file.set_transient_for(*this);
    file.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    file.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

    if (file.run() == Gtk::RESPONSE_OK) {
	std::string selected_file = file.get_filename();
	mEditor->openTab(selected_file);
    }
}
void GVLE::onMenuOpenPackage()
{
    mEditor->closeAllTab();
    mOpenPackageBox->show();
    if (utils::Path::path().package() != "")
	mMenuAndToolbar->onPackageMode();
}

void GVLE::onMenuOpenVpz()
{
    if (m_modeling->isModified() == false or
	gvle::Question(_("Do you really want load a new Model ?\nCurrent"
			 "model will be destroy and not save"))) {
	try {
	    mOpenVpzBox->show();
	    mMenuAndToolbar->onViewMode();
	} catch(utils::InternalError) {
	    Error(_("No experiments in the package ") +
		    utils::Path::path().package());
	}
    }
}

void GVLE::onMenuLoad()
{
    if (m_modeling->isModified() == false or
            gvle::Question(_("Do you really want load a new Model ?\nCurrent"
                             "model will be destroy and not save"))) {
        Gtk::FileChooserDialog file("VPZ file", Gtk::FILE_CHOOSER_ACTION_OPEN);
        file.set_transient_for(*this);
        file.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
        file.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
        Gtk::FileFilter filter;
        filter.set_name(_("Vle Project gZipped"));
        filter.add_pattern("*.vpz");
        file.add_filter(filter);
	if (mGlobalVpzPrevDirPath != "") {
	    file.set_current_folder(mGlobalVpzPrevDirPath);
        }

        if (file.run() == Gtk::RESPONSE_OK) {
	    mGlobalVpzPrevDirPath = file.get_current_folder();
	    mEditor->closeAllTab();
            m_modeling->parseXML(file.get_filename());
	    utils::Path::path().setPackage("");
	    mMenuAndToolbar->onGlobalMode();
	    mMenuAndToolbar->onViewMode();
	    mFileTreeView->clear();
        }
    }
}

void GVLE::saveFile()
{
    int page = mEditor->get_current_page();
    if (page != -1) {
	DocumentText* doc = dynamic_cast < DocumentText* >(
	    mEditor->get_nth_page(page));
	if (not doc->isNew()) {
	    doc->save();
	} else {
	    Gtk::FileChooserDialog file(_("VPZ file"), Gtk::FILE_CHOOSER_ACTION_SAVE);
	    file.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	    file.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

	    if (file.run() == Gtk::RESPONSE_OK) {
		std::string filename(file.get_filename());
		doc->saveAs(filename);
	    }
	}
	buildPackageHierarchy();
    }
}

void GVLE::onMenuSave()
{
    std::vector<std::string> vec;
    m_modeling->vpz_is_correct(vec);

    if (vec.size() != 0) {
        //vpz is correct
        std::string error = _("Vpz incorrect :\n");
        std::vector<std::string>::const_iterator it = vec.begin();
        while (it != vec.end()) {
            error += *it + "\n";

            ++it;
        }
        Error(error);
        return;
    }

    if (m_modeling->isSaved()) {
	m_modeling->saveXML(m_modeling->getFileName());
	Editor::Documents::iterator it =
	    mEditor->getDocumentsList().find(m_modeling->getFileName());
	if (it != mEditor->getDocumentsList().end())
	    it->second->setTitle(m_modeling->getFileName(),
				 m_modeling->getTopModel(), false);
    } else if (utils::Path::path().package() != "") {
	mSaveVpzBox->show();
    } else {
	Gtk::FileChooserDialog file(_("VPZ file"), Gtk::FILE_CHOOSER_ACTION_SAVE);
	file.set_transient_for(*this);
	file.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	file.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
	Gtk::FileFilter filter;
	filter.set_name(_("Vle Project gZipped"));
	filter.add_pattern("*.vpz");
	file.add_filter(filter);

	if (file.run() == Gtk::RESPONSE_OK) {
	    std::string filename(file.get_filename());
	    vpz::Vpz::fixExtension(filename);
	    m_modeling->saveXML(filename);
	    Editor::Documents::iterator it =
		mEditor->getDocumentsList().find(filename);
	    if (it != mEditor->getDocumentsList().end())
		it->second->setTitle(filename,
				     m_modeling->getTopModel(), false);
	}
    }
}

void GVLE::saveFileAs()
{
    int page = mEditor->get_current_page();
    if (page != -1) {
	DocumentText* doc = dynamic_cast < DocumentText* >(
	    mEditor->get_nth_page(page));
	Gtk::FileChooserDialog file(_("VPZ file"), Gtk::FILE_CHOOSER_ACTION_SAVE);
	file.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	file.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

	if (file.run() == Gtk::RESPONSE_OK) {
	    std::string filename(file.get_filename());
	    doc->saveAs(filename);
	}
	buildPackageHierarchy();
    }
}

void GVLE::closeFile()
{
    int page = mEditor->get_current_page();
    if (page != -1) {
	Gtk::Widget* tab = mEditor->get_nth_page(page);
	Editor::Documents::iterator it = mEditor->getDocumentsList().begin();
	while (it != mEditor->getDocumentsList().end()) {
	    if (it->second == tab) {
		mEditor->closeTab(it->first);
		break;
	    }
	    ++it;
	}
    }
}

void GVLE::tabClosed()
{
    if (utils::Path::path().package() == "")
	mMenuAndToolbar->onGlobalMode();
    else
	mMenuAndToolbar->onPackageMode();
}

void GVLE::onMenuQuit()
{
    if (m_modeling->isModified() == true and
	gvle::Question(_("Changes have been made,\n"
                         "Do you want the model to be saved?"))) {
	std::vector<std::string> vec;
	m_modeling->vpz_is_correct(vec);
	if (vec.size() != 0) {
	    //vpz is correct
	    std::string error = _("incorrect VPZ:\n");
	    std::vector<std::string>::const_iterator it = vec.begin();
	    while (it != vec.end()) {
		error += *it + "\n";

		++it;
	    }
	    Error(error);
	    return;
	} else {
	    onMenuSave();
	}
    }
    hide();
}


void GVLE::onPreferences()
{
    mPreferencesBox->show();
}

void GVLE::onSimulationBox()
{
    if (m_modeling->isSaved()) {
        mSimulationBox->show();
    } else {
        gvle::Error(_("Save or load a project before simulation"));
    }
}

void GVLE::onParameterExecutionBox()
{
    ParameterExecutionBox* box = new ParameterExecutionBox(m_modeling);
    box->run();
    delete box;
}

void GVLE::onExperimentsBox()
{
    ExperimentBox box(mRefXML, m_modeling);
    box.run();
}

void GVLE::onConditionsBox()
{
    mConditionsBox->show();
}

void GVLE::onHostsBox()
{
    HostsBox* box = new HostsBox(mRefXML);
    box->run();
    delete box;
}

void GVLE::onHelpBox()
{
    if (m_helpbox == 0)
        m_helpbox = new HelpBox;

    m_helpbox->show_all();
}

void GVLE::onViewOutputBox()
{
    ViewOutputBox box(*m_modeling, mRefXML, m_modeling->views());
    box.run();
}

void GVLE::onShowAbout()
{
    About box(mRefXML);
    box.run();
}

void GVLE::setTitle(const Glib::ustring& name)
{
    Glib::ustring title(WINDOW_TITLE);

    if (not utils::Path::path().package().empty())
	title += " - " + utils::Path::path().package();
    if (not name.empty()) {
        title += " - " + Glib::path_get_basename(name);
    }
    set_title(title);
}

std::string valuetype_to_string(value::Value::type type)
{
    switch (type) {
    case(value::Value::BOOLEAN):
        return "boolean";
        break;
    case(value::Value::INTEGER):
        return "integer";
        break;
    case(value::Value::DOUBLE):
        return "double";
        break;
    case(value::Value::STRING):
        return "string";
        break;
    case(value::Value::SET):
        return "set";
        break;
    case(value::Value::MAP):
        return "map";
        break;
    case(value::Value::TUPLE):
        return "tuple";
        break;
    case(value::Value::TABLE):
        return "table";
        break;
    case(value::Value::XMLTYPE):
        return "xml";
        break;
    case(value::Value::NIL):
        return "null";
        break;
    case(value::Value::MATRIX):
        return "matrix";
        break;
    default:
        return "(no value)";
        break;
    }
}

void GVLE::configureProject()
{
    std::string out, err;
    utils::CMakePackage::configure(out, err);
    mLog->get_buffer()->insert(mLog->get_buffer()->end(), out);
    if (not err.empty())
	mLog->get_buffer()->insert(mLog->get_buffer()->end(), err);
}

void GVLE::buildProject()
{
    std::string out, err;
    utils::CMakePackage::build(out, err);
    mLog->get_buffer()->insert(mLog->get_buffer()->end(), out);
    if (not err.empty())
	mLog->get_buffer()->insert(mLog->get_buffer()->end(), err);
}

void GVLE::cleanProject()
{
    std::string out, err;
    utils::CMakePackage::clean(out, err);
    mLog->get_buffer()->insert(mLog->get_buffer()->end(), out);
    if (not err.empty())
	mLog->get_buffer()->insert(mLog->get_buffer()->end(), err);
}

void GVLE::packageProject()
{
    std::string out, err;
    utils::CMakePackage::package(out, err);
    mLog->get_buffer()->insert(mLog->get_buffer()->end(), out);
    if (not err.empty())
	mLog->get_buffer()->insert(mLog->get_buffer()->end(), err);
}

void GVLE::onCutModel()
{
    mCurrentView->onCutModel();
}

void GVLE::onCopyModel()
{
    mCurrentView->onCopyModel();
}

void GVLE::onPasteModel()
{
    mCurrentView->onPasteModel();
}

void GVLE::clearCurrentModel()
{
    mCurrentView->clearCurrentModel();
}

void GVLE::importModel()
{
    mCurrentView->importModel();
}

void GVLE::exportCurrentModel()
{
    mCurrentView->exportCurrentModel();
}

void GVLE::exportGraphic()
{
    ViewDrawingArea* tab = dynamic_cast<DocumentDrawingArea*>(
	mEditor->get_nth_page(mCurrentTab))->getDrawingArea();
    vpz::Experiment& experiment = m_modeling->vpz().project().experiment();
    if (experiment.name().empty() || experiment.duration() == 0) {
        Error(_("Fix a Value to the name and the duration of the experiment before exportation."));
        return;
    }

    Gtk::FileChooserDialog file(_("Image file"), Gtk::FILE_CHOOSER_ACTION_SAVE);
    file.set_transient_for(*this);
    file.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    file.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    Gtk::FileFilter filterAuto;
    Gtk::FileFilter filterPng;
    Gtk::FileFilter filterPdf;
    Gtk::FileFilter filterSvg;
    filterAuto.set_name(_("Guess type from file name"));
    filterAuto.add_pattern("*");
    filterPng.set_name(_("Portable Newtork Graphics (.png)"));
    filterPng.add_pattern("*.png");
    filterPdf.set_name(_("Portable Format Document (.pdf)"));
    filterPdf.add_pattern("*.pdf");
    filterSvg.set_name(_("Scalable Vector Graphics (.svg)"));
    filterSvg.add_pattern("*.svg");
    file.add_filter(filterAuto);
    file.add_filter(filterPng);
    file.add_filter(filterPdf);
    file.add_filter(filterSvg);


    if (file.run() == Gtk::RESPONSE_OK) {
        std::string filename(file.get_filename());
	std::string extension(file.get_filter()->get_name());

	if (extension == _("Guess type from file name")) {
	    size_t ext_pos = filename.find_last_of('.');
	    if (ext_pos != std::string::npos) {
		std::string type(filename, ext_pos+1);
		filename.resize(ext_pos);
		if (type == "png")
		    tab->exportPng(filename);
		else if (type == "pdf")
		    tab->exportPdf(filename);
		else if (type == "svg")
		    tab->exportSvg(filename);
		else
		    Error(_("Unsupported file format"));
	    }
	}
	else if (extension == _("Portable Newtork Graphics (.png)"))
	    tab->exportPng(filename);
	else if (extension == _("Portable Format Document (.pdf)"))
	    tab->exportPdf(filename);
	else if (extension == _("Scalable Vector Graphics (.svg)"))
	    tab->exportSvg(filename);
    }
}

void GVLE::addCoefZoom()
{
    ViewDrawingArea* tab = dynamic_cast<DocumentDrawingArea*>(
	mEditor->get_nth_page(mCurrentTab))->getDrawingArea();
    tab->addCoefZoom();
}

void GVLE::delCoefZoom()
{
    ViewDrawingArea* tab = dynamic_cast<DocumentDrawingArea*>(
	mEditor->get_nth_page(mCurrentTab))->getDrawingArea();
    tab->delCoefZoom();
}

void GVLE::setCoefZoom(double coef)
{
    ViewDrawingArea* tab = dynamic_cast<DocumentDrawingArea*>(
	mEditor->get_nth_page(mCurrentTab))->getDrawingArea();
    tab->setCoefZoom(coef);
}

void  GVLE::updateAdjustment(double h, double v)
{
    DocumentDrawingArea* tab = dynamic_cast<DocumentDrawingArea*>(
	mEditor->get_nth_page(mCurrentTab));
    tab->setHadjustment(h);
    tab->setVadjustment(v);
}

void GVLE::onRandomOrder()
{
    ViewDrawingArea* tab = dynamic_cast<DocumentDrawingArea*>(
	mEditor->get_nth_page(mCurrentTab))->getDrawingArea();
    tab->onRandomOrder();
}

}} // namespace vle gvle
