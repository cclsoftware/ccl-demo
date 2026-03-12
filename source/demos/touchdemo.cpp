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
// Filename    : touchdemo.cpp
// Description : Multi-touch Demo
//
//************************************************************************************************

#include "../demoitem.h"

#include "ccl/base/message.h"
#include "ccl/base/collections/objectlist.h"

#include "ccl/app/controls/usercontrol.h"

#include "ccl/public/gui/graphics/igraphics.h"
#include "ccl/public/gui/framework/itheme.h"

using namespace CCL;

//************************************************************************************************
// TouchDemoView
//************************************************************************************************

class TouchDemoView: public UserControl
{
public:
	TouchDemoView (RectRef size)
	: UserControl (size),
	touchEvent (nullptr)
	{
		gestures.objectCleanup ();
		handlesSwipe (true);
		handlesZoom (true);
	}
	
	~TouchDemoView ()
	{
		cancelSignals ();
	}

	PROPERTY_FLAG (flags, 1<<0, handlesSwipe)
	PROPERTY_FLAG (flags, 1<<1, handlesZoom)

	ITouchHandler* CCL_API createTouchHandler (const TouchEvent& event) override
	{
		class DemoTouchHandler: public UserControl::TouchHandler
		{
		public:
			DemoTouchHandler (TouchDemoView& control)
			: UserControl::TouchHandler (control)
			{
				addRequiredGesture (GestureEvent::kSingleTap, GestureEvent::kPriorityHighest);
				addRequiredGesture (GestureEvent::kDoubleTap, GestureEvent::kPriorityHighest);
				addRequiredGesture (GestureEvent::kLongPress, GestureEvent::kPriorityHighest);
				if(control.handlesSwipe ())
					addRequiredGesture (GestureEvent::kSwipe, GestureEvent::kPriorityHighest);
				if(control.handlesZoom ())
					addRequiredGesture (GestureEvent::kZoom, GestureEvent::kPriorityHighest);
			}
			
			TouchDemoView* getControl () { return static_cast<TouchDemoView*> (UserControl::cast_IView <UserControl> (getView ())); }
			
			bool onHover (const TouchEvent& event) override { updateTouches (event); return true; }
			void onBegin (const TouchEvent& event) override { updateTouches (event); }
			bool onMove (const TouchEvent& event) override { updateTouches (event); return true; }
			void onRelease (const TouchEvent& event, bool canceled) override { updateTouches (event); }
			
			tbool CCL_API onGesture (const GestureEvent& event) override { getControl ()->updateGesture (event, touches);  return false; }

			tbool CCL_API allowsCompetingGesture (int gestureType) override
			{
				return (gestureType == GestureEvent::kZoom && !getControl ()->handlesZoom ())
					|| (gestureType == GestureEvent::kSwipe && !getControl ()->handlesSwipe ());
			}
			
		private:
			Vector<TouchInfo> touches;
			
			void updateTouches (const TouchEvent& event)
			{
				// collect touches (for knowing which touches participate in gesture)
				for(int i = 0, num = event.touches.getTouchCount (); i < num; i++)
				{
					const TouchInfo& touch = event.touches.getTouchInfo (i);
					touches.remove (touch);
					if(touch.type != TouchEvent::kEnd)
						touches.add (touch);
				}
				getControl ()->updateTouches (event);
			}
		};
		
		return NEW DemoTouchHandler (*this);
	}
	
	void draw (const DrawEvent& event) override
	{
		IGraphics& graphics (event.graphics);
		AntiAliasSetter smoother (graphics);
		
		Rect client;
		getClientRect (client);
		if(getParent ()->getChildren ().getFirstView () == *this)
			graphics.fillRect (client, SolidBrush (Colors::kWhite));
		
		static Color touchColors[] = { Colors::kBlue, Colors::kGreen, Colors::kYellow, Colors::kRed };
		static Font font (getTheme ().getStatics ().getStandardFont ().getFace (), 12, Font::kNormal, Font::kAntiAlias);
		SolidBrush textBrush (Colors::kBlack);
		enum { kSymbolSize = 40 };
		
		if(touchEvent)
		{
			VectorForEachFast (touches, TouchInfo, touch)
			String title, info;
			Color color;
			Coord symbolSize = 40;
			
			info << "ID: "; info.appendIntValue ((int64)touch.id) << ENDLINE;
			info << "Position: " << touch.where.x << ", " << touch.where.y << ENDLINE;
			
			if(touchEvent->inputDevice == TouchEvent::kPenInput)
			{
				title << "Pen ";
				color = Colors::kBlack;
				symbolSize = Coord(symbolSize * 0.5f * (1.f + touchEvent->penInfo.pressure));
				
				info << "Pressure: " << String ().appendFloatValue (touchEvent->penInfo.pressure * 100, 1) << " %" << ENDLINE;
				info << "tiltX: " << touchEvent->penInfo.tiltX << ENDLINE;
				info << "tiltY: " << touchEvent->penInfo.tiltY << ENDLINE;
				info << "twist: " << touchEvent->penInfo.twist << ENDLINE;
				
				if(touchEvent->keys.isSet (KeyState::kPenBarrel))
					info << "Barrel Button pressed " << ENDLINE;
				if(touchEvent->keys.isSet (KeyState::kPenEraser))
					info << "Eraser Button pressed " << ENDLINE;
			}
			else if(touchEvent->inputDevice == TouchEvent::kTouchInput)
			{
				title = "Touch";
				color = touchColors[touch.id % ARRAY_COUNT (touchColors)];
			}
			else
			{
				title = "Pointer";
				color = Colors::kGray;
			}
			
			Point p (touch.where);
			windowToClient (p);
			Rect r (p, p);
			r.expand (Coord (symbolSize / 2.));
			if(touch.type >= TouchEvent::kEnter)
				graphics.drawEllipse (r, Pen (color, 3));
			else
				graphics.fillEllipse (r, SolidBrush (color));
			
			r = Rect (r.right + 10, r.top, Point (400, 400));
			graphics.drawString (r, title, font, textBrush, Alignment::kLeftTop);
			
			r.offset (0, Coord (font.getSize ()) + 2);
			graphics.drawText (r, info, font, textBrush, TextFormat (Alignment::kLeftTop));
			
			EndFor
		}
		
		for(auto gesture : iterate_as<Gesture> (gestures))
		{
			GestureEvent& event (gesture->event);
			
			Coord symbolSize = 20;
			
			Point p (pointFToInt (event.whereF));
			windowToClient (p);
			Rect r (p, p);
			r.expand (Coord (symbolSize / 2.));
			
			graphics.fillEllipse (r, SolidBrush (Colors::kGray));
			
			String info;
			switch(event.getType ())
			{
				case GestureEvent::kSwipe :		info << "Swipe"; break;
				case GestureEvent::kZoom :		info << "Zoom"   << ENDLINE << "Amount: " << String ().appendFloatValue (event.amountX, 3) << ", " << String ().appendFloatValue (event.amountY, 3); break;
				case GestureEvent::kRotate :	info << "Rotate" << ENDLINE << "Amount: " << String ().appendFloatValue (event.amountX, 3) << ", " << String ().appendFloatValue (event.amountY, 3); break;
				case GestureEvent::kLongPress : info << "Long Press"; break;
				case GestureEvent::kSingleTap :	info << "Tap"; break;
				case GestureEvent::kDoubleTap :	info << "Double Tap"; break;
			}
			Rect textSize;
			graphics.measureString (textSize, info, font);
			r = Rect (r.left - textSize.getWidth () - 10, r.top, Point (400, 400));
			graphics.drawText (r, info, font, textBrush, TextFormat (Alignment::kLeftTop));
		}
	}
	
	void updateTouches (const TouchEvent& event)
	{
		delete touchEvent;
		if(event.eventType == TouchEvent::kEnd && event.touches.getTouchCount () <= 1)
		{
			touchEvent = nullptr;
			touches.removeAll ();
		}
		else
		{
			touchEvent = NEW TouchEvent (event);
			
			for(int i = 0, num = event.touches.getTouchCount (); i < num; i++)
			{
				const TouchInfo& touch (event.touches.getTouchInfo (i));
				touches.remove (touch);
				
				if(touch.type != TouchEvent::kEnd && touch.type != TouchEvent::kLeave)
					touches.add (event.touches.getTouchInfo (i));
			}
		}
		
		// remove gesture with no more touches
		for(auto gesture : iterate_as<Gesture> (gestures))
		{
			if(!gesture->containsAnyTouch (touches))
			{
				if(gesture->event.getType () == GestureEvent::kSingleTap || gesture->event.getType () == GestureEvent::kDoubleTap)
					(NEW Message ("cleanupGestures"))->post (this, 100);
				else if(gestures.remove (gesture))
					gesture->release ();
			}
		}
		
		invalidate ();
	}
	
	void updateGesture (const GestureEvent& event, const Vector<TouchInfo>& touches)
	{
		Gesture* gesture = findGesture (event, touches);
		
		if(event.getState () == GestureEvent::kEnd)
		{
			if(gesture && gestures.remove (gesture))
				gesture->release ();
		}
		else if(event.getState () != GestureEvent::kPossible)
		{
			if(gesture)
				*gesture = Gesture (event, touches);
			else
				gestures.add (NEW Gesture (event, touches));
		}
		invalidate ();
	}
	
	void CCL_API notify (ISubject* s, MessageRef msg) override
	{
		if(msg == "cleanupGestures")
		{
			for(auto gesture : iterate_as<Gesture> (gestures))
				if(!gesture->containsAnyTouch (touches))
					if(gestures.remove (gesture))
						gesture->release ();
			invalidate ();
			
			if(!gestures.isEmpty ())
				(NEW Message ("cleanupGestures"))->post (this, 100);
		}
		SuperClass::notify (s, msg);
	}
	
private:
	Vector<TouchInfo> touches;
	TouchEvent* touchEvent;
	ObjectList gestures;
	int flags = 0;
	
	struct Gesture: public Object
	{
		GestureEvent event;
		Vector<TouchInfo> touches;
		
		Gesture (const GestureEvent& event, const Vector<TouchInfo>& touches)
		: event (event), touches (touches)
		{}
		
		bool containsAnyTouch (const Vector<TouchInfo>& otherTouches)
		{
			for(auto& t : otherTouches)
				if(touches.contains (t))
					return true;
			
			return false;
		}
	};
	
	Gesture* findGesture (const GestureEvent& event, const Vector<TouchInfo>& touches)
	{
		for(auto gesture : iterate_as<Gesture> (gestures))
			if(gesture->event.getType () == event.getType () && gesture->containsAnyTouch (touches)) // at least one touch id must match
				return gesture;
		
		return nullptr;
	}
};

//************************************************************************************************
// TouchDemo
//************************************************************************************************

class TouchDemo: public DemoComponent
{
public:
	TouchDemo ()
	{
		paramList.addParam ("multipleViews");
	}
	
	// Component
	IView* CCL_API createView (StringID name, VariantRef data, const Rect& bounds) override
	{
		if(name.startsWith ("TouchDemo"))
		{
			auto view = NEW TouchDemoView (bounds);
			int64 index = 0;
			if(name.subString (9).getIntValue (index))
			{
				if(index == 1)
					view->handlesSwipe (false);
				else if(index == 2)
					view->handlesZoom (false);
			}
			return *view;
		}
		return nullptr;
	}
	
protected:
};

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("System", "Multi-touch", TouchDemo)
