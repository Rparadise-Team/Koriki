#ifndef _COMMANDER_ERROR_DIALOG_
#define _COMMANDER_ERROR_DIALOG_

#include <string>
#include <vector>

#include "def.h"
#include "dialog.h"

enum class ErrorDialogResult
{
    ABORT,
    CONTINUE
};

inline ErrorDialogResult ErrorDialog(
    const std::string &title, const std::string &error, bool is_last = true)
{
    CDialog dlg{title};
    dlg.setBorderColor({COLOR_BORDER_ERROR});
    dlg.addLabel(error);
    std::vector<ErrorDialogResult> options { ErrorDialogResult::ABORT };
    const auto add_option = [&](std::string text, ErrorDialogResult value) {
        dlg.addOption(text);
        options.push_back(value);
    };
    if (!is_last)
    {
        add_option("Continue", ErrorDialogResult::CONTINUE);
        add_option("Abort", ErrorDialogResult::ABORT);
    }
    else
    {
        add_option("OK", ErrorDialogResult::ABORT);
    }
    dlg.init();
    return options[dlg.execute()];
}

#endif // _COMMANDER_ERROR_DIALOG_
