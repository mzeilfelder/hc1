#include <irrlicht.h>
#include "driverChoice.h"

// include the gui creator element factory
#include "CGUIEditFactory.h"

using namespace irr;
using namespace gui;

#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#endif

int main()
{
	// ask user for driver
	video::E_DRIVER_TYPE driverType=driverChoiceConsole(false);
	if (driverType==video::EDT_COUNT)
		return 1;

	IrrlichtDevice *device = createDevice(driverType, core::dimension2du(800, 600));
	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();
	gui::IGUIEnvironment *env = device->getGUIEnvironment();

	device->setResizable(true);

	/*
		first we create the factory which can make new GUI elements
		and register it with the gui environment.
	*/

	IGUIElementFactory* factory = new CGUIEditFactory(env);
	env->registerGUIElementFactory(factory);
	// remember to drop since we created with a create call
	factory->drop();

	IGUISkin *skin = env->createSkin(EGST_WINDOWS_METALLIC);
	env->setSkin(skin);

	IGUIFont *font = env->getFont("../../media/lucida.xml");
	if (font)
		skin->setFont(font);
	skin->drop();

	// change transparency of skin
	for (s32 i=0; i<gui::EGDC_COUNT ; ++i)
	{
		video::SColor col = env->getSkin()->getColor((gui::EGUI_DEFAULT_COLOR)i);
		col.setAlpha(250);
		env->getSkin()->setColor((gui::EGUI_DEFAULT_COLOR)i, col);
	}

	/*
		now we add the GUI Editor Workspace
	*/

	env->addGUIElement("GUIEditor");

	while(device->run())
	{
		if (!device->isWindowMinimized())
		{
			const core::dimension2d<u32>& screenSize = driver->getScreenSize();
			wchar_t caption[512];
			swprintf_irr(caption, 512, L"screen (%4u/%4u)", screenSize.Width, screenSize.Height);
			device->setWindowCaption(caption);
			driver->beginScene(video::ECBF_COLOR | video::ECBF_DEPTH, video::SColor(0,200,200,200));
			smgr->drawAll();
			env->drawAll();
			driver->endScene();
		}

		// be nice to CPU
		device->sleep(10);
		if (!device->isWindowActive())
			device->sleep(90);
	}

	device->closeDevice();
	device->drop();

	return 0;
}
