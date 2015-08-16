// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#include "gui_menu_licenses.h"
#include "gui_menu_credits.h"
#include "../gui.h"
#include "../main.h"

using namespace irr;
using namespace gui;


GuiMenuLicenses::GuiMenuLicenses(const Config& config)
    : GuiDialog(config)
{
    SetSuppressSceneRendering(true);
}

bool GuiMenuLicenses::Load(const char* filename_, bool reloadLast_)
{
    bool ok = GuiDialog::Load(filename_, reloadLast_);
    if ( ok )
    {
		ADD_EVENT_HANDLER( "id_back", GuiMenuLicenses, OnButtonBack );
    }
    return ok;
}

bool GuiMenuLicenses::OnButtonBack(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetGui()->PlayOverrideClickSound(GS_CLICK_CANCEL);
        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuCredits() );
    }

    return false;
}
