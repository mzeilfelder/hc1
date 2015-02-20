// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#include "gui.h"
#include "gui_ids.h"
#include "gui_dialogs/gui_controller_display.h"
#include "gui_dialogs/gui_user_controls.h"
#include "gui_dialogs/gui_developer_settings.h"
#include "gui_dialogs/gui_levelmanager.h"
#include "gui_dialogs/gui_editor.h"
#include "gui_dialogs/gui_intro.h"
#include "gui_dialogs/gui_game.h"
#include "gui_dialogs/gui_ai.h"
#include "gui_dialogs/gui_hud.h"
#include "gui_dialogs/gui_touch.h"
#include "gui_dialogs/gui_menu_main.h"
#include "gui_dialogs/gui_menu_championship.h"
#include "gui_dialogs/gui_menu_championship_progress.h"
#include "gui_dialogs/gui_menu_championship_winner.h"
#include "gui_dialogs/gui_menu_arcade.h"
#include "gui_dialogs/gui_menu_timeattack.h"
#include "gui_dialogs/gui_menu_rivals.h"
#include "gui_dialogs/gui_menu_rivals_score.h"
#include "gui_dialogs/gui_menu_selecthover.h"
//#include "gui_dialogs/gui_menu_replaytheatre.h"
#include "gui_dialogs/gui_menu_highscores.h"
#include "gui_dialogs/gui_menu_options.h"
#include "gui_dialogs/gui_menu_profiles.h"
#include "gui_dialogs/gui_menu_controls.h"
#include "gui_dialogs/gui_menu_analog.h"
#include "gui_dialogs/gui_menu_newprofile.h"
#include "gui_dialogs/gui_menu_pause.h"
#include "gui_dialogs/gui_menu_gameend.h"
#include "gui_dialogs/gui_menu_gameend_champ.h"
#include "gui_dialogs/gui_menu_gameend_rivals.h"
#include "gui_dialogs/gui_menu_credits.h"
#include "gui_dialogs/gui_menu_licenses.h"
#include "gui_dialogs/gui_menu_nagscreen.h"
#include "gui_dialogs/gui_menu_graphics.h"
#include "gui_dialogs/gui_menu_loadingscreen.h"
#include "gui_dialogs/gui_menu_tutorial3.h"
#include "gui_dialogs/gui_menu_hover_unlocked.h"
#include "gui_dialogs/gui_dlg_okcancel.h"
#include "config.h"
#include "globals.h"
#include "controller.h"
#include "font_manager.h"
#include "physics.h"
#include "game.h"
#include "main.h"
#include "player.h"
#include "gui_dialog.h"
#include "sound.h"
#include "EGUIEventTypesHC.h"
#include "logging.h"

#ifdef _IRR_ANDROID_PLATFORM_
#include "android_tools.h"
#endif

#include <string>
#include <sstream>

using namespace irr;
using namespace gui;

Gui::Gui()
    : mEnvironment(NULL)
    , mVideoDriver(NULL)
{
#ifdef HC1_ENABLE_EDITOR
    mMenu = NULL;
    mBar = NULL;
    mControllerDisplay = NULL;
    mUserControls = NULL;
    mDeveloperSettings  = NULL;
    mGuiLevelManager = NULL;
    mGuiEditor = NULL;
    mGuiAi = NULL;
    mGuiGame = NULL;
    mTestDialog = NULL;
#endif
#if !defined(HOVER_RELEASE)
    mInfoText = NULL;
    mLevelText = NULL;
    mSpeedText = NULL;
    mInfoTextTime = -1;
#endif
	mGuiProfiler = NULL;
	mGuiProfiler2 = NULL;
	mProfilerBackground = NULL;

    mGuiIntro = NULL;
    mGuiHud = NULL;
    mGuiTouch = NULL;
    mMenuMain = NULL;
    mMenuChampionship = NULL;
    mMenuChampionshipProgress = NULL;
    mMenuChampWinner = NULL;
    mMenuArcade = NULL;
    mMenuTimeAttack = NULL;
    mMenuRivals = NULL;
    mMenuRivalsScore = NULL;
    mMenuSelectHover = NULL;
//    mMenuReplayTheatre = NULL;
    mMenuHighscores = NULL;
    mMenuOptions = NULL;
    mMenuProfiles = NULL;
    mMenuControls = NULL;
    mMenuAnalog = NULL;
    mMenuNewProfile = NULL;
    mMenuGameEnd = NULL;
    mMenuGameEndChampionship = NULL;
    mMenuGameEndRivals = NULL;
    mMenuPause = NULL;
    mMenuCredits = NULL;
    mMenuLicenses = NULL;
    mMenuNagscreen = NULL;
    mMenuGraphics = NULL;
    mMenuLoadingScreen = NULL;
    mMenuTutorial3 = NULL;
    mMenuHoverUnlocked = NULL;
    mGuiDlgOkCancel = NULL;

    mHoveredElement = NULL;
    mHoveredElementNoSub = NULL;
    mFocusElement = NULL;
    mActiveDialog = NULL;
    mPreviousActiveDialog = NULL;
    mModalDialog = NULL;
    mClickSoundOverride = false;
}

Gui::~Gui()
{
	SetHoveredElement(0);

	if (mGuiProfiler)
		mGuiProfiler->remove();
	if (mGuiProfiler2)
		mGuiProfiler2->remove();
	if (mProfilerBackground)
		mProfilerBackground->remove();
	mGuiProfiler = 0;
	mProfilerBackground = 0;

    delete mGuiDlgOkCancel;
    delete mMenuHoverUnlocked;
    delete mMenuTutorial3;
    delete mMenuLoadingScreen;
    delete mMenuNagscreen;
    delete mMenuCredits;
    delete mMenuLicenses;
    delete mMenuGraphics;
    delete mMenuPause;
    delete mMenuGameEndRivals;
    delete mMenuGameEnd;
    delete mMenuGameEndChampionship;
    delete mMenuNewProfile;
    delete mMenuAnalog;
    delete mMenuControls;
    delete mMenuProfiles;
    delete mMenuOptions;
    delete mMenuHighscores;
//    delete mMenuReplayTheatre;
    delete mMenuSelectHover;
    delete mMenuRivalsScore;
    delete mMenuRivals;
    delete mMenuTimeAttack;
    delete mMenuArcade;
    delete mMenuChampWinner;
    delete mMenuChampionshipProgress;
    delete mMenuChampionship;
    delete mMenuMain;
    delete mGuiHud;
    delete mGuiTouch;
#ifdef HC1_ENABLE_EDITOR
    delete mTestDialog;
    delete mGuiAi;
    delete mGuiEditor;
    delete mGuiLevelManager;
    delete mDeveloperSettings;
    delete mUserControls;
    if ( mControllerDisplay )
        mControllerDisplay->drop();
    delete mGuiGame;
#endif
    delete mGuiIntro;
}

void Gui::Init(gui::IGUIEnvironment * environment_, video::IVideoDriver* videoDriver_, irr::io::IFileSystem* fileSystem_, FontManager* fontManager)
{
    mEnvironment = environment_;
    mVideoDriver = videoDriver_;
    if ( !mEnvironment || !mVideoDriver )
        return;

	// set a nicer font (TODO: add some bitmap-font to the media-folder for that)
	IGUISkin* skin = mEnvironment->getSkin();
	IGUIFont* font = fontManager->GetTtFont(mVideoDriver, fileSystem_, APP.GetConfig()->MakeFilenameUI("andika/Andika-R.ttf").c_str(), 12, true, true);
	if (font)
		skin->setFont(font);

#ifdef HC1_ENABLE_EDITOR
	// create menu
	mMenu = mEnvironment->addMenu(NULL, GUI_MENU);
	mMenu->addItem(L"File", -1, true, true);
	mMenu->addItem(L"View", -1, true, true);
	mMenu->addItem(L"Settings", -1, true, true);
	mMenu->addItem(L"Recorder", -1, true, true);
//	mMenu->addItem(L"Help", -1, true, true);

	gui::IGUIContextMenu* submenu;
	submenu = mMenu->getSubMenu(0);
	submenu->addItem(L"Load Level", GUI_LOAD_LEVEL);
	submenu->addItem(L"Open Model File...", GUI_OPEN_MODEL_FILE);
	submenu->addItem(L"Open GUI Dialog", GUI_OPEN_GUI_DLG);
	submenu->addSeparator();
	submenu->addItem(L"Quit", GUI_QUIT);

	submenu = mMenu->getSubMenu(1);
//	submenu->addItem(L"toggle sky box visibility", GUI_TOGGLE_SKYBOX);
	submenu->addItem(L"toggle physics debugging", GUI_TOGGLE_PHYSICS_DEBUG);

    submenu = mMenu->getSubMenu(2);
    submenu->addItem(L"Save Settings", GUI_SAVE_SETTINGS);
    submenu->addItem(L"Load Settings", GUI_LOAD_SETTINGS);
    submenu->addItem(L"User controls", GUI_OPEN_USER_CONTROLS);

    submenu = mMenu->getSubMenu(3);
    submenu->addItem(L"Save Last Recording", GUI_SAVE_LAST_RECORDING);
    submenu->addItem(L"Toggle Autosave Recording", GUI_TOGGLE_AUTOSAVE_RECORDING);
    submenu->addItem(L"Toggle Autoload Recording", GUI_TOGGLE_AUTOLOAD_RECORDING);

//	submenu = mMenu->getSubMenu(4);
//	submenu->addItem(L"About", GUI_ABOUT);


	// create toolbar
	mBar = mEnvironment->addToolBar();
	video::ITexture* image = mVideoDriver->getTexture(APP.GetConfig()->MakeFilenameUI("open.png").c_str());
	mVideoDriver->makeColorKeyTexture(image, core::position2d<s32>(0,0));
	mBar->addButton(GUI_ICON_OPEN_FILE, 0, 0, image, 0, false, true);


	image = mVideoDriver->getTexture(APP.GetConfig()->MakeFilenameUI("tools.png").c_str());
	mVideoDriver->makeColorKeyTexture(image, core::position2d<s32>(0,0));
	mBar->addButton(GUI_ICON_TOOLBAR, 0, 0, image, 0, false, true);

    IGUIElement* root = mEnvironment->getRootGUIElement();
	mControllerDisplay = new ControllerDisplay(gui::EGUIET_ELEMENT, mEnvironment, root, GUI_CONTROLLER_DISPLAY, core::rect<s32>(550,400,600,450));
	mControllerDisplay->setVisible(false);
	mControllerDisplay->SetController(APP.GetController());

    mUserControls = new GuiUserControls (mEnvironment);
    mDeveloperSettings = new GuiDeveloperSettings(mEnvironment);
    mGuiLevelManager = new GuiLevelManager(mEnvironment);
    mGuiEditor = new GuiEditor(mEnvironment);
    mGuiAi = new GuiAi();
    mGuiAi->Load(APP.GetConfig()->MakeFilenameUI("edit_ai.xml").c_str());
    mGuiAi->Hide();

    mGuiGame = new GuiGame(mEnvironment);

    // TEST
    mTestDialog = new GuiDialog();
#endif // HC1_ENABLE_EDITOR

#if !defined(HOVER_RELEASE)
	// static texts
	mInfoText = mEnvironment->addStaticText(L"", core::rect<s32>(110,30,280,50), true);
    mLevelText = mEnvironment->addStaticText(L"", core::rect<s32>(300,30,500,50), true);
    mSpeedText = mEnvironment->addStaticText(L"", core::rect<s32>(510,30,620,50), true);
#endif

	// set window caption
	std::wstring caption;
	caption = APP.GetConfig()->GetDlgMainCaption();
	caption += L" - [";
	caption += mVideoDriver->getName();
	caption += L"]";
	APP.GetIrrlichtManager()->GetIrrlichtDevice()->setWindowCaption(caption.c_str());

	// create all game dialogs
    mGuiIntro = new GuiIntro();
    mGuiHud = new GuiHud();
    mGuiTouch = new GuiTouch();
    mMenuMain = new GuiMenuMain();
    mMenuChampionship = new GuiMenuChampionship();
    mMenuChampionshipProgress = new GuiMenuChampionshipProgress();
    mMenuChampWinner = new GuiMenuChampWinner();
    mMenuArcade = new GuiMenuArcade();
    mMenuTimeAttack = new GuiMenuTimeAttack();
    mMenuRivals = new GuiMenuRivals();
    mMenuRivalsScore = new GuiMenuRivalsScore();
    mMenuSelectHover = new GuiMenuSelectHover();
//    mMenuReplayTheatre = new GuiMenuReplayTheatre();
    mMenuHighscores = new GuiMenuHighscores();
    mMenuOptions = new GuiMenuOptions();
    mMenuProfiles = new GuiMenuProfiles();
    mMenuControls = new GuiMenuControls();
    mMenuAnalog = new GuiMenuAnalog();
    mMenuNewProfile = new GuiMenuNewProfile();
    mMenuGameEnd = new GuiMenuGameEnd();
    mMenuGameEndChampionship = new GuiMenuGameEndChampionship();
    mMenuGameEndRivals = new GuiMenuGameEndRivals();
    mMenuPause = new GuiMenuPause();
    mMenuCredits = new GuiMenuCredits();
    mMenuLicenses = new GuiMenuLicenses();
    mMenuNagscreen = new GuiMenuNagscreen();
    mMenuGraphics = new GuiMenuGraphics();
    mMenuLoadingScreen = new GuiMenuLoadingScreen();
    mMenuTutorial3 = new GuiMenuTutorial3();
    mMenuHoverUnlocked = new GuiMenuHoverUnlocked();
    mGuiDlgOkCancel = new GuiDlgOkCancel();

    const core::dimension2d<u32> & videoDimension = mVideoDriver->getCurrentRenderTargetSize();
    const core::recti profilerRect(1, 1, videoDimension.Width-1, videoDimension.Height-1);
	mGuiProfiler = mEnvironment->addProfilerDisplay(profilerRect);
	if ( profilerRect.getWidth() > 400 )
	{
		mGuiProfiler2 = mEnvironment->addProfilerDisplay(profilerRect);
		mGuiProfiler->setRelativePosition(core::recti(profilerRect.UpperLeftCorner.X, profilerRect.UpperLeftCorner.Y, (profilerRect.LowerRightCorner.X+profilerRect.UpperLeftCorner.X)/2, profilerRect.LowerRightCorner.Y));
		mGuiProfiler2->setRelativePosition(core::recti((profilerRect.LowerRightCorner.X+profilerRect.UpperLeftCorner.X)/2, profilerRect.UpperLeftCorner.Y, profilerRect.LowerRightCorner.X, profilerRect.LowerRightCorner.Y));
		mGuiProfiler2->setOverrideFont(font);
	}
	mProfilerBackground = mEnvironment->addStaticText( L"", profilerRect, false, false, 0, -1, true);
	mProfilerBackground->setBackgroundColor(video::SColor(210, 0, 0, 0));
	mGuiProfiler->setOverrideFont(font);
#if IRR_REV >= 4
	mGuiProfiler->setDrawBackground(true);	// nice try, but our background is invisible outside the editor
	mGuiProfiler->setFilters(1, 0, 0.f, 0);
	if ( mGuiProfiler2 )
	{
		mGuiProfiler2->setDrawBackground(true);	// nice try, but our background is invisible outside the editor
		mGuiProfiler2->setFilters(1, 0, 0.f, 0);
	}
#endif
	mGuiProfiler->setVisible(false);
	if (mGuiProfiler2)
		mGuiProfiler2->setVisible(false);
	mProfilerBackground->setVisible(false);

    mSoundFocus = APP.GetConfig()->MakeFilenameSound("hc_menu_click01.wav");
    mSoundRefFocus = -1;
    mSoundClick1 = APP.GetConfig()->MakeFilenameSound("hc_menu_confirm01.wav");
    mSoundRefClick1 = -1;
    mSoundClick2 = APP.GetConfig()->MakeFilenameSound("hc_menu_deny01.wav");
    mSoundRefClick2 = -1;
}

// intro is special as we want to show it already before the rest of the gui is loading
void Gui::LoadIntro()
{
    LOG.Debug(L"Loading intro\n");
    if ( mGuiIntro )
    {
        LOG.Debug(L"intro\n");
        if ( !mGuiIntro->Load(APP.GetConfig()->MakeFilenameUI("menu_intro.xml").c_str()) )
        {
            LOG.Warn(L"load intro failed\n");
        }
        mGuiIntro->Hide();
    }
}

// This ain't done in init, because we need to access some info which ain't available there (like levels)
void Gui::LoadMenuDialogs()
{
    LOG.Info(L"Loading menu dialogs\n");
    if ( mGuiHud )
    {
        LOG.Debug(L"hud\n");
        if ( !mGuiHud->Load(APP.GetConfig()->MakeFilenameUI("menu_hud.xml").c_str()) )
        {
            LOG.Warn(L"load hud failed\n");
        }
        mGuiHud->Hide();
    }

    if ( mGuiTouch )
    {
        LOG.Debug(L"touch\n");
        if ( !mGuiTouch->Load(APP.GetConfig()->MakeFilenameUI("menu_touch.xml").c_str()) )
        {
            LOG.Warn(L"load touch failed\n");
        }
        mGuiTouch->Hide();
    }

    if ( mMenuMain )
    {
        LOG.Debug(L"mainmenu01\n");
        if ( !mMenuMain->Load(APP.GetConfig()->MakeFilenameUI("menu_main.xml").c_str()) )
        {
            LOG.Warn(L"load mainmenu01 failed\n");
        }
        mMenuMain->Hide();
    }
    if ( mMenuChampionship )
    {
        LOG.Debug(L"championship\n");
        if ( !mMenuChampionship->Load(APP.GetConfig()->MakeFilenameUI("menu_championship.xml").c_str()) )
        {
            LOG.Warn(L"load championship failed\n");
        }
        mMenuChampionship->Hide();
    }
    if ( mMenuChampionshipProgress )
    {
        LOG.Debug(L"championship_progress\n");
#if defined(_IRR_ANDROID_PLATFORM_) || defined(HC1_SIMULATE_MOBILE_UI)
		if ( !mMenuChampionshipProgress->Load(APP.GetConfig()->MakeFilenameUI("menu_championship_progress_touch.xml").c_str()) )
#else
        if ( !mMenuChampionshipProgress->Load(APP.GetConfig()->MakeFilenameUI("menu_championship_progress.xml").c_str()) )
#endif
        {
            LOG.Warn(L"load championship_progress failed\n");
        }
        mMenuChampionshipProgress->Hide();
    }
    if ( mMenuChampWinner )
    {
        LOG.Debug(L"ChampWinner\n");
        if ( !mMenuChampWinner->Load(APP.GetConfig()->MakeFilenameUI("menu_champ_winner.xml").c_str()) )
        {
            LOG.Warn(L"load champ_winner.xml failed\n");
        }
        mMenuChampWinner->Hide();
    }
    if ( mMenuArcade )
    {
        LOG.Debug(L"arcade\n");
        if ( !mMenuArcade->Load(APP.GetConfig()->MakeFilenameUI("menu_arcade.xml").c_str()) )
        {
            LOG.Warn(L"load arcade failed\n");
        }
        mMenuArcade->Hide();
    }
    if ( mMenuTimeAttack )
    {
        LOG.Debug(L"timeattack\n");
        if  (!mMenuTimeAttack->Load(APP.GetConfig()->MakeFilenameUI("menu_timeattack.xml").c_str()) )
        {
            LOG.Warn(L"load timeattack failed\n");
        }
        mMenuTimeAttack->Hide();
    }
    if ( mMenuRivals )
    {
        LOG.Debug(L"rivals_menu\n");
        if  (!mMenuRivals->Load(APP.GetConfig()->MakeFilenameUI("menu_rivals.xml").c_str()) )
        {
            LOG.Warn(L"load rivals_menu failed\n");
        }
        mMenuRivals->Hide();
    }
    if ( mMenuRivalsScore )
    {
        LOG.Debug(L"rivals_score\n");
        if  (!mMenuRivalsScore->Load(APP.GetConfig()->MakeFilenameUI("menu_rivals_score.xml").c_str()) )
        {
            LOG.Warn(L"load rivals_score.xml failed\n");
        }
        mMenuRivalsScore->Hide();
    }
    if ( mMenuSelectHover )
    {
        LOG.Debug(L"selecthover\n");
        if ( !mMenuSelectHover->Load(APP.GetConfig()->MakeFilenameUI("menu_selecthover.xml").c_str()) )
        {
            LOG.Warn(L"load selecthover failed\n");
        }
        mMenuSelectHover->Hide();
    }
//    if ( mMenuReplayTheatre )
//    {
//        LOG.Info(L"replays\n");
//        if ( !mMenuReplayTheatre->Load(APP.GetConfig()->MakeFilenameUI("replays.xml").c_str()) )
//        {
//            LOG.Warn(L"load replays failed\n");
//        }
//        mMenuReplayTheatre->Hide();
//    }
    if ( mMenuHighscores )
    {
        LOG.Debug(L"highscores\n");
        if ( !mMenuHighscores->Load(APP.GetConfig()->MakeFilenameUI("menu_highscores.xml").c_str()) )
        {
            LOG.Warn(L"load highscores failed\n");
        }
        mMenuHighscores->Hide();
    }
    if  (mMenuOptions )
    {
        LOG.Debug(L"options\n");
        if ( !mMenuOptions->Load(APP.GetConfig()->MakeFilenameUI("menu_options.xml").c_str()) )
        {
            LOG.Warn(L"load options failed\n");
        }
        mMenuOptions->Hide();
    }
    if ( mMenuProfiles )
    {
        LOG.Debug(L"profiles\n");
        if ( !mMenuProfiles->Load(APP.GetConfig()->MakeFilenameUI("menu_profiles.xml").c_str()) )
        {
            LOG.Warn(L"load profiles failed\n");
        }
        mMenuProfiles->Hide();
    }
    if ( mMenuControls )
    {
        LOG.Debug(L"controls\n");
        if ( !mMenuControls->Load(APP.GetConfig()->MakeFilenameUI("menu_controls.xml").c_str()) )
        {
            LOG.Warn(L"load controls failed\n");
        }
        mMenuControls->Hide();
    }
    if ( mMenuAnalog )
    {
        LOG.Debug(L"analog\n");
        if ( !mMenuAnalog->Load(APP.GetConfig()->MakeFilenameUI("menu_analog.xml").c_str()) )
        {
            LOG.Warn(L"load analog failed\n");
        }
        mMenuAnalog->Hide();
    }
    if ( mMenuNewProfile )
    {
        LOG.Debug(L"newprofile\n");
        if ( !mMenuNewProfile->Load(APP.GetConfig()->MakeFilenameUI("menu_newprofile.xml").c_str()) )
        {
            LOG.Warn(L"load newprofile failed\n");
        }
        mMenuNewProfile->Hide();
    }
    if ( mMenuGameEnd )
    {
        LOG.Debug(L"hud_gameend\n");
        if ( !mMenuGameEnd->Load(APP.GetConfig()->MakeFilenameUI("menu_hud_gameend.xml").c_str()) )
        {
            LOG.Warn(L"load hud_gameend failed\n");
        }
        mMenuGameEnd->Hide();
    }
    if ( mMenuGameEndChampionship )
    {
        LOG.Debug(L"hud_gameend_c\n");
        if (  !mMenuGameEndChampionship->Load(APP.GetConfig()->MakeFilenameUI("menu_hud_gameend_c.xml").c_str()) )
        {
            LOG.Warn(L"load hud_gameend_c failed\n");
        }
        mMenuGameEndChampionship->Hide();
    }
    if ( mMenuGameEndRivals)
    {
        LOG.Debug(L"rivals_gameend\n");
        if (  !mMenuGameEndRivals->Load(APP.GetConfig()->MakeFilenameUI("menu_rivals_gameend.xml").c_str()) )
        {
            LOG.Warn(L"load rivals_gameend failed\n");
        }
        mMenuGameEndRivals->Hide();
    }
    if ( mMenuPause )
    {
        LOG.Debug(L"pause\n");
        if ( !mMenuPause->Load(APP.GetConfig()->MakeFilenameUI("menu_pause.xml").c_str()) )
        {
            LOG.Warn(L"load pause failed\n");
        }
        mMenuPause->Hide();
    }
    if ( mMenuCredits )
    {
        LOG.Debug(L"credits\n");
        if ( ! mMenuCredits->Load(APP.GetConfig()->MakeFilenameUI("menu_credits.xml").c_str()) )
        {
            LOG.Warn(L"load credits failed\n");
        }
        mMenuCredits->Hide();
    }
	if ( mMenuLicenses )
    {
        LOG.Debug(L"licenses\n");
        if ( ! mMenuLicenses->Load(APP.GetConfig()->MakeFilenameUI("menu_licenses.xml").c_str()) )
        {
            LOG.Warn(L"load licenses failed\n");
        }
        mMenuLicenses->Hide();
    }
    if ( mMenuNagscreen )
    {
        if ( ! mMenuNagscreen->Load(APP.GetConfig()->MakeFilenameUI("menu_nagscreen.xml").c_str()) )
        {
            LOG.Warn(L"load nagscreen failed\n");
        }
        mMenuNagscreen->Hide();
    }
    if ( mMenuGraphics )
    {
        LOG.Debug(L"graphic_setup\n");
        if ( ! mMenuGraphics->Load(APP.GetConfig()->MakeFilenameUI("menu_graphic_setup.xml").c_str()) )
        {
            LOG.Warn(L"load graphic_setup failed\n");
        }
        mMenuGraphics->Hide();
    }
    if ( mMenuLoadingScreen )
    {
        LOG.Debug(L"loadingscreen\n");
        if ( !mMenuLoadingScreen->Load(APP.GetConfig()->MakeFilenameUI("menu_loadingscreen.xml").c_str()) )
        {
            LOG.Warn(L"load loadingscreen failed\n");
        }
        mMenuLoadingScreen->Hide();
    }
    if ( mMenuTutorial3 )
    {
        LOG.Debug(L"MenuTutorial3\n");
        if ( !mMenuTutorial3->Load(APP.GetConfig()->MakeFilenameUI("menu_tutorial03.xml").c_str()) )
        {
            LOG.Warn(L"load MenuTutorial3 failed\n");
        }
        mMenuTutorial3->Hide();
    }
    if ( mMenuHoverUnlocked )
    {
        LOG.Debug(L"hcunlocked\n");
        if ( !mMenuHoverUnlocked->Load(APP.GetConfig()->MakeFilenameUI("menu_hcunlocked.xml").c_str()) )
        {
            LOG.Warn(L"load hcunlocked failed\n");
        }
        mMenuHoverUnlocked->Hide();
    }
    if ( mGuiDlgOkCancel )
    {
        LOG.Debug(L"GuiDlgOkCancel\n");
        if ( !mGuiDlgOkCancel->Load(APP.GetConfig()->MakeFilenameUI("dlg_okcancel.xml").c_str()) )
        {
            LOG.Warn(L"load GuiDlgOkCancel failed\n");
        }
        mGuiDlgOkCancel->Hide();
    }
}

void Gui::SetEditGuiVisible(bool visible_)
{
#ifdef HC1_ENABLE_EDITOR
    if ( mMenu )
        mMenu->setVisible(visible_);
    if ( mBar )
        mBar->setVisible(visible_);
#endif

#if !defined(HOVER_RELEASE)
    if ( mInfoText )
        mInfoText->setVisible(visible_);
    if ( mLevelText )
        mLevelText->setVisible(visible_);
    if ( mSpeedText )
        mSpeedText->setVisible(visible_);
 #endif
}

void Gui::Update(irr::u32 timeTick)
{
    if ( !mEnvironment || !mVideoDriver )
        return;

    if ( mActiveDialog && mActiveDialog->GetCallOnUpdate() )
	{
		mActiveDialog->OnUpdate(timeTick);
	}

    // A hack to allow menucontrol by joystick.
    if (    APP.GetController()
        &&  APP.GetMode() == MODE_GUI )
    {
        Controller* controller = APP.GetController();
		IrrlichtManager* irrManager = APP.GetIrrlichtManager();
        if ( controller && irrManager )
        {
            if ( controller->GetMenuUp() )
            {
                irrManager->SendFakeKeyEvent(KEY_UP, true);
                irrManager->SendFakeKeyEvent(KEY_UP, false);
            }
            else if ( controller->GetMenuDown() )
            {
                irrManager->SendFakeKeyEvent(KEY_DOWN, true);
                irrManager->SendFakeKeyEvent(KEY_DOWN, false);
            }
            else if ( controller->GetMenuLeft() )
            {
                irrManager->SendFakeKeyEvent(KEY_LEFT, true);
                irrManager->SendFakeKeyEvent(KEY_LEFT, false);
            }
            else if ( controller->GetMenuRight() )
            {
                irrManager->SendFakeKeyEvent(KEY_RIGHT, true);
                irrManager->SendFakeKeyEvent(KEY_RIGHT, false);
            }
            else if ( controller->GetMenuExecute() )
            {
                irrManager->SendFakeKeyEvent(KEY_SPACE, true);
                irrManager->SendFakeKeyEvent(KEY_SPACE, false);
            }
        }
    }

#if !defined(HOVER_RELEASE)
    if ( mInfoTextTime > APP.GetIrrlichtManager()->GetIrrlichtTimer()->getRealTime() )
    {
        std::wstring str(mInfoTextStr.begin(), mInfoTextStr.end());
        mInfoText->setText(str.c_str());
    }
    else
    {
		std::wstringstream wss;
		wss << L"FPS: " << mVideoDriver->getFPS() << L" pc: " << mVideoDriver->getPrimitiveCountDrawn();
        mInfoText->setText(wss.str().c_str());
    }

	// levelname
    if ( APP.GetLevelManager() )
    {
        int levIdx = APP.GetLevelManager()->GetCurrentLevelIndex();
        if ( levIdx >= 0 )
        {
            const LevelSettings &levelSettings = APP.GetLevelManager()->GetLevel(levIdx);
            std::wstring str(levelSettings.mName);
            if ( str.size() )
                mLevelText->setText(str.c_str());
        }
    }
    // player speed
    const Player* player = APP.GetGame()->GetPlayer(0);
    if ( player )
    {
        PhysicsObject* hoverPhys = APP.GetPhysics()->GetPhysicsObject(player->GetPhysicsId());
        if ( hoverPhys )
        {
            std::wstringstream wss;
            float speed = hoverPhys->GetVelocity().getLength();
            wss << L"speed: " << core::round32(speed);
            mSpeedText->setText(wss.str().c_str());
        }
    }
#endif // !HOVER_RELEASE
}

void Gui::PrintDriverFeatures()
{
    if ( !mVideoDriver )
        return;

    printf("Current driver supports:\n");
    printf("Is driver able to render to a surface? %s\n", mVideoDriver->queryFeature(video::EVDF_RENDER_TO_TARGET) ? "yes" : "no");
    printf("Is driver able to render with a bilinear filter applied? %s\n", mVideoDriver->queryFeature(video::EVDF_BILINEAR_FILTER) ? "yes" : "no");
    printf("Is hardeware transform and lighting supported? %s\n", mVideoDriver->queryFeature(video::EVDF_HARDWARE_TL) ? "yes" : "no");
    printf("Can the driver handle mip maps? %s\n", mVideoDriver->queryFeature(video::EVDF_MIP_MAP ) ? "yes" : "no");
    printf("Are stencilbuffers switched on and does the device support stencil buffers? %s\n", mVideoDriver->queryFeature(video::EVDF_STENCIL_BUFFER ) ? "yes" : "no");
    printf("Is Vertex Shader 1.1 supported? %s\n", mVideoDriver->queryFeature(video::EVDF_VERTEX_SHADER_1_1) ? "yes" : "no");
    printf("Is Vertex Shader 2.0 supported? %s\n", mVideoDriver->queryFeature(video::EVDF_VERTEX_SHADER_2_0) ? "yes" : "no");
    printf("Is Vertex Shader 3.0 supported? %s\n", mVideoDriver->queryFeature(video::EVDF_VERTEX_SHADER_3_0) ? "yes" : "no");
    printf("Is Pixel Shader 1.1 supported? %s\n", mVideoDriver->queryFeature(video::EVDF_PIXEL_SHADER_1_1) ? "yes" : "no");
    printf("Is Pixel Shader 1.2 supported? %s\n", mVideoDriver->queryFeature(video::EVDF_PIXEL_SHADER_1_2) ? "yes" : "no");
    printf("Is Pixel Shader 1.3 supported? %s\n", mVideoDriver->queryFeature(video::EVDF_PIXEL_SHADER_1_3) ? "yes" : "no");
    printf("Is Pixel Shader 1.4 supported? %s\n", mVideoDriver->queryFeature(video::EVDF_PIXEL_SHADER_1_4) ? "yes" : "no");
    printf("Is Pixel Shader 2.0 supported? %s\n", mVideoDriver->queryFeature(video::EVDF_PIXEL_SHADER_2_0) ? "yes" : "no");
    printf("Is Pixel Shader 3.0 supported? %s\n", mVideoDriver->queryFeature(video::EVDF_PIXEL_SHADER_3_0) ? "yes" : "no");
    printf("Are ARB vertex programs v1.0 supported? %s\n", mVideoDriver->queryFeature(video::EVDF_ARB_VERTEX_PROGRAM_1) ? "yes" : "no");
    printf("Are ARB fragment programs v1.0 supported? %s\n", mVideoDriver->queryFeature(video::EVDF_ARB_FRAGMENT_PROGRAM_1) ? "yes" : "no");
    printf("Is GLSL supported? %s\n", mVideoDriver->queryFeature(video::EVDF_ARB_GLSL) ? "yes" : "no");
    printf("Is HLSL supported? %s\n", mVideoDriver->queryFeature(video::EVDF_HLSL) ? "yes" : "no");
}

void Gui::DisplayInfoText(const std::string text_, float time_)
{
#if !defined(HOVER_RELEASE)
    mInfoTextTime = APP.GetIrrlichtManager()->GetIrrlichtTimer()->getRealTime() + time_;
    mInfoTextStr = text_;
#endif
}

void Gui::SetHoveredElement(irr::gui::IGUIElement* hovered)
{
	if ( hovered != mHoveredElement )
	{
		if ( mHoveredElement )
			mHoveredElement->drop();
		mHoveredElement = hovered;
		if ( mHoveredElement )
			mHoveredElement->grab();
		if ( mHoveredElementNoSub )
			mHoveredElementNoSub->drop();
		mHoveredElementNoSub = hovered;
		while ( mHoveredElementNoSub && mHoveredElementNoSub->isSubElement() )
		{
			mHoveredElementNoSub = mHoveredElementNoSub->getParent();
		}
		if (mHoveredElementNoSub)
			mHoveredElementNoSub->grab();
	}
}

bool Gui::OnEvent(const SEvent &event)
{
	switch ( event.EventType )
    {
#if !defined(_IRR_ANDROID_PLATFORM_) && !defined(HC1_SIMULATE_MOBILE_UI)
    	case EET_MOUSE_INPUT_EVENT:
    	{
    		// We set the focus on mouse-over instead of just on mouse-click.
			if ( mHoveredElement )
			{
				IGUIEnvironment* env = APP.GetIrrlichtManager()->GetIrrlichtDevice()->getGUIEnvironment();
				if ( env
					&& mHoveredElement != env->getRootGUIElement()
					&& mHoveredElement != env->getFocus()
					&& GuiDialog::AllowedToGetFocus(mHoveredElementNoSub) )
				{
					env->setFocus(mHoveredElement);
				}
			}
			break;
    	}
#endif
#ifndef _IRR_ANDROID_PLATFORM_
    	case EET_KEY_INPUT_EVENT:
    	{
			if ( event.KeyInput.PressedDown )
            {
            	IGUIEnvironment* env = APP.GetIrrlichtManager()->GetIrrlichtDevice()->getGUIEnvironment();
            	IGUIElement* focused = env->getFocus();
                switch(event.KeyInput.Key)
                {
                    case KEY_LEFT:
                    {
                    	IGUIElement* focusedNoSub = focused;
						while ( focusedNoSub && focusedNoSub->isSubElement() )
						{
							focusedNoSub = focusedNoSub->getParent();
						}
						if ( (focused && focused->hasType(EGUIET_EDIT_BOX)) ||
							 (focusedNoSub && focusedNoSub->hasType((EGUI_ELEMENT_TYPE)EGUIET_TEXT_SLIDER)) )
							break;
                    }
					// fall-through
                    case KEY_UP:
                    {
						IGUIElement *next = env->getNextElement(true, false);
						if (next && next != focused)
						{
							env->setFocus(next);
						}
                    }
                    break;
                    case KEY_RIGHT:
                    {
						IGUIElement* focusedNoSub = focused;
						while ( focusedNoSub && focusedNoSub->isSubElement() )
						{
							focusedNoSub = focusedNoSub->getParent();
						}
						if ( (focused && focused->hasType(EGUIET_EDIT_BOX)) ||
							 (focusedNoSub && focusedNoSub->hasType((EGUI_ELEMENT_TYPE)EGUIET_TEXT_SLIDER)) )
							break;
                    }
					// fall-through
                    case KEY_DOWN:
                    {
						IGUIElement *next = env->getNextElement(false, false);
						if (next && next != focused)
						{
							env->setFocus(next);
						}
                    }
                    break;
                    default:
                    break;
                }
            }
    		break;
    	}
#endif

#ifdef _IRR_ANDROID_PLATFORM_
    	case EET_KEY_INPUT_EVENT:
    	{
			if ( !event.KeyInput.PressedDown )
            {
                switch(event.KeyInput.Key)
                {
                    case KEY_RETURN:
					{
						hc1::android::setSoftInputVisibility(APP.GetAndroidApp(), false);
					}
					break;
					default:
					break;
				}
			}
			break;
		}
#endif

        case EET_GUI_EVENT:
        {
#ifdef HC1_ENABLE_EDITOR
            if ( GetUserControls()->OnEvent(event) )
                return true;
            if ( GetDeveloperSettings()->OnEvent(event) )
                return true;
            if ( GetGuiLevelManager()->OnEvent(event) )
                return true;
#endif
            IGUIEnvironment* env = APP.GetIrrlichtManager()->GetIrrlichtDevice()->getGUIEnvironment();
            if ( !env )
                break;
            switch((int)event.GUIEvent.EventType)
            {
                case EGET_ELEMENT_HOVERED:
                    SetHoveredElement(event.GUIEvent.Caller);
                break;
                case EGET_ELEMENT_LEFT:
                    SetHoveredElement(NULL);
                break;

                case EGET_ELEMENT_FOCUS_LOST:
                    //fprintf(stderr, "focus lost: %d", event.GUIEvent.Caller ? event.GUIEvent.Caller->getID() : -99);
#ifdef _IRR_ANDROID_PLATFORM_
					if ( event.GUIEvent.Caller->getType() == irr::gui::EGUIET_EDIT_BOX )
					{
						hc1::android::setSoftInputVisibility(APP.GetAndroidApp(), false);
					}
#endif
                    SetFocusElement(NULL);
                break;
                case EGET_ELEMENT_FOCUSED:
                    //fprintf(stderr, "focus set: %d", event.GUIEvent.Caller ? event.GUIEvent.Caller->getID() : -99);
                    if ( event.GUIEvent.Caller->getID() < 0 )
                    {
                        SetFocusElement(NULL);
                    }
                    else
                    {
                        PlayGuiSound( GS_FOCUS );
                        SetFocusElement(event.GUIEvent.Caller);
#ifdef _IRR_ANDROID_PLATFORM_
						if ( event.GUIEvent.Caller->getType() == irr::gui::EGUIET_EDIT_BOX )
						{
							// TODO: couldn't figure out yet how to send existing text to Android.
							// Wouldn't matter much - except that Android does for some unknown reason
							// absorb all delete-key events when it thinks there is no text.
							// So it's not possible to remove text which is already in the editbox.
							static_cast<gui::IGUIEditBox*>(event.GUIEvent.Caller)->setText(L"");

							hc1::android::setSoftInputVisibility(APP.GetAndroidApp(), true);
						}
#endif
                    }
                break;

                case (EGUI_EVENT_TYPE_HC)EGET_TEXTSLIDER_CHANGED:
                    PlayGuiSound(GS_FOCUS);
                break;

                default:
                break;
            }

            break;
        }
        default:
        break;
    }

#ifdef HC1_ENABLE_EDITOR
    if ( GetGuiEditor() && GetGuiEditor()->OnEvent(event) )
        return true;
#endif

    if ( event.EventType == EET_GUI_EVENT && event.GUIEvent.Caller )
    {
        EventObjectMap::iterator it = mGuiEventFunctors.find(event.GUIEvent.Caller->getID());
        if ( it != mGuiEventFunctors.end() )
        {
            if ( it->second->CallEvent(event) )
                return true;
        }
    }

    switch ( event.EventType )
    {
        case EET_GUI_EVENT:
        {
            switch(event.GUIEvent.EventType)
            {
                case EGET_BUTTON_CLICKED:
                {
                    if ( mClickSoundOverride )
                    {
                        mClickSoundOverride = false;
                    }
                    else
                    {
                        PlayGuiSound( GS_CLICK_OK );
                    }
                }
                break;
                default:
                break;
            }
         }
        break;
        default:
        break;
    }

    return false;
}

void Gui::PlayOverrideClickSound(GUI_SOUNDS sound_)
{
    mClickSoundOverride = true;
    PlayGuiSound( sound_ );
}

void Gui::PlayGuiSound( GUI_SOUNDS sound_)
{
#ifdef HC1_ENABLE_SOUND
    if ( ! APP.GetSound() )
        return;
	if ( APP.GetMode() != MODE_GUI )
		return;

    switch ( sound_ )
    {
        case GS_FOCUS:
            if (    (mSoundRefClick1 < 0 || !APP.GetSound()->IsPlaying(mSoundRefClick1))
                &&  (mSoundRefClick2 < 0 || !APP.GetSound()->IsPlaying(mSoundRefClick2))
                )
            {
                mSoundRefFocus = APP.GetSound()->PlaySample(mSoundFocus.c_str(), 1.f, false);
            }
        break;
        case GS_CLICK_OK:
            mSoundRefClick1 = APP.GetSound()->PlaySample(mSoundClick1.c_str(), 1.f, false);
        break;
        case GS_CLICK_CANCEL:
            mSoundRefClick2 = APP.GetSound()->PlaySample(mSoundClick2.c_str(), 1.f, false);
        break;
    }
#endif
}

void Gui::AddGuiEventFunctor(int elementId_, IEventFunctor * functor_ )
{
    mGuiEventFunctors[elementId_] = functor_;
}

void Gui::RemoveGuiEventFunctor(int elementId_)
{
    EventObjectMap::iterator it = mGuiEventFunctors.find(elementId_);
    if ( it != mGuiEventFunctors.end() )
    {
        delete it->second;
        mGuiEventFunctors.erase(it);
    }
}

void Gui::SetActiveDialog(GuiDialog * dialog_)
{
    if ( mEnvironment )
    {
        mEnvironment->setFocus( NULL );
    }
	mPreviousActiveDialog = mActiveDialog;
    if ( mActiveDialog )
    {
        mActiveDialog->Hide();
    }
    mActiveDialog = dialog_;
    if ( !APP.GetEditGui() && mActiveDialog )
    {
        mActiveDialog->Show();
        mActiveDialog->SetActiveFocus();
    }
}

GuiDialog* Gui::GetActiveDialog()
{
    return mActiveDialog;
}

GuiDialog* Gui::GetPreviousActiveDialog()
{
	return mPreviousActiveDialog;
}

void Gui::SetModalDialog(GuiDialog * dialog_)
{
    if ( mEnvironment )
    {
        mEnvironment->setFocus( NULL );
    }
    if ( mModalDialog )
    {
        if ( mActiveDialog )
        {
            mActiveDialog->Enable();
        }
        mModalDialog->Hide();
    }
    mModalDialog = dialog_;
    if ( mModalDialog )
    {
        if ( mActiveDialog )
        {
            mActiveDialog->Disable();
        }
        mModalDialog->Show();
        mModalDialog->SetActiveFocus();
    }
    else if ( mActiveDialog )
    {
        mActiveDialog->SetActiveFocus();
    }
}

GuiDialog* Gui::GetActiveModalDialog()
{
    return mModalDialog;
}

void Gui::SetShowProfiler(bool show)
{
	if (mGuiProfiler && mEnvironment)
	{
		mEnvironment->getRootGUIElement()->bringToFront( mProfilerBackground );
		mEnvironment->getRootGUIElement()->bringToFront( mGuiProfiler );
        mGuiProfiler->setVisible(show);
        mProfilerBackground->setVisible(show);
        if (mGuiProfiler2)
		{
			mEnvironment->getRootGUIElement()->bringToFront( mGuiProfiler2 );
			mGuiProfiler2->setVisible(show);
		}
	}
}

bool Gui::GetShowProfiler() const
{
	if (mGuiProfiler)
		return mGuiProfiler->isVisible();
	return false;
}

irr::gui::IGUIProfiler * Gui::GetGuiProfiler()
{
	return mGuiProfiler;
}

irr::gui::IGUIProfiler * Gui::GetGuiProfiler2()
{
	return mGuiProfiler2;
}
