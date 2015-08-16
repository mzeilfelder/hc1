// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#include "gui_menu_main.h"
#include "gui_menu_rivals.h"
#include "gui_menu_championship.h"
#include "gui_menu_arcade.h"
#include "gui_menu_timeattack.h"
#include "gui_menu_selecthover.h"
#include "gui_menu_replaytheatre.h"
#include "gui_menu_highscores.h"
#include "gui_menu_options.h"
#include "gui_menu_profiles.h"
#include "gui_menu_nagscreen.h"
#include "../config.h"
#include "../gui.h"
#include "../logging.h"
#include "../main.h"
#include "../profiles.h"
#include "../string_table.h"
#include "../mobile/app_restrictions.h"
#include "../tinyxml/tinyxml.h"

using namespace irr;
using namespace gui;

GuiMenuMain::GuiMenuMain(const Config& config)
    : GuiDialog(config)
    , mButtonChampionShip(0)
    , mButtonArcade(0)
    , mButtonTimeattack(0)
    , mButtonSelectHover(0)
    , mButtonQuit(0)
    , mButtonOptions(0)
    , mButtonRivalsMode(0)
    , mActiveProfile(0)
    , mDemoText(0)
    , mDebugText(0)
{
    SetSuppressSceneRendering(true);
}

bool GuiMenuMain::Load(const char* filename_, bool reloadLast_)
{
    bool ok = GuiDialog::Load(filename_, reloadLast_);
    if ( ok )
    {
		ADD_EVENT_HANDLER( "id_partymode", GuiMenuMain, OnButtonRivals ); // partymode is now called rivalsmode. Can't change it here easily
        ADD_EVENT_HANDLER( "id_championship", GuiMenuMain, OnButtonChampionship );
        ADD_EVENT_HANDLER( "id_arcade", GuiMenuMain, OnButtonArcade );
        ADD_EVENT_HANDLER( "id_timeattack", GuiMenuMain, OnButtonTimeAttack );
        ADD_EVENT_HANDLER( "id_selecth", GuiMenuMain, OnButtonSelectHover );
        //ADD_EVENT_HANDLER( "id_replay", GuiMenuMain, OnButtonReplayTheatre );
        ADD_EVENT_HANDLER( "id_highscores", GuiMenuMain, OnButtonHighscores );
        ADD_EVENT_HANDLER( "id_options", GuiMenuMain, OnButtonOptions );
        ADD_EVENT_HANDLER( "id_profiles", GuiMenuMain, OnButtonProfiles );
        ADD_EVENT_HANDLER( "id_quit", GuiMenuMain, OnButtonQuit );

        IGUIElement * root = GetDialogParent();
        if ( !root )
            return false;

        std::string errorMsg("GuiMenuMain::Load");
        mButtonChampionShip = static_cast<IGUIButton*>(GetElementByName(root, "id_championship", errorMsg));
        mButtonArcade = static_cast<IGUIButton*>(GetElementByName(root, "id_arcade", errorMsg));
        mButtonTimeattack = static_cast<IGUIButton*>(GetElementByName(root, "id_timeattack", errorMsg));
        mButtonSelectHover = static_cast<IGUIButton*>(GetElementByName(root, "id_selecth", errorMsg));
        mButtonQuit = static_cast<IGUIButton*>(GetElementByName(root, "id_quit", errorMsg));
        mButtonOptions = static_cast<IGUIButton*>(GetElementByName(root, "id_options", errorMsg));
        mButtonRivalsMode = static_cast<IGUIButton*>(GetElementByName(root, "id_partymode", errorMsg)); // partymode is now called rivalsmode. Can't change it here easily
        mActiveProfile = static_cast<IGUIStaticText*>(GetElementByName(root, "act_profile2", errorMsg));
        mDemoText = static_cast<IGUIStaticText*>(GetElementByName(root, "demoversion", errorMsg));
        mDebugText = static_cast<IGUIStaticText*>(GetElementByName(root, "debug", errorMsg));
    }

    return ok;
}

void GuiMenuMain::Show()
{
    if ( APP.GetIrrlichtManager() && APP.GetIrrlichtManager()->GetIrrlichtDevice() )
    {
        APP.GetIrrlichtManager()->GetIrrlichtDevice()->clearSystemMessages();
    }
    GuiDialog::Show();

    bool hasProfile = APP.GetProfileManager()->GetSelectedProfile() != NULL ? true : false;

    bool hasTutorial = true;
    bool hasTrack = true;
	if ( mDemoText )
    {
		mDemoText->setVisible(false);
	}

    TiXmlElement * ele = APP.GetConfig()->GetEtcSettings();
    if ( ele && !hasTutorial)
    {
        int tut=0;
        if ( ele->Attribute("tutorial_done", &tut ) )
            hasTutorial = tut;
    }
    if ( !hasTrack && APP.GetProfileManager() )
    {
        if ( APP.GetProfileManager()->IsLevelUnlocked(18) )
            hasTrack = true;
    }

    if ( mButtonChampionShip )
        mButtonChampionShip->setEnabled(hasProfile);
    if ( mButtonArcade )
        mButtonArcade->setEnabled(hasProfile && hasTutorial && hasTrack);
    if ( mButtonTimeattack )
        mButtonTimeattack->setEnabled(hasProfile&& hasTutorial && hasTrack);
    if ( mButtonSelectHover )
        mButtonSelectHover->setEnabled(hasProfile);
    if ( mButtonOptions )
        mButtonOptions->setEnabled(hasProfile);
    if ( mButtonRivalsMode )
        mButtonRivalsMode->setEnabled(hasProfile && hasTutorial && hasTrack);

    if ( mActiveProfile )
    {
        PlayerProfile* profile = APP.GetProfileManager()->GetSelectedProfile();
        if ( profile )
        {
            mActiveProfile->setText( profile->mName.c_str() );
        }
        else
        {
            mActiveProfile->setText( APP.GetStringTable()->Get("id_showactiveprofile").c_str() );
        }
    }

#if !defined(HOVER_RELEASE)	// just to be certain it doesn't get in
	if ( mDebugText )
	{
//		mDebugText->setText( LOG.GetStream().str().c_str() );
	}
#endif
}

bool GuiMenuMain::OnButtonRivals(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuRivals() );
    }

    return false;
}

bool GuiMenuMain::OnButtonChampionship(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuChampionship() );
    }

    return false;
}

bool GuiMenuMain::OnButtonArcade(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuArcade() );
    }

    return false;
}

bool GuiMenuMain::OnButtonTimeAttack(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuTimeAttack() );
    }

    return false;
}

bool GuiMenuMain::OnButtonSelectHover(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuSelectHover() );
    }

    return false;
}

bool GuiMenuMain::OnButtonReplayTheatre(const irr::SEvent &event_)
{
//    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
//    {
//        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuReplayTheatre() );
//    }

    return false;
}

bool GuiMenuMain::OnButtonHighscores(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuHighscores() );
    }

    return false;
}

bool GuiMenuMain::OnButtonOptions(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuOptions() );
    }

    return false;
}

bool GuiMenuMain::OnButtonProfiles(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuProfiles() );
    }

    return false;
}

bool GuiMenuMain::OnButtonQuit(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        if (DO_DISPLAY_ADS)
        {
            APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuNagscreen() );
        }
        else
        {
            APP.StopApp();
        }
    }

    return false;
}
