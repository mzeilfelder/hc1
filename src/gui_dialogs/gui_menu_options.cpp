// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#include "gui_menu_options.h"
#include "gui_menu_main.h"
#include "gui_menu_controls.h"
#include "gui_menu_analog.h"
#include "gui_menu_credits.h"
#include "gui_menu_graphics.h"
#include "gui_touch.h"
#include "gui_helper.h"
#include "../profiles.h"
#include "../string_table.h"
#include "../gui.h"
#include "../main.h"
#include "../game.h"
#include "../config.h"
#include "../sound.h"
#include "../logging.h"
#include "../input_device_manager.h"
#include <string>

using namespace irr;
using namespace gui;


GuiMenuOptions::GuiMenuOptions(const Config& config)
    : GuiDialog(config)
    , mSfxVolumeSlider(0)
    , mMusicVolumeSlider(0)
    , mActiveProfile(0)
{
    SetSuppressSceneRendering(true);
}

bool GuiMenuOptions::Load(const char* filename_, bool reloadLast_)
{
    bool ok = GuiDialog::Load(filename_, reloadLast_);
    if ( ok )
    {
		if ( GetConfig().GetUseTouchInput() != ETI_NO_TOUCH )
		{
			ADD_EVENT_HANDLER( "id_touch_controls", GuiMenuOptions, OnButtonTouchControls );
		}
		else
		{
			ADD_EVENT_HANDLER( "id_controls", GuiMenuOptions, OnButtonController );
			ADD_EVENT_HANDLER( "id_analog", GuiMenuOptions, OnButtonAnalog );
		}
        ADD_EVENT_HANDLER( "id_accept", GuiMenuOptions, OnButtonAccept );
        ADD_EVENT_HANDLER( "id_cancel", GuiMenuOptions, OnButtonCancel );
        ADD_EVENT_HANDLER( "id_watchcredits", GuiMenuOptions, OnButtonCredits );
        ADD_EVENT_HANDLER( "id_graphics", GuiMenuOptions, OnButtonGraphics );
        ADD_EVENT_HANDLER( "sfxvolume", GuiMenuOptions, OnSliderSfx );
        ADD_EVENT_HANDLER( "musicvolume", GuiMenuOptions, OnSliderMusic );

        IGUIElement * root = GetDialogParent();
        if ( !root )
            return false;

        std::string errorMsg("GuiMenuOptions::Load");

        mActiveProfile = static_cast<IGUIStaticText*>(GetElementByName(root, "act_profile2", errorMsg));

        mSfxVolumeSlider = static_cast<IGUITextSlider*>(GetElementByName(root, "sfxvolume", errorMsg));
        GuiHelper::FillTextSliderWithNumbers(mSfxVolumeSlider, 0, 10, 1, true);

        mMusicVolumeSlider = static_cast<IGUITextSlider*>(GetElementByName(root, "musicvolume", errorMsg));
        GuiHelper::FillTextSliderWithNumbers(mMusicVolumeSlider, 0, 10, 1, true);
    }
    return ok;
}

void GuiMenuOptions::Show()
{
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

    GuiDialog::Show();

	IGUIElement * root = GetDialogParent();
	if ( root )
	{
		std::string errorMsg("");
        IGUIElement * ele = NULL;


		if ( GetConfig().GetUseTouchInput() != ETI_NO_TOUCH )
		{
			// No analog setup and also no normal controller setup.
			// (normal controller may make some sense on OUYA, but I can't code for it without a test-device)
			ele = GetElementByName(root, "id_controls", errorMsg);
			if ( ele )
			{
				ele->setEnabled(false);
				ele->setVisible(false);
			}
			ele = GetElementByName(root, "id_analog", errorMsg);
			if ( ele )
			{
				ele->setEnabled(false);
				ele->setVisible(false);
			}
		}
		else
		{
			ele = GetElementByName(root, "id_touch_controls", errorMsg);
			if ( ele )
			{
				ele->setEnabled(false);
				ele->setVisible(false);
			}
			ele = GetElementByName(root, "id_analog", errorMsg);
			if ( ele )
			{
				bool hasAnalogDevice = APP.GetInputDeviceManager() && (APP.GetInputDeviceManager()->GetNumJoysticks() > 0);
				ele->setEnabled(hasAnalogDevice);
			}
		}
	}
}

bool GuiMenuOptions::OnSliderSfx(const irr::SEvent &event_)
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

bool GuiMenuOptions::OnSliderMusic(const irr::SEvent &event_)
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

bool GuiMenuOptions::OnButtonTouchControls(const irr::SEvent &event_)
{
	if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
	{
        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiTouch() );
        APP.GetGui()->GetGuiTouch()->SetDialogState(ETDS_SETUP);
	}

    return false;
}

bool GuiMenuOptions::OnButtonController(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuControls() );
    }

    return false;
}

bool GuiMenuOptions::OnButtonAnalog(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuAnalog() );
    }

    return false;
}

bool GuiMenuOptions::OnButtonAccept(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
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

        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuMain() );
    }

    return false;
}

bool GuiMenuOptions::OnButtonCancel(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetGui()->PlayOverrideClickSound(GS_CLICK_CANCEL);
        APP.GetGame()->ReloadSettingsSound();
        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuMain() );
    }

    return false;
}

bool GuiMenuOptions::OnButtonCredits(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuCredits() );
    }

    return false;
}

bool GuiMenuOptions::OnButtonGraphics(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuGraphics() );
    }

    return false;
}
