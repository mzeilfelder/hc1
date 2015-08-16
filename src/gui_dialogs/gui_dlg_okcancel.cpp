// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#include "gui_dlg_okcancel.h"
#include "../gui.h"
#include "../main.h"

using namespace irr;
using namespace gui;

GuiDlgOkCancel::GuiDlgOkCancel(const Config& config)
    : GuiDialog(config)
    , mButtonOk(0)
    , mFunctorOk(0)
    , mFunctorCancel(0)
    , mInfoText(0)
{
}

GuiDlgOkCancel::~GuiDlgOkCancel()
{
    delete mFunctorOk;
    delete mFunctorCancel;
}

bool GuiDlgOkCancel::Load(const char* filename_, bool reloadLast_)
{
    bool ok = GuiDialog::Load(filename_, reloadLast_);
    if ( ok )
    {
		ADD_EVENT_HANDLER( "id_ok", GuiDlgOkCancel, OnButtonOk );
        ADD_EVENT_HANDLER( "id_cancel", GuiDlgOkCancel, OnButtonCancel );

        IGUIElement * root = GetDialogParent();
        if ( !root )
            return false;
        std::string errorMsg("GuiDlgOkCancel::Load");

        mButtonOk = static_cast<IGUIButton*>(GetElementByName(root, "id_ok", errorMsg));
        mInfoText = static_cast<IGUIStaticText*>(GetElementByName(root, "id_dummytext", errorMsg));
    }
    return ok;
}

void GuiDlgOkCancel::SetInfoText(const wchar_t * text_)
{
    if ( !mInfoText )
        return;

    mInfoText->setText( text_ );
}

void GuiDlgOkCancel::SetOkFunctor(IEventFunctor * functor_ )
{
    delete mFunctorOk;
    mFunctorOk = functor_;
}

void GuiDlgOkCancel::SetCancelFunctor(IEventFunctor * functor_ )
{
    delete mFunctorCancel;
    mFunctorCancel = functor_;
}

bool GuiDlgOkCancel::OnButtonOk(const irr::SEvent &event_)
{
    bool result = false;
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetGui()->SetModalDialog(NULL);
        if ( mFunctorOk && mFunctorOk->CallEvent(event_) )
        {
            result = true;
        }
    }
    return result;
}

bool GuiDlgOkCancel::OnButtonCancel(const irr::SEvent &event_)
{
    bool result = false;
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetGui()->SetModalDialog(NULL);
        APP.GetGui()->PlayOverrideClickSound(GS_CLICK_CANCEL);
        if ( mFunctorCancel && mFunctorCancel->CallEvent(event_) )
        {
            result = true;
        }
    }
    return result;
}

void GuiDlgOkCancel::SetFocusOkButton()
{
    GetEnvironment()->setFocus(mButtonOk);
}
