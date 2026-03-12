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
// Filename    : svgdemo.cpp
// Description : SVG Demo
//
//************************************************************************************************

#include "../demoitem.h"

#include "ccl/base/storage/url.h"
#include "ccl/base/storage/textfile.h"
#include "ccl/base/collections/objectarray.h"

#include "ccl/public/gui/idatatarget.h"
#include "ccl/public/gui/iparameter.h"
#include "ccl/public/gui/commanddispatch.h"

#include "ccl/public/gui/graphics/iimage.h"
#include "ccl/public/gui/graphics/graphicsfactory.h"

#include "ccl/public/gui/framework/viewbox.h"
#include "ccl/public/gui/framework/controlproperties.h"
#include "ccl/public/gui/framework/idragndrop.h"

#include "ccl/public/system/ifileutilities.h"
#include "ccl/public/system/inativefilesystem.h"
#include "ccl/public/systemservices.h"

using namespace CCL;

//************************************************************************************************
// SvgDemo
//************************************************************************************************

class SvgDemo: public DemoComponent,
			   public CommandDispatcher<SvgDemo>,
			   public IDataTarget
{
public:
	SvgDemo ()
	: imageIndex (0)
	{
		if(auto* ft = System::GetFileTypeRegistry ().getFileTypeByExtension (CCLSTR ("svg")))
			svgFileType = *ft;

		imagePaths.objectCleanup (true);
		addObject ("datatarget", this->asUnknown ());

		paramList.addString (CSTR ("filename"));
		paramList.addString (CSTR ("source"))->setReadOnly (true);
		paramList.addParam (CSTR ("hasReference"));
		paramList.addParam (CSTR ("showSource"));

		#if 0
		Url folder (CCLSTR ("resource:///svg"), Url::kFolder);
		scanFolder (folder);
		#else // can't iterate resource folder names on all platforms
		const String folderNames[] =
		{
			String ("basic_shapes"),
			String ("coords"),
			String ("masking"),
			String ("paths"),
			String ("struct"),
			String ("text"),
			String ("various")
		};
		for(int i = 0; i < ARRAY_COUNT (folderNames); i++)
		{
			ResourceUrl folder (CCLSTR ("svg"), Url::kFolder);
			folder.descend (folderNames[i], Url::kFolder);
			scanFolder (folder);
		}
		#endif
	}

	void scanFolder (UrlRef folder)
	{
		IFileIterator* fileIter = System::GetFileSystem ().newIterator (folder, IFileIterator::kAll);
		ForEachFile (fileIter, url)
			if(url->isFolder ())
				scanFolder (*url);
			else
				addImage (*url);
		EndFor
	}

	bool addImage (UrlRef url)
	{
		if(url.getFileType () == svgFileType)
		{
			imagePaths.addOnce (Url (url));
			return true;
		}
		return false;
	}

	void loadImage ()
	{
		imageIndex = ccl_bound (imageIndex, 0, imagePaths.count ());
		if(Url* url = ccl_cast<Url>(imagePaths.at (imageIndex)))
			loadImage (*url);
	}

	void loadImage (UrlRef url)
	{
		// load svg
		AutoPtr<IImage> image1 = GraphicsFactory::loadImageFile (url);
		if(imageView1)
			ViewBox (imageView1).setAttribute (kImageViewBackground, image1);

		// load png
		Url url2 (url);
		url2.setExtension (CCLSTR ("png"));
		AutoPtr<IImage> image2 = GraphicsFactory::loadImageFile (url2);
		if(imageView2)
			ViewBox (imageView2).setAttribute (kImageViewBackground, image2);

		paramList.lookup ("hasReference")->setValue (image2 != nullptr);

		// path
		String urlString;
		url.toDisplayString (urlString);
		paramList.lookup ("filename")->fromString (urlString);

		// svg source code
		String source = TextUtils::loadRawString (url);
		paramList.lookup ("source")->fromString (source);
	}

	bool nextImage (CmdArgs args)
	{
		if(!args.checkOnly ())
		{
			++imageIndex;
			loadImage ();
		}
		return true;
	}

	bool previousImage (CmdArgs args)
	{
		if(!args.checkOnly ())
		{
			--imageIndex;
			loadImage ();
		}
		return true;
	}

	bool reloadImage (CmdArgs args)
	{
		if(!args.checkOnly ())
			loadImage ();
		return true;
	}

	// Component
	IView* CCL_API createView (StringID name, VariantRef data, const Rect& bounds) override
	{
		if(name == "image1" && !imageView1)
		{
			imageView1 = ViewBox (ClassID::ImageView, bounds, StyleFlags (0, Styles::kImageViewAppearanceColorize));
			loadImage ();
			return imageView1;
		}
		else if(name == "image2" && !imageView2)
		{
			imageView2 = ViewBox (ClassID::ImageView, bounds, StyleFlags (0, Styles::kImageViewAppearanceColorize));
			loadImage ();
			return imageView2;
		}
		return nullptr;
	}

	tbool CCL_API canInsertData (const IUnknownList& data, IDragSession* session, IView* targetView, int insertIndex) override
	{
		UnknownPtr<IUrl> url (data.getFirst ());
		if(url)
		{
			session->setResult (IDragSession::kDropCopyReal);
			return true;
		}
		return false;
	}

	tbool CCL_API insertData (const IUnknownList& data, IDragSession* session, int insertIndex) override
	{
		UnknownPtr<IUrl> url (data.getFirst ());
		if(url)
		{
			if(url->isFile ())
			{
				addImage (*url);
				imageIndex = imagePaths.index (Url (*url));
			}
			else
			{
				scanFolder (*url);
				imageIndex = imagePaths.count () - 1;
			}

			loadImage ();
			return true;
		}
		return false;
	}

	DECLARE_COMMANDS (SvgDemo)
	DECLARE_COMMAND_CATEGORY2 ("Navigation", "SVG", Component)

	CLASS_INTERFACE (IDataTarget, DemoComponent)

protected:
	FileType svgFileType;
	int imageIndex;
	ObjectArray imagePaths;
	ObservedPtr<IView> imageView1;
	ObservedPtr<IView> imageView2;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_COMMANDS (SvgDemo)
	DEFINE_COMMAND ("Navigation", "Right", SvgDemo::nextImage)
	DEFINE_COMMAND ("Navigation", "Left", SvgDemo::previousImage)
	DEFINE_COMMAND ("SVG", "Next Image", SvgDemo::nextImage)
	DEFINE_COMMAND ("SVG", "Previous Image", SvgDemo::previousImage)
	DEFINE_COMMAND ("SVG", "Reload Image", SvgDemo::reloadImage)
END_COMMANDS (SvgDemo)

IMPLEMENT_COMMANDS (SvgDemo, DemoComponent)

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("Graphics", "SVG", SvgDemo)
