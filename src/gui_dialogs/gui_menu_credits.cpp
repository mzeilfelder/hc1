// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#include "gui_menu_credits.h"
#include "gui_menu_licenses.h"
#include "gui_menu_options.h"
#include "../gui.h"
#include "../main.h"
#include "../config.h"

using namespace irr;
using namespace gui;


GuiMenuCredits::GuiMenuCredits(const Config& config)
    : GuiDialog(config)
{
    SetSuppressSceneRendering(true);
}

bool GuiMenuCredits::Load(const char* filename_, bool reloadLast_)
{
    bool ok = GuiDialog::Load(filename_, reloadLast_);
    if ( ok )
    {
		ADD_EVENT_HANDLER( "id_back", GuiMenuCredits, OnButtonBack );
		if ( GetConfig().GetUseTouchInput() != ETI_NO_TOUCH )
		{
			ADD_EVENT_HANDLER( "id_licenses", GuiMenuCredits, OnButtonLicenses );
		}
    }
    return ok;
}

bool GuiMenuCredits::OnButtonLicenses(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetGui()->PlayOverrideClickSound(GS_CLICK_CANCEL);
        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuLicenses() );
    }

    return false;
}

bool GuiMenuCredits::OnButtonBack(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetGui()->PlayOverrideClickSound(GS_CLICK_CANCEL);
        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuOptions() );
    }

    return false;
}
