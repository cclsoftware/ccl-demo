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
// Filename    : skindemo.cpp
// Description : Skin Demo
//
//************************************************************************************************

#include "../demoitem.h"

#include "ccl/base/storage/attributes.h"

#include "ccl/app/controls/listviewmodel.h"

#include "ccl/public/gui/graphics/igraphics.h"
#include "ccl/public/gui/framework/itheme.h"
#include "ccl/public/gui/framework/iskinmodel.h"
#include "ccl/public/gui/framework/skinxmldefs.h"
#include "ccl/public/gui/framework/icolorscheme.h"
#include "ccl/public/plugservices.h"

using namespace CCL;

//************************************************************************************************
// SkinDemo
//************************************************************************************************

class SkinDemo: public DemoComponent
{
public:
	DECLARE_STRINGID_MEMBER (kCommentID)
	DECLARE_STRINGID_MEMBER (kColorID)

	class IconItem: public ListViewItem
	{
	public:
		// ListViewItem
		bool getTooltip (String& tooltip, StringID id) override
		{
			tooltip = getTitle ();
			return true;
		}
	};

	class ColorItem: public ListViewItem
	{
	public:
		PROPERTY_SHARED_AUTO (IColorScheme, colorScheme, ColorScheme)
		PROPERTY_MUTABLE_CSTRING (colorName, ColorName)

		// ListViewItem
		bool drawDetail (const IItemModel::DrawInfo& info, StringID id, AlignmentRef alignment) override
		{
			if(id == kColorID)
			{
				Color color = colorScheme->getColor (colorName);
				Rect r (info.rect);
				r.contract (2);
				info.graphics.fillRect (r, SolidBrush (color));
				return true;
			}
			return ListViewItem::drawDetail (info, id, alignment);
		}
	};

	SkinDemo ()
	: icons (NEW ListViewModel),
	  colors (NEW ListViewModel),
	  styles (NEW ListViewModel)
	{
		// configure list columns
		colors->getColumns ().addColumn (IColumnHeaderList::kAutoWidth, 0, ListViewModel::kTitleID);
		colors->getColumns ().addColumn (50, 0, kColorID);
		colors->getColumns ().addColumn (IColumnHeaderList::kAutoWidth, 0, kCommentID);

		styles->getColumns ().addColumn (IColumnHeaderList::kAutoWidth, 0, ListViewModel::kTitleID);
		styles->getColumns ().addColumn (IColumnHeaderList::kAutoWidth, 0, kCommentID);

		addObject ("icons", icons);
		addObject ("colors", colors);
		addObject ("styles", styles);

		if(UnknownPtr<ISkinModel> skinModel = getTheme ())
		{
			// collect standard icons
			if(ISkinModel* subModel = skinModel->getSubModel (ThemeNames::kStandard))
				if(IContainer* imagesContainer = subModel->getContainerForType (ISkinModel::kImagesElement))
				{
					ForEachUnknown (*imagesContainer, unk)
						UnknownPtr<ISkinElement> element (unk);
						UnknownPtr<ISkinImageElement> imageElement (unk);
						if(element && imageElement)
						{
							auto* item = NEW IconItem;
							item->setTitle (String (element->getName ()));
							item->setIcon (imageElement->getImage ());
							icons->addItem (item);
						}
					EndFor
				}

			// collect main scheme colors
			AutoPtr<IColorSchemes> colorSchemes = ccl_new<IColorSchemes> (ClassID::ColorSchemes);
			IColorScheme* mainScheme = colorSchemes->getScheme (ThemeNames::kMain);
			ASSERT (mainScheme)

			if(ISkinElement* mainSchemeElement = SkinModelAccessor (*skinModel).findResource (ThemeNames::kMain, TAG_COLORSCHEME))
			{
				if(UnknownPtr<IContainer> c = mainSchemeElement)
					ForEachUnknown (*c, unk)
						if(UnknownPtr<ISkinElement> colorElement = unk)
						{
							auto* item = NEW ColorItem;
							item->setColorScheme (mainScheme);
							item->setColorName (colorElement->getName ());
							item->setTitle (String (colorElement->getName ()));

							String comment;
							colorElement->getComment (comment);
							item->getDetails ().set (kCommentID, comment);

							colors->addSorted (item);
						}
					EndFor
			}

			// collect public styles
			if(IContainer* stylesContainer = skinModel->getContainerForType (ISkinModel::kStylesElement))
			{
				ForEachUnknown (*stylesContainer, unk)
					if(UnknownPtr<ISkinElement> styleElement = unk)
					{
						Variant appStyle;
						styleElement->getAttributeValue (appStyle, ATTR_APPSTYLE);
						if(appStyle.parseBool ())
						{
							auto* item = NEW ListViewItem;
							item->setTitle (String (styleElement->getName ()));

							String comment;
							styleElement->getComment (comment);
							item->getDetails ().set (kCommentID, comment);

							styles->addSorted (item);
						}
					}
				EndFor
			}
		}
	}

protected:
	AutoPtr<ListViewModel> icons;
	AutoPtr<ListViewModel> colors;
	AutoPtr<ListViewModel> styles;
};

DEFINE_STRINGID_MEMBER_ (SkinDemo, kCommentID, "comment")
DEFINE_STRINGID_MEMBER_ (SkinDemo, kColorID, "color")

REGISTER_DEMO ("Experimental", "Skin Content", SkinDemo)
