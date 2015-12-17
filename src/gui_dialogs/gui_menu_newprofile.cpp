// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#include "gui_menu_newprofile.h"
#include "gui_menu_profiles.h"
#include "gui_menu_main.h"
#include "../gui.h"
#include "../main.h"
#include "../profiles.h"
#include "../config.h"
#include "../string_table.h"
#include "../tinyxml/tinyxml.h"

using namespace irr;
using namespace gui;

GuiMenuNewProfile::GuiMenuNewProfile(const Config& config)
    : GuiDialog(config)
    , mFirstProfile(false)
    , mSliderSelectKb(0)
    , mEditName(0)
{
    SetSuppressSceneRendering(true);
}

bool GuiMenuNewProfile::Load(const char* filename_, bool reloadLast_)
{
    bool ok = GuiDialog::Load(filename_, reloadLast_);
    if ( ok )
    {
		ADD_EVENT_HANDLER( "id_accept", GuiMenuNewProfile, OnButtonAccept );
        ADD_EVENT_HANDLER( "id_cancel", GuiMenuNewProfile, OnButtonCancel );
        ADD_EVENT_HANDLER( "profilename", GuiMenuNewProfile, OnEditName );

        const IGUIElement * root = GetDialogRoot();
        if ( !root )
            return false;

        std::string errorMsg("GuiMenuNewProfile::Load");

        if ( GetConfig().GetUseTouchInput() != ETI_NO_TOUCH )
		{
			mSliderSelectKb = static_cast<IGUITextSlider*>(GetElementByName(root, "slider_kb", errorMsg));
			if ( mSliderSelectKb )
			{
				mSliderSelectKb->clearTexts();
				mSliderSelectKb->addText( APP.GetStringTable()->Get("id_kb_system").c_str() );
				mSliderSelectKb->addText( APP.GetStringTable()->Get("id_kb_alternative").c_str() );
				ADD_EVENT_HANDLER( "slider_kb", GuiMenuNewProfile, OnSliderSelectKb );
			}
		}

        mEditName = static_cast<IGUIEditBox*>(GetElementByName(root, "profilename", errorMsg));
        if ( mEditName )
        {
            mEditName->setMax(11);
        }
    }
    return ok;
}

void GuiMenuNewProfile::Show()
{
    if ( mEditName )
    {
        mEditName->setText(L"");
    }

    if ( GetConfig().GetUseTouchInput() != ETI_NO_TOUCH && mSliderSelectKb )
	{
		mSliderSelectKb->setCurrentTextId(GetConfig().GetVirtualKeyboard());
	}

    GuiDialog::Show();
}

bool GuiMenuNewProfile::OnButtonAccept(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        if ( mEditName )
        {
            std::wstring wName(mEditName->getText());
            if ( wName.size() )
            {
                APP.GetProfileManager()->AddProfile(wName);
                APP.GetProfileManager()->SaveProfiles();
            }
        }

        if ( mFirstProfile )
        {
            mFirstProfile = false;
            APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuMain() );
        }
        else
        {
            APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuProfiles() );
        }
    }

    return false;
}

bool GuiMenuNewProfile::OnButtonCancel(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetGui()->PlayOverrideClickSound(GS_CLICK_CANCEL);
        if ( mFirstProfile )
        {
            mFirstProfile = false;
            APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuMain() );
        }
        else
        {
            APP.GetGui()->SetActiveDialog( APP.GetGui()->GetGuiMenuProfiles() );
        }
    }

    return false;
}

bool GuiMenuNewProfile::OnEditName(const irr::SEvent &event_)
{
#ifndef __ANDROID__
    if ( event_.GUIEvent.EventType == EGET_EDITBOX_ENTER )
    {
        if ( GetEnvironment() )
        {
        	IGUIElement* next = GetEnvironment()->getNextElement(false, false);
        	GetEnvironment()->setFocus(next);
        }
    }
    else if ( event_.GUIEvent.EventType == EGET_EDITBOX_CHANGED )
    {
		core::stringw text(event_.GUIEvent.Caller->getText());
		u32 oldsize = text.size();
		text.removeChars( core::stringw(L"%"));
		if ( oldsize != text.size() )
		{
			event_.GUIEvent.Caller->setText(text.c_str());
		}
    }
#endif
    return false;
}

bool GuiMenuNewProfile::OnSliderSelectKb(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == (gui::EGUI_EVENT_TYPE)EGET_TEXTSLIDER_CHANGED && mSliderSelectKb )
	{
		TiXmlElement * ele = APP.GetConfig()->GetEtcSettings();
		if ( ele )
		{
			ele->SetAttribute("keyboard", mSliderSelectKb->getCurrentTextId() );
			APP.GetConfig()->Save();
		}
		gui::IGUIEnvironment* env = APP.GetIrrlichtManager()->GetIrrlichtDevice()->getGUIEnvironment();
		if ( env )
		{
			env->setFocus(mEditName);
		}
    }

	return false;
}
