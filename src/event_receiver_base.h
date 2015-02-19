// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#ifndef EVENT_RECEIVER_BASE_H
#define EVENT_RECEIVER_BASE_H

#include "irrlicht_manager.h"

class EventReceiverBase : public irr::IEventReceiver
{
public:
	virtual bool OnEvent(const irr::SEvent& event);
};

#endif // EVENT_RECEIVER_BASE_H
