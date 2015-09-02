// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#include "gui_dialog.h"
#include "CGUIDialogRoot.h"
#include "irrlicht_manager.h"
#include "config.h"
#include "main.h"
#include "gui_ids.h"
#include "gui.h"
#include "tinyxml/tinyxml.h"
#include "string_table.h"
#include "font_manager.h"
#include "logging.h"
#include "globals.h"
#include "CGUITextSlider.h"

#include <iostream>
#include <sstream>

using namespace irr;

int GuiDialog::mHighestId = GUI_MAX_ID+1;
int GuiDialog::mDefaultScreenWidth = 640;
int GuiDialog::mDefaultScreenHeight = 480;

GuiDialog::GuiDialog(const Config& config)
: mConfig(config)
, mDialogRoot(0)
, mDisplayMode(EDM_NORMAL)
, mScaleX(1.f)
, mScaleY(1.f)
, mHighestFocusId(0)
, mSkin(NULL)
, mOldSkin(NULL)
, mDefaultFocus(NULL)
, mAlternateDefaultFocus(NULL)
, mIsLoaded(false)
, mSuppressSceneRendering(false)
, mIsVisible(false)
, mDoCallOnUpdate(false)
{
}

GuiDialog::~GuiDialog()
{
    Clear();
}

void GuiDialog::Clear()
{
	AutoRemoveFunctors();

    mDefaultFocus = NULL;
    mAlternateDefaultFocus = NULL;

    irr::gui::IGUIEnvironment *env = GetEnvironment();
    irr::gui::IGUIElement* root = NULL;
    if ( env )
        root = env->getRootGUIElement();

    if ( !root )
    {
        mGUIElements.clear();
        return;
    }

    if ( mDialogRoot )
    {
    	if ( mDialogRoot->isMyChild(env->getFocus()) )
    	{
			env->setFocus(0);
        }
		mDialogRoot->remove();
		mDialogRoot = 0;
    }
    mGUIElements.clear();

    if ( mSkin )
    {
        mSkin->drop();
        mSkin = NULL;
    }
    if ( mOldSkin )
    {
        mOldSkin->drop();
        mOldSkin = NULL;
    }
    mIdNameMap.clear();
}

void GuiDialog::CheckScaling()
{
    irr::video::IVideoDriver* driver = GetDriver();
    if ( driver )
	{
		mScreenDim = driver->getScreenSize();
		mScaleX = (float)mScreenDim.Width / (float)mDefaultScreenWidth;
		mScaleY = (float)mScreenDim.Height  / (float)mDefaultScreenHeight;

		// triple-screen mode?
		if ( mConfig.DoesNeedTripleHeadMode(mScreenDim.Width, mScreenDim.Height) )
		{
			mDisplayMode = EDM_TRIPLE_HEAD;
			mScaleX /= 3.f;
		}
		else
		{
			mDisplayMode = EDM_NORMAL;
		}
	}
}

bool GuiDialog::Load(const char* filename_, bool reloadLast_)
{
	LOG.DebugLn("Load Dialog ", filename_);
    Hide();
    Clear();
    CheckScaling();
    mIsLoaded = false;
    if ( !reloadLast_ )
    {
        mFilename = filename_;
    }
    TiXmlDocument xmlDoc(mFilename.c_str());
    xmlDoc.SetIrrFs(GetFileSystem(), TiXmlDocument::E_ON_READ_ANDROID);

	if ( !xmlDoc.LoadFile() )
	{
		LOG.DebugLn("TiXmlDocument::ErrorDesc: ", xmlDoc.ErrorDesc());
		return mIsLoaded;
	}

    TiXmlNode * nodeDialog = xmlDoc.FirstChild("dialog");
	if (!nodeDialog)
		return mIsLoaded;

    irr::gui::IGUIEnvironment *env = GetEnvironment();
    if ( !env )
    {
        return mIsLoaded;
    }

	irr::core::recti rootRect(0, 0, (irr::s32)(mDefaultScreenWidth*mScaleX), (irr::s32)(mDefaultScreenHeight*mScaleY));
    mDialogRoot = new CGUIDialogRoot(env, env->getRootGUIElement(), -1, rootRect);

    TiXmlElement * elementDialog = nodeDialog->ToElement();
    CreateSkinFromXml(elementDialog);
    CreateStyleTemplates(elementDialog);
    CreateElementsFromXml(nodeDialog->ToElement(), mDialogRoot);

    mIsLoaded = true;

    Show();

    return mIsLoaded;
}

void GuiDialog::Show()
{
    LOG.LogLn( LP_DEBUG, "Show dialog ", mFilename.c_str() );

    irr::gui::IGUIEnvironment *env = GetEnvironment();
    if ( !env )
        return;

    mIsVisible = true;
    if ( mOldSkin )
    {
        mOldSkin->drop();
        mOldSkin = 0;
    }
    mOldSkin = env->getSkin();
    mOldSkin->grab();
    if ( mSkin )
    {
        env->setSkin(mSkin);
    }
    if ( mDialogRoot )
		mDialogRoot->setVisible(true);

    LOG.LogLn( LP_DEBUG, "Show finished" );
}

void GuiDialog::Hide()
{
    mIsVisible = false;
    irr::gui::IGUIEnvironment *env = GetEnvironment();
    if ( !env )
        return;

    if ( mOldSkin )
    {
        env->setSkin(mOldSkin);
        mOldSkin->drop();
        mOldSkin = 0;
    }

    if ( mDialogRoot )
	{
		if ( mDialogRoot->isMyChild(env->getFocus()) )
		{
			env->setFocus(0);
		}
		mDialogRoot->setVisible(false);
	}
}

void GuiDialog::Enable()
{
    std::vector<irr::gui::IGUIElement*>::iterator it;
    for ( it = mGUIElements.begin(); it != mGUIElements.end(); ++it )
    {
        (*it)->setEnabled(true);
    }
}

void GuiDialog::Disable()
{
	irr::gui::IGUIEnvironment *env = GetEnvironment();
    std::vector<irr::gui::IGUIElement*>::iterator it;
    for ( it = mGUIElements.begin(); it != mGUIElements.end(); ++it )
    {
        (*it)->setEnabled(false);
        if ( env->hasFocus(*it) )
        {
			env->setFocus(0);
        }
    }
}

void GuiDialog::SetActiveFocus()
{
    irr::gui::IGUIEnvironment *env = GetEnvironment();
    if ( !env )
        return;

    if ( GetDefaultFocus() && AllowedToGetFocus(GetDefaultFocus()) )
        env->setFocus( GetDefaultFocus() );
    else if ( GetAlternateDefaultFocus() && AllowedToGetFocus(GetAlternateDefaultFocus()) )
        env->setFocus( GetAlternateDefaultFocus() );
    else
    {
        std::vector<irr::gui::IGUIElement*>::iterator it;
        for ( it = mGUIElements.begin(); it != mGUIElements.end(); ++it )
        {
            if ( AllowedToGetFocus(*it) )
            {
                env->setFocus( (*it) );
                break;
            }
        }
    }
}

irr::gui::IGUIEnvironment * GuiDialog::GetEnvironment()
{
    if ( !APP.GetIrrlichtManager() )
        return NULL;
    if (!APP.GetIrrlichtManager()->GetIrrlichtDevice())
        return NULL;
    return APP.GetIrrlichtManager()->GetIrrlichtDevice()->getGUIEnvironment();
}

const irr::gui::IGUIElement* GuiDialog::GetDialogRoot()
{
	return mDialogRoot;
}

irr::video::IVideoDriver* GuiDialog::GetDriver()
{
   if ( APP.GetIrrlichtManager() )
		return APP.GetIrrlichtManager()->GetVideoDriver();
	return NULL;
}

irr::io::IFileSystem * GuiDialog::GetFileSystem()
{
	if ( APP.GetIrrlichtManager() && APP.GetIrrlichtManager()->GetIrrlichtDevice() )
		return APP.GetIrrlichtManager()->GetIrrlichtDevice()->getFileSystem();
	return NULL;
}

void GuiDialog::CreateSkinFromXml(TiXmlElement * xmlParent_)
{
    if ( !xmlParent_)
        return;

    TiXmlNode* nodeSkin = xmlParent_->IterateChildren("skin", NULL);
    if ( nodeSkin )
    {
        TiXmlElement * elementSkin = nodeSkin->ToElement();
        if ( elementSkin )
        {
            if ( mSkin )
            {
                mSkin->drop();
                mSkin = NULL;
            }
            mSkin = AddSkin(elementSkin);
        }
    }
}

void GuiDialog::CreateStyleTemplates(TiXmlElement * xmlParent_)
{
    if ( !xmlParent_)
        return;

    // Create a map with the style elements (could also be called templates)
    mStyleMap.clear();
    TiXmlNode* nodeStyle = xmlParent_->IterateChildren("style", NULL);
    while ( nodeStyle )
	{
		TiXmlElement * elementStyle = nodeStyle->ToElement();
		const char * val = elementStyle->Attribute("name");
		TiXmlNode* nodeTemplate = elementStyle->FirstChild("template");
		if ( val && nodeTemplate )
		{
			mStyleMap[std::string(val)] = nodeTemplate->ToElement();
		}

		nodeStyle = xmlParent_->IterateChildren("style", nodeStyle);
	}
}

void GuiDialog::CreateElementsFromXml(TiXmlElement * xmlParent_, irr::gui::IGUIElement* guiParent_)
{
    if ( !xmlParent_)
        return;

	// Active filter for elements which should only be loaded when a certain platform is active.
    // This stuff is still handcoded in many dialogs because I started out using different xml's
    // before I realized putting all platforms in one XML is easier to handle.
    // Should probably even be dynamic instead of using compile-flags... but one step at a time.
	const char * FILTER = mConfig.GetUseTouchInput() == ETI_NO_TOUCH ? "PC" : "TOUCH";

    TiXmlNode* nodeGui = xmlParent_->IterateChildren("gui", NULL);
    while ( nodeGui )
    {
        TiXmlElement * elementGui = nodeGui->ToElement();

        if ( elementGui )
        {
            InsertStyleTemplate(elementGui);

            gui::IGUIElement* guiElement = NULL;
            const char * val = elementGui->Attribute("type");
            const char * filter = elementGui->Attribute("filter");
            if ( val && strlen(val) && (!filter || 0 == strcmp(filter, FILTER)) )
            {
                if ( 0 == strcmp(val, "IMAGE") )
                {
                    guiElement = AddImage(elementGui, guiParent_);
                }
                else if ( 0 == strcmp(val, "BUTTON") )
                {
                    guiElement = AddButton(elementGui, guiParent_);
                }
                else if ( 0 == strcmp(val, "STATIC") )
                {
                    guiElement = AddStatic(elementGui, guiParent_);
                }
                else if ( 0 == strcmp(val, "CHECKBOX") )
                {
                    guiElement = AddCheckBox(elementGui, guiParent_);
                }
                else if ( 0 == strcmp(val, "EDITBOX") )
                {
                    guiElement = AddEditBox(elementGui, guiParent_);
                }
                else if ( 0 == strcmp(val, "LISTBOX") )
                {
                    guiElement = AddListBox(elementGui, guiParent_);
                }
                else if ( 0 == strcmp(val, "SCROLLBAR") )
                {
                    guiElement = AddScrollBar(elementGui, guiParent_);
                }
                else if ( 0 == strcmp(val, "TOOLBAR") )
                {
                    guiElement = AddToolBar(elementGui, guiParent_);
                }
                else if ( 0 == strcmp(val, "WINDOW") )
                {
                    guiElement = AddWindow(elementGui, guiParent_);
                }
                else if ( 0 == strcmp(val, "SPINBOX") )
                {
                    guiElement = AddSpinBox(elementGui, guiParent_);
                }
                else if ( 0 == strcmp(val, "TEXTSLIDER") )
                {
                    guiElement = AddTextSlider(elementGui, guiParent_);
                }
            }
            if ( guiElement )
            {
                int iVal = 0;
                elementGui->QueryIntAttribute("default_focus", &iVal );
                if ( iVal )
                {
                    mDefaultFocus = guiElement;
                }

                mGUIElements.push_back(guiElement);

				CreateElementsFromXml(elementGui->ToElement(), guiElement);
            }
        }

        nodeGui = xmlParent_->IterateChildren("gui", nodeGui);
    }

    // HACK: text-sliders are larger on very small screens and because of that do overlap other elements.
    // This might be called more than once and for wrong guiParent_'s but shouldn't hurt much.
    if ( guiParent_ )
    {
		for ( size_t i=0;i<mGUIElements.size(); ++i )
		{
			if (mGUIElements[i]->getType() == (gui::EGUI_ELEMENT_TYPE)EGUIET_TEXT_SLIDER )
			{
				guiParent_->bringToFront(mGUIElements[i]);
			}
		}
	}
}

void GuiDialog::InsertStyleTemplate(TiXmlElement * xmlElement_)
{
	const char * styleAttrib = xmlElement_->Attribute("style");
	if ( styleAttrib )
	{
		StyleXmlMap::const_iterator mStyleMapIt = mStyleMap.find(std::string(styleAttrib));
		if ( mStyleMapIt != mStyleMap.end() )
		{
			const TiXmlElement* templateElement = mStyleMapIt->second;

			// add all attributes
			const TiXmlAttribute* attribute = templateElement->FirstAttribute();
			while ( attribute )
			{
				xmlElement_->SetAttribute( attribute->Name(), attribute->Value() );
				attribute = attribute->Next();
			}

			// add all child-nodes
			const TiXmlNode* childNode = templateElement->FirstChild();
            while ( childNode )
			{
				xmlElement_->InsertEndChild(*childNode);
				childNode = childNode->NextSibling();
			}
		}
		else
		{
			LOG.Warn(std::string("Unknown style: ") +  std::string(styleAttrib));
		}
	}
}


irr::gui::IGUISkin* GuiDialog::AddSkin(TiXmlElement * xmlElement_)
{
    irr::gui::IGUIEnvironment *env = GetEnvironment();
    if ( !env )
        return NULL;

//    const char * name = xmlElement_->Attribute("name");   // currently not used, might get back in later
    const char * type = xmlElement_->Attribute("default_type");

    if ( type && strlen(type) )
    {
        irr::gui::IGUISkin* skin = NULL;
        if ( 0 == strcmp(type, "CLASSIC") )
        {
            skin = env->createSkin(irr::gui::EGST_WINDOWS_CLASSIC);
        }
        else if ( 0 == strcmp(type, "METALLIC") )
        {
            skin = env->createSkin(irr::gui::EGST_WINDOWS_METALLIC);
        }

        if ( skin )
        {
#if IRR_SVN >= 4321
			skin->setSize(gui::EGDS_BUTTON_PRESSED_IMAGE_OFFSET_X, 0);
			skin->setSize(gui::EGDS_BUTTON_PRESSED_IMAGE_OFFSET_Y, 0);
			skin->setSize(gui::EGDS_BUTTON_PRESSED_TEXT_OFFSET_X, 0);
			skin->setSize(gui::EGDS_BUTTON_PRESSED_TEXT_OFFSET_Y, 0);
#endif

            irr::gui::IGUIFont * font = ReadScaledFont(xmlElement_);
            if ( font )
            {
                skin->setFont(font);
            }

            TiXmlNode* nodeCol = xmlElement_->IterateChildren("color", NULL);
            while ( nodeCol )
            {
                TiXmlElement * elementCol = nodeCol->ToElement();
                if ( elementCol )
                {
                    const char * val = elementCol->Attribute("type");
                    if ( val && strlen(val))
                    {
                        irr::gui::EGUI_DEFAULT_COLOR colorType = irr::gui::EGDC_COUNT;
                        if ( 0 == strcmp(val, "3D_DARK_SHADOW") )
                        {
                            colorType = irr::gui::EGDC_3D_DARK_SHADOW;
                        }
                        else if ( 0 == strcmp(val, "3D_SHADOW") )
                        {
                            colorType = irr::gui::EGDC_3D_SHADOW;
                        }
                        else if ( 0 == strcmp(val, "3D_FACE") )
                        {
                            colorType = irr::gui::EGDC_3D_FACE;
                        }
                        else if ( 0 == strcmp(val, "3D_HIGH_LIGHT") )
                        {
                            colorType = irr::gui::EGDC_3D_HIGH_LIGHT;
                        }
                        else if ( 0 == strcmp(val, "3D_LIGHT") )
                        {
                            colorType = irr::gui::EGDC_3D_LIGHT;
                        }
                        else if ( 0 == strcmp(val, "ACTIVE_BORDER") )
                        {
                            colorType = irr::gui::EGDC_ACTIVE_BORDER;
                        }
                        else if ( 0 == strcmp(val, "ACTIVE_CAPTION") )
                        {
                            colorType = irr::gui::EGDC_ACTIVE_CAPTION;
                        }
                        else if ( 0 == strcmp(val, "APP_WORKSPACE") )
                        {
                            colorType = irr::gui::EGDC_APP_WORKSPACE;
                        }
                        else if ( 0 == strcmp(val, "BUTTON_TEXT") )
                        {
                            colorType = irr::gui::EGDC_BUTTON_TEXT;
                        }
                        else if ( 0 == strcmp(val, "GRAY_TEXT") )
                        {
                            colorType = irr::gui::EGDC_GRAY_TEXT;
                        }
                        else if ( 0 == strcmp(val, "HIGH_LIGHT") )
                        {
                            colorType = irr::gui::EGDC_HIGH_LIGHT;
                        }
                        else if ( 0 == strcmp(val, "HIGH_LIGHT_TEXT") )
                        {
                            colorType = irr::gui::EGDC_HIGH_LIGHT_TEXT;
                        }
                        else if ( 0 == strcmp(val, "INACTIVE_BORDER") )
                        {
                            colorType = irr::gui::EGDC_INACTIVE_BORDER;
                        }
                        else if ( 0 == strcmp(val, "INACTIVE_CAPTION") )
                        {
                            colorType = irr::gui::EGDC_INACTIVE_CAPTION;
                        }
                        else if ( 0 == strcmp(val, "TOOLTIP") )
                        {
                            colorType = irr::gui::EGDC_TOOLTIP;
                        }
                        else if ( 0 == strcmp(val, "SCROLLBAR") )
                        {
                            colorType = irr::gui::EGDC_SCROLLBAR;
                        }
                        else if ( 0 == strcmp(val, "WINDOW") )
                        {
                            colorType = irr::gui::EGDC_WINDOW;
                        }
                        if ( colorType != irr::gui::EGDC_COUNT )
                        {
                            irr::video::SColor oldCol =	skin->getColor(colorType);
                            int col;
                            if ( TIXML_SUCCESS == elementCol->QueryIntAttribute("r", &col) )
                                oldCol.setRed(col);
                            if ( TIXML_SUCCESS == elementCol->QueryIntAttribute("g", &col) )
                                oldCol.setGreen(col);
                            if ( TIXML_SUCCESS == elementCol->QueryIntAttribute("b", &col) )
                                oldCol.setBlue(col);
                            if ( TIXML_SUCCESS == elementCol->QueryIntAttribute("a", &col) )
                                oldCol.setAlpha(col);
                            skin->setColor(colorType, oldCol);

							// Those editbox colors didn't exist yet in Irrlicht when this was written.
							// Hacking it here for now as I don't want to modify all media files.
							if ( colorType == irr::gui::EGDC_WINDOW )
							{
								skin->setColor(irr::gui::EGDC_FOCUSED_EDITABLE, oldCol);
								skin->setColor(irr::gui::EGDC_EDITABLE, oldCol);
							}
                        }
                    }
                }
                nodeCol = xmlElement_->IterateChildren("color", nodeCol);
            }

            return skin;
        }
    }
    return NULL;
}

irr::gui::IGUIFont* GuiDialog::ReadScaledFont(TiXmlElement * xmlElement_)
{
    irr::gui::IGUIEnvironment *env = GetEnvironment();
    if ( !env )
        return NULL;

    const char * fontFile = xmlElement_->Attribute("font");
    if ( fontFile )
    {
        std::string completeFontName(mConfig.GetPathMedia());
        completeFontName += fontFile;

        if ( strstr(completeFontName.c_str(), ".ttf") )
        {
            int size = 12;
            xmlElement_->QueryIntAttribute("font_size", &size);
			size = (int)(size*mScaleY);

            int antiAlias = 1;
            xmlElement_->QueryIntAttribute("anti_alias", &antiAlias);

            int transparency = 1;
            xmlElement_->QueryIntAttribute("transp", &transparency);

            return APP.GetFontManager()->GetTtFont(GetDriver(), GetFileSystem(), completeFontName.c_str(), size, (bool)antiAlias, (bool)transparency);
        }

        return env->getFont(completeFontName.c_str());
    }
    return NULL;
}

bool GuiDialog::ReadScaledRect(irr::core::rect<s32>& rect_, TiXmlElement * xmlElement_)
{
    xmlElement_->QueryIntAttribute("x", &rect_.UpperLeftCorner.X);
    xmlElement_->QueryIntAttribute("y", &rect_.UpperLeftCorner.Y);
    xmlElement_->QueryIntAttribute("w", &rect_.LowerRightCorner.X);
    rect_.LowerRightCorner.X += rect_.UpperLeftCorner.X-1;
    ++rect_.LowerRightCorner.X; // irrlicht dimensions don't work with pixels but with units
    xmlElement_->QueryIntAttribute("h", &rect_.LowerRightCorner.Y);
    rect_.LowerRightCorner.Y += rect_.UpperLeftCorner.Y-1;
    ++rect_.LowerRightCorner.Y; // irrlicht dimensions don't work with pixels but with units

    // For display with 3 screens on triplehead technology
    // 0 = normal, 1=show on left screen, 2=show on right screen
    int triplehead = 0;
    xmlElement_->QueryIntAttribute("triplehead", &triplehead);

    if ( mDisplayMode == EDM_NORMAL && triplehead == 0 )
	{
        rect_.UpperLeftCorner.X = (int)round(rect_.UpperLeftCorner.X*mScaleX);
        rect_.LowerRightCorner.X = (int)round(rect_.LowerRightCorner.X*mScaleX);
        rect_.UpperLeftCorner.Y = (int)round(rect_.UpperLeftCorner.Y*mScaleY);
        rect_.LowerRightCorner.Y = (int)round(rect_.LowerRightCorner.Y*mScaleY);
	}
    // triple-head mode?
    else if ( mDisplayMode == EDM_TRIPLE_HEAD )
    {
        int newLeftSide = 0;
        if ( 0 == triplehead )
            newLeftSide = (int)(mScreenDim.Width / 3.f);
        else if ( 2 == triplehead )
            newLeftSide = (int)(2.f*mScreenDim.Width / 3.f);

        rect_.UpperLeftCorner.X = newLeftSide + (int)round(rect_.UpperLeftCorner.X*mScaleX);
        rect_.LowerRightCorner.X = newLeftSide + (int)round(rect_.LowerRightCorner.X*mScaleX);
        rect_.UpperLeftCorner.Y = (int)round(rect_.UpperLeftCorner.Y*mScaleY);
        rect_.LowerRightCorner.Y = (int)round(rect_.LowerRightCorner.Y*mScaleY);
    }
    else
    {
		// don't show triple-head elements outside triple-head mode
        return false;
    }

    return true;
}

int GuiDialog::ReadNameForId(TiXmlElement * xmlElement_)
{
    int id = ++mHighestId;
    const char * name = xmlElement_->Attribute("name");
    if ( name )
        mIdNameMap[name] = id;
    return id;
}

irr::gui::IGUIElement* GuiDialog::GetElementByName(const irr::gui::IGUIElement* parent_, const std::string& name_, const std::string& error_) const
{
	if ( !parent_ )
		return NULL;
    int id = GetIdForName(name_);
    if ( id < 0 )
    {
        std::ostringstream str;
        str << "Warning: " << error_ << " GUI element not known: " << name_ << std::endl;
        LOG.Warn( str.str() );
        return NULL;
    }
    irr::gui::IGUIElement* element = parent_->getElementFromId(id, true);
    if ( !element )
    {
        std::ostringstream str;
        str << "Warning: " << error_ << " GUI element not found: " << name_ << std::endl;
        LOG.Warn( str.str() );
    }
    return element;
}

int GuiDialog::GetIdForName(const std::string& name_) const
{
    IdNameMap::const_iterator it = mIdNameMap.find(name_);
    if  (it != mIdNameMap.end() )
        return it->second;

	// Element got never added (in ReadNameForId)
    return -1;
}

irr::video::ITexture * GuiDialog::GetGuiTexture(const char * name)
{
	HC1_PROFILE(CProfileScope ps1(L"GetGuiTexture", L"STARTUP");)
	video::ITexture * texture = 0;
	irr::video::IVideoDriver* driver = GetDriver();
	if ( driver && name && strlen(name) )
	{
		// no mips for gui-textures
		bool oldMips = driver->getTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS);
		driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);

        std::string completeTexName(mConfig.GetPathMedia());
        completeTexName += name;
        texture = driver->getTexture( completeTexName.c_str() );

        driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, oldMips);

        if ( !texture )
        {
			LOG.WarnLn("GUI texture not created: ", completeTexName.c_str());
		}

	}
	return texture;
}

irr::core::stringw GuiDialog::ReadText(TiXmlElement * xmlElement_) const
{
	const char* val = xmlElement_->Attribute("text");
	if ( val )
		return APP.GetStringTable()->Get(xmlElement_->Attribute("text"));
	return irr::core::stringw(L"");
}

irr::gui::IGUIElement* GuiDialog::AddButton(TiXmlElement * xmlElement_, irr::gui::IGUIElement* guiParent_)
{
    irr::gui::IGUIEnvironment *env = GetEnvironment();
    if ( !env )
        return NULL;

    int id = ReadNameForId(xmlElement_);

    core::rect<s32> rect;
    if ( !ReadScaledRect(rect, xmlElement_) )
        return NULL;

    int alphaChannel = 0;
    xmlElement_->QueryIntAttribute("alpha", &alphaChannel);

    int isPushButton = 0;
    xmlElement_->QueryIntAttribute("push", &isPushButton);

    const char * texNorm = xmlElement_->Attribute("tex_norm");
    const char * texPressed = xmlElement_->Attribute("tex_pressed");
    const char * texFocused = xmlElement_->Attribute("tex_focused");
    const char * texDisabled = xmlElement_->Attribute("tex_disabled");

    irr::core::stringw wtext(ReadText(xmlElement_));

    irr::gui::IGUIButton * button = env->addButton(rect, guiParent_, id, wtext.c_str());
    if ( !button )
        return NULL;

	button->setTabStop(true);
	button->setTabOrder(mHighestFocusId);
    ++mHighestFocusId;

    button->setUseAlphaChannel((bool)alphaChannel);
    button->setScaleImage(true);
    button->setIsPushButton((bool)isPushButton);

    if ( texNorm )
    {
		button->setImage(gui::EGBIS_IMAGE_UP, GetGuiTexture(texNorm));
		button->setDrawBorder(false);	// not needed and it costs noticeable render-time otherwise.
    }
    if ( texPressed )
    {
		button->setImage(gui::EGBIS_IMAGE_DOWN, GetGuiTexture(texPressed));
    }
    if ( texFocused )
    {
		button->setImage(gui::EGBIS_IMAGE_UP_FOCUSED, GetGuiTexture(texFocused));
    }
    if ( texDisabled )
    {
		button->setImage(gui::EGBIS_IMAGE_DISABLED, GetGuiTexture(texDisabled));
    }

	if ( isPushButton )
	{
		const char * texDownFocused = xmlElement_->Attribute("tex_pressed_focused");
		button->setImage(gui::EGBIS_IMAGE_DOWN_FOCUSED, GetGuiTexture(texDownFocused));
	}

    irr::gui::IGUIFont * font = ReadScaledFont(xmlElement_);
    if ( font )
    {
        button->setOverrideFont(font);
    }

    return button;
}

irr::gui::IGUIElement* GuiDialog::AddImage(TiXmlElement * xmlElement_, irr::gui::IGUIElement* guiParent_)
{
    irr::video::IVideoDriver* driver = GetDriver();
    irr::gui::IGUIEnvironment *env = GetEnvironment();
    if ( !env || !driver)
        return NULL;

    int id = ReadNameForId(xmlElement_);

    core::rect<s32> rect;
    if ( !ReadScaledRect(rect, xmlElement_) )
        return NULL;

    const char *textureName = xmlElement_->Attribute("texture");
    video::ITexture * texture = NULL;
    if ( textureName )
    {
		texture = GetGuiTexture(textureName);
        if ( !texture )
            return NULL;
    }

    irr::gui::IGUIImage * image = env->addImage(rect, guiParent_, id, NULL);
    if ( image )
    {
        image->setTabStop(false);
        image->setImage(texture);
		image->setScaleImage(true);
        int alphaChannel = 1;
        xmlElement_->QueryIntAttribute("alpha", &alphaChannel);
        image->setUseAlphaChannel(alphaChannel);
    }

    return image;
}

irr::gui::IGUIElement* GuiDialog::AddStatic(TiXmlElement * xmlElement_, irr::gui::IGUIElement* guiParent_)
{
    irr::gui::IGUIEnvironment *env = GetEnvironment();
    if ( !env )
        return NULL;

    int id = ReadNameForId(xmlElement_);

    core::rect<s32> rect;
    if ( !ReadScaledRect(rect, xmlElement_) )
        return NULL;

    irr::core::stringw wtext(ReadText(xmlElement_));

    int border = 0;
    xmlElement_->QueryIntAttribute("border", &border);

    int wordWrap = 0;
    xmlElement_->QueryIntAttribute("word_wrap", &wordWrap);

    int fillBackground = 0;
    xmlElement_->QueryIntAttribute("fill_bk", &fillBackground);

    // HACK: We never need to draw borders for static elements in the game. They might still be needed for editing,
    // in which case we should add a special edit-mode for this. But for now disable always as those are expensive in drawing.
#if defined(HOVER_RELEASE) || defined(_IRR_ANDROID_PLATFORM_)
	border = 0;
#endif
	if ( mConfig.GetUseTouchInput() == ETI_TOUCH_SIMULATION )
		border = 0;

    irr::gui::IGUIStaticText *staticText = env->addStaticText(wtext.c_str(), rect, (bool)border, (bool)wordWrap, guiParent_, id, (bool)fillBackground);
    if ( staticText )
    {
        staticText->setTabStop(false);
        int hcenter = 0;
        xmlElement_->QueryIntAttribute("hcenter", &hcenter);

        staticText->setTextAlignment( (bool)hcenter? irr::gui::EGUIA_CENTER : irr::gui::EGUIA_UPPERLEFT, irr::gui::EGUIA_UPPERLEFT);

        irr::gui::IGUIFont * font = ReadScaledFont(xmlElement_);
        if ( font )
        {
            staticText->setOverrideFont(font);
        }

        TiXmlNode* nodeCol = xmlElement_->IterateChildren("color", NULL);
        if ( nodeCol )
        {
            TiXmlElement * elementCol = nodeCol->ToElement();
            if ( elementCol )
            {
                irr::video::SColor textColor(255, 255, 255, 255);
                int col=0;
                if ( TIXML_SUCCESS == elementCol->QueryIntAttribute("r", &col) )
                    textColor.setRed(col);
                if ( TIXML_SUCCESS == elementCol->QueryIntAttribute("g", &col) )
                    textColor.setGreen(col);
                if ( TIXML_SUCCESS == elementCol->QueryIntAttribute("b", &col) )
                    textColor.setBlue(col);
                if ( TIXML_SUCCESS == elementCol->QueryIntAttribute("a", &col) )
                    textColor.setAlpha(col);
                staticText->setOverrideColor(textColor);
            }
        }
    }

    return staticText;
}

irr::gui::IGUIElement* GuiDialog::AddCheckBox(TiXmlElement * xmlElement_, irr::gui::IGUIElement* guiParent_)
{
    irr::gui::IGUIEnvironment *env = GetEnvironment();
    if ( !env )
        return NULL;

    int id = ReadNameForId(xmlElement_);

    core::rect<s32> rect;
    if ( !ReadScaledRect(rect, xmlElement_) )
        return NULL;

    irr::core::stringw wtext(ReadText(xmlElement_));

    bool checked = false;

    irr::gui::IGUICheckBox * checkBox = env->addCheckBox(checked, rect, guiParent_, id, wtext.c_str());
	checkBox->setTabStop(true);
	checkBox->setTabOrder(mHighestFocusId);
    ++mHighestFocusId;

    return checkBox;
}

irr::gui::IGUIElement* GuiDialog::AddEditBox(TiXmlElement * xmlElement_, irr::gui::IGUIElement* guiParent_)
{
    irr::gui::IGUIEnvironment *env = GetEnvironment();
    if ( !env )
        return NULL;

    int id = ReadNameForId(xmlElement_);

    core::rect<s32> rect;
    if ( !ReadScaledRect(rect, xmlElement_) )
        return NULL;

    irr::core::stringw wtext(ReadText(xmlElement_));

    int border = 0;
    xmlElement_->QueryIntAttribute("border", &border);

    irr::gui::IGUIEditBox* editBox = env->addEditBox(wtext.c_str(), rect, (bool)border, guiParent_, id);
    irr::gui::IGUIFont * font = ReadScaledFont(xmlElement_);
    if ( font )
    {
		editBox->setTabStop(true);
		editBox->setTabOrder(mHighestFocusId);
        ++mHighestFocusId;

        editBox->setOverrideFont(font);
    }

    return editBox;
}

irr::gui::IGUIElement* GuiDialog::AddListBox(TiXmlElement * xmlElement_, irr::gui::IGUIElement* guiParent_)
{
    irr::gui::IGUIEnvironment *env = GetEnvironment();
    if ( !env )
        return NULL;

    int id = ReadNameForId(xmlElement_);

    core::rect<s32> rect;
    if ( !ReadScaledRect(rect, xmlElement_) )
        return NULL;

    int drawBackground = 0;
    xmlElement_->QueryIntAttribute("draw_bk", &drawBackground);

    irr::gui::IGUIListBox* listBox = env->addListBox(rect, guiParent_, id, drawBackground);
    if ( listBox )
    {
		listBox->setTabStop(true);
		listBox->setTabOrder(mHighestFocusId);
        ++mHighestFocusId;

        irr::gui::IGUIFont * font = ReadScaledFont(xmlElement_);
        if ( font )
        {
//            listBox->setIconFont(font);	// TODO: patch not yet done
        }
    }
    return listBox;
}

irr::gui::IGUIElement* GuiDialog::AddScrollBar(TiXmlElement * xmlElement_, irr::gui::IGUIElement* guiParent_)
{
    irr::gui::IGUIEnvironment *env = GetEnvironment();
    if ( !env )
        return NULL;

    int id = ReadNameForId(xmlElement_);

    core::rect<s32> rect;
    if ( !ReadScaledRect(rect, xmlElement_) )
        return NULL;

    int horizontal = 1;
    xmlElement_->QueryIntAttribute("horizontal", &horizontal);

    irr::gui::IGUIScrollBar* scrollBar = env->addScrollBar(horizontal, rect, guiParent_, id);
    if ( scrollBar )
    {
		scrollBar->setTabStop(true);
		scrollBar->setTabOrder(mHighestFocusId);
        ++mHighestFocusId;

        int max = 100;
        xmlElement_->QueryIntAttribute("max", &max);
        scrollBar->setMax(max);
    }

    return scrollBar;
}

irr::gui::IGUIElement* GuiDialog::AddToolBar(TiXmlElement * xmlElement_, irr::gui::IGUIElement* guiParent_)
{
    irr::gui::IGUIEnvironment *env = GetEnvironment();
    if ( !env )
        return NULL;

    int id = ReadNameForId(xmlElement_);

    irr::gui::IGUIToolBar* toolBar = env->addToolBar(guiParent_, id);
    toolBar->setTabStop(false);

    return toolBar;
}

irr::gui::IGUIElement* GuiDialog::AddWindow(TiXmlElement * xmlElement_, irr::gui::IGUIElement* guiParent_)
{
    irr::gui::IGUIEnvironment *env = GetEnvironment();
    if ( !env )
        return NULL;

    int id = ReadNameForId(xmlElement_);

    core::rect<s32> rect;
    if ( !ReadScaledRect(rect, xmlElement_) )
        return NULL;

    irr::core::stringw wtext(ReadText(xmlElement_));

    bool modal = false;

    irr::gui::IGUIWindow* window = env->addWindow(rect, modal, wtext.c_str(), guiParent_, id);
    window->setTabStop(false);

    return window;
}

irr::gui::IGUIElement* GuiDialog::AddSpinBox(TiXmlElement * xmlElement_, irr::gui::IGUIElement* guiParent_)
{
    irr::gui::IGUIEnvironment *env = GetEnvironment();
    if ( !env )
        return NULL;

    int id = ReadNameForId(xmlElement_);

    core::rect<s32> rect;
    if ( !ReadScaledRect(rect, xmlElement_) )
        return NULL;

    irr::core::stringw wtext(ReadText(xmlElement_));

    irr::gui::IGUISpinBox* spinBox = env->addSpinBox(wtext.c_str(), rect, true, guiParent_, id);

    if ( spinBox )
    {
		spinBox->setTabStop(true);
		spinBox->setTabOrder(mHighestFocusId);
        ++mHighestFocusId;

        float rangeMin=0.f;
        float rangeMax=100.f;
        xmlElement_->QueryFloatAttribute("range_min", &rangeMin);
        xmlElement_->QueryFloatAttribute("range_max", &rangeMax);
        spinBox->setRange(rangeMin, rangeMax);

        float stepSize=1.f;
        xmlElement_->QueryFloatAttribute("step_size", &stepSize);
        spinBox->setStepSize(stepSize);

        float value=0.f;
        xmlElement_->QueryFloatAttribute("value", &value);
        spinBox->setValue(value);
    }

    return spinBox;
}

irr::gui::IGUIElement* GuiDialog::AddTextSlider(TiXmlElement * xmlElement_, irr::gui::IGUIElement* guiParent_)
{
    irr::gui::IGUIEnvironment *env = GetEnvironment();
    if ( !env )
        return NULL;

    int id = ReadNameForId(xmlElement_);

    core::rect<s32> rect;
    if ( !ReadScaledRect(rect, xmlElement_) )
        return NULL;

    float buttonAspectRatio = 1.f;
    xmlElement_->QueryFloatAttribute("btn_aspect_ratio", &buttonAspectRatio);

    CGUITextSlider* slider = new CGUITextSlider(env, guiParent_ ? guiParent_ : env->getRootGUIElement(), id, rect, buttonAspectRatio);
    if ( !slider )
        return NULL;
	slider->drop(); // should be ok here because we know it's still kept by the parent and we don't want to manage it ourself.

	slider->setTabStop(true);
	slider->setTabOrder(mHighestFocusId);
	++mHighestFocusId;

    irr::gui::IGUIFont * font = ReadScaledFont(xmlElement_);
    if ( font )
    {
        slider->setOverrideFont(font);
    }

    int alphaChannel = 1;
    xmlElement_->QueryIntAttribute("alpha", &alphaChannel);
    slider->setUseAlphaChannel(alphaChannel);

    int wrapAround = 1;
    xmlElement_->QueryIntAttribute("wrap", &wrapAround);
    slider->setWrapAround(wrapAround);

    const char * leftTexNorm = xmlElement_->Attribute("left_tex_norm");
    if ( leftTexNorm )
    {
        slider->setLeftImage(GetGuiTexture(leftTexNorm));
    }

    const char * leftTexPressed = xmlElement_->Attribute("left_tex_pressed");
    if ( leftTexPressed )
    {
        slider->setLeftImagePressed(GetGuiTexture(leftTexPressed));
    }

    const char * rightTexNorm = xmlElement_->Attribute("right_tex_norm");
    if ( rightTexNorm )
    {
        slider->setRightImage(GetGuiTexture(rightTexNorm));
    }

    const char * rightTexPressed = xmlElement_->Attribute("right_tex_pressed");
    if ( rightTexPressed )
    {
        slider->setRightImagePressed(GetGuiTexture(rightTexPressed));
    }

    if ( mConfig.GetUseTouchInput() == ETI_NO_TOUCH )
	{
		const char * texFocused = xmlElement_->Attribute("tex_focused");
		if ( texFocused )
		{
			slider->setFocusedImage(GetGuiTexture(texFocused));
		}
	}

    // TEST, but can stay
    slider->addText(L"dummy one");
    slider->addText(L"dummy two");
    slider->addText(L"dummy three");
    slider->setCurrentTextId(0);

    if ( mConfig.DoesNeedLargeButtons() )
    {
		slider->makeLargeButtons();
	}

    return slider;
}

void GuiDialog::AddGuiEventFunctor(int elementId_, IEventFunctor * functor_ )
{
    if ( APP.GetGui() )
    {
        APP.GetGui()->AddGuiEventFunctor(elementId_, functor_ );
    }
}

void GuiDialog::AddGuiEventFunctor(const std::string& name, IEventFunctor * functor, bool autoRemove)
{
	int id = GetIdForName(name);
	if ( id >= 0 )
	{
		mEventFunctorIds.push_back(id);
		AddGuiEventFunctor(id, functor);
	}
}

void GuiDialog::RemoveGuiEventFunctor(int elementId_)
{
    if ( APP.GetGui() )
    {
        APP.GetGui()->RemoveGuiEventFunctor(elementId_);
    }
}

void GuiDialog::AutoRemoveFunctors()
{
	for ( size_t i=0; i < mEventFunctorIds.size(); ++i )
	{
		RemoveGuiEventFunctor(mEventFunctorIds[i]);
	}
	mEventFunctorIds.clear();
}


TestDialog::TestDialog(const Config& config)
: GuiDialog(config)
{
    AddGuiEventFunctor(0, new EventFunctor<TestDialog>(this, &TestDialog::EventCallback));
}

bool TestDialog::EventCallback(const irr::SEvent &event_)
{
    return false;
}
