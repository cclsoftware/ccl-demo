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
// Filename    : compositiondemo.cpp
// Description : Composition Demo
//
//************************************************************************************************

#define DEBUG_LOG 0

#include "../demoitem.h"

#include "ccl/app/controls/usercontrol.h"

#include "ccl/base/asyncoperation.h"
#include "ccl/base/message.h"

#include "ccl/public/math/mathprimitives.h"

#include "ccl/public/gui/graphics/igraphics.h"
#include "ccl/public/gui/graphics/igraphicslayer.h"
#include "ccl/public/gui/graphics/graphicsfactory.h"
#include "ccl/public/gui/graphics/iuivalue.h"
#include "ccl/public/gui/framework/iskinmodel.h"
#include "ccl/public/gui/framework/skinxmldefs.h"
#include "ccl/public/gui/framework/controlproperties.h"
#include "ccl/public/gui/framework/ianimation.h"
#include "ccl/public/gui/framework/iwindow.h"
#include "ccl/public/gui/framework/itheme.h"
#include "ccl/public/gui/framework/ialert.h"
#include "ccl/public/gui/iparameter.h"

#include "ccl/public/base/itypelib.h"
#include "ccl/public/plugins/itypelibregistry.h"
#include "ccl/public/plugservices.h"
#include "ccl/public/cclversion.h"

using namespace CCL;

//************************************************************************************************
// CompositedView
//************************************************************************************************

class CompositedView: public UserControl,
					  public IGraphicsLayerContent
{
public:
	DECLARE_CLASS_ABSTRACT (CompositedView, UserControl)

	CompositedView (RectRef size, ITimingFunction* timingFunction);
	~CompositedView ();

	// UserControl
	void attached (IView* parent) override;
	void removed (IView* parent) override;
	void CCL_API notify (ISubject* subject, MessageRef msg) override;

	CLASS_INTERFACE (IGraphicsLayerContent, UserControl)

protected:
	SharedPtr<ITimingFunction> timingFunction;
	IGraphicsLayer* rootLayer;
	AutoPtr<IGraphicsLayer> testLayer;
	SharedPtr<IImage> testImage;

	void updateAnimation ();

	// IGraphicsLayerContent
	LayerHint CCL_API getLayerHint () const override { return kGraphicsContentHintDefault; }
	void CCL_API drawLayer (IGraphics& graphics, const UpdateRgn& updateRgn, PointRef offset) override;
};

//************************************************************************************************
// TimingCurveView
//************************************************************************************************

class TimingCurveView: public UserControl
{
public:
	DECLARE_CLASS_ABSTRACT (TimingCurveView, UserControl)

	TimingCurveView (RectRef size, ITimingFunction* timingFunction);
	~TimingCurveView ();
	
	// UserControl
	void draw (const DrawEvent& event) override;
	void CCL_API notify (ISubject* subject, MessageRef msg) override;

protected:
	SharedPtr<ITimingFunction> timingFunction;
};

//************************************************************************************************
// CompositedView
//************************************************************************************************

DEFINE_CLASS_ABSTRACT_HIDDEN (CompositedView, UserControl)

//////////////////////////////////////////////////////////////////////////////////////////////////

CompositedView::CompositedView (RectRef size, ITimingFunction* timingFunction)
: UserControl (size), 
  timingFunction (timingFunction),
  rootLayer (nullptr)
{
	ISubject::addObserver (timingFunction, this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

CompositedView::~CompositedView ()
{
	ISubject::removeObserver (timingFunction, this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void CompositedView::attached (IView* parent)
{
	SuperClass::attached (parent);

	IWindow* window = getWindow ();
	IView* view = UnknownPtr<IView> (window);
	Point parentOffset;
	rootLayer = view->getParentLayer (parentOffset);
	if(rootLayer == nullptr)
	{
		Promise (Alert::infoAsync ("Composition not available on this system!"));
		return;
	}

	testImage = getTheme ().getImage ("CompositionDemo");
	ASSERT (testImage != nullptr)

	Rect clientRect;
	getClientRect (clientRect);

	Point offset;
	clientToWindow (offset);
	Rect bounds (clientRect);
	bounds.offset (offset);
	
	testLayer = GraphicsFactory::createGraphicsLayer (ClassID::GraphicsLayer);
	testLayer->construct (testImage, bounds, 0, window->getContentScaleFactor ()); //IGraphicsLayer::kIgnoreAlpha
	testLayer->setUpdateNeeded ();
	
	updateAnimation ();

	rootLayer->addSublayer (testLayer);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void CompositedView::updateAnimation ()
{
	if(!testLayer)
		return;

	CCL_PRINTLN ("Updating animation...")
	testLayer->removeAnimation (IGraphicsLayer::kTransform);

	// Animation
	AnimationDescription description;
	description.duration = 3.;
	description.timingType = kTimingCubicBezier;
	timingFunction->getControlPoints (description.controlPoints);
	description.repeatCount = IAnimation::kRepeatForever;
	description.options = IAnimation::kAutoReverse;
	
	AutoPtr<ITransformAnimation> transform = ccl_new<ITransformAnimation> (ClassID::TransformAnimation);
	transform->setDescription (description);
	transform->addRotation (0., 360.);
	transform->addScalingX (.5, 1.);
	transform->addScalingY (.5, 1.);

	testLayer->addAnimation (IGraphicsLayer::kTransform, transform);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void CompositedView::removed (IView* parent)
{
	if(!rootLayer)
		return;

	rootLayer->removeSublayer (testLayer);
	testLayer.release ();

	SuperClass::removed (parent);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void CCL_API CompositedView::notify (ISubject* subject, MessageRef msg)
{
	if(msg == kChanged)
		updateAnimation ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void CCL_API CompositedView::drawLayer (IGraphics& graphics, const UpdateRgn& updateRgn, PointRef offset)
{
	Rect src (0, 0, testImage->getWidth (), testImage->getHeight ());
	Rect dst;
	getClientRect (dst);
	dst.offset (offset);
	graphics.drawImage (testImage, src, dst);
}

//************************************************************************************************
// TimingCurveView
//************************************************************************************************

DEFINE_CLASS_ABSTRACT_HIDDEN (TimingCurveView, UserControl)

//////////////////////////////////////////////////////////////////////////////////////////////////

TimingCurveView::TimingCurveView (RectRef size, ITimingFunction* timingFunction)
: UserControl (size),
  timingFunction (timingFunction)
{
	ISubject::addObserver (timingFunction, this);
}
	
//////////////////////////////////////////////////////////////////////////////////////////////////

TimingCurveView::~TimingCurveView ()
{
	ISubject::removeObserver (timingFunction, this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void CCL_API TimingCurveView::notify (ISubject* subject, MessageRef msg)
{
	if(msg == kChanged)
		updateClient ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void TimingCurveView::draw (const DrawEvent& event)
{
	IGraphics& g = event.graphics;
	const IVisualStyle& vs = getVisualStyle ();

	Rect rect;
	getClientRect (rect);
	g.fillRect (rect, vs.getBackBrush ());
	g.drawRect (rect, vs.getForePen ());
	AntiAliasSetter smoother (g);

	Coord width = rect.getWidth ();
	Coord height = rect.getHeight ();

	AutoPtr<IGraphicsPath> path = GraphicsFactory::createPath ();

	const Coord kPixelStep = 2;
	for(Coord x = 0; x < width + kPixelStep; x += kPixelStep)
	{
		double inPosition = (double)x / (double)width;
		double outPosition = timingFunction->getTime (inPosition);

		CoordF y = (CoordF)(height - (outPosition * height));
		PointF p ((CoordF)x, y);
		if(x == 0)
			path->startFigure (p);
		else 
			path->lineTo (p);		
	}

	Pen curvePen (vs.getHiliteColor (), 3.f);
	g.drawPath (path, curvePen);

	AnimationControlPoints controlPoints;
	timingFunction->getControlPoints (controlPoints);

	Point p1;
	p1.x = (Coord)(controlPoints.c1x * width);
	p1.y = height - (Coord)(controlPoints.c1y * height);
	curvePen.setWidth (1.f);
	g.drawLine (Point (0, height), p1, curvePen);

	Point p2;
	p2.x = (Coord)(controlPoints.c2x * width);
	p2.y = height - (Coord)(controlPoints.c2y * height);
	g.drawLine (Point (width, 0), p2, curvePen);

	const Coord kWeight = 6;
	Rect r1 (p1, p1);
	r1.expand (kWeight);
	Rect r2 (p2, p2);
	r2.expand (kWeight);

	SolidBrush pointBrush (vs.getColor ("pointcolor", Colors::kRed));
	g.fillEllipse (r1, pointBrush);
	g.fillEllipse (r2, pointBrush);
}

//************************************************************************************************
// CompositionDemo
//************************************************************************************************

class CompositionDemo: public DemoComponent
{
public:
	CompositionDemo ()
	: timingFunction (ccl_new<ITimingFunction> (ClassID::CubicBezierTimingFunction))
	{
		const double kMin = 0.;//-2.;
		const double kMax = 1.;//2.;
		controlParams[0] = paramList.addFloat (kMin, kMax, "c1x");
		controlParams[1] = paramList.addFloat (kMin, kMax, "c1y");
		controlParams[2] = paramList.addFloat (kMin, kMax, "c2x");
		controlParams[3] = paramList.addFloat (kMin, kMax, "c2y");

		controlParams[0]->setValue (0.42);
		controlParams[1]->setValue (0.);
		controlParams[2]->setValue (0.58);
		controlParams[3]->setValue (1.);
	}

	// Component
	IView* CCL_API createView (StringID name, VariantRef data, const Rect& bounds) override
	{
		if(name == "CompositedView")
			return *NEW CompositedView (bounds, timingFunction);
		if(name == "TimingCurveView")
			return *NEW TimingCurveView (bounds, timingFunction);
		return nullptr;
	}

	tbool CCL_API paramChanged (IParameter* param) override
	{
		AnimationControlPoints values
		(
			controlParams[0]->getValue ().asDouble (), 
			controlParams[1]->getValue ().asDouble (), 
			controlParams[2]->getValue ().asDouble (),
			controlParams[3]->getValue ().asDouble ()
		);

		timingFunction->setControlPoints (values);
		UnknownPtr<ISubject> (timingFunction)->signal (Message (kChanged));
		return true;
	}

protected:
	IParameter* controlParams[4];
	AutoPtr<ITimingFunction> timingFunction;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("Graphics", "Composition", CompositionDemo)

//************************************************************************************************
// TransitionDemo
//************************************************************************************************

class TransitionDemo: public DemoComponent
{
public:
	TransitionDemo ()
	: transitionTypeEnum (nullptr)
	{
		paramList.addParam ("viewIndex", kViewIndex);
		paramList.addParam ("startTransition", kStartTransition);
		UnknownPtr<IListParameter> typeList (paramList.addList ("transitionType", kTransitionType));

		// add all transition types known in skin
		if(auto typeLib = System::GetTypeLibRegistry ().findTypeLib (CCL_SKIN_TYPELIB_NAME))
			transitionTypeEnum = typeLib->findEnumTypeInfo (MAKE_SKIN_ENUMERATION (TAG_VIEW, ATTR_TRANSITION));
		ASSERT (transitionTypeEnum)
		if(transitionTypeEnum) 
			for(int i = 0; i < transitionTypeEnum->getEnumeratorCount (); i++)
			{
				MutableCString name;
				Variant value;
				transitionTypeEnum->getEnumerator (name, value, i);
				typeList->appendString (String (name));
			}

		// additional transition demo with 4 variants
		paramList.addInteger (0, 3, "fourVariants" ,kFourVariants);
	}

	~TransitionDemo ()
	{
		cancelSignals ();
	}

	// DemoComponent
	IView* CCL_API createView (StringID viewName, VariantRef data, const Rect& bounds) override
	{
		CString kTransitionPrefix ("Transition");
		if(viewName.startsWith (kTransitionPrefix))
		{
			int index = 0;
			viewName.subString (kTransitionPrefix.length ()).getIntValue (index);
			
			Variant value;
			MutableCString transitionName;
			if(transitionTypeEnum)
				transitionTypeEnum->getEnumerator (transitionName, value, index);

			IParameter* param = paramList.byTag (kViewIndex);
			if(UnknownPtr<ISkinCreateArgs> args = data.asUnknown ())
			{
				String paramName;
				if(args->getElement ()->getDataDefinition (paramName, "parameter"))
				{
					param = paramList.lookup (MutableCString (paramName));
					ASSERT (param != nullptr)
				}
			}			

			ControlBox variantView (ClassID::VariantView, param, bounds);
			variantView.setName (String (viewName));
			variantView.setAttribute (kVariantViewTransitionType, value);
			variantView.setTooltip (String (transitionName));
			return variantView;
		}
		return nullptr;
	}

	tbool CCL_API paramChanged (IParameter* param) override
	{
		switch(param->getTag ())
		{
		case kStartTransition :
			startTransition ();
			break;

		case kTransitionType :
			(NEW Message ("start"))->post (this);
		}
		return DemoComponent::paramChanged (param);
	}

	void CCL_API notify (ISubject* subject, MessageRef msg) override
	{
		if(msg == "start")
			startTransition ();

		DemoComponent::notify (subject, msg);
	}

	tbool CCL_API getProperty (Variant& var, MemberID propertyId) const override
	{
		if(propertyId == "transitionCount")
		{
			var = transitionTypeEnum ? transitionTypeEnum->getEnumeratorCount () : 0;
			return true;
		}
		return DemoComponent::getProperty (var, propertyId);
	}

protected:
	const IEnumTypeInfo* transitionTypeEnum;

	enum Tag { kViewIndex = 1, kTransitionType, kStartTransition, kFourVariants };

	void startTransition ()
	{
		if(IParameter* viewParam = paramList.byTag (kViewIndex))
			viewParam->setValue (1 - viewParam->getValue ().asInt ());
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("Graphics", "Transitions", TransitionDemo)
