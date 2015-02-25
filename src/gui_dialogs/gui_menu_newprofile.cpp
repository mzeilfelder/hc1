// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#include "gui_menu_newprofile.h"
#include "gui_menu_profiles.h"
#include "gui_menu_main.h"
#include "../gui.h"
#include "../main.h"
#include "../profiles.h"

using namespace irr;
using namespace gui;

GuiMenuNewProfile::GuiMenuNewProfile()
    : GuiDialog()
    , mFirstProfile(false)
    , mEditName(0)
{
    SetSuppressSceneRendering(true);
}

GuiMenuNewProfile::~GuiMenuNewProfile()
{
    RemoveFunctors();
}

bool GuiMenuNewProfile::Load(const char* filename_, bool reloadLast_)
{
    RemoveFunctors();
    bool ok = GuiDialog::Load(filename_, reloadLast_);
    if ( ok )
    {
        AddGuiEventFunctor( GetIdForName(std::string("id_accept")), new EventFunctor<GuiMenuNewProfile>(this, &GuiMenuNewProfile::OnButtonAccept) );
        AddGuiEventFunctor( GetIdForName(std::string("id_cancel")), new EventFunctor<GuiMenuNewProfile>(this, &GuiMenuNewProfile::OnButtonCancel) );
        AddGuiEventFunctor( GetIdForName(std::string("profilename")), new EventFunctor<GuiMenuNewProfile>(this, &GuiMenuNewProfile::OnEditName) );

        IGUIElement * root = GetDialogParent();
        if ( !root )
            return false;

        std::string errorMsg("GuiMenuNewProfile::Load");

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
    GuiDialog::Show();
}

void GuiMenuNewProfile::RemoveFunctors()
{
    if ( !IsLoaded() )
        return;
    RemoveGuiEventFunctor( GetIdForName(std::string("id_accept")) );
    RemoveGuiEventFunctor( GetIdForName(std::string("id_cancel")) );
    RemoveGuiEventFunctor( GetIdForName(std::string("profilename")) );
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
#ifndef _IRR_ANDROID_PLATFORM_
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
