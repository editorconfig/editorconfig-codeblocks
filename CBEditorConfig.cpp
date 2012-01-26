#include <sdk.h> // Code::Blocks SDK
#include <configurationpanel.h>
#include <editormanager.h>
#include <manager.h>
#include "cbeditor.h"
#include "cbstyledtextctrl.h"
#include "CBEditorConfig.hpp"
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

void CBEditorConfig::LoadConfig(const wxString& fileName)
{
    editorconfig_handle eh = editorconfig_handle_init();

    /* start parsing */
    int err_num;
    if ((err_num = editorconfig_parse(fileName.ToAscii(), eh)) != 0 &&
            /* Ignore full path error, whose error code is
             * EDITORCONFIG_PARSE_NOT_FULL_PATH */
            err_num != EDITORCONFIG_PARSE_NOT_FULL_PATH) {
        wxString err_msg;
        err_msg << wxT("EditorConfig Error: ") << err_num;
        wxMessageDialog(NULL, err_msg, wxT("EditorConfig"), wxOK);
        editorconfig_handle_destroy(eh);
        return;
    }

    // apply the settings

    struct
    {
        const char*     indent_style;
        int             indent_size;
        int             tab_width;
        const char*     end_of_line;
    } ecConf; // obtained EditorConfig settings will be here

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
        else if (!strcmp(name, "indent_size"))
            ecConf.indent_size = atoi(value);
        else if (!strcmp(name, "end_of_line"))
            ecConf.end_of_line = value;
    }

    // get the cbEditor and cbStyledTextCtrl
    cbEditor *ed =
        Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    cbStyledTextCtrl* control = ed->GetControl();

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
        control->SetTabWidth(ecConf.indent_size);

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
}

void CBEditorConfig::OnEditorOpen(CodeBlocksEvent& event)
{
    LoadConfig(event.GetEditor()->GetFilename());
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

void CBEditorConfig::BuildMenu(wxMenuBar* /*menuBar*/)
{
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
