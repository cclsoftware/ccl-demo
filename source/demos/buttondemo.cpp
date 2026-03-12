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
// Filename    : buttondemo.cpp
// Description : Button Demo
//
//************************************************************************************************

#include "../demoitem.h"

#include "ccl/public/gui/iparameter.h"

using namespace CCL;

//************************************************************************************************
// ButtonDemo
//************************************************************************************************

class ButtonDemo: public DemoComponent
{
public:
	ButtonDemo ()
	{
		paramList.addParam (CSTR ("bool"));

		UnknownPtr<IListParameter> listParam = paramList.addList (CSTR ("list"));
		listParam->appendString (CCLSTR ("State 1"));
		listParam->appendString (CCLSTR ("State 2"));
		listParam->appendString (CCLSTR ("State 3"));
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("Controls", "Button", ButtonDemo)
