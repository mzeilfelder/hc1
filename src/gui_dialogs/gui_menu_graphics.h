// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#ifndef GUI_MENU_GRAPHICS_H
#define GUI_MENU_GRAPHICS_H

#include "../gui_dialog.h"
#include <vector>

class GuiMenuGraphics : public GuiDialog
{
public:
    GuiMenuGraphics(const Config& config);

    virtual bool Load(const char* filename_, bool reloadLast_=false);
    virtual void Show();

protected:

    bool OnSliderResolution(const irr::SEvent &event_);
    bool OnSliderFilter(const irr::SEvent &event_);
    bool OnSliderAntialias(const irr::SEvent &event_);
    bool OnSliderLights(const irr::SEvent &event_);
    bool OnSliderTextureResolution(const irr::SEvent &event_);
    bool OnSliderParticles(const irr::SEvent &event_);
    bool OnSliderVsync(const irr::SEvent &event_);
    bool OnButtonAccept(const irr::SEvent &event_);
    bool OnButtonCancel(const irr::SEvent &event_);

	void AddNeedRestartCheck(IGUITextSlider* slider);
	bool CheckRestartNeeded(bool updateText = true);

private:
    IGUITextSlider* mSliderResolution;
    IGUITextSlider* mSliderFilter;
    IGUITextSlider* mSliderAntialias;
    IGUITextSlider* mSliderLights;
    IGUITextSlider* mSliderTextureResolution;
    IGUITextSlider* mSliderParticles;
    IGUITextSlider* mSliderVsync;
    irr::gui::IGUIStaticText* mActiveProfile;
    irr::gui::IGUIStaticText* mStaticInfo;
    irr::gui::IGUIStaticText* mStaticRestartInfo;

    struct NeedsRestart
    {
		IGUITextSlider* mTextSlider;
		irr::s32 mOldTextId;
    };
	std::vector<NeedsRestart> mNeedsRestart;
};

#endif // GUI_MENU_CREDITS_H

