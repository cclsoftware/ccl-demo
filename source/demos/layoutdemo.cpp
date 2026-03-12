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
// Filename    : layoutdemo.cpp
// Description : Layout Demo
//
//************************************************************************************************

#include "../demoitem.h"

#include "ccl/base/message.h"

#include "ccl/app/controls/usercontrol.h"

#include "ccl/public/gui/iparameter.h"
#include "ccl/public/gui/graphics/igraphics.h"

using namespace CCL;

//************************************************************************************************
// LayoutDemoView
//************************************************************************************************

class LayoutDemoView: public UserControl
{
public:
	LayoutDemoView (RectRef size);

	// UserControl
	void draw (const DrawEvent& event) override;
	void onSize (PointRef delta) override;

protected:
	void drawAndOffset (const DrawEvent& event, Rect& rect, StringRef text, FontRef font, SolidBrush& brush);
};

//************************************************************************************************
// LayoutDemoView
//************************************************************************************************

LayoutDemoView::LayoutDemoView (RectRef size)
: UserControl (size)
{
	setSizeLimits (SizeLimit (10, 10, kMaxCoord, kMaxCoord));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void LayoutDemoView::onSize (PointRef delta)
{
	UserControl::onSize (delta);
	invalidate ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void LayoutDemoView::draw (const DrawEvent& event)
{
	Rect r;
	getClientRect (r);
	event.graphics.fillRect (r, getVisualStyle ().getBackBrush ());
	event.graphics.drawRect (r, getVisualStyle ().getForePen ());

	Coord w = getWidth ();
	Coord h = getHeight ();
	const SizeLimit& limits = getSizeLimits ();
	bool hasMinW = limits.minWidth > 0;
	bool hasMaxW = limits.maxWidth > 0 && limits.maxWidth != kMaxCoord;
	bool hasMinH = limits.minHeight > 0;
	bool hasMaxH = limits.maxHeight > 0 && limits.maxHeight != kMaxCoord;

	int numLines = 1;
	if(hasMinW || hasMaxW)
		numLines++;
	if(hasMinH || hasMaxH)
		numLines++;

	Font font (getVisualStyle ().getTextFont ());
	font.setSize (ccl_bound (float(r.bottom / numLines), 8.f, 11.f));
	r.bottom = (int)font.getSize () + 2;

	SolidBrush brush (getVisualStyle ().getTextBrush ());
	Color color = brush.getColor ();

	// title & coords
	char str[1024];
	if(getTitle ().isEmpty ())
		::snprintf (str, sizeof(str), "%d, %d", w,h );
	else
	{
		char title[1024];
		getTitle ().toASCII (title, sizeof(title));
		::snprintf (str, sizeof(str), "%s: %d, %d", title, w, h);
	}
	event.graphics.drawString (r, str, font, brush);
	
	// width limits
	if(hasMinW || hasMaxW)
	{
		r.offset (0, r.getHeight ());

		drawAndOffset (event, r, "W: [", font, brush);
		brush.setColor (w > limits.minWidth ? Colors::kBlack : Colors::kRed);
		::snprintf (str, sizeof(str), "%d", limits.minWidth);
		drawAndOffset (event, r, str, font, brush);

		drawAndOffset (event, r, ", ", font, brush);

		brush.setColor (w < limits.maxWidth ? Colors::kBlack : Colors::kRed);
		if(hasMaxW)
			::snprintf (str, sizeof(str), "%d]", limits.maxWidth);
		else
			::snprintf (str, sizeof(str), "oo]");
		drawAndOffset (event, r, str, font, brush);
	}

	if(hasMinH || hasMaxH)
	{
		r.offset (-r.left, r.getHeight ());

		drawAndOffset (event, r, "H: [", font, brush);
		brush.setColor (h > limits.minHeight ? Colors::kBlack : Colors::kRed);
		::snprintf (str, sizeof(str), "%d", limits.minHeight);
		drawAndOffset (event, r, str, font, brush);

		drawAndOffset (event, r, ", ", font, brush);

		brush.setColor (h < limits.maxHeight ? Colors::kBlack : Colors::kRed);
		if(hasMaxW)
			::snprintf (str, sizeof(str), "%d]", limits.maxHeight);
		else
			::snprintf (str, sizeof(str), "oo]");
		drawAndOffset (event, r, str, font, brush);
	}

	UserControl::draw (event);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void LayoutDemoView::drawAndOffset (const DrawEvent& event, Rect& rect, StringRef text, FontRef font, SolidBrush& brush)
{
	Rect used;
	event.graphics.measureString (used, text, font);
	event.graphics.drawString (rect, text, font, brush);
	rect.offset (used.getWidth (), 0);
	brush.setColor (Colors::kBlack);
}

//************************************************************************************************
// LayoutDemo
//************************************************************************************************

class LayoutDemo: public DemoComponent
{
public:
	LayoutDemo ()
	{
		paramList.addInteger (0, kMaxCoord, CSTR ("value1"));
		paramList.addInteger (0, kMaxCoord, CSTR ("value2"));
	}

	// Component
	IView* CCL_API createView (StringID name, VariantRef data, const Rect& bounds) override
	{
		if(name == "demoView")
			return  *NEW LayoutDemoView (bounds);
		return nullptr;
	}

	tbool CCL_API getProperty (Variant& var, MemberID propertyId) const override
	{
		IParameter* param = paramList.lookup (propertyId);
		if(param)
		{
			var = param->getValue ();
			return true;
		}
		return DemoComponent::getProperty (var, propertyId);
	}

	tbool CCL_API paramChanged (IParameter* param) override
	{
		signal (Message (kPropertyChanged));
		return true;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("Layout", "Anchor Layout", LayoutDemo)
REGISTER_DEMO ("Layout", "Anchor Layout - Table", LayoutDemo)
