// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#include "gui_menu_pause.h"
#include "gui_hud.h"
#include "gui_helper.h"
#include "gui_touch.h"
#include "../game.h"
#include "../gui.h"
#include "../main.h"
#include "../string_table.h"
#include "../sound.h"
#include "../sound_settings.h"
#include "../profiles.h"
#include "../config.h"
#include "../rivalsmode.h"
#include "../logging.h"

using namespace irr;
using namespace gui;

GuiMenuPause::GuiMenuPause(const Config& config)
    : GuiDialog(config)
    , mButtonRestart(0)
    , mButtonQuit(0)
    , mStaticTrackName(0)
    , mSfxVolumeSlider(0)
    , mMusicVolumeSlider(0)
{
}

bool GuiMenuPause::Load(const char* filename_, bool reloadLast_)
{
    bool ok = GuiDialog::Load(filename_, reloadLast_);
    if ( ok )
    {
		ADD_EVENT_HANDLER( "id_hdresume", GuiMenuPause, OnButtonResume );
        ADD_EVENT_HANDLER( "id_hdrestart", GuiMenuPause, OnButtonRestart );
        ADD_EVENT_HANDLER( "id_hdquit", GuiMenuPause, OnButtonQuit );
        ADD_EVENT_HANDLER( "sfxvolume", GuiMenuPause, OnSliderSfx );
        ADD_EVENT_HANDLER( "musicvolume", GuiMenuPause, OnSliderMusic );

        if ( GetConfig().GetUseTouchInput() != ETI_NO_TOUCH )
		{
			ADD_EVENT_HANDLER( "id_touch_controls", GuiMenuPause, OnButtonTouchControls );
		}

        const IGUIElement * root = GetDialogRoot();
        if ( !root )
            return false;

        std::string errorMsg("GuiMenuPause::Load");

        mButtonRestart = static_cast<IGUIButton*>(GetElementByName(root, "id_hdrestart", errorMsg));
        mButtonQuit = static_cast<IGUIButton*>(GetElementByName(root, "id_hdquit", errorMsg));

        mSfxVolumeSlider = static_cast<IGUITextSlider*>(GetElementByName(root, "sfxvolume", errorMsg));
        GuiHelper::FillTextSliderWithNumbers(mSfxVolumeSlider, 0, 10, 1, true);

        mMusicVolumeSlider = static_cast<IGUITextSlider*>(GetElementByName(root, "musicvolume", errorMsg));
        GuiHelper::FillTextSliderWithNumbers(mMusicVolumeSlider, 0, 10, 1, true);

        mStaticTrackName = static_cast<IGUIStaticText*>(GetElementByName(root, "id_trackname", errorMsg));
	}
    return ok;
}

void GuiMenuPause::Show()
{
    GuiDialog::Show();

    if ( mStaticTrackName )
    {
        const GameSettings& gameSettings = APP.GetGame()->GetSettings();

        mStaticTrackName->setText( gameSettings.mLevelName.c_str() );
    }

    if ( APP.GetSound() )
    {
        SoundSettings& soundSettings = APP.GetSound()->GetSettings();
        if ( mSfxVolumeSlider )
        {
            mSfxVolumeSlider->setCurrentTextId( (int)round(soundSettings.mSfxVolume*10.f) );
        }
        if ( mMusicVolumeSlider )
        {
            mMusicVolumeSlider->setCurrentTextId( (int)round(soundSettings.mMusicVolume*10.f) );
        }
    }
    else
    {
        if ( mSfxVolumeSlider )
            mSfxVolumeSlider->setEnabled(false);
        if ( mMusicVolumeSlider )
            mMusicVolumeSlider->setEnabled(false);
    }

    if ( mButtonRestart && mButtonQuit && APP.GetGame() )
    {
        const GameSettings& gameSettings = APP.GetGame()->GetSettings();
        if ( gameSettings.mGameType == GT_RIVALS )
        {
            mButtonRestart->setVisible(false);
            mButtonQuit->setText( APP.GetStringTable()->Get("id_giveup").c_str() );
        }
        else
        {
            mButtonQuit->setText( APP.GetStringTable()->Get("id_exit").c_str() );
            mButtonRestart->setVisible(true);
        }
    }

    if ( GetConfig().GetUseTouchInput() == ETI_NO_TOUCH )
	{
		const IGUIElement * root = GetDialogRoot();
		if ( root )
		{
			std::string errorMsg("");
			IGUIElement * ele = NULL;
			ele = GetElementByName(root, "id_touch_controls", errorMsg);
			if ( ele )
			{
				ele->setEnabled(false);
				ele->setVisible(false);
			}
		}
	}
}

bool GuiMenuPause::OnSliderSfx(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType != (gui::EGUI_EVENT_TYPE)EGET_TEXTSLIDER_CHANGED )
        return false;

    if ( mSfxVolumeSlider && APP.GetSound() )
    {
        SoundSettings& soundSettings = APP.GetSound()->GetSettings();
        soundSettings.mSfxVolume = (float)mSfxVolumeSlider->getCurrentTextId() * 0.1f;
    }

    return false;
}

bool GuiMenuPause::OnSliderMusic(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType != (gui::EGUI_EVENT_TYPE)EGET_TEXTSLIDER_CHANGED )
        return false;

    if ( mMusicVolumeSlider && APP.GetSound() )
    {
        SoundSettings& soundSettings = APP.GetSound()->GetSettings();
        soundSettings.mMusicVolume = (float)mMusicVolumeSlider->getCurrentTextId() * 0.1f;
    }

    return false;
}

bool GuiMenuPause::OnButtonResume(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        SaveSoundSettings();
        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiHud() );
        APP.SetMode(MODE_GAME);
        APP.GetGame()->Resume();
    }

    return false;
}

bool GuiMenuPause::OnButtonRestart(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        SaveSoundSettings();
        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiHud() );
        APP.GetGame()->PrepareStart();
        APP.SetMode(MODE_GAME);
        APP.GetGame()->Start();
    }

    return false;
}

bool GuiMenuPause::OnButtonQuit(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetGui()->PlayOverrideClickSound(GS_CLICK_CANCEL);
        SaveSoundSettings();

        const GameSettings& gameSettings = APP.GetGame()->GetSettings();
        if ( gameSettings.mGameType == GT_RIVALS )
        {
            APP.GetRivalsMode()->InfoGameGiveUp();
            APP.GetRivalsMode()->InfoContinue();
        }

        GuiHelper::SetGameModeMenu(APP.GetGame()->GetSettings().mGameType);
    }

    return false;
}

void GuiMenuPause::SaveSoundSettings()
{
    if ( APP.GetSound() )
    {
        PlayerProfile* profile = APP.GetProfileManager()->GetSelectedProfile();
        if ( profile )
        {
            profile->mSoundSettings = APP.GetSound()->GetSettings();
            APP.GetProfileManager()->SaveProfiles();
        }

        APP.GetSound()->GetSettings().WriteToXml(APP.GetConfig()->GetSoundSettings());
        // Not saving config because sound is per profile, we don't want to change the common config.
    }
}

bool GuiMenuPause::OnButtonTouchControls(const irr::SEvent &event_)
{
	if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
	{
        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiTouch() );
        APP.GetGui()->GetGuiTouch()->SetDialogState(ETDS_SETUP);
	}

    return false;
}
