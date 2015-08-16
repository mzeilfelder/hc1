// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#include "gui_menu_championship_winner.h"
#include "gui_menu_championship_progress.h"
#include "../gui.h"
#include "../main.h"

using namespace irr;
using namespace gui;

GuiMenuChampWinner::GuiMenuChampWinner(const Config& config)
: GuiDialog(config)
{
}

bool GuiMenuChampWinner::Load(const char* filename_, bool reloadLast_)
{
    bool ok = GuiDialog::Load(filename_, reloadLast_);
    if ( ok )
    {
		ADD_EVENT_HANDLER( "id_continue", GuiMenuChampWinner, OnButtonContinue );
    }
    return ok;
}

bool GuiMenuChampWinner::OnButtonContinue(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuChampionshipProgress() );
    }
    return false;
}

