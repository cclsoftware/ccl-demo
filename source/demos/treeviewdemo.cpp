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
// Filename    : treeviewdemo.cpp
// Description : Scalable TreeView Demo
//
//************************************************************************************************

#include "../demoitem.h"

#include "ccl/base/collections/objectlist.h"

#include "ccl/public/collections/unknownlist.h"

#include "ccl/public/gui/iparameter.h"
#include "ccl/public/gui/framework/iitemmodel.h"
#include "ccl/public/gui/framework/viewbox.h"
#include "ccl/public/plugservices.h"

using namespace CCL;

//************************************************************************************************
// DemoTreeModel
//************************************************************************************************

class DemoTreeModel: public Object,
					 public AbstractItemModel
{
public:
	DemoTreeModel ()
	: root (NEW ObjectNode (CCLSTR ("root")))
	{
		root->addChild (NEW ObjectNode (CCLSTR ("aaa")));
		root->addChild (NEW ObjectNode (CCLSTR ("bbb")));
		root->addChild (NEW ObjectNode (CCLSTR ("ccc")));
	}

	// IItemModel
	tbool CCL_API getRootItem (ItemIndex& index) override
	{
		index = ItemIndex (ccl_as_unknown (root));
		return true;
	}

	tbool CCL_API canExpandItem (ItemIndexRef index) override
	{
		auto* node = unknown_cast<ObjectNode> (index.getObject ());
		return node && node->countChildren () > 0;
	}

	tbool CCL_API getSubItems (IUnknownList& items, ItemIndexRef index) override
	{
		auto* node = unknown_cast<ObjectNode> (index.getObject ());
		if(!node)
			return false;

		AutoPtr<Iterator> iterator (node->newIterator ());
		for(auto child : *iterator)
			items.add (child->asUnknown (), true);

		return true;
	}

	tbool CCL_API getItemTitle (String& title, ItemIndexRef index) override
	{
		auto* node = unknown_cast<ObjectNode> (index.getObject ());
		if(!node)
			return false;

		title = node->getName ();
		return true;
	}

	CLASS_INTERFACE (IItemModel, Object)

protected:
	AutoPtr<ObjectNode> root;
};

//************************************************************************************************
// TreeViewDemo
//************************************************************************************************

class TreeViewDemo: public DemoComponent
{
public:
	TreeViewDemo ()
	: treeModel (NEW DemoTreeModel)
	{
		IParameter* zoomParam = paramList.addFloat (0.2f, 20.f, CSTR ("zoom"), 'zoom');
		zoomParam->setValue (1.);
	}

	// Component
	IView* CCL_API createView (StringID name, VariantRef data, const Rect& bounds) override
	{
		if(name == "TreeView" && !treeControl)
		{
			treeControl = ccl_new<IView> (ClassID::TreeControl);
			ViewBox (treeControl).setSize (bounds);
			UnknownPtr<IItemView> (treeControl)->setModel (treeModel);
			return treeControl;
		}
		return nullptr;
	}

	tbool CCL_API paramChanged (IParameter* param) override
	{
		if(param->getTag () == 'zoom')
		{
			float zoom = param->getValue ();
			if(treeControl)
				treeControl->setZoomFactor (zoom);
		}
		return true;
	}

protected:
	AutoPtr<DemoTreeModel> treeModel;
	ObservedPtr<IView> treeControl;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("Controls", "TreeView", TreeViewDemo)
