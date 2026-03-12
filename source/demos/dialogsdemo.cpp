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
// Filename    : dialogsdemo.cpp
// Description : Dialogs Demo
//
//************************************************************************************************

#include "../demoitem.h"
#include "exampletext.h"

#include "ccl/app/components/fontselector.h"
#include "ccl/app/components/imageselector.h"

#include "ccl/base/storage/textfile.h"
#include "ccl/base/asyncoperation.h"

#include "ccl/public/gui/framework/ifileselector.h"
#include "ccl/public/gui/framework/iprintservice.h"
#include "ccl/public/gui/framework/dialogbox.h"
#include "ccl/public/gui/framework/itheme.h"
#include "ccl/public/gui/framework/ialert.h"
#include "ccl/public/gui/framework/iform.h"
#include "ccl/public/gui/framework/iview.h"
#include "ccl/public/gui/framework/imenu.h"
#include "ccl/public/gui/graphics/igraphics.h"
#include "ccl/public/gui/graphics/iimage.h"
#include "ccl/public/gui/iparameter.h"

#include "ccl/public/math/mathprimitives.h"
#include "ccl/public/storage/iurl.h"
#include "ccl/public/storage/filetype.h"
#include "ccl/public/system/isysteminfo.h"
#include "ccl/public/gui/commanddispatch.h"

#include "ccl/public/plugservices.h"
#include "ccl/public/guiservices.h"
#include "ccl/public/systemservices.h"

using namespace CCL;

//************************************************************************************************
// DialogsDemo
//************************************************************************************************

class DialogsDemo: public DemoComponent
{
public:
	enum Tags
	{
		kYesNo = 100,
		kYesNoCancel,
		kOkCancel,
		kRetryCancel,
		k123,
		kResult,

		kOpenFile,
		kSelectFolder,

		kPageSetup,
		kPrintJob,
		kPrintJobSilent,
		kPrintJobPdf,
		kDefaultPrinter,
		
		kSelectFont,
		kTransparentDialog,
		kTransparentWindow,
		kMenu
	};

	DialogsDemo ()
	{
		addComponent (imageSelector = NEW ImageSelector);
		imageSelector->setImage (getTheme ()->getImage ("SaveTestImage"));

		paramList.addParam ("yesno", kYesNo);
		paramList.addParam ("yesnocancel", kYesNoCancel);
		paramList.addParam ("okcancel", kOkCancel);
		paramList.addParam ("retrycancel", kRetryCancel);
		paramList.addParam ("123", k123);
		paramList.addString ("result", kResult);

		paramList.addParam ("openfile", kOpenFile);
		paramList.addParam ("selectfolder", kSelectFolder);
		
		paramList.addParam ("pagesetup", kPageSetup);
		paramList.addParam ("printjob", kPrintJob);
		paramList.addParam ("printjobsilent", kPrintJobSilent);
		paramList.addParam ("printjobpdf", kPrintJobPdf);
		paramList.addParam ("defaultprinter", kDefaultPrinter);
		
		paramList.addParam ("selectFont", kSelectFont);		
		paramList.addParam ("transparentDialog", kTransparentDialog);
		paramList.addParam ("transparentWindow", kTransparentWindow);

		paramList.addMenu ("menu", kMenu);
	}

	tbool CCL_API paramChanged (IParameter* param) override
	{
		static StringRef question = CCLSTR ("What is the result?");

		switch(param->getTag ())
		{
		case kYesNo :		handleAlert (Alert::askAsync (question, Alert::kYesNo)); break;
		case kYesNoCancel :	handleAlert (Alert::askAsync (question, Alert::kYesNoCancel)); break;
		case kOkCancel :	handleAlert (Alert::askAsync (question, Alert::kOkCancel)); break;
		case kRetryCancel :	handleAlert (Alert::askAsync (question, Alert::kRetryCancel)); break;
		case k123 :			handleAlert (Alert::askAsync (question, "1", "2", "3"), true); break;
		
		case kOpenFile : openFile (); break;
		case kSelectFolder : selectFolder (); break;

		case kPageSetup : pageSetupDialog (); break;
		case kPrintJob : startPrintJob (false); break;
		case kPrintJobSilent : startPrintJob (true); break;
		case kPrintJobPdf : startPrintJob (true, true); break;
		case kDefaultPrinter : showDefaultPrinter (); break;

		case kSelectFont : selectFont (); break;
		case kTransparentDialog : showTransparentDialog (); break;
		case kTransparentWindow : toggleTransparentWindow (); break;
		}
		return true;
	}

	void showResult (StringRef string);
	void showResult (VariantRef result);
	void handleAlert (IAsyncOperation* alertOperation, bool customButtons = false);

	void openFile ();
	void selectFolder ();
	void pageSetupDialog ();
	void startPrintJob (bool silent, bool writePdf = false);
	void showDefaultPrinter ();
	void selectFont ();
	void showTransparentDialog ();
	void toggleTransparentWindow ();

	// DemoComponent
	void CCL_API notify (ISubject* subject, MessageRef msg) override;

private:
	ObservedPtr<IForm> transparentWindowForm;
	ImageSelector* imageSelector;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("System", "Dialogs", DialogsDemo)

//////////////////////////////////////////////////////////////////////////////////////////////////

void DialogsDemo::showResult (StringRef string)
{
	paramList.byTag (kResult)->setValue (string);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DialogsDemo::showResult (VariantRef result)
{
	String s;
	result.toString (s);
	showResult (s);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DialogsDemo::handleAlert (IAsyncOperation* alertOperation, bool customButtons)
{
	Promise (alertOperation).then ([&, customButtons] (IAsyncOperation& operation) 
	{
		#define CHECK_RESULT(code) if(operation.getResult ().asInt () == Alert::code) { showResult (String (#code)); return; }
		if(customButtons)
		{
			CHECK_RESULT (kFirstButton)
			CHECK_RESULT (kSecondButton)
			CHECK_RESULT (kThirdButton)
			CHECK_RESULT (kEscapePressed)
		}
		else
		{
			CHECK_RESULT (kYes)
			CHECK_RESULT (kNo)
			CHECK_RESULT (kOk)
			CHECK_RESULT (kCancel)
			CHECK_RESULT (kRetry)
		}
		#undef CHECK_RESULT

		// fallback
		ASSERT (false)
		showResult (operation.getResult ());
	});
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DialogsDemo::openFile ()
{
	if(AutoPtr<IFileSelector> fileSelector = ccl_new<IFileSelector> (ClassID::FileSelector))
	{
		Promise p = fileSelector->runAsync (IFileSelector::kOpenFile, CCLSTR ("Open File"), 0, nullptr);
		p.then ([&, fileSelector] (IAsyncOperation& operation) 
		{
			const IUrl* path = nullptr;
			if(operation.getResult ().asBool ())
				path = fileSelector->getPath (0);

			String string;
			if(path)
			{
				string = UrlDisplayString (*path);

				String ext;
				if(path->getExtension (ext) && ext == "txt")
				{
					String text = TextUtils::loadString (*path);
					text.replace ("\n", "|");
					Debugger::println (MutableCString (text).str ());
				}
			}
			else
				string = CCLSTR ("-/-");

			showResult (string);
		});
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DialogsDemo::selectFolder ()
{
	if(AutoPtr<IFolderSelector> folderSelector = ccl_new<IFolderSelector> (ClassID::FolderSelector))
	{
		IAsyncOperation* op = folderSelector->runAsync (CCLSTR ("Select Folder"), nullptr);

		Promise (op).then ([&, folderSelector] (IAsyncOperation& operation) 
		{
			String string;
			if(operation.getResult ().asBool ())
				string = UrlDisplayString (folderSelector->getPath ());
			else
				string = CCLSTR ("-/-");

			showResult (string);
		});
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DialogsDemo::pageSetupDialog ()
{
	AutoPtr<IPageSetupDialog> pageSetupDialog = System::GetPrintService ().createPageSetupDialog ();
	if(pageSetupDialog)
	{
		static PageSetup setup;
		if(setup.margins == RectF ())
		{
			setup.orientation = kPageOrientationPortrait;
			setup.margins.left = 10;
			setup.margins.right = 10;		
			setup.size = System::GetPrintService ().getPaperFormat (kPaperFormatA3).size;
		}
		
		if(pageSetupDialog->run (setup))
		{			
			auto& format = System::GetPrintService ().lookupPaperFormatBySize (setup.size);
			if(format.isValid ())
			{
				String formatInfo;
				formatInfo.append (CCLSTR ("Selected Format: ")).append (format.name);			
				Promise p (Alert::infoAsync (formatInfo));
			}
			else
				setup.size (0, 0);
		}
	}
	else
		Promise p (Alert::warnAsync (CCLSTR ("PageSetupDialog not implemented")));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DialogsDemo::showDefaultPrinter ()
{
	String printerInfoString;
	PrinterInfo printerInfo;

	if(System::GetPrintService ().getDefaultPrinterInfo (printerInfo) == kResultOk)
	{
		printerInfoString.append (CCLSTR ("Default Printer: ")).append (printerInfo.name);			
	}
	else
		printerInfoString = CCLSTR ("No Default Printer");			

	Promise p (Alert::infoAsync (printerInfoString));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DialogsDemo::startPrintJob (bool silent, bool writePdf)
{
	AutoPtr<IPrintJob> printJob;
	if(writePdf)
	{
		Url pdfUrl;
		System::GetSystem ().getLocation (pdfUrl, System::kUserDocumentFolder);
		pdfUrl.descend ("CCLDemo.pdf");
		printJob = System::GetPrintService ().createPdfPrintJob (pdfUrl);

		silent = true;
	}
	else
		printJob = System::GetPrintService ().createPrintJob ();

	if(printJob)
	{
		struct PageRenderer: Unknown, 
							 IPageRenderer
		{
			SharedPtr<IImage> testImage;

			PageRenderer (ITheme* theme)
			{
				if(theme)
					testImage = theme->getImage ("PrintTestImage");
			}

			tresult CCL_API renderPage (PageRenderData& data) override
			{
				//if(data.pageNumber > 0)
				//	return kResultAborted; // test
				
				float dpi = Math::dpiFromCoordSizeMillimeter (0.2f); // want one dot/coord to be 0.2 mm wide
				float dpiFactor = data.dpi / dpi;
				if(dpiFactor != 1.f)
				{
					Transform transform;
					transform.scale (dpiFactor, dpiFactor);
					data.graphics.addTransform (transform);
				}
				
				Font font (getStandardFont ());
				font.setSize (32);
				SolidBrush brush;
				Point topLeft ((Coord)Math::millimeterToCoord (data.printableArea.left, dpi), (Coord)Math::millimeterToCoord  (data.printableArea.top, dpi));
								
				Coord maxX = Coord (Math::millimeterToCoord  (data.printableArea.right, dpi)) - 1;

				Point p1 (topLeft);
				Point p2 (p1);
				p2.x = maxX;

				Pen pen;
				for(int i = 0; i < 5; i++)
				{
					data.graphics.drawLine (p1, p2, pen);

					Point p3 (p1.x, p1.y+16);
					data.graphics.drawLine (p1, p3, pen);

					p3 = p2;
					for(int i2 = 0; i2 < 5; i2++)
					{
						Point p4 (p3.x,  p3.y+16 );
						data.graphics.drawLine (p3, p4, pen);
						p3.x -= 32;					
					}

					p1.y += 50;
					p2.y += 50;
				}

				String text (kExampleText);
				data.graphics.drawString (Rect (topLeft.x, topLeft.y).setWidth (maxX).setHeight (60), text, font, brush, Alignment::kLeftTop);
				
				// 2 cm line
				p1.y += 60;
				p2.y += 60;
				p1.x = Coord (Math::millimeterToCoord (data.pageSize.x / 2, dpi));
				p2.x = p1.x + ccl_to_int<Coord> (Math::millimeterToCoord (20.f, dpi));

				data.graphics.drawLine (p1, p2, pen);

				// 20 cm line
				p2.y += ccl_to_int<Coord> (Math::millimeterToCoord (200.f, dpi));
				p2.x = p1.x;
				
				data.graphics.drawLine (p1, p2, pen);

				
				if(testImage)
				{
					p1.y += 60;
					data.graphics.drawImage (testImage, p1);							
				}

				return kResultOk; 
			}

			tresult CCL_API updateStatus (PrintJobStatus status) override
			{
				String text ("Printing ");
				switch(status)
				{
				//case kPrinting: text.append ("started"); break;
				case kFinished: text.append ("finished"); break;
				case kCanceled: text.append ("canceled"); break;
				case kFailed: text.append ("failed"); break;
				default:
					return kResultOk;
				}
				Promise p (Alert::infoAsync (text));
				
				return kResultOk; 
			}
			CLASS_INTERFACE (IPageRenderer, Unknown)
		};

		PrinterDocumentInfo docInfo ("CCL print Test", 0, 3);
		IPrintJob::JobMode jobMode = IPrintJob::kJobModeNormal;
		if(silent)
		{
			jobMode = IPrintJob::kJobModeSilent;
			docInfo.maxPage = 0;
			docInfo.pageSize = System::GetPrintService ().getPaperFormat (kPaperFormatA3).size;
		}

		//docInfo.setOrientation (PageSetup::kPortrait);
		AutoPtr<PageRenderer> renderer = NEW PageRenderer (getTheme ());
		tresult result = printJob->run (docInfo, renderer, jobMode);
	}
	else
		Promise (Alert::warnAsync (CCLSTR ("PrintJob not implemented")));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DialogsDemo::selectFont ()
{
	static Font font;
	
	AutoPtr<FontSelectorComponent> fontSelector (NEW FontSelectorComponent (Font::kCollectAllFonts));
	Promise promise (fontSelector->runDialogAsync (font));
	promise.then ([&] (IAsyncOperation& operation)
	{
		if(operation.getResult ())
			fontSelector->getSelectedFont (font);
	});
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DialogsDemo::showTransparentDialog ()
{
	ITheme* theme = getTheme ();
	IView* view = theme ? theme->createView ("TransparentDialog", this->asUnknown ()) : nullptr;
	if(view)
	{
		int windowStyle = Styles::kWindowCombinedStyleDialog;
		if(UnknownPtr<IForm> form = view)
			windowStyle = form->getWindowStyle ().custom;
	
		Promise promise (DialogBox ()->runDialogAsync (view, windowStyle, Styles::kOkayButton | Styles::kCancelButton));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DialogsDemo::toggleTransparentWindow ()
{
	if(transparentWindowForm)
	{
		transparentWindowForm->closeWindow ();
		transparentWindowForm->release ();
		transparentWindowForm = nullptr;
	}
	else
	{

		ITheme* theme = getTheme ();
		transparentWindowForm = theme ? UnknownPtr<IForm> (theme->createView ("TransparentWindow", this->asUnknown ())) : nullptr;
		if(transparentWindowForm)
			transparentWindowForm->openWindow ();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void CCL_API DialogsDemo::notify (ISubject* subject, MessageRef msg)
{
	if(msg == IParameter::kExtendMenu)
	{
		UnknownPtr<IMenu> menu (msg[0]);
		if(menu)
		{
			struct MenuBuilder
			{
				DialogsDemo* demo;
				MenuBuilder (DialogsDemo* demo)
				: demo (demo)
				{}

				void build (IMenu& parentMenu, String parentName = 0, int level = 1)
				{
					constexpr int kNumMenus = 2;
					constexpr int kNumItems = 3;
					constexpr int kMaxLevel = 2;

					if(level < kMaxLevel)
					{
						for(int m = 0; m < kNumMenus; m++)
						{
							String subMenuName (parentName);
							if(!subMenuName.isEmpty ())
								subMenuName << ".";;
							subMenuName << m + 1;

							IMenu* subMenu = parentMenu.createMenu ();
							subMenu->setMenuAttribute (IMenu::kMenuTitle, String ("Menu ") << subMenuName);
							parentMenu.addMenu (subMenu);

							build (*subMenu, subMenuName, level + 1);
						}
						parentMenu.addSeparatorItem ();
					}

					for(int i = 0; i < kNumItems; i++)
					{
						String itemName (parentName);
						if(!itemName.isEmpty ())
							itemName << ".";;
						itemName << i + 1;

						addCommand (parentMenu, String ("Item ") << itemName);
					}
				}

				void addCommand (IMenu& menu, String name)
				{
					DialogsDemo* demo = this->demo;

					menu.addCommandItem (CommandWithTitle ("Test", MutableCString (name), name),
						makeCommandDelegate ([demo] (const CommandMsg& msg, VariantRef data)
						{
							if(!msg.checkOnly ())
								demo->showResult (String (msg.name));
							return true;
						}, nullptr));
				};
			};

			MenuBuilder (this).build (*menu);
		}
	}
	SuperClass::notify (subject, msg);
}
