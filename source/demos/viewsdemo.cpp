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
// Filename    : viewsdemo.cpp
// Description : Views Demo
//
//************************************************************************************************

#include "../demoitem.h"

#include "ccl/app/controls/usercontrol.h"

#include "ccl/public/gui/iparameter.h"
#include "ccl/public/gui/graphics/igraphics.h"
#include "ccl/public/gui/framework/iskinmodel.h"
#include "ccl/public/gui/framework/itheme.h"

using namespace CCL;

//************************************************************************************************
// TransformTest
//************************************************************************************************

class TransformTest: public UserControl
{
public:
	float scaleFactor;
	
	TransformTest (RectRef size, float scaleFactor = 1.f)
	: UserControl (size),
	  scaleFactor (scaleFactor)
	{
		testImage = getTheme ().getImage ("TransformTestImage");
	}
	
	void draw (const DrawEvent& event) override
	{
		IGraphics& graphics (event.graphics);
		AntiAliasSetter smoother (graphics);

		Rect client;
		getClientRect (client);
		client.setSize (client.getSize () *  (1.f / scaleFactor));
		
		// outer transform (simulate window scaling)
		graphics.saveState ();
		graphics.addTransform (Transform ().scale (scaleFactor, scaleFactor));
		{
			graphics.drawRect (client, Pen (Colors::kBlack));
			FontRef standardFont = getTheme ().getStatics ().getStandardFont ();
			graphics.drawString (client, String ("abcdefghijk"), standardFont, SolidBrush (Colors::kWhite), Alignment::kLeftTop);
			
			if(testImage)
				graphics.drawImage (testImage, Point (client.getCenter ()));
			
			// some inner transform
			Transform t;
			t.translate (client.getWidth () * 0.5f, client.getHeight () * .5f);
			t.rotate (.785f);
			t.translate (client.getWidth () * -0.25f, client.getHeight () * -0.25f);
			graphics.saveState ();
			graphics.addTransform (t);
			
			Rect rect (0, 0, client.getSize () * 0.5f);
			graphics.drawRect (rect, Pen (Colors::kBlue));
			graphics.drawLine (rect.getLeftTop (), rect.getRightBottom (), Pen (Colors::kBlue));
			graphics.drawLine (rect.getRightTop (), rect.getLeftBottom (), Pen (Colors::kBlue));
			graphics.restoreState ();
		}
		graphics.restoreState ();
	}
	
private:
	SharedPtr<IImage> testImage;
};

//************************************************************************************************
// ViewsDemo
//************************************************************************************************

class ViewsDemo: public DemoComponent
{
public:
	ViewsDemo ()
	{
		IParameter* p = paramList.addString (CSTR ("string1"), 'str1');
		p->fromString (CCLSTR ("View 1"));
		p = paramList.addString (CSTR ("string2"), 'str2');
		p->fromString (CCLSTR ("View 2"));
		p = paramList.addString (CSTR ("string3"), 'str3');
		p->fromString (CCLSTR ("View 3"));
	}
	
	// Component
	IView* CCL_API createView (StringID name, VariantRef data, const Rect& bounds) override
	{
		if(name == "TransformTest")
		{
			double scaleFactor = 1.;
			UnknownPtr<ISkinCreateArgs> args (data.asUnknown ());
			if(args)
			{
				String string;
				args->getElement ()->getDataDefinition (string, "scaleFactor");
				string.getFloatValue (scaleFactor);
			}
			return *NEW TransformTest (bounds, (float)scaleFactor);
		}
		return nullptr;
	}
	
	tbool CCL_API paramChanged (IParameter* param) override
	{
		return true;
	}
	
protected:
};

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("Controls", "Views", ViewsDemo)
