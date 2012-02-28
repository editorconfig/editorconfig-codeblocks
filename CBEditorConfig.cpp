// Copyright (c) 2012 EditorConfig Team
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

#include <sdk.h> // Code::Blocks SDK
#include <configurationpanel.h>
#include <editormanager.h>
#include <manager.h>
#include "cbeditor.h"
#include "cbstyledtextctrl.h"
#include "CBEditorConfig.hpp"
#include <wx/event.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <editorconfig/editorconfig.h>

// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
    PluginRegistrant<CBEditorConfig> reg(_T("CBEditorConfig"));
}

// constructor
CBEditorConfig::CBEditorConfig()
{
    // Make sure our resources are available.
    // In the generated boilerplate code we have no resources but when
    // we add some, it will be nice that this code is in place already ;)
    if (!Manager::LoadResource(_T("CBEditorConfig.zip"))) {
        NotifyMissingFile(_T("CBEditorConfig.zip"));
    }
}

// destructor
CBEditorConfig::~CBEditorConfig()
{
}

void CBEditorConfig::OnAttach()
{
    // do initialization

    // register event
    Manager::Get()->RegisterEventSink(cbEVT_EDITOR_OPEN,
            new cbEventFunctor<CBEditorConfig, CodeBlocksEvent>(this,
                &CBEditorConfig::OnEditorOpen));
}

int CBEditorConfig::LoadConfig()
{
    // get the cbEditor and cbStyledTextCtrl
    cbEditor *ed =
        Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();

    if (!ed)
        return -1;

    cbStyledTextCtrl* control = ed->GetControl();

    if (!control)
        return -1;

    editorconfig_handle eh = editorconfig_handle_init();

    /* start parsing */
    int err_num;
    if ((err_num = editorconfig_parse(ed->GetFilename().ToAscii(), eh)) != 0 &&
            /* Ignore full path error, whose error code is
             * EDITORCONFIG_PARSE_NOT_FULL_PATH */
            err_num != EDITORCONFIG_PARSE_NOT_FULL_PATH) {
        editorconfig_handle_destroy(eh);
        return err_num;
    }

    // apply the settings

    struct
    {
        const char*     indent_style;
#define INDENT_SIZE_TAB (-1000) /* indent_size = -1000 means indent_size = tab*/
        int             indent_size;
        int             tab_width;
        const char*     end_of_line;
    } ecConf; // obtained EditorConfig settings will be here

    memset(&ecConf, 0, sizeof(ecConf));

    int name_value_count = editorconfig_handle_get_name_value_count(eh);

    // get settings
    for (int i = 0; i < name_value_count; ++i) {
        const char* name;
        const char* value;

        editorconfig_handle_get_name_value(eh, i, &name, &value);

        if (!strcmp(name, "indent_style"))
            ecConf.indent_style = value;
        else if (!strcmp(name, "tab_width"))
            ecConf.tab_width = atoi(value);
        else if (!strcmp(name, "indent_size")) {
            int     value_i = atoi(value);

            if (!strcmp(value, "tab"))
                ecConf.indent_size = INDENT_SIZE_TAB;
            else if (value_i > 0)
                ecConf.indent_size = value_i;
        }
        else if (!strcmp(name, "end_of_line"))
            ecConf.end_of_line = value;
    }

    if (ecConf.indent_style) {
        if (!strcmp(ecConf.indent_style, "tab"))
            control->SetUseTabs(true);
        else if (!strcmp(ecConf.indent_style, "space"))
            control->SetUseTabs(false);
    }
    if (ecConf.indent_size > 0) {
        control->SetIndent(ecConf.indent_size);

        // We set the tab width here, so that this could be overrided then
        // if ecConf.tab_wdith > 0
        control->SetTabWidth(ecConf.indent_size);
    }

    if (ecConf.tab_width > 0)
        control->SetTabWidth(ecConf.tab_width);

    if (ecConf.indent_size == INDENT_SIZE_TAB)
        /* set indent_size to tab_width here */
        control->SetIndent(control->GetTabWidth());

    // set eol
    if (ecConf.end_of_line) {
        if (!strcmp(ecConf.end_of_line, "lf"))
            control->SetEOLMode(wxSCI_EOL_LF);
        else if (!strcmp(ecConf.end_of_line, "crlf"))
            control->SetEOLMode(wxSCI_EOL_CRLF);
        else if (!strcmp(ecConf.end_of_line, "cr"))
            control->SetEOLMode(wxSCI_EOL_CR);
    }

    editorconfig_handle_destroy(eh);

    return 0;
}

void CBEditorConfig::OnEditorOpen(CodeBlocksEvent& /* event */)
{
    LoadConfig();
}

void CBEditorConfig::OnRelease(bool /* appShutDown */)
{
}

int CBEditorConfig::Configure()
{
    return 0;
}

int CBEditorConfig::GetConfigurationPriority() const
{
    return  50;
}

int CBEditorConfig::GetConfigurationGroup() const
{
    return  cgUnknown;
}

void CBEditorConfig::BuildMenu(wxMenuBar* menuBar)
{
    // Add a menu under "Plugins"
    int pluginMenuPos = menuBar->FindMenu(_T("Plugins"));

    if (pluginMenuPos == wxNOT_FOUND)
        return;

    wxMenu* pluginMenu = menuBar->GetMenu(pluginMenuPos);
    long menuIdReload = wxNewId();
    pluginMenu->Prepend(menuIdReload, _T("Reload &EditorConfig"),
            _T("Reload EditorConfig"));

    //event handling
    Connect(menuIdReload, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(CBEditorConfig::OnReloadEditorConfig));

    return;
}

void CBEditorConfig::BuildModuleMenu(const ModuleType /*type*/,
                                     wxMenu* /*menu*/,
                                     const FileTreeData* /*data*/)
{
    return;
}

bool CBEditorConfig::BuildToolBar(wxToolBar* /*toolBar*/)
{
    return false;
}

void CBEditorConfig::OnReloadEditorConfig(wxCommandEvent& event)
{
    int err_num;

    // Reload EditorConfig
    if ((err_num = LoadConfig()) != 0) {
        wxString err_msg;

        err_msg << wxT("EditorConfig Error: ") <<
            wxString(editorconfig_get_error_msg(err_num), wxConvUTF8) <<
            _T("(") << err_num << _T(")");
        wxMessageDialog(NULL, err_msg, wxT("EditorConfig"), wxOK).ShowModal();

        return;
    }

    wxMessageDialog(NULL, _T("EditorConfig successfully reloaded."),
            _T("EditorConfig"), wxOK).ShowModal();
}
