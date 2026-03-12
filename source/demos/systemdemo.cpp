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
// Filename    : systeminfodemo.cpp
// Description : System Info Demo
//
//************************************************************************************************

#include "../demoitem.h"

#include "ccl/app/controls/listviewmodel.h"

#include "ccl/base/storage/url.h"
#include "ccl/base/storage/settings.h"

#include "ccl/public/system/formatter.h"
#include "ccl/public/system/isysteminfo.h"
#include "ccl/public/system/iexecutable.h"
#include "ccl/public/system/ilocalemanager.h"
#include "ccl/public/gui/framework/iwindow.h"
#include "ccl/public/gui/framework/iuserinterface.h"
#include "ccl/public/gui/graphics/dpiscale.h"
#include "ccl/public/guiservices.h"
#include "ccl/public/systemservices.h"

using namespace CCL;

//************************************************************************************************
// SystemDemo
//************************************************************************************************

class SystemDemo: public DemoComponent
{
public:
	SystemDemo ()
	{
		listModel = NEW ListViewModel;
		listModel->getColumns ().addColumn (200, 0, "label", 160);
		listModel->getColumns ().addColumn (300, 0, "value", 20);
		addObject ("list", listModel);

		Url path;

		AutoPtr<IExecutableImage> image = System::GetExecutableLoader ().createImage (System::GetCurrentModuleRef ());
		if(image)
			image->getPath (path);
		addInfo ("Executable Path",  path);

		String computerName, userName;
		System::GetSystem ().getComputerName (computerName);	addInfo ("ComputerName", computerName);
		System::GetSystem ().getUserName (userName);			addInfo ("UserName", userName);

		addInfo ("SytemRegion", String () << System::GetLocaleManager ().getSystemRegion ());
		addInfo ("SytemLanguage", String () << System::GetLocaleManager ().getSystemLanguage ());
		addInfo ("MeasureSystem", String () << System::GetLocaleManager ().getMeasureSystem ());
		
		WaitCursor waitCursor (System::GetGUI ());

		// not displayed but trigger call
		AutoPtr<IUnknownIterator> regionIterator = System::GetLocaleManager ().createGeographicRegionIterator ();

		Attributes computerInfo;
		System::GetSystem ().getComputerInfo (computerInfo, System::kQueryExtendedComputerInfo);
		ForEachAttribute (computerInfo, key, value)
			addInfo (String () << key, value.toString ());
		EndFor

		addInfo ("SystemFolder",				System::kSystemFolder);
		addInfo ("ProgramsFolder",				System::kProgramsFolder);
		addInfo ("SharedDataFolder",			System::kSharedDataFolder);
		addInfo ("SharedSettingsFolder",		System::kSharedSettingsFolder);
		addInfo ("SharedSupportFolder",			System::kSharedSupportFolder);
		addInfo ("TempFolder",					System::kTempFolder);
		addInfo ("DesktopFolder",				System::kDesktopFolder);
		addInfo ("UserSettingsFolder",			System::kUserSettingsFolder);
		addInfo ("UserPreferencesFolder",		System::kUserPreferencesFolder);
		addInfo ("UserDocumentFolder",			System::kUserDocumentFolder);
		addInfo ("UserMusicFolder",				System::kUserMusicFolder);
		addInfo ("UserDownloadsFolder",			System::kUserDownloadsFolder);
		addInfo ("CompanySettingsFolder",		System::kCompanySettingsFolder);
		addInfo ("CompanySupportFolder",		System::kCompanySupportFolder);
		addInfo ("CompanyContentFolder",		System::kCompanyContentFolder);
		addInfo ("UserContentFolder",			System::kUserContentFolder);
		addInfo ("SharedContentFolder",			System::kSharedContentFolder);
		addInfo ("AppSettingsFolder",			System::kAppSettingsFolder);
		addInfo ("AppSettingsPlatformFolder",	System::kAppSettingsPlatformFolder);
		addInfo ("SharedAppSettingsFolder",		System::kSharedAppSettingsFolder);
		addInfo ("AppSupportFolder",			System::kAppSupportFolder);
		addInfo ("AppDeploymentFolder",			System::kAppDeploymentFolder);
		addInfo ("AppPluginsFolder",			System::kAppPluginsFolder);

		addInfo ("Saved Setting", Settings::instance ().getAttributes ("DemoSettings").getString ("demo"));

		Rect size;
		for(int i = 0, numMonitors = System::GetDesktop ().countMonitors (); i < numMonitors; i++)
		{
			String monitor = String ("Monitor ") << i << ": ";
			float scaleFactor = System::GetDesktop ().getMonitorScaleFactor (i);
			addInfo (String (monitor) << "scale factor", String () << scaleFactor);

			for(int workArea = 0; workArea < 2; workArea++)
			{
				System::GetDesktop ().getMonitorSize (size, i, workArea != 0);
				PixelPoint pixelSize (size.getSize (), scaleFactor);

				String title = String (monitor) << (workArea ? "work area" : "total size");
				String value = String () << size.left << ", " << size.top << ", "<< size.right << ", "<< size.bottom
					<< " (" << size.getWidth () << " x " << size.getHeight () << ")";
				if(scaleFactor != 1.f)
					value << " Pixels: " << pixelSize.x << " x " << pixelSize.y;
				addInfo (title, value);
			}
		}
	}

	~SystemDemo ()
	{
		// save a string in settings
		DateTime dateTime;
		System::GetSystem ().getLocalTime (dateTime);
		Settings::instance ().getAttributes ("DemoSettings").set ("demo", Format::DateTime::print (dateTime, Format::DateTime::kFriendlyDateTime));
	}

	void addInfo (StringRef label, StringRef value)
	{
		ListViewItem* item = NEW ListViewItem (label);
		item->getDetails ().set ("label", label);
		item->getDetails ().set ("value", value);
		listModel->addItem (item);
	}

	void addInfo (StringRef label, UrlRef path)
	{
		addInfo (label, path.isEmpty () ? String ("-/-") : UrlDisplayString (path));
	}

	void addInfo (StringRef label, System::FolderType folderType)
	{
		Url path;
		if(System::GetSystem ().getLocation (path, folderType))
			addInfo (label, path);
		else
			addInfo (label, "(Location not available)");
	}

private:
	AutoPtr<ListViewModel> listModel;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("System", "System Info", SystemDemo)

//************************************************************************************************
// AutofillDemo
//************************************************************************************************

class AutofillDemo: public DemoComponent
{
public:
	AutofillDemo ()
	{
		paramList.addString (CSTR ("name"), 'usrn');
		paramList.addString (CSTR ("password"), 'pswd');
		paramList.addString (CSTR ("email"), 'emal');
		paramList.addString (CSTR ("newpassword"), 'npsw');
		paramList.addString (CSTR ("firstname"), 'fstn');
		paramList.addString (CSTR ("lastname"), 'lstn');
		paramList.addString (CSTR ("country"), 'ctry');
		paramList.addParam (CSTR ("newAccountForm"));
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("System", "Autofill", AutofillDemo)
