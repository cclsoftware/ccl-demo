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
// Filename    : coreviewdemo.h
// Description : Core View Demo, embedded part of embeddeddemo.cpp
//
//************************************************************************************************

// PLEASE NOTE: Core Library (corelib) needs to be linked to ccldemo for this to work.
// It's an exception to the general rule.

#include "core/portable/gui/coreviewbuilder.h"
#include "core/portable/gui/corelistview.h"
#include "core/portable/gui/corefont.h"
#include "core/portable/gui/corealertbox.h"
#include "core/portable/corecomponent.h"
#include "core/portable/corefile.h"

//************************************************************************************************
// CoreUserControl
//************************************************************************************************

class CoreUserControl: public Core::Portable::View
{
public:
	CoreUserControl (Core::RectRef size = Core::Rect ())
	: View (size)
	{}

	void draw (const Core::Portable::DrawEvent& e) override
	{
		using namespace Core;

		const Core::Portable::Style& style = getStyle ();

		Rect r;
		getClientRect (r);		
		e.graphics.fillRect (r, style.getBackColor ());
		e.graphics.drawRect (r, style.getForeColor ());		

		const int kMargin = 10;
		r.top += kMargin;
		r.bottom -= kMargin;

		e.graphics.setMode (Core::Portable::Graphics::kAntiAlias);
		e.graphics.drawLine (r.getLeftTop (), r.getRightBottom (), style.getForeColor ());
		e.graphics.drawLine (r.getRightTop (), r.getLeftBottom (), style.getForeColor ());
		e.graphics.setMode (0);

		r.contract (2);
		e.graphics.drawString (r, "UserControl", style.getTextColor (), style.getFontName (), style.getTextAlign ());
	}
};

//************************************************************************************************
// CoreComponentDemo
//************************************************************************************************

class CoreComponentDemo: public Core::Portable::Component,
						 public Core::Portable::ViewController
{
public:
	BEGIN_CORE_CLASS ('CrCD', CoreComponentDemo)
		ADD_CORE_CLASS_ (ViewController)
	END_CORE_CLASS (Component)

	enum ParamTags
	{
		kTest = 100,
		kText,
		kList,
		kValue
	};

	CoreComponentDemo ()
	{
		static Core::CStringPtr ListItems[] = 
		{
			"Item A",
			"Item B"
		};

		BEGIN_PARAMINFO (theParamList)
			PARAM_TOGGLE (kTest, "test", 0, "", 0),
			PARAM_STRING (kText, "text", 0),
			PARAM_LIST (kList, "list", ListItems, 0, 0),
			PARAM_FLOAT (kValue, "value", 0.f, 100.f, 50.f, 1.f, 0, 0, 0, 0)
		END_PARAMINFO

		paramList.add (theParamList, ARRAY_COUNT (theParamList));
		updateText ();

		testList = NEW Core::Portable::ListViewModel;
		for(int i = 0; i < 16; i++)
		{
			Core::CString32 title;
			title.appendFormat ("Item %d", i+1);
			testList->addItem (NEW Core::Portable::ListViewItem (title));
		}
	}

	~CoreComponentDemo ()
	{
		delete testList;
	}

	void updateText ()
	{
		paramList.byTag (kText)->fromString (paramList.byTag (kTest)->getValue () != 0 ? "Toggle is on" : "Toggle is off");
	}

	// ViewController
	Core::Portable::View* createView (Core::CStringPtr _type) override
	{
		#if 1
		Core::ConstString type (_type);
		if(type == "UserControl")
			return NEW CoreUserControl;
		else
		#endif
			return 0;
	}

	void* getObjectForView (Core::CStringPtr name, Core::CStringPtr type) override
	{
		if(type == Core::Portable::kParamType)
			return lookupParameter (name);
		//else if(type == Core::Portable::kSubcontrollerType)
		//	return  dynamic_cast<ViewController*> (lookupChild (name)); RTTI is disabled here!
		else if(type == Core::Portable::kListViewModelType)
			return testList;
		else
			return 0;
	}

	// Component
	void paramChanged (Core::Portable::Parameter* p, int msg) override
	{
		if(msg == Core::Portable::Parameter::kEdit)
			switch(p->getTag ())
			{
			case kTest :
				updateText ();

				#if 1 // alert test
				{
					Core::Portable::AlertBox alertBox;
					alertBox.setText ("You pushed the button!");
					alertBox.initButtons (Core::Portable::AlertBox::kOk, Core::Portable::AlertBox::kCancel);
					alertBox.runAsync ();
				}
				#endif
				break;
			}
	}

protected:
	Core::Portable::ListViewModel* testList;
};

