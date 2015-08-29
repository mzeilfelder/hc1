// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#ifndef __C_GUI_TEXT_SLIDER_H_INCLUDED__
#define __C_GUI_TEXT_SLIDER_H_INCLUDED__

#include "IGUITextSlider.h"

namespace irr
{
	namespace gui
	{
		class IGUIButton;
		class IGUIStaticText;
	}
}

class CGUITextSlider : public IGUITextSlider
{
public:

	//! constructor
	CGUITextSlider(irr::gui::IGUIEnvironment* environment, irr::gui::IGUIElement* parent, irr::s32 id, irr::core::rect<irr::s32> rectangle, irr::f32 buttonAspectRatio);

	//! destructor
	~CGUITextSlider();

	virtual irr::s32 getCurrentTextId() const;
	virtual void setCurrentTextId(irr::s32 id_);

	virtual void addText(const wchar_t* text);
	virtual void addText(const irr::core::stringw& text);
	virtual const wchar_t* getCurrentText() const;
	virtual irr::s32 getNumTexts() const;
	virtual void clearTexts();

	virtual void setOverrideFont(irr::gui::IGUIFont* font=0);
	virtual void setLeftImage(irr::video::ITexture* image);
	virtual void setLeftImagePressed(irr::video::ITexture* image);
	virtual void setRightImage(irr::video::ITexture* image);
	virtual void setRightImagePressed(irr::video::ITexture* image);
	virtual void setFocusedImage(irr::video::ITexture* image);

	//! Sets if the alpha channel should be used for drawing images on the button (default is false)
	virtual void setUseAlphaChannel(bool useAlphaChannel);

	//! Returns if the alpha channel should be used for drawing images on the button
	virtual bool getUseAlphaChannel() const;

	//! Sets the enabled state of this element.
	virtual void setEnabled(bool enabled);

	//! draws the element and its children
	virtual void draw();

	//! called if an event happened.
	virtual bool OnEvent(const irr::SEvent& event);

	//! when the last/first text is reached, wrap around to first/last text
	virtual void setWrapAround(bool wrap_);
	virtual bool getWrapAround(bool wrap_) const;

	//! only element returned is textslider itself. No children.
	virtual IGUIElement* getElementFromPoint(const irr::core::position2d<irr::s32>& point)
	{
		if (!IsVisible)
			return 0;

		if (AbsoluteClippingRect.isPointInside(point) )
			return this;

		return 0;
	}

	// Workaround for mobile-phones with small screens
	virtual void makeLargeButtons();

protected:
	// that text is most likely not the one any other class wants to use as it's not the one displayed
	virtual const wchar_t* getText() const  { return IGUITextSlider::getText(); }

	bool hasFocus();
	void changeSlider(bool increase_);

private:
	irr::s32  mCurrentTextId;
	irr::core::array<irr::core::stringw>  mTexts;

	irr::gui::IGUIStaticText * StaticText;
	irr::gui::IGUIButton *     ButtonLeft;
	irr::gui::IGUIButton *     ButtonRight;
	irr::video::ITexture* FocusedImage;
	irr::video::ITexture* LeftButtonImage;	// Irrlicht has no getter functions yet for images so we have to keep them for the large-button hack
	irr::video::ITexture* LeftButtonPressedImage;
	irr::video::ITexture* RightButtonImage;
	irr::video::ITexture* RightButtonPressedImage;
	bool UseAlphaChannel;
	bool WrapAround;
	bool HasLargeButtons;
};

#endif // __C_GUI_TEXT_SLIDER_H_INCLUDED__

