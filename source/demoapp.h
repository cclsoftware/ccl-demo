//************************************************************************************************
//
// CCL Demo Application
//
// This file is part of Crystal Class Library (R)
// Copyright (c) 2025 CCL Software Licensing GmbH.
// All Rights Reserved.
//
// Licensed for use under either:
//  1. a Commercial License provided by CCL Software Licensing GmbH, or
//  2. GNU Affero General Public License v3.0 (AGPLv3).
// 
// You must choose and comply with one of the above licensing options.
// For more information, please visit ccl.dev.
//
// Filename    : demoapp.h
// Description : Demo Application
//
//************************************************************************************************

#ifndef _demoapp_h
#define _demoapp_h

#include "ccl/app/application.h"

#include "ccl/public/plugins/iservicemanager.h"

namespace CCL {

//************************************************************************************************
// DemoApp
//************************************************************************************************

class DemoApp: public Application,
			   public IServiceNotification
{
public:
	DECLARE_CLASS (DemoApp, Application)

	DemoApp ();

	// Application
	bool startup () override;
	bool shutdown () override;
	tresult CCL_API appendContextMenu (IContextMenu& contextMenu) override;
	void CCL_API notify (ISubject* subject, MessageRef msg) override;
	void CCL_API extendMenu (IMenu& menu, StringID name) override;
	
	// IServiceNotification
	tresult CCL_API onServiceNotification (const IServiceDescription& description, int eventCode) override;

	CLASS_INTERFACE (IServiceNotification, Application)

protected:	
	MenuPosition appearanceMenu;
};

} // namespace CCL

#endif // _demoapp_h
