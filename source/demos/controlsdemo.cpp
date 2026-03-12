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
// Filename    : controlsdemo.cpp
// Description : Controls Demo
//
//************************************************************************************************

#include "../demoitem.h"

#include "ccl/app/navigation/webnavigator.h"
#include "ccl/app/components/colorpicker.h"

#include "ccl/base/message.h"
#include "ccl/base/storage/textfile.h"
#include "ccl/base/asyncoperation.h"

#include "ccl/public/gui/iparameter.h"
#include "ccl/public/gui/framework/itextmodel.h"
#include "ccl/public/gui/framework/ialert.h"
#include "ccl/public/gui/framework/ihelpmanager.h"
#include "ccl/public/gui/framework/guievent.h"
#include "ccl/public/gui/graphics/graphicsfactory.h"
#include "ccl/public/gui/graphics/igraphics.h"
#include "ccl/public/gui/graphics/markuptags.h"
#include "ccl/public/guiservices.h"
#include "ccl/public/plugservices.h"

#include "ccl/public/cclversion.h"

using namespace CCL;

//************************************************************************************************
// DemoTextModel
//************************************************************************************************

class DemoTextModel: public Object,
                     public AbstractTextModel
{
public:
	DemoTextModel ()
	: modelText ("Text Model Demo"),
	  wordStart (-1),
	  wordEnd (-1),
	  colors ({ Colors::kBlack, Colors::kRed, Colors::kBlue }),
	  colorIndex (0)
	{}

	// ITextModel
	void CCL_API toDisplayString (String& string) const override
	{
		string = modelText;
	}

	void CCL_API updateLayout (ITextLayout& layout) override
	{
		if(wordStart >= 0 && wordEnd >= 0)
		{
			// change color of some characters in existing layout
			ITextLayout::Range range (wordStart, wordEnd - wordStart);
			layout.setTextColor (range, colors[colorIndex]);
			layout.setFontStyle (range, Font::kBold, colors[colorIndex] == Colors::kRed);
		}
	}

	int CCL_API insertText (int textIndex, StringRef text, ITextModel::EditOptions options) override
	{
		String toInsert (text == "x" ? "**" : text); // we can insert what we like
		modelText.insert (textIndex, toInsert);
		signal (Message (kChanged));
		return toInsert.length ();
	}

	int CCL_API removeText (int textIndex, int length, ITextModel::EditOptions options) override
	{
		if(length < 0)
		{
			length = -length;
			textIndex = ccl_max (0, textIndex - length);
		}

		ccl_upper_limit (length, modelText.length () - textIndex);

		modelText.remove (textIndex, length);
		signal (Message (kChanged));
		return length;
	}

	tbool CCL_API drawBackground (const ITextLayout& layout, const DrawInfo& info) override
	{
		Rect rect (info.rect);
		rect.contract (2);
		info.graphics.drawEllipse (rect, Colors::kBlue);
		return true;
	}

	tbool CCL_API onTextInteraction (const ITextLayout& layout, const InteractionInfo& info) override
	{
		wordStart = -1;
		wordEnd = -1;

		// change color on cmd+click
		auto mouseEvent = info.editEvent.as<MouseEvent> ();
		if(mouseEvent && mouseEvent->keys.isSet (KeyState::kCommand))
		{
			// find clicked text position
			int textIndex = -1;
			PointF position (pointIntToF (mouseEvent->where));
			if(layout.hitTest (textIndex, position) == kResultOk)
			{
				// find word boundaries
				wordStart = modelText.subString (0, textIndex).lastIndex (" ");
				wordEnd = modelText.subString (textIndex).index (" ");
				wordStart = (wordStart >= 0) ? wordStart + 1 : 0;
				wordEnd = (wordEnd >= 0) ? wordEnd + textIndex : modelText.length ();

				colorIndex++;
				if(colorIndex >= colors.count ())
					colorIndex = 0;

				signal (Message (kRequestLayoutUpdate));
			}
			return true;
		}
		return false;
	}

	void CCL_API fromParamString (StringRef string) override
	{
		modelText = string;
		signal (Message (kChanged));
	}

	CLASS_INTERFACE (ITextModel, Object)

private:
	String modelText;
	int wordStart;
	int wordEnd;
	Vector<Color> colors;
	int colorIndex;
};

//************************************************************************************************
// ControlsDemo
//************************************************************************************************

class ControlsDemo: public DemoComponent
{
public:
	ControlsDemo ()
	: highlightIndex (-1)
	{
		// buttons
		paramList.addParam (CSTR ("button"), 'butt');
		paramList.addParam (CSTR ("blink"), 'blnk');
		
		// scrollbar
		IParameter* param = ccl_new<IParameter> (ClassID::ScrollParam);
		param->setName (CSTR ("scroll"));
		UnknownPtr<IScrollParameter> scrollParam = param;
		scrollParam->setRange (1000, 0.3f);
		paramList.add (param);
		
		// scrollParam2 for rangeSlider
		IParameter* param2 = ccl_new<IParameter> (ClassID::ScrollParam);
		param2->setName (CSTR ("scroll2"));
		UnknownPtr<IScrollParameter> scrollParam2 = param2;
		scrollParam2->setRange (1000, 0.3f);
		paramList.add (param2);
		
		// scrollParam3 for TriVectorPad
		IParameter* param3 = ccl_new<IParameter> (ClassID::ScrollParam);
		param3->setName (CSTR ("scroll3"));
		UnknownPtr<IScrollParameter> scrollParam3 = param3;
		scrollParam3->setRange (1000, 0.3f);
		paramList.add (param3);
		
		// editbox
		paramList.addString (CSTR ("string"), 'strn')->fromString (MarkupBuilder ().append ("Dynamic ").italic ("Text").append (" ").underline ("Size"));
		
		// checkbox
		paramList.addParam (CSTR ("check"), 'chek');
		
		// continuous
		paramList.addInteger (0, 1000, CSTR ("continuous"), 'slid');
		
		// discrete
		paramList.addInteger (-5, 30, CSTR ("discrete"), 'scdc');
		
		// scrollpicker parameters
		paramList.addInteger (-5, 12, CSTR ("picker"), 'scpi');
		paramList.addParam (CSTR ("wrapMode"), 'scpw');
		paramList.addParam (CSTR ("flatMode"), 'scfm');
		paramList.addParam (CSTR ("digitMode"), 'scdm');
		paramList.addParam (CSTR ("reverse"), 'scpr');
		
		// list
		UnknownPtr<IListParameter> listParam = paramList.addList (CSTR ("list"), 'list');
		listParam->appendString (CCLSTR ("Item 1"));
		listParam->appendString (CCLSTR ("Item 2"));
		listParam->appendString (CCLSTR ("Item 3"));

		// listadd for ComboBox
		paramList.addString (CSTR ("listadd"), 'lsta');

		// highlights
		paramList.addParam ("nextHighlight", 'next');
		paramList.addParam ("removeHighlight", 'remh');

		// text model
		ITextModelProvider* textModelProvider = paramList.addTextModel ("textModel", 'txmo');
		textModelProvider->setTextModel (AutoPtr<ITextModel> (NEW DemoTextModel));
		
		// color
		auto* colorParam = paramList.addColor (CSTR ("color"), 'clrp');
		UnknownPtr<IPaletteProvider> (colorParam)->setPalette (CustomColorPresets::instance ().getPalette ());
		addComponent (NEW ColorPicker (colorParam, false)); // false: no presets available
	}

	// Component
	IView* CCL_API createView (StringID name, VariantRef data, const Rect& bounds) override
	{
		return nullptr;
	}

	tbool CCL_API paramChanged (IParameter* param) override
	{
		if(param->getTag () == 'butt')
		{
			Promise (Alert::infoAsync (CCLSTR("You have clicked the button")));
		}
		else if(param->getTag () == 'lsta')
		{
			String itemName;
			param->toString (itemName);

			IParameter* p = paramList.lookup ("list");
			UnknownPtr<IListParameter> listParam = p;

			int index = p->getValue ();
			Variant newValue (itemName);

			listParam->setValueAt (index, newValue);
		}
		else if(param->getTag () == 'next')
		{
			// highlight based on param names (which controls use as helpId)
			if(++highlightIndex > paramList.count () - 1)
				highlightIndex = 0;

			System::GetHelpManager ().highlightControl (String (paramList.at (highlightIndex)->getName ()), nullptr, true);
		}
		else if(param->getTag () == 'remh')
		{
			System::GetHelpManager ().discardHighlights ();
		}
		return true;
	}

	void CCL_API paramEdit (IParameter* param, tbool begin) override
	{
		if(param->getTag () == 'blnk') // test kSilentTracking option
		{
			if(!begin)
				param->setValue (!param->getValue ().asBool ());
		}
	}

protected:
	int highlightIndex;
};

//************************************************************************************************
// WebViewDemo
//************************************************************************************************

class WebViewDemo: public DemoComponent
{
public:
	WebViewDemo ()
	{
		WebNavigator* webNavigator = NEW WebNavigator;
		webNavigator->setHomeUrl (Url (CCL_PRODUCT_WEBSITE));
		addComponent (webNavigator);
	}
};

//************************************************************************************************
// TextEditorDemo
//************************************************************************************************

class TextEditorDemo: public DemoComponent
{
public:
	TextEditorDemo ()
	{
		IParameter* textParam = paramList.addString (CSTR ("text"), 'text');

		Url path (CCLSTR ("resource:///commands.xml"));
		textParam->fromString (TextUtils::loadString (path));
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("Controls", "WebView", WebViewDemo)
REGISTER_DEMO ("Controls", "TextEditor", TextEditorDemo)
REGISTER_DEMO ("Controls", "Headings", DemoComponent)
REGISTER_DEMO ("Controls", "ImageView", DemoComponent)
REGISTER_DEMO ("Experimental", "More Controls", ControlsDemo)
