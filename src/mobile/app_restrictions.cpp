// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#include "app_restrictions.h"
#include "billing.h"
#include "../config.h"
#include "../main.h"
#include "../logging.h"
#include "../tinyxml/tinyxml.h"

bool AppRestrictions::DoDisplayAds()
{
	PollBillingServer();

    if ( ! APP.GetConfig() )
        return false;

	TiXmlElement * billingElement = APP.GetConfig()->GetBillingSettings();
	if (!billingElement)
		return false;

	int adfree=0;
	if ( billingElement->Attribute("adfree", &adfree ) )
	{
		if (adfree < 0)
			return true;
	}

	return false;
}

int AppRestrictions::PollBillingServer()
{
	IBilling* billing = APP.GetBilling();
	Config* config = APP.GetConfig();

    if (!billing || !config)
	{
		LOG.LogLn(LP_DEBUG, "Ads: missing billing or config");
        return -1;
	}

	int syncroState = billing->getSynchronizeState();
	if ( syncroState < 0 )
	{
		LOG.LogLn(LP_DEBUG, "Ads: syncroState is ", syncroState);
		return -1;
	}
	if ( syncroState > 0 )
	{
		LOG.LogLn(LP_DEBUG, "Ads: syncroState is ", syncroState);
		return 1;
	}

	TiXmlElement * billingElement = APP.GetConfig()->GetBillingSettings();
	if (!billingElement)
	{
		LOG.LogLn(LP_DEBUG, "Ads: missing billingElement");
		return -1;
	}

	int numPurchasedItems = billing->getNumPurchasedItems();

	// we only have one element which can be bought. Could compare names etc... but who cares - easy to cheat this anyway if someone wants to play free that urgently.
	int old_adfree=0;
	billingElement->Attribute("adfree", &old_adfree );
	int new_adfree = numPurchasedItems > 0 ? 1 : -1;
	if (old_adfree != new_adfree)
	{
		if ( new_adfree == 1 )
		{
			std::string purchasedItem(billing->getPurchasedItem(0));
			LOG.LogLn(LP_INFO, "Ads: *** bought item: ", purchasedItem.c_str());
		}
		else if ( old_adfree > 0 )
		{
			LOG.Info(L"Ads: *** sold previously bought item\n");
		}
		else
		{
			LOG.LogLn(LP_DEBUG, "Ads: first check, no buying so far");
		}
		billingElement->SetAttribute("adfree", new_adfree);
		config->Save();
	}

	return 0;
}
