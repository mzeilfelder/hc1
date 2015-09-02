// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#ifndef __C_GUI_DIALOG_ROOT_H_INCLUDED__
#define __C_GUI_DIALOG_ROOT_H_INCLUDED__

#include "irrlicht.h"

// Background element for dialogs.
// For now it just allows hiding/showing all dialog elements together. But might also be used to cache drawing later on.
class CGUIDialogRoot : public irr::gui::IGUIElement
{
public:

	//! constructor
	CGUIDialogRoot(irr::gui::IGUIEnvironment* environment, irr::gui::IGUIElement* parent, irr::s32 id, irr::core::rect<irr::s32> rectangle)
	: irr::gui::IGUIElement(irr::gui::EGUIET_ELEMENT, environment, parent, id, rectangle)
	{
	}

	virtual const irr::c8* getTypeName() const	{ return "dialog_root"; }

	// can't be clicked
	virtual bool isPointInside(const irr::core::position2d<irr::s32>& point) const
	{
		return false;
	}

};

#endif // __C_GUI_DIALOG_ROOT_H_INCLUDED__

