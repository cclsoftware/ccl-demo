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
// Filename    : workspacedemo.cpp
// Description : Workspace Demo
//
//************************************************************************************************

#include "../demoitem.h"

#include "ccl/public/gui/framework/iworkspace.h"
#include "ccl/public/gui/framework/viewbox.h"
#include "ccl/public/guiservices.h"

using namespace CCL;

//************************************************************************************************
// WorkspaceDemo
//************************************************************************************************

class WorkspaceDemo: public DemoComponent
{
public:
	WorkspaceDemo ()
	{
		workspace = System::GetWorkspaceManager ().getWorkspace ("WorkspaceDemo");
	}

	// Component
	IView* CCL_API createView (StringID name, VariantRef data, const Rect& bounds) override
	{
		if(name == "workspace" && workspace)
		{
			ViewBox vb (ClassID::View, bounds);
			UnknownPtr<IViewFactory> viewFactory (workspace);
			if(viewFactory)
			{
				IView* container = viewFactory->createView ("PerspectiveContainer", data, bounds);
				if(container)
					vb.getChildren ().add (container);
			}
			return vb;
		}
		return nullptr;
	}

	tbool CCL_API checkCommandCategory (CStringRef category) const override
	{
		return true;
	}

	tbool CCL_API interpretCommand (const CommandMsg& msg) override
	{
		if(msg.category == "Open" && workspace)
		{
			if(!msg.checkOnly ())
			{
				if(workspace->isViewOpen (msg.name))
					workspace->closeView (msg.name);
				else
					workspace->openView (msg.name);
			}
			return true;
		}
		return false;
	}

protected:
	IWorkspace* workspace;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("Layout", "Workspace", WorkspaceDemo)
