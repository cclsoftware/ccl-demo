//************************************************************************************************
//
// CCL Demo Application
//
// This file is part of Crystal Class Library (R)
// Copyright (c) 2026 CCL Software Licensing GmbH.
// All Rights Reserved.
//
// Licensed for use under either:
//  1. a Commercial License provided by CCL Software Licensing GmbH, or
//  2. GNU Affero General Public License v3.0 (AGPLv3).
// 
// You must choose and comply with one of the above licensing options.
// For more information, please visit ccl.dev.
//
// Filename    : blockviewdemo.cpp
// Description : BlockView Demo
//
//************************************************************************************************

#include "../demoitem.h"

#include "ccl/base/storage/textfile.h"

#include "ccl/public/gui/framework/iblockcontent.h"
#include "ccl/public/gui/iparameter.h"
#include "ccl/public/plugservices.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
// Tags
//////////////////////////////////////////////////////////////////////////////////////////////////

namespace Tag
{
	enum BlockViewDemoTags
	{
		kMarkdownSource = 100
	};
}

using namespace CCL;

//************************************************************************************************
// BlockViewDemo
//************************************************************************************************

class BlockViewDemo: public DemoComponent
{
public:
	BlockViewDemo ()
	: blockContent (ccl_new<IBlockContentRoot> (ClassID::BlockContentRoot))
	{
		addObject ("blockContent", blockContent);
		paramList.addString (CSTR ("markdownSource"), Tag::kMarkdownSource);

		ResourceUrl markdownUrl ("markdowndemo.md");
		setMarkdownSource (TextUtils::loadRawString (markdownUrl));
	}

	// DemoComponent
	tbool CCL_API paramChanged (IParameter* param)
	{
		if(param->getTag () == Tag::kMarkdownSource)
			setMarkdownSource (param->getValue ().toString ());

		return true;
	}

private:
	CCL::AutoPtr<CCL::IBlockContentRoot> blockContent;

	void setMarkdownSource (StringRef source)
	{
		IBlockContentNode* newNode = nullptr;
		AutoPtr<IBlockContentBuilder> builder (blockContent->createBuilder ());
		builder->loadContent (newNode, source, kMarkdown);

		blockContent->getChildren ().removeNodes ();
		if(newNode)
			blockContent->getChildren ().insertNode (newNode);

		paramList.byTag (Tag::kMarkdownSource)->fromString (source);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("Controls", "BlockView", BlockViewDemo, DEMO_PAGE_DEFAULT)
