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
// Filename    : focusdemo.cpp
// Description : Layout Demo
//
//************************************************************************************************

#include "../demoitem.h"

#include "ccl/app/controls/usercontrol.h"

#include "ccl/public/gui/graphics/igraphics.h"

using namespace CCL;

//************************************************************************************************
// FocusContainerView
// A container that can take the focus
//************************************************************************************************

class FocusContainerView: public UserControl
{
public:
	FocusContainerView (RectRef size);

	// UserControl
	void draw (const DrawEvent& event) override;
	bool onFocus (const FocusEvent& event) override;

protected:
	bool hasFocus;
};

//************************************************************************************************
// FocusContainerView
//************************************************************************************************

FocusContainerView::FocusContainerView (RectRef size)
: UserControl (size),
  hasFocus (false)
{
	wantsFocus (true);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void FocusContainerView::draw (const DrawEvent& event)
{
	UserControl::draw (event);
	if(hasFocus)
	{
		Rect r;
		getClientRect (r);
		Color c (Colors::kWhite);
		c.setAlphaF (0.5);
		event.graphics.fillRect (r, SolidBrush (c));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool FocusContainerView::onFocus (const FocusEvent& event)
{
	hasFocus = (event.eventType == FocusEvent::kSetFocus);
	invalidate ();
	return true;
}

//************************************************************************************************
// FocusDemo
//************************************************************************************************

class FocusDemo: public DemoComponent
{
public:
	// Component
	IView* CCL_API createView (StringID name, VariantRef data, const Rect& bounds) override
	{
		if(name == "focusContainer")
			return *NEW FocusContainerView (bounds);
		return nullptr;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("Layout", "Focus", FocusDemo)
