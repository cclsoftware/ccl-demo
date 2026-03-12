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
// Filename    : embeddeddemo.cpp
// Description : Embedded Graphics Demo
//
//************************************************************************************************

#define DEBUG_LOG 1
#define USE_RGB565_BITMAP_FORMAT 1 // use RGB565 instead of RGBA

#include "../demoitem.h"

#include "ccl/app/controls/usercontrol.h"

#include "ccl/public/gui/framework/idleclient.h"
#include "ccl/public/gui/framework/ialert.h"
#include "ccl/public/gui/graphics/graphicsfactory.h"
#include "ccl/public/gui/graphics/igraphics.h"
#include "ccl/public/gui/framework/iembeddedviewhost.h"

#undef DEFINE_SINGLETON // avoid core/ccl macro conflict
#include "ccl/extras/portable/resourcepackage.h"
#include "coreviewdemo.h"

using namespace CCL;

//************************************************************************************************
// EmbeddedGraphicsView
//************************************************************************************************

class EmbeddedGraphicsView: public CCL::UserControl,
							public IEmbeddedViewHost,
							public IdleClient
{
public:
	DECLARE_CLASS_ABSTRACT (EmbeddedGraphicsView, UserControl)

	EmbeddedGraphicsView (RectRef size, CStringPtr pageName, bool isMonochrome = false);
	~EmbeddedGraphicsView ();

	Core::Portable::RootView* getCoreView () const { return coreView; }

	// UserControl
	void attached (IView* parent) override;
	void removed (IView* parent) override;
	void draw (const DrawEvent& event) override;
	IMouseHandler* CCL_API createMouseHandler (const MouseEvent& event) override;
	bool onMouseWheel (const MouseWheelEvent& event) override;
	bool onKeyDown (const KeyEvent& event) override;

	// IEmbeddedViewHost
	tbool CCL_API getViewProperty (ViewProperty& value, ViewRef view) const override;
	int CCL_API getSubViewCount (ViewRef parent) const override;
	ViewRef CCL_API getSubViewAt (ViewRef parent, int index) const override;

	CLASS_INTERFACE2 (ITimerTask, IEmbeddedViewHost, UserControl)

protected:
	AutoPtr<IImage> cclBitmap;
	CoreComponentDemo* coreComponent;
	Core::Portable::RootView* coreView;
	bool isMonochrome;

	class InputHandler: public UserControl::MouseHandler
	{
	public:
		InputHandler (EmbeddedGraphicsView& view)
		: MouseHandler (&view),
		  coreView (view.getCoreView ())
		{}

		void onBegin () override
		{
			Core::Portable::TouchEvent e (Core::Portable::TouchEvent::kDown, current.where);
			coreView->receiveTouchInput (e);
		}

		bool onMove (int moveFlags) override
		{
			Core::Portable::TouchEvent e (Core::Portable::TouchEvent::kMove, current.where);
			coreView->receiveTouchInput (e);
			return true;
		}

		void onRelease (bool canceled) override
		{
			Core::Portable::TouchEvent e (Core::Portable::TouchEvent::kUp, current.where);
			coreView->receiveTouchInput (e);
		}

	protected:
		Core::Portable::RootView* coreView;
	};

	Core::Portable::View* resolveView (ViewRef view) const;

	// IdleClient
	void onIdleTimer () override;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

CCL_KERNEL_TERM_LEVEL (CleanupEmbeddedGraphics, kFirstRun)
{
	auto& viewBuilder = Core::Portable::ViewBuilder::instance ();
	viewBuilder.removeAll ();
}

//************************************************************************************************
// EmbeddedGraphicsView
//************************************************************************************************

DEFINE_CLASS_ABSTRACT_HIDDEN (EmbeddedGraphicsView, UserControl)

//////////////////////////////////////////////////////////////////////////////////////////////////

EmbeddedGraphicsView::EmbeddedGraphicsView (RectRef size, CStringPtr pageName, bool isMonochrome)
: UserControl (size),
  coreComponent (nullptr),
  coreView (nullptr),
  isMonochrome (isMonochrome)
{
	cclBitmap = GraphicsFactory::createBitmap (size.getWidth (), size.getHeight (), IBitmap::kRGBAlpha);

	auto& bitmapManager = Core::Portable::BitmapManager::instance ();
	auto& fontManager = Core::Portable::FontManager::instance ();	
	auto& viewBuilder = Core::Portable::ViewBuilder::instance ();

	static bool resourcesLoaded = false;
	if(resourcesLoaded == false)
	{
		resourcesLoaded = true;

		#if USE_RGB565_BITMAP_FORMAT
		bitmapManager.setDefaultFormat (Core::kBitmapRGB565);
		#endif

		bool delayDecoding = true;
		static ResourcePackage package ("embedded"); // package must exist longer for delayed decoding!
		bitmapManager.loadBitmaps (package, delayDecoding);
		fontManager.loadFonts (package);
		viewBuilder.loadViews (package);
	}

	coreComponent = NEW CoreComponentDemo;

	Rect offscreenSize (0, 0, size.getWidth (), size.getHeight ());
	Core::BitmapPixelFormat colorPixelFormat = USE_RGB565_BITMAP_FORMAT ? Core::kBitmapRGB565 : Core::kBitmapRGBAlpha;
	Core::Portable::RootView::RenderMode renderMode = Core::Portable::RootView::kOffscreenMode;
	coreView = NEW Core::Portable::RootView (offscreenSize, isMonochrome ? Core::kBitmapMonochrome : colorPixelFormat, renderMode);
	viewBuilder.buildView (*coreView, pageName, coreComponent);
	coreView->findFirstFocusView ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

EmbeddedGraphicsView::~EmbeddedGraphicsView ()
{
	delete coreView;
	delete coreComponent;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

Core::Portable::View* EmbeddedGraphicsView::resolveView (ViewRef view) const
{
	return view ? reinterpret_cast<Core::Portable::View*> (view) : coreView;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

tbool CCL_API EmbeddedGraphicsView::getViewProperty (ViewProperty& value, ViewRef view) const
{
	Core::IPropertyHandler* propertyhandler = resolveView (view);
	if(propertyhandler)
	{
		propertyhandler->getProperty (value);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

int CCL_API EmbeddedGraphicsView::getSubViewCount (ViewRef parent) const
{
	Core::Portable::View* coreView = resolveView (parent);
	Core::Portable::ContainerView* container = coreView ? coreView->asContainer () : nullptr;
	return container ? container->getChildren ().count () : 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

IEmbeddedViewHost::ViewRef CCL_API EmbeddedGraphicsView::getSubViewAt (ViewRef parent, int index) const
{
	Core::Portable::View* coreView = resolveView (parent);
	Core::Portable::ContainerView* container = coreView ? coreView->asContainer () : nullptr;
	return container ? container->getChildren ().at (index) : 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void EmbeddedGraphicsView::attached (IView* parent)
{
	SuperClass::attached (parent);

	if(isMonochrome == false)
	{
		Core::Portable::AlertBox::setRootView (coreView);
		takeFocus ();
	}

	startTimer ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void EmbeddedGraphicsView::removed (IView* parent)
{
	if(isMonochrome == false)
	{
		Core::Portable::AlertBox::setRootView (nullptr);
	}

	stopTimer ();
	SuperClass::removed (parent);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void EmbeddedGraphicsView::onIdleTimer ()
{
	if(!coreView->getDirtyRegion ().isEmpty ())
	{
		#if (0 && DEBUG_LOG)
		dumpRect (coreView->getDirtyRegion ().getBoundingBox (), "Core view dirty rect");
		#endif

		coreView->redraw ();
		ASSERT (coreView->getDirtyRegion ().isEmpty ())

		if(const BitmapData* srcData = coreView->accessForRead ())
		{
			BitmapDataLocker locker (UnknownPtr<IBitmap> (cclBitmap), IBitmap::kRGBAlpha, IBitmap::kLockWrite);
			if(locker.result == kResultOk)
			{
				// TODO: copy dirty rect only...

				if(srcData->format == Core::kBitmapMonochrome)
				{
					Core::BitmapPrimitivesMonochrome::convertToRGBA (locker.data, *srcData);
				}
				else if(srcData->format == Core::kBitmapRGB565)
				{
					Core::BitmapPrimitives16::convertToRGBA (locker.data, *srcData);
				}
				else
				{
					ASSERT (srcData->format == Core::kBitmapRGBAlpha)
					//Core::BitmapPrimitives32::clear (locker.data);
					Core::BitmapPrimitives32::copyFrom (locker.data, *srcData);
					//Core::BitmapPrimitives32::premultiplyAlpha (locker.data, locker.data);
				}
			}
		}

		updateClient ();
	}
	else
		coreView->onIdle ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void EmbeddedGraphicsView::draw (const DrawEvent& event)
{
	event.graphics.drawImage (cclBitmap, Point ());
}

//////////////////////////////////////////////////////////////////////////////////////////////////

IMouseHandler* CCL_API EmbeddedGraphicsView::createMouseHandler (const MouseEvent& event)
{
	return NEW InputHandler (*this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool EmbeddedGraphicsView::onMouseWheel (const MouseWheelEvent& event)
{
	Core::Portable::WheelEvent::Axis axis = Core::Portable::WheelEvent::kVertical;
	if(event.eventType == MouseWheelEvent::kWheelLeft || event.eventType == MouseWheelEvent::kWheelRight)
		axis = Core::Portable::WheelEvent::kHorizontal;

	Core::Portable::WheelEvent e (event.delta < 0.f ? 1 : -1, axis);
	coreView->onWheelInput (e);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool EmbeddedGraphicsView::onKeyDown (const KeyEvent& event)
{
	if(event.vKey == VKey::kLeft || event.vKey == VKey::kRight)
	{
		bool isNext = event.vKey == VKey::kRight;
		//CCL_PRINTF ("Navigate focus %s\n", isNext ? "next" : "previous")
		Core::Portable::VirtualKeyEvent e (isNext ? Core::Portable::VirtualKeyEvent::kNext : Core::Portable::VirtualKeyEvent::kPrev);
		coreView->onKeyInput (e);
	}
	return true;
}

//************************************************************************************************
// EmbeddedGraphicsDemo
//************************************************************************************************

class EmbeddedGraphicsDemo: public DemoComponent
{
public:
	// Component
	IView* CCL_API createView (StringID name, VariantRef data, const Rect& bounds) override
	{
		if(name == "EmbeddedGraphicsView")
			return *NEW EmbeddedGraphicsView (bounds, "StartPage");
		if(name == "EmbeddedGraphicsViewMonochrome")
			return *NEW EmbeddedGraphicsView (bounds, "MonochromePage", true);
		return nullptr;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("Graphics", "Embedded Graphics", EmbeddedGraphicsDemo)
