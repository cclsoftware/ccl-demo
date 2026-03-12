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
// Filename    : demoitem.cpp
// Description : Demo Registration
//
//************************************************************************************************

#include "demoitem.h"

#include "ccl/base/storage/url.h"

#include "ccl/public/gui/iparameter.h"
#include "ccl/public/gui/framework/iskinmodel.h"
#include "ccl/public/gui/framework/itheme.h"

namespace CCL {

//************************************************************************************************
// DemoInfoComponent
//************************************************************************************************

class DemoInfoComponent: public Component
{
public:
	DECLARE_CLASS_ABSTRACT (DemoInfoComponent, Component)

	DemoInfoComponent (const DemoPageItem& demoItem);

	static String baseUrlString;
};

} // namespace CCL

using namespace CCL;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Tags
//////////////////////////////////////////////////////////////////////////////////////////////////

namespace Tag
{
	enum DemoInfoTags
	{
		kDemoTitle = 100,
		kDemoSourceFileName,
		kDemoSourceFileLink,
		kDemoSkinFileName,
		kDemoSkinFileLink
	};
}

//************************************************************************************************
// DemoInfoComponent
//************************************************************************************************

DEFINE_CLASS_ABSTRACT_HIDDEN (DemoInfoComponent, Component)
String DemoInfoComponent::baseUrlString;

//////////////////////////////////////////////////////////////////////////////////////////////////

DemoInfoComponent::DemoInfoComponent (const DemoPageItem& demoItem)
: Component (CCLSTR ("DemoInfo"))
{
	paramList.addString ("demoTitle", Tag::kDemoTitle)->fromString (demoItem.getTitle ());

	// determine source file
	Url sourcePath;
	sourcePath.fromDisplayString (demoItem.getSourceFile ());
	String sourceFileName;
	sourcePath.getName (sourceFileName);

	paramList.addString ("demoSourceFileName", Tag::kDemoSourceFileName)->fromString (sourceFileName);

	Url sourceFileLink (baseUrlString);
	sourceFileLink.descend ("source/demos");
	sourceFileLink.descend (sourceFileName);

	paramList.addString ("demoSourceFileLink", Tag::kDemoSourceFileLink)->fromString (UrlFullString (sourceFileLink));

	// determine skin file
	String skinFileName;
	int skinLineNumber = 0;
	if(UnknownPtr<ISkinModel> skinModel = getTheme ())
		if(ISkinElement* formElement = SkinModelAccessor (*skinModel).findForm (demoItem.getFormName ()))
			formElement->getSourceInfo (skinFileName, skinLineNumber);

	paramList.addString ("demoSkinFileName", Tag::kDemoSkinFileName)->fromString (String () << skinFileName << ":" << skinLineNumber);

	Url skinFileLink (baseUrlString);
	skinFileLink.descend ("skin");
	skinFileLink.descend (skinFileName);

	paramList.addString ("demoSkinFileLink", Tag::kDemoSkinFileLink)->fromString (UrlFullString (skinFileLink));
}

//************************************************************************************************
// DemoComponent
//************************************************************************************************

void DemoComponent::setBaseUrl (StringRef urlString)
{
	DemoInfoComponent::baseUrlString = urlString;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_CLASS_HIDDEN (DemoComponent, Component)

//////////////////////////////////////////////////////////////////////////////////////////////////

DemoComponent::DemoComponent ()
{}

//////////////////////////////////////////////////////////////////////////////////////////////////

DemoComponent::~DemoComponent ()
{}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DemoComponent::setDemoItem (const DemoPageItem& demoItem)
{
	addComponent (NEW DemoInfoComponent (demoItem));
}

//************************************************************************************************
// DemoItem
//************************************************************************************************

DEFINE_CLASS_ABSTRACT_HIDDEN (DemoItem, Object)

//************************************************************************************************
// DemoPageItem
//************************************************************************************************

DEFINE_CLASS_ABSTRACT_HIDDEN (DemoPageItem, DemoItem)

//////////////////////////////////////////////////////////////////////////////////////////////////

String DemoPageItem::makeDisplayTitle () const
{
	if(parentCategory)
		return String () << parentCategory->getTitle () << " / " << getTitle ();
	else
		return getTitle ();
}

//************************************************************************************************
// DemoCategory
//************************************************************************************************

DEFINE_CLASS_ABSTRACT_HIDDEN (DemoCategory, DemoItem)

//************************************************************************************************
// DemoRegistry
//************************************************************************************************

DEFINE_SINGLETON (DemoRegistry)
