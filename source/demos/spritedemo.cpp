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
// Filename    : spritedemo.cpp
// Description : Sprite Demo
//
//************************************************************************************************

#include "../demoitem.h"

#include "ccl/app/controls/usercontrol.h"

#include "ccl/public/gui/framework/idleclient.h"
#include "ccl/public/gui/graphics/igraphics.h"
#include "ccl/public/plugservices.h"

using namespace CCL;

//************************************************************************************************
// SpriteDemoView
//************************************************************************************************

class SpriteDemoView: public UserControl,
					  public IdleClient
{
public:
	SpriteDemoView (RectRef size);

	// UserControl
	void draw (const DrawEvent& event) override;
	void attached (IView* parent) override;
	void removed (IView* parent) override;

	// IdleClient
	void onIdleTimer () override;

	CLASS_INTERFACE (IdleClient, UserControl)

protected:
	using SuperClass = UserControl;
	AutoPtr<ISprite> sprite;
	Point direction;
	Point grow;

	static constexpr int kMinSize = 50;
	static constexpr int kMaxSize = 1200;
};

//************************************************************************************************
// SpriteDemoView
//************************************************************************************************

SpriteDemoView::SpriteDemoView (RectRef size)
: UserControl (size),
  direction (2, 2),
  grow (1, 1)
{}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SpriteDemoView::attached (IView* parent)
{
	UID cid (ClassID::FloatingSprite);
	//UID cid (ClassID::SublayerSprite); // todo: choose sprite class

	AutoPtr<IDrawable> drawable = NEW SolidDrawable (Color (Colors::kBlue).setAlphaF (0.4f));

	sprite = ccl_new<ISprite> (cid);

	Rect rect (0, 0, 100, 100);
	sprite->construct (*this, rect, drawable);
	sprite->show ();

	startTimer (1);

	SuperClass::attached (parent);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SpriteDemoView::removed (IView* parent)
{
	stopTimer ();

	if(sprite)
		sprite->hide ();

	SuperClass::attached (parent);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SpriteDemoView::onIdleTimer ()
{
	if(sprite)
	{
		Rect client;
		getClientRect (client);

		Rect rect (sprite->getSize ());

		// resize
		rect.setSize (rect.getSize () + grow);
		if(rect.getWidth () > kMaxSize || rect.getWidth () < kMinSize)
			grow.x *= -1;
		if(rect.getHeight () > kMaxSize || rect.getHeight () < kMinSize)
			grow.y *= -1;
		rect.right = ccl_bound (rect.right, rect.left + kMinSize, rect.left + kMaxSize);
		rect.bottom = ccl_bound (rect.bottom, rect.top + kMinSize, rect.top + kMaxSize);

		// move
		rect.offset (direction);
		if(rect.right > client.right)
		{
			rect.offset (client.right - rect.right);
			direction.x *= -1;
		}
		else if(rect.left < client.left)
		{
			rect.offset (client.left - rect.left);
			direction.x *= -1;
		}

		if(rect.bottom > client.bottom)
		{
			rect.offset (0, client.bottom - rect.bottom);
			direction.y *= -1;
		}
		else if(rect.top < client.top)
		{
			rect.offset (0, client.top - rect.top);
			direction.y *= -1;
		}
		sprite->move (rect);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SpriteDemoView::draw (const DrawEvent& event)
{
	SuperClass::draw (event);
}

//************************************************************************************************
// SpriteDemo
//************************************************************************************************

class SpriteDemo: public DemoComponent
{
public:
	// Component
	IView* CCL_API createView (StringID name, VariantRef data, const Rect& bounds) override
	{
		if(name == "SpriteDemo")
			return *NEW SpriteDemoView (bounds);
		return nullptr;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("Graphics", "Sprites", SpriteDemo)
