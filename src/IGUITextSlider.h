// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#ifndef __I_GUI_TEXT_SLIDER_H_INCLUDED__
#define __I_GUI_TEXT_SLIDER_H_INCLUDED__

#include "EGUIElementTypesHC.h"

#include <irrlicht.h>

namespace irr
{
	namespace video
	{
		class ITexture;
	}
	namespace gui
	{
		class IGUIFont;
	}
}


//! Single line edit box + spin buttons
class IGUITextSlider : public irr::gui::IGUIElement
{
public:

	//! constructor
	IGUITextSlider(irr::gui::IGUIEnvironment* environment, irr::gui::IGUIElement* parent, irr::s32 id, irr::core::rect<irr::s32> rectangle, irr::f32 buttonAspectRatio)
		: irr::gui::IGUIElement((irr::gui::EGUI_ELEMENT_TYPE)EGUIET_TEXT_SLIDER, environment, parent, id, rectangle)
	{
	}

	//! destructor
	~IGUITextSlider()
	{
	}

	virtual irr::s32 getCurrentTextId() const = 0;
	virtual void setCurrentTextId(irr::s32 id_) = 0;

	virtual void addText(const wchar_t* text) = 0;
	virtual void addText(const irr::core::stringw& text) = 0;
	virtual const wchar_t* getCurrentText() const = 0;
	virtual irr::s32 getNumTexts() const = 0;
	virtual void clearTexts() = 0;

	virtual void setOverrideFont(irr::gui::IGUIFont* font=0) = 0;
	virtual void setLeftImage(irr::video::ITexture* image) = 0;
	virtual void setLeftImagePressed(irr::video::ITexture* image) = 0;
	virtual void setRightImage(irr::video::ITexture* image) = 0;
	virtual void setRightImagePressed(irr::video::ITexture* image) = 0;
	virtual void setFocusedImage(irr::video::ITexture* image) = 0;

	//! Sets if the alpha channel should be used for drawing images on the button (default is false)
	virtual void setUseAlphaChannel(bool useAlphaChannel) = 0;

	//! Returns if the alpha channel should be used for drawing images on the button
	virtual bool getUseAlphaChannel() const = 0;

	//! when the last/first text is reached, wrap around to first/last text
	//! default is true
	virtual void setWrapAround(bool wrap_) = 0;
	virtual bool getWrapAround(bool wrap_) const = 0;

	// HACK: The buttons had been too small for mobile phones.
	// But as the game always put some text in line following a text-slider we can
	// work around it. This doubles the element-height and increases the button-width somewhat.
	virtual void makeLargeButtons() = 0;
};

#endif // __I_GUI_TEXT_SLIDER_H_INCLUDED__

