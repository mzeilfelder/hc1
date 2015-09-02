// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#include "gui_ai.h"

#ifdef HC1_ENABLE_EDITOR

#include "../gui_dialog.h"
#include "../gui.h"
#include "../config.h"
#include "../level.h"
#include "../level_manager.h"
#include "../recorder.h"
#include "../main.h"
#include "../game.h"
#include <assert.h>

using namespace irr;
using namespace gui;

GuiAi::GuiAi(const Config& config)
    : GuiDialog(config)
    , mWindow(NULL)
{
}

GuiAi::~GuiAi()
{
}

bool GuiAi::Load(const char* filename_, bool reloadLast_)
{
    bool ok = GuiDialog::Load(filename_, reloadLast_);
    if ( ok )
    {
		ADD_EVENT_HANDLER( "cbShowTrack", GuiAi, OnCheckBoxShowTrack );
        ADD_EVENT_HANDLER( "cbShowTrack", GuiAi, OnCheckBoxShowTrack);
        ADD_EVENT_HANDLER( "btnRecordToTrack", GuiAi, OnButtonRecordToTrack );
        ADD_EVENT_HANDLER( "btnFixedSteps", GuiAi, OnButtonFixedSteps );
        ADD_EVENT_HANDLER( "btnSmoothCenters", GuiAi, OnButtonSmoothCenters );
        ADD_EVENT_HANDLER( "btnMakeBorders", GuiAi, OnButtonMakeBorders );
        ADD_EVENT_HANDLER( "btnRealBorders", GuiAi, OnButtonRealBorders );
        ADD_EVENT_HANDLER( "btnCenters", GuiAi, OnButtonCenterCenters );
        ADD_EVENT_HANDLER( "btnSmoothAll", GuiAi, OnButtonSmoothAll );
        ADD_EVENT_HANDLER( "btnSmoothLeft", GuiAi, OnButtonSmoothLeft );
        ADD_EVENT_HANDLER( "btnSmoothRight", GuiAi, OnButtonSmoothRight );
        ADD_EVENT_HANDLER( "btnRecordSpeed", GuiAi, OnButtonRecordSpeed );
        ADD_EVENT_HANDLER( "btnRecordLine", GuiAi, OnButtonRecordLine );
        ADD_EVENT_HANDLER( "btnSmoothIdealLine", GuiAi, OnButtonSmoothIdealLine );
        ADD_EVENT_HANDLER( "btnFixTp", GuiAi, OnButtonFixTp );
        ADD_EVENT_HANDLER( "cbAiTraining", GuiAi, OnCheckBoxAiTraining );
        ADD_EVENT_HANDLER( "aidialog", GuiAi, OnWindowEvent );

        const IGUIElement * root = GetDialogRoot();
        if ( !root )
            return false;

        std::string errorMsg("GuiAi::Load");

        mWindow = static_cast<IGUIWindow*>(GetElementByName(root, "aidialog", errorMsg));
        IGUICheckBox* cb = static_cast<IGUICheckBox*>(GetElementByName(root, "cbShowTrack", errorMsg));
        if ( cb )
        {
            cb->setChecked(APP.GetDrawAiTrack());
        }
    }
    return ok;
}

bool GuiAi::GetLevelRecord(Record& record_)
{
    int levelIndex = APP.GetLevelManager()->GetCurrentLevelIndex();
    if ( levelIndex < 0 )
        return false;
    const LevelSettings & levelSettings = APP.GetLevelManager()->GetLevelSettings(levelIndex);

    std::string filenameLapRecord( APP.GetConfig()->MakeLapRecordName(levelSettings.mId, APP.GetStringTable()) );
    bool loaded = record_.Load(filenameLapRecord);
    if ( !loaded )
        return false;

    return true;
}

bool GuiAi::OnCheckBoxShowTrack(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_CHECKBOX_CHANGED )
    {
        IGUICheckBox* cb = static_cast<IGUICheckBox*>(event_.GUIEvent.Caller);
        assert(cb);

        APP.SetDrawAiTrack(cb->isChecked());
    }
    return false;
}

bool GuiAi::OnButtonRecordToTrack(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        Record record;
        if ( !GetLevelRecord(record) )
            return false;

        APP.GetLevel()->GetAiTrack().RecordCenters(record);

        return true;
    }
    return false;
}

bool GuiAi::OnButtonFixedSteps(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetLevel()->GetAiTrack().MakeFixedStepSize();
        return true;
    }
    return false;
}

bool GuiAi::OnButtonSmoothCenters(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetLevel()->GetAiTrack().SmoothTrack(false);
        return true;
    }
    return false;
}

bool GuiAi::OnButtonMakeBorders(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetLevel()->GetAiTrack().MakeBorders(100.f);
        return true;
    }
    return false;
}

bool GuiAi::OnButtonRealBorders(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        float radius = APP.GetConfig()->GetHoverRadius();
        APP.GetLevel()->GetAiTrack().FindRealBorders(radius);
        return true;
    }
    return false;
}

bool GuiAi::OnButtonCenterCenters(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetLevel()->GetAiTrack().CalculateCenters();
        return true;
    }
    return false;
}

bool GuiAi::OnButtonSmoothAll(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetLevel()->GetAiTrack().SmoothTrack(true);
        return true;
    }
    return false;
}

bool GuiAi::OnButtonSmoothLeft(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetLevel()->GetAiTrack().SmoothBorders(/*left_*/ true, /*right_*/ false);
        return true;
    }
    return false;
}

bool GuiAi::OnButtonSmoothRight(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetLevel()->GetAiTrack().SmoothBorders(/*left_*/ false, /*right_*/ true);
        return true;
    }
    return false;
}

bool GuiAi::OnButtonRecordSpeed(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        Record record;
        if ( !GetLevelRecord(record) )
            return false;

        APP.GetLevel()->GetAiTrack().RecordSpeed(record);
        return true;
    }
    return false;
}

bool GuiAi::OnButtonRecordLine(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        Record record;
        if ( !GetLevelRecord(record) )
            return false;

        APP.GetLevel()->GetAiTrack().RecordIdealLine(record);
        return true;
    }
    return false;
}

bool GuiAi::OnButtonSmoothIdealLine(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        APP.GetLevel()->GetAiTrack().SmoothIdealLine();
        return true;
    }
    return false;
}

bool GuiAi::OnButtonFixTp(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_BUTTON_CLICKED )
    {
        TrackMarker& tpTarget = APP.GetLevel()->GetTpSource();
        APP.GetLevel()->GetAiTrack().AddTrackBehindTp(&tpTarget);
        return true;
    }
    return false;
}

bool GuiAi::OnCheckBoxAiTraining(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_CHECKBOX_CHANGED )
    {
        IGUICheckBox* cb = static_cast<IGUICheckBox*>(event_.GUIEvent.Caller);
        assert(cb);

        APP.GetGame()->SetAiTraining(cb->isChecked());

        return true;
    }
    return false;
}

bool GuiAi::OnWindowEvent(const irr::SEvent &event_)
{
    if ( event_.GUIEvent.EventType == EGET_ELEMENT_CLOSED )
    {
        Hide();
        return true;
    }

    return false;
}

#endif // HC1_ENABLE_EDITOR
