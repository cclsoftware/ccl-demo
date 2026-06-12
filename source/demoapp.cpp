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
// Filename    : demoapp.cpp
// Description : Demo Application
//
//************************************************************************************************

#include "demoapp.h"
#include "demoitem.h"
#include "appversion.h"

#include "ccl/app/components/eulacomponent.h"
#include "ccl/app/navigation/navigator.h"
#include "ccl/app/options/mainoption.h"
#include "ccl/app/applicationspecifics.h"
#include "ccl/app/params.h"

#include "ccl/base/storage/attributes.h"
#include "ccl/base/development.h"

#include "ccl/public/app/inavigationserver.h"
#include "ccl/public/base/itypelib.h"
#include "ccl/public/gui/framework/ialert.h"
#include "ccl/public/gui/framework/isystemshell.h"
#include "ccl/public/gui/framework/itheme.h"
#include "ccl/public/gui/framework/viewbox.h"
#include "ccl/public/plugins/itypelibregistry.h"
#include "ccl/public/system/ifileutilities.h"
#include "ccl/public/text/istringdict.h"
#include "ccl/public/text/stringbuilder.h"

#include "ccl/public/guiservices.h"
#include "ccl/public/plugservices.h"
#include "ccl/public/systemservices.h"

namespace CCL {

//************************************************************************************************
// GitHubDocs
//************************************************************************************************

namespace GitHubDocs
{
	// build a URL to compare with the URL listed in the XML file
	static String buildComparisonUrl (CStringPtr typeLibraryName, CStringPtr elementName)
	{
		String comparisonUrl (typeLibraryName);
		return comparisonUrl.append ("/").append (elementName).toLowercase ().remove (" ");
	}

	// perform character replacement in names that make up URL
	static String formatName (CStringPtr name)
	{
		String formattedName (name);
		formattedName.toLowercase ();
		formattedName.replace (" ", "-");
		formattedName.replace (".", "-");
		return formattedName;
	}

	// build an HTML URL corresponding to the documentation location
	static String buildHtmlUrl (CStringPtr typeLibraryName, CStringPtr elementName, CStringPtr elementType)
	{
		static const String kHtmlFile = "%(1)-classmodel.html";
		static const String kFragmentationID = "%(1)-classmodel-%(2)-%(3)";

		String typeLibraryNameFormatted = formatName (typeLibraryName);
		String elementNameFormatted = formatName (elementName);

		String htmlUrl = SKINXML_DOCUMENTATION_URL;
		htmlUrl.appendFormat (kHtmlFile, typeLibraryNameFormatted).append ("#"). appendFormat (kFragmentationID, typeLibraryNameFormatted, elementType, elementNameFormatted);

		return htmlUrl;
	}
}

//************************************************************************************************
// DemoResult
//************************************************************************************************

typedef ObjectArray DemoResult;

//************************************************************************************************
// DemoNavigationServer
//************************************************************************************************

class DemoNavigationServer: public Component,
							public INavigationServer,
							public ComponentSingleton<DemoNavigationServer>
{
public:
	DECLARE_CLASS (DemoNavigationServer, Component)

	DemoNavigationServer ();

	// INavigationServer
	tresult CCL_API navigateTo (NavigateArgs& args) override;

	CLASS_INTERFACE (INavigationServer, Component)

protected:
	DemoResult currentResult;

	// IObject
	tbool CCL_API getProperty (Variant& var, MemberID propertyId) const override;
};

//************************************************************************************************
// DocumentationLinkHandler
//************************************************************************************************

class DocumentationLinkHandler: public Object,
								public AbstractFileHandler,
								public Singleton<DocumentationLinkHandler>
{
public:
	DECLARE_CLASS (DocumentationLinkHandler, Object)

	// IFileHandler
	tbool CCL_API openFile (UrlRef path) override;

	CLASS_INTERFACE (IFileHandler, Object)
};

} // namespace CCL

DEFINE_SINGLETON (DocumentationLinkHandler)

using namespace CCL;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Initialization
//////////////////////////////////////////////////////////////////////////////////////////////////

void ccl_app_init ()
{
	NEW DemoApp;

	DemoNavigationServer::instance ();

	Navigator& navigator = Navigator::instance ();
	navigator.setHomeUrl (Url (CCLSTR ("object://" APP_ID "/Demo")));

	DemoComponent::setBaseUrl (CCLDEMO_GITHUB_URL);
}

//************************************************************************************************
// DemoNavigationServer
//************************************************************************************************

DEFINE_CLASS_HIDDEN (DemoNavigationServer, Component)
DEFINE_COMPONENT_SINGLETON (DemoNavigationServer)

//////////////////////////////////////////////////////////////////////////////////////////////////

DemoNavigationServer::DemoNavigationServer ()
: Component (CCLSTR ("Demo"))
{}

//////////////////////////////////////////////////////////////////////////////////////////////////

tresult CCL_API DemoNavigationServer::navigateTo (NavigateArgs& args)
{
	ITheme* theme = getTheme ();
	ASSERT (theme)
	IView* contentView = nullptr;

	StringRef idString = args.url.getParameters ().lookupValue (CCLSTR ("id"));
	const DemoItem* currentItem = DemoRegistry::instance ().findItem (idString);

	if(auto* pageItem = ccl_cast<DemoPageItem> (currentItem))
	{
		// a demo page
		AutoPtr<DemoComponent> demoComponent = pageItem->createComponent ();
		ASSERT (demoComponent)
		if(demoComponent)
		{
			demoComponent->setDemoItem (*pageItem);

			Attributes arguments;
			MutableCString formName = pageItem->getFormName ();
			arguments.set ("demoFormName", formName);
			contentView = theme->createView (pageItem->getPageType (), demoComponent->asUnknown (), &arguments);
		}
	}
	else
	{
		// a category
		currentResult.removeAll ();
		if(auto* categoryItem = ccl_cast<DemoCategory> (currentItem))
		{
			currentResult.addAll (categoryItem->getDemos ());
		}
		else
		{
			const auto& categories = DemoRegistry::instance ().getCategories ();
			#if 1 // nested nagivation
			currentResult.addAll (categories);
			#else // flat navigation
			for(auto* category : iterate_as<DemoCategory> (categories))
				currentResult.addAll (category->getDemos ());
			#endif
		}

		contentView = theme->createView ("DemoIndex", asUnknown ());
	}

	ASSERT (contentView)
	if(!contentView)
		return kResultFalse;

	Rect size (args.contentFrame.getSize ());
	size.moveTo (Point ());
	if(!size.isEmpty ())
		contentView->setSize (size);
	
	args.contentFrame.getChildren ().removeAll ();
	args.contentFrame.getChildren ().add (contentView);
	return kResultOk;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

tbool CCL_API DemoNavigationServer::getProperty (Variant& var, MemberID propertyId) const
{
	MutableCString arrayKey;
	if(propertyId == "resultCount")
	{
		var = currentResult.count ();
		return true;
	}
	else if(propertyId.getBetween (arrayKey, "resultId[", "]"))
	{
		if(auto* item = static_cast<DemoItem*> (currentResult.at (arrayKey.scanInt (-1))))
			var = item->getUniqueID ();
		return true;
	}
	else if(propertyId.getBetween (arrayKey, "resultTitle[", "]"))
	{
		if(auto* item = static_cast<DemoItem*> (currentResult.at (arrayKey.scanInt (-1))))
		{
			String title = item->makeDisplayTitle ();
			var = title;
			var.share ();
		}
		return true;
	}
	else if(propertyId.getBetween (arrayKey, "resultForm[", "]"))
	{
		if(auto* item = static_cast<DemoItem*> (currentResult.at (arrayKey.scanInt (-1))))
		{
			String formName (item->getFormName ());
			var = formName;
			var.share ();
		}
		return true;
	}
	return SuperClass::getProperty (var, propertyId);
}

//************************************************************************************************
// DocumentationLinkHandler
//************************************************************************************************

DEFINE_CLASS_HIDDEN (DocumentationLinkHandler, Object)

//////////////////////////////////////////////////////////////////////////////////////////////////

tbool CCL_API DocumentationLinkHandler::openFile (UrlRef path)
{
	// URL in XML file has the format "documentation:///type-library/element.enum"
	// Examples: "documentation:///skinelement/button", "documentation:///skinelement/button.options"
	// Note that there are three slashes after the protocol.

	static const String kProtocol ("documentation");

	if(path.getProtocol () != kProtocol)
		return false;

	StringRef urlFromXml = path.getPath ();

	// If a class or enum from a particular library type matches the URL in the XML file,
	// it means that the URL in the XML is valid and a corresponding HTML URL should be opened
	IterForEachUnknown (System::GetTypeLibRegistry ().newIterator (), unk)
		UnknownPtr<ITypeLibrary> typeLibrary (unk);
		ASSERT (typeLibrary.isValid ())
		if(!typeLibrary)
			continue;
		CStringPtr libraryName = typeLibrary->getLibraryName ();

		// check classes for match
		IterForEachUnknown (typeLibrary->newTypeIterator (), unk)
			UnknownPtr<ITypeInfo> typeInfo (unk);
			ASSERT (typeInfo.isValid ())
			if(!typeInfo)
				continue;
			CStringPtr className = typeInfo->getClassName ();

			String comparisonUrl = GitHubDocs::buildComparisonUrl (libraryName, className);
			if(urlFromXml == comparisonUrl)
			{
				String htmlUrl = GitHubDocs::buildHtmlUrl (libraryName, className, "class");
				System::GetSystemShell ().openUrl (Url (htmlUrl));
				return true;
			}
		EndFor

		// Check enumerations for match
		IterForEachUnknown (typeLibrary->newEnumIterator (), unk)
			UnknownPtr<IEnumTypeInfo> enumTypeInfo (unk);
			ASSERT (enumTypeInfo.isValid ())
			if(!enumTypeInfo)
				continue;
			CStringPtr enumName = enumTypeInfo->getName ();

			String comparisonUrl = GitHubDocs::buildComparisonUrl (libraryName, enumName);
			if(urlFromXml == comparisonUrl)
			{
				String htmlUrl = GitHubDocs::buildHtmlUrl (libraryName, enumName, "enumeration");
				System::GetSystemShell ().openUrl (Url (htmlUrl));
				return true;
			}
		EndFor
	EndFor

	ASSERT (false)
	return true;
}

//************************************************************************************************
// DemoApp
//************************************************************************************************

DEFINE_CLASS_HIDDEN (DemoApp, Application)

//////////////////////////////////////////////////////////////////////////////////////////////////

DemoApp::DemoApp ()
: Application (APP_ID, /*APP_COMPANY*/nullptr, APP_NAME, APP_PACKAGE_ID),
  appSchemeOption (nullptr)
{
	setBuildInformation (APP_FULL_NAME);

	addApplicationMenu ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool DemoApp::startup ()
{
	if(!SuperClass::startup ())
		return false;

	// init color scheme
	appSchemeOption = UserOption::init<MainColorSchemeOption> ();
	appSchemeOption->addConfigurationSavers ();
	paramList.addAlias ("colorInversion", 'ciap')->setOriginal (appSchemeOption->findParameter ("colorInversion"));

	// load theme
	Url skinFolder;
	GET_DEVELOPMENT_FOLDER_LOCATION (skinFolder, CCL_APPLICATION_BASEDIR, "skin")
	if(!loadTheme (skinFolder))
		return false;

	// scan plugins
	scanPlugIns ();

	// start services
	System::GetServiceManager ().registerNotification (this);
	System::GetServiceManager ().startup ();

	#if CCL_PLATFORM_DESKTOP
	// EULA
	Url eulaFolder;
	GET_DEVELOPMENT_FOLDER_LOCATION (eulaFolder, VENDOR_IDENTITY_DIR, "eula")
	if(!EULAComponent ().startup (&eulaFolder))
		return false;

	DemoRegistry::instance ().finishSorting ();

	// main window
	createWindow ();
	
	// notification icon
	ApplicationSpecifics* specifics = getSpecifics<ApplicationSpecifics> ();
	specifics->enableNotifyIcon (true, false);
	specifics->getNotifyIcon ()->setHandler (this->asUnknown ());
	
	specifics->getNotifyIcon ()->reportEvent (Alert::Event (CCLSTR ("A demonstration for a notification"), static_cast<tresult> (1910), Alert::kInformation));
	#endif

	// support for documentation references from within Skin XML
	System::GetFileTypeRegistry ().registerHandler (&DocumentationLinkHandler::instance ());

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool DemoApp::shutdown ()
{
	System::GetFileTypeRegistry ().unregisterHandler (&DocumentationLinkHandler::instance ());

	// stop services
	System::GetServiceManager ().unregisterNotification (this);
	System::GetServiceManager ().shutdown ();

	return SuperClass::shutdown ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void CCL_API DemoApp::extendMenu (IMenu& menu, StringID name)
{
	if(name == "Appearance" && appSchemeOption)
		appSchemeOption->makeAppearanceMenu (menu);
	else
		SuperClass::extendMenu (menu, name);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

tresult CCL_API DemoApp::appendContextMenu (IContextMenu& contextMenu)
{
	if(contextMenu.getContextID () == INotifyIcon::kContextID)
	{
		contextMenu.addCommandItem ("Quit", "File", "Quit", nullptr);
	}
	return kResultOk;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void CCL_API DemoApp::notify (ISubject* subject, MessageRef msg)
{
	if(msg == INotifyIcon::kIconClicked)
	{
		Alert::info ("Notification icon clicked.");
	}
	else if(msg == INotifyIcon::kIconDoubleClicked)
	{
		Alert::info ("Notification icon double-clicked.");
	}
	else
		SuperClass::notify (subject, msg);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

tresult CCL_API DemoApp::onServiceNotification (const IServiceDescription& description, int eventCode)
{
	if(eventCode == kServiceActivate)
	{
		if(description.getServiceName () == CCL_SPY_NAME)
			return kResultOk;
		else
			return kResultFailed; // ignore other services
	}
	else
		return kResultOk;
}
