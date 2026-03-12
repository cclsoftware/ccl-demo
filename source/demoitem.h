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
// Filename    : demoitem.h
// Description : Demo Registration
//
//************************************************************************************************

#ifndef _demoitem_h
#define _demoitem_h

#include "ccl/base/singleton.h"
#include "ccl/base/collections/objectarray.h"

#include "ccl/app/component.h"

namespace CCL {

class DemoPageItem;
class DemoCategory;

//************************************************************************************************
// How to add a demo page:
// - Implement a component derived from DemoComponent
// - Use macro REGISTER_DEMO (category, title, class) to register the component class
// - Add a <Form> with name "Category.Title" to the skin
//************************************************************************************************

#define REGISTER_DEMO(category, title, Class) \
static RegisterDemoItem<Class> UNIQUE_IDENT (__register##Class) (CCLSTR (category), CCLSTR (title), __FILE__);

//************************************************************************************************
// DemoComponent
//************************************************************************************************

class DemoComponent: public Component
{
public:
	DECLARE_CLASS (DemoComponent, Component)

	DemoComponent ();
	~DemoComponent ();

	static void setBaseUrl (StringRef urlString);

	void setDemoItem (const DemoPageItem& demoItem);
};

//************************************************************************************************
// DemoItem
//************************************************************************************************

class DemoItem: public Object
{
public:
	DECLARE_CLASS_ABSTRACT (DemoItem, Object)

	DemoItem (StringRef title)
	: title (title)
	{}

	PROPERTY_STRING (title, Title)
	PROPERTY_STRING (uniqueId, UniqueID)
	PROPERTY_MUTABLE_CSTRING (formName, FormName)

	virtual String makeDisplayTitle () const { return title; }

	// Object
	bool equals (const Object& obj) const override
	{
		const DemoItem& other = static_cast<const DemoItem&> (obj);
		return getTitle () == other.getTitle ();
	}

	int compare (const Object& obj) const override
	{
		const DemoItem& other = static_cast<const DemoItem&> (obj);
		return getTitle ().compare (other.getTitle ());
	}
};

//************************************************************************************************
// DemoPageItem
//************************************************************************************************

class DemoPageItem: public DemoItem
{
public:
	DECLARE_CLASS_ABSTRACT (DemoPageItem, DemoItem)

	typedef DemoComponent* (*CreateFunction) ();

	DemoPageItem (CreateFunction createFunction, StringID formName, StringRef title, StringRef sourceFile)
	: DemoItem (title),
	  parentCategory (nullptr),
	  createFunction (createFunction),
	  sourceFile (sourceFile)
	{
		setFormName (formName);
	}

	PROPERTY_POINTER (DemoCategory, parentCategory, ParentCategory)
	PROPERTY_STRING (sourceFile, SourceFile)
	
	DemoComponent* createComponent () const
	{
		return createFunction ? createFunction () : nullptr;
	}

	// DemoItem
	String makeDisplayTitle () const override;

protected:
	CreateFunction createFunction;
};

//************************************************************************************************
// DemoCategory
//************************************************************************************************

class DemoCategory: public DemoItem
{
public:
	DECLARE_CLASS_ABSTRACT (DemoCategory, DemoItem)

	DemoCategory (StringRef title)
	: DemoItem (title)
	{
		setFormName (MutableCString (title));

		demos.objectCleanup (true);
	}

	void addDemo (DemoPageItem* item)
	{
		demos.addSorted (item); 
	}

	const ObjectArray& getDemos () const 
	{
		return demos; 
	}

protected:
	ObjectArray demos;
};

//************************************************************************************************
// DemoRegistry
//************************************************************************************************

class DemoRegistry: public Object,
					public Singleton<DemoRegistry>
{
public:
	DemoRegistry ()
	{
		categories.objectCleanup (true);
	}

	void addDemo (StringRef categoryTitle, DemoPageItem* item)
	{		
		auto toID = [] (StringRef title)
		{
			String id (title);
			id.replace (CCLSTR (" "), CCLSTR ("_"));
			id.toLowercase ();
			return id;
		};

		auto* category = findCategory (categoryTitle);
		if(!category)
		{
			String categoryId = toID (categoryTitle);
			category = NEW DemoCategory (categoryTitle);
			category->setUniqueID (categoryId);
			categories.addSorted (category);
		}

		String demoId;
		demoId << category->getUniqueID () << "." << toID (item->getTitle ());
		item->setUniqueID (demoId);
		item->setParentCategory (category);
		category->addDemo (item);
	}

	void finishSorting ()
	{
		// make experimental category last (if any)
		const String kExperimental ("Experimental");
		if(auto* category = findCategory (kExperimental))
		{
			categories.remove (category);
			categories.add (category);
		}
	}

	const ObjectArray& getCategories () const
	{
		return categories; 
	}

	const DemoItem* findItem (StringRef uniqueId) const
	{
		for(auto* category : iterate_as<DemoCategory> (categories))
			if(uniqueId.startsWith (category->getUniqueID ()))
			{
				if(uniqueId == category->getUniqueID ())
					return category;
				else
				{
					for(auto* item : iterate_as<DemoPageItem> (category->getDemos ()))
						if(uniqueId == item->getUniqueID ())
							return item;
				}
			}
		return nullptr;
	}

protected:
	ObjectArray categories;

	DemoCategory* findCategory (StringRef categoryTitle) const
	{
		return static_cast<DemoCategory*> (categories.findEqual (DemoCategory (categoryTitle)));
	}
};

//************************************************************************************************
// RegisterDemoItem
//************************************************************************************************

template <class Class>
class RegisterDemoItem
{
public:
	static DemoComponent* createInstance ()
	{
		return NEW Class;
	}

	RegisterDemoItem (StringRef category, StringRef title, StringRef sourceFile)
	{
		MutableCString formName (category);
		formName.append (".");
		formName.append (title);

		DemoRegistry::instance ().addDemo (category, NEW DemoPageItem (createInstance, formName, title, sourceFile));
	}
};

} // namespace CCL

#endif // _demoitem_h
