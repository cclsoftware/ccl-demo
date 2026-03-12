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
// Filename    : graphicsdemo.cpp
// Description : Graphics Demo
//
//************************************************************************************************

#include "../demoitem.h"
#include "exampletext.h"

#include "ccl/app/controls/usercontrol.h"
#include "ccl/base/collections/stringlist.h"

#include "ccl/public/gui/iparameter.h"

#include "ccl/public/gui/graphics/igraphics.h"
#include "ccl/public/gui/graphics/itextlayout.h"
#include "ccl/public/gui/graphics/graphicsfactory.h"

#include "ccl/public/gui/framework/itimer.h"
#include "ccl/public/gui/framework/itheme.h"
#include "ccl/public/gui/framework/iuserinterface.h"

#include "ccl/public/systemservices.h"
#include "ccl/public/guiservices.h"
#include "ccl/public/plugservices.h"

#include "ccl/public/cclversion.h"

using namespace CCL;

//////////////////////////////////////////////////////////////////////////////////////////////////

FontRef CCL::getStandardFont ()
{
	AutoPtr<IThemeStatics> themeStatics = ccl_new<IThemeStatics> (ClassID::ThemeStatics);
	return themeStatics->getStandardFont ();
}

//************************************************************************************************
// TestView
//************************************************************************************************

class TestView: public UserControl,
				public ITimerTask
{
public:

	TestView (RectRef size = Rect (), IParameter* performance = nullptr, IParameter* stats = nullptr)
	: UserControl (size),
	  performance (performance),
	  stats (stats),
	  standardFont (getStandardFont ())
	{
		System::GetGUI ().addIdleTask (this);
	}

	~TestView ()
	{
		System::GetGUI ().removeIdleTask (this);
	}

	// ITimerTask
	void CCL_API onTimer (ITimer* timer) override
	{
		updateClient ();
	}

	CLASS_INTERFACE (ITimerTask, UserControl)

protected:
	SharedPtr<IParameter> performance;
	SharedPtr<IParameter> stats;
	Vector<double> drawTimes;
	Font standardFont;

	struct PerformanceScope
	{
		PerformanceScope (TestView* view)
		: view (view), startTime (System::GetProfileTime ())
		{}

		~PerformanceScope ()
		{
			double endTime = System::GetProfileTime ();
			double ms = (endTime - startTime) * 1000.;
			if(view->performance)
			{
				String s;
				s << "Draw took ";
				s.appendFloatValue (ms, 2);
				s << "ms";
				view->performance->fromString (s);
			}

			if(view->stats)
			{
				Vector<double>& times = view->drawTimes;

				times.add (ms);
				if(times.count () > 100)
					times.removeFirst ();

				double min = 1000;
				double max = 0;
				double sum = 0;
				for(double ms : times)
				{
					min = ccl_min (min, ms);
					max = ccl_max (max, ms);
					sum += ms;
				}

				String s;
				s << "min ";
				s.appendFloatValue (min, 2);
				s << "ms / max ";
				s.appendFloatValue (max, 2);
				s << "ms / avg ";
				s.appendFloatValue (sum / times.count (), 2);
				s << "ms";
				view->stats->fromString (s);
			}
		}

		TestView* view;
		double startTime;
	};
};

//************************************************************************************************
// GraphicsTestView
//************************************************************************************************

class GraphicsTestView: public TestView
{
public:
	GraphicsTestView (RectRef size, IParameter* performance, IParameter* stats)
	: TestView (size, performance, stats)
	{}

	// UserControl
	void draw (const DrawEvent& event) override
	{
		IGraphics& graphics = event.graphics;

		PerformanceScope scope (this);

		Rect clientRect;
		getClientRect (clientRect);

		graphics.fillRect (clientRect, SolidBrush (Colors::kWhite));
		graphics.drawRect (clientRect, Pen (Colors::kRed));

		String zeros = CCLSTR ("000000");

		Font f (standardFont);
		Rect s;

		Rect r (0, 0, 78, 28);
		graphics.drawRect (r, Pen (Colors::kRed));
		graphics.drawString (r, zeros, f, SolidBrush (Colors::kBlack));

		r.offset (80, 0);
		graphics.drawRect (r, Pen (Colors::kRed));
		graphics.drawString (r, CCLSTR ("wwwwww"), f, SolidBrush (Colors::kBlack));

		graphics.measureString (s, zeros, f);
		s.center (r);
		graphics.drawRect (s, Pen (Colors::kGray));

		f.setSize(16);
		r.offset (80, 0);
		graphics.drawRect (r, Pen (Colors::kRed));
		graphics.drawString (r, zeros, f, SolidBrush (Colors::kBlack), Alignment (Alignment::kCenter));

		graphics.measureString (s, zeros, f);
		s.center (r);
		graphics.drawRect (s, Pen (Colors::kGray));

		f.setSize (28);
		r.offset (80, 0);
		graphics.drawRect (r, Pen (Colors::kRed));
		graphics.drawString (r, zeros, f, SolidBrush (Colors::kBlack), Alignment (Alignment::kCenter));

		graphics.measureString (s, zeros, f);
		s.center (r);
		graphics.drawRect (s, Pen (Colors::kGray));

		// Test with Skia vs. Quartz - the expected origin of drawString has to be the same.
		f.setSize (16);
		r.offset (80, 0);
		graphics.drawRect (r, Pen (Colors::kRed));
		graphics.drawString (r.getLeftTop (), zeros, f, SolidBrush (Colors::kBlack));

		f.isBold (true);
		f.setSize (12);
		r.offset (80, 0);
		graphics.drawRect (r, Pen (Colors::kRed));
		graphics.drawString (r, CCLSTR ("Apply"), f, SolidBrush (Colors::kBlack), Alignment (Alignment::kCenter));

		f.setSize (12);
		r.offset (80, 0);
		graphics.drawRect (r, Pen (Colors::kRed));
		graphics.drawString (r, CCLSTR ("Cancel"), f, SolidBrush (Colors::kBlack), Alignment (Alignment::kCenter));

		f.setSize (12);
		r.offset (80, 0);
		graphics.drawRect (r, Pen (Colors::kRed));
		graphics.drawString (r, CCLSTR ("OK"), f, SolidBrush (Colors::kBlack), Alignment (Alignment::kCenter));

		f.setSize (12);
		f.setLineSpacing (1.1f);
		Rect textRect;
		String multiLineText (kExampleText);
		graphics.measureText (textRect, 500, multiLineText, f);
		textRect.offset (20, 40);
		graphics.drawRect (textRect, Pen (Colors::kRed));
		TextFormat textFormat;
		textFormat.isWordBreak (true);
		graphics.drawText (textRect, multiLineText, f, SolidBrush (Colors::kBlack), textFormat);
		f.setLineSpacing (1.f);

		float start = 120.f;
		float range = 300.f;
		Pen pen (Colors::kGreen);
		pen.setWidth (7);
		Rect circle (340, 140, 380, 180);
		AutoPtr<IGraphicsPath> path = GraphicsFactory::createPath ();
		path->addArc (circle, start, range);
		graphics.drawPath (path, pen);
		graphics.drawRect (circle, Pen (Color (0,0,0,128)));

		circle.offset (circle.getWidth () + 20);
		circle.setWidth (circle.getWidth () * 2);
		path = GraphicsFactory::createPath ();
		path->addArc (circle, start, range);
		graphics.drawPath (path, pen);
		graphics.drawRect (circle, Pen (Color (0,0,0,128)));

		circle.offset (circle.getWidth () + 20);
		circle.setWidth (circle.getHeight ());
		path = GraphicsFactory::createPath ();
		path->addArc (circle, 0.f, 360.f); // special case: full circle
		graphics.drawPath (path, pen);
		graphics.drawRect (circle, Pen (Color (0,0,0,128)));

		GradientBrush gradientBrush;
		Rect gradientRect (Point (100, 140), Point (250, 160));
		gradientBrush = LinearGradientBrush (pointIntToF (gradientRect.getLeftTop ()),
											 pointIntToF (gradientRect.getRightTop ()),
											 Colors::kRed, Colors::kBlue);
		graphics.fillRect (gradientRect, gradientBrush);

		gradientRect = Rect (Point (255, 140), Point (275, 180));
		gradientBrush = LinearGradientBrush (pointIntToF (gradientRect.getLeftTop ()),
											 pointIntToF (gradientRect.getLeftBottom ()),
											 gradientBrush); // copy stops from other gradient
		graphics.fillRect (gradientRect, gradientBrush);
		
		gradientRect = Rect (Point (310, 140), Point (330, 180));
		gradientBrush = RadialGradientBrush (pointIntToF (gradientRect.getCenter ().offset (5, 5)), 
											 15, Colors::kYellow, Colors::kTransparentBlack);
		graphics.fillEllipse (gradientRect, gradientBrush);
				
		Rect clipRect (0, 0, 20, 20);
		clipRect.offset (10, 150);
		graphics.saveState ();
		graphics.addClip (clipRect);
		graphics.fillRect (clientRect, SolidBrush (Colors::kGreen));
		clipRect.contract (5);
		graphics.addClip (clipRect);
		graphics.fillRect (clientRect, SolidBrush (Colors::kRed));
		graphics.restoreState ();

		r = Rect (0, 200, 78, 228);
		f.isBold (false);
		graphics.drawRect (r, Pen (Colors::kRed));
		graphics.drawString (r, zeros, f, SolidBrush (Colors::kBlack), Alignment::kLeft);
		r.offset (80, 0);
		graphics.drawRect (r, Pen (Colors::kRed));
		graphics.drawString (r, zeros, f, SolidBrush (Colors::kBlack), Alignment::kCenter);
		r.offset (80, 0);
		graphics.drawRect (r, Pen (Colors::kRed));
		graphics.drawString (r, zeros, f, SolidBrush (Colors::kBlack), Alignment::kRight);

		// digit studies
		r = Rect (0, 250, 20, 280);
		f.setSize (28);
		for(char i = 0; i < 10; i++)
		{
			char c[] = {char('0' + i), 0};
			String digit (c);

			Rect size;
			graphics.measureString (size, digit, f);
			graphics.drawRect (size.moveTo (r.getLeftTop ()), Pen (Colors::kGray));
			graphics.drawString (r, digit, f, SolidBrush (Colors::kBlack), Alignment::kLeftTop);
			r.offset (20, 0);
		}

		{
			f.setSize (28);
			String text ("CCL");
			int hAlignments[] = {Alignment::kHCenter, Alignment::kLeft, Alignment::kRight};
			int vAlignments[] = {Alignment::kVCenter, Alignment::kTop, Alignment::kBottom};
			for(int i = 0; i < 4; i++)
			{
				RectF r = RectF (400, 200 + i * 70.f, PointF (200, 60));
				if(i < 3)
				{
					PointF markerFrom (r.left, (r.top + r.bottom) / 2);
					PointF markerTo (r.right, (r.top + r.bottom) / 2);
					graphics.drawLine (markerFrom, markerTo, Pen (Colors::kBlack));
					graphics.drawRect (r, Pen (Colors::kBlack));
					RectF bounds, imageBounds;
					AutoPtr<ITextLayout> textLayout = GraphicsFactory::createTextLayout ();

					textLayout->construct (text, r.getWidth (), r.getHeight (), f, ITextLayout::kSingleLine, TextFormat (hAlignments[i] | vAlignments[i]));
					textLayout->getBounds (bounds);
					textLayout->getImageBounds (imageBounds);

					graphics.drawTextLayout (r.getLeftTop (), textLayout, SolidBrush (Colors::kBlack));
					graphics.drawRect (bounds.offset (r.getLeftTop ()), Pen (Colors::kBlue));
					graphics.drawRect (imageBounds.offset (r.getLeftTop ()), Pen (Colors::kRed));
				}
				else
				{
					String forte;
					const uchar forteSign[] = {0x0192, 0};
					forte.assign (forteSign);

					AutoPtr<ITextLayout> textLayout = GraphicsFactory::createTextLayout ();
					textLayout->construct (String (forte) << " ab AB fgj pqy", r.getWidth (), r.getHeight (), f, ITextLayout::kSingleLine, TextFormat (Alignment::kLeftTop));
					textLayout->setSubscript ({9, 2});
					textLayout->setSuperscript ({10, 1});
					textLayout->setSubscript ({14, 1});

					Point origin ((Coord)r.left, (Coord)(r.top + r.bottom) / 2);
					graphics.drawTextLayout (origin, textLayout, SolidBrush (Colors::kBlack), IGraphics::kDrawAtBaseline);
					graphics.drawLine (Point (origin.x - 2, origin.y), Point (origin.x + 200, origin.y), Pen (Colors::kGreen));
					graphics.drawLine (Point (origin.x, origin.y - 2), Point (origin.x, origin.y + 2), Pen (Colors::kGreen));
				}
			}
		}

		// text with float coords
		f.setSize (12);
		RectF rectF (0, 300, PointF (20, 20));
		for(char i = 0; i < 20; i++)
		{
			String x ("X");
			RectF size;
			graphics.measureString (size, x, f);

			AutoPtr<IGraphicsPath> path = GraphicsFactory::createPath ();
			path->addRect (size.moveTo (rectF.getLeftTop ()));
			graphics.drawPath (path, Pen (Colors::kBlack));

			graphics.drawString (rectF, x, f, SolidBrush (Colors::kBlack), Alignment::kLeftTop);
			rectF.offset (size.getWidth (), 0.1f);
		}

		rectF = RectF (300, 200, PointF (20, 20));
		for(char i = 0; i < 20; i++)
		{
			String x ("X");
			RectF size;
			graphics.measureString (size, x, f);

			AutoPtr<IGraphicsPath> path = GraphicsFactory::createPath ();
			path->addRect (size.moveTo (rectF.getLeftTop ()));
			graphics.drawPath (path, Pen (Colors::kBlack));

			graphics.drawString (rectF, x, f, SolidBrush (Colors::kBlack), Alignment::kLeftTop);
			rectF.offset (0.1f, size.getHeight ());
		}

		// line caps / join
		r = Rect (0, 350, Point (100, 30));
		const Pen::LineCap lineCaps[] = { Pen::kLineCapButt, Pen::kLineCapSquare, Pen::kLineCapRound };
		const Pen::LineJoin lineJoins[] = { Pen::kLineJoinMiter, Pen::kLineJoinBevel, Pen::kLineJoinRound };
		for(int c = 0; c < ARRAY_COUNT (lineCaps); c++)
		{
			for(int j = 0; j < ARRAY_COUNT (lineJoins); j++)
			{
				Pen pen (Colors::kBlack, 7);
				pen.setLineCap (lineCaps[c]);
				pen.setLineJoin (lineJoins[j]);

				AutoPtr<IGraphicsPath> path = GraphicsFactory::createPath ();
				path->startFigure (r.getLeftTop ());
				path->lineTo (Point (r.left + Coord (r.getWidth () * 0.3), r.bottom));
				path->lineTo (Point (r.left + Coord (r.getWidth () * 0.6), r.top));
				path->lineTo (r.getRightTop ());
				graphics.drawPath (path, pen);

				r.offset (r.getWidth () + 10, 0);
			}
			r.offset (0, r.getHeight ());
			r.moveTo (Point (0, r.top));
		}

		// clipping with path
		auto makeClipPath = [] (IGraphicsPath& path, RectFRef pathRect)
		{
			path.setFillMode (IGraphicsPath::kFillEvenOdd);
			path.addRect (pathRect);
			path.closeFigure ();
			RectF circleRect = pathRect;
			circleRect.offset (35, 10).setWidth (30).setHeight (30);
			path.addArc (circleRect, 45, 360);
			circleRect.offset (10, 50).setWidth (10).setHeight (10);
			path.addArc (circleRect, 0, 360);
		};
		
		AutoPtr<IGraphicsPath> path1 = GraphicsFactory::createPath (IGraphicsPath::kPaintPath);		
		RectF pathRect (20, 450, PointF (100, 100));
		makeClipPath (*path1, pathRect);
		graphics.fillPath (path1, SolidBrush (Colors::kBlack));

		pathRect.offset (130, 0);
		RectF outsideRect = pathRect;
		outsideRect.expand (100);

		AutoPtr<IGraphicsPath> path2 = GraphicsFactory::createPath (IGraphicsPath::kPaintPath);
		makeClipPath (*path2, pathRect);

		graphics.saveState ();
		graphics.addClip (path2);	
		
		graphics.fillRect (outsideRect, SolidBrush (Colors::kRed));				
		graphics.drawLine (PointF (outsideRect.left, outsideRect.getCenter ().y), PointF (outsideRect.right, outsideRect.getCenter ().y), pen);
		graphics.drawLine (PointF (outsideRect.getCenter ().x, outsideRect.top), PointF (outsideRect.getCenter ().x, outsideRect.bottom), pen);

		graphics.restoreState ();
	}
};

//************************************************************************************************
// TextAlignView
//************************************************************************************************

class TextAlignView: public TestView
{
public:
	TextAlignView (FontRef font, IParameter* _customText)
	: customText (_customText),
	  font (font),
	  boxHeight (10)
	{
		updateTexts ();
		ISubject::addObserver (customText, this);
	}

	~TextAlignView ()
	{
		ISubject::removeObserver (customText, this);
	}

	// UserControl
	void notify (ISubject* subject, MessageRef msg) override
	{
		if(msg == kChanged && isEqualUnknown (customText, subject))
			updateTexts ();
		else
			TestView::notify (subject, msg);
	}
	
	void draw (const DrawEvent& event) override
	{
		IGraphics& graphics = event.graphics;

		RectF rect (1, 1);
		Font headerFont (standardFont);
		graphics.drawString (Rect (rect.left, 0, Point (200, 50)), String ("size: ") << font.getSize (), headerFont, SolidBrush (Colors::kBlack), Alignment::kLeftTop);

		texts.forEach ([&] (StringRef text)
		{
			Rect measuredSize;
			Font::measureString (measuredSize, text, font);

			rect.top = + kHeaderH;
			rect.setWidth (measuredSize.getWidth () * 1.3);
			rect.setHeight (boxHeight);

			RectF r (rect);

			int hAlignments[] = {Alignment::kHCenter, Alignment::kLeft, Alignment::kRight};
			int vAlignments[] = {Alignment::kVCenter, Alignment::kTop, Alignment::kBottom};
			for(int i = 0; i < 4; i++)
			{
				if(i < 3)
				{
					PointF markerFrom (r.left, (r.top + r.bottom) / 2);
					PointF markerTo (r.right, (r.top + r.bottom) / 2);
					graphics.drawLine (markerFrom, markerTo, Pen (Color (Colors::kBlack).setAlphaF (0.2)));
					graphics.drawRect (r, Pen (Colors::kBlack));
					RectF bounds, imageBounds;
					AutoPtr<ITextLayout> textLayout = GraphicsFactory::createTextLayout ();

					textLayout->construct (text, r.getWidth (), r.getHeight (), font, ITextLayout::kSingleLine, TextFormat (hAlignments[i] | vAlignments[i]));
					textLayout->getBounds (bounds);
					textLayout->getImageBounds (imageBounds);

					graphics.drawTextLayout (r.getLeftTop (), textLayout, SolidBrush (Colors::kBlack));
					graphics.drawRect (bounds.offset (r.getLeftTop ()), Pen (Colors::kBlue));
					graphics.drawRect (imageBounds.offset (r.getLeftTop ()), Pen (Colors::kRed));
				}
				else
				{
					Point origin (r.left, (r.top + r.bottom) / 2);
					graphics.drawString (origin, text, font, SolidBrush (Colors::kBlack), IGraphics::kDrawAtBaseline);

					RectF size;
					Font::measureStringImage (size, text, font, true);
					graphics.drawRect (size.offset (pointIntToF (origin)), Pen (Colors::kRed));

					graphics.drawLine (Point (origin.x - 2, origin.y), Point (origin.x + rect.getWidth (), origin.y), Pen (Colors::kGreen));
					graphics.drawLine (Point (origin.x, origin.y - 2), Point (origin.x, origin.y + 2), Pen (Colors::kGreen));
				}
				r.offset (0, rect.getHeight () + kSpacing);
			}
			rect.offset (rect.getWidth () + kSpacing);
		});
	}

protected:
	static const Coord kSpacing = 8;
	static const Coord kHeaderH = 16;

	StringList texts;
	SharedPtr<IParameter> customText;
	Font font;
	Coord boxHeight;

	void updateTexts ()
	{
		String text;
		customText->toString (text);

		bool changed = false;
		if(text.isEmpty ())
		{
			if(texts.count () <= 1)
			{
				texts.removeAll ();

				const uchar forteSign[] = { 0x0192, 0 };
				String forte (forteSign);

				texts.add (forte);
				texts.add ("fgj");
				texts.add ("pqy");
				texts.add ("X");

				const uchar kanjiKanaSigns[] = { 0x751f, 0x304b, 0 };
				String kanjiKana (kanjiKanaSigns);
				texts.add (kanjiKana);

				changed = true;
			}
		}
		else
		{
			if(texts.count () != 1 || texts[0] != text)
			{
				texts.removeAll ();
				texts.add (text);

				changed = true;
			}
		}

		if(!changed)
			return;

		Rect totalSize;
		texts.forEach ([&] (StringRef text)
		{
			Rect measuredSize;
			Font::measureString (measuredSize, text, font);
			totalSize.right += (Coord)(measuredSize.right * 1.3 + kSpacing);

			ccl_lower_limit (boxHeight, (Coord)(measuredSize.bottom * 1.3));
		});

		setSize (Rect (0, 0, totalSize.getWidth (), (boxHeight + kSpacing) * 4 + kHeaderH));
	}
};

//************************************************************************************************
// GraphicsDemo
//************************************************************************************************

class GraphicsDemo: public DemoComponent
{
public:
	GraphicsDemo ()
	{
		performance = paramList.addString ("performance");
		stats = paramList.addString ("stats");
	}

	// Component
	IView* CCL_API createView (StringID name, VariantRef data, const Rect& bounds) override
	{
		if(name == "TestView")
			return *NEW GraphicsTestView (bounds, performance, stats);
		return nullptr;
	}

protected:
	IParameter* performance;
	IParameter* stats;
};

//************************************************************************************************
// TextAlignDemo
//************************************************************************************************

class TextAlignDemo: public DemoComponent
{
public:
	TextAlignDemo ()
	{
		customText = paramList.addString ("customText");
	}

	// Component
	IView* CCL_API createView (StringID name, VariantRef data, const Rect& bounds) override
	{
		if(name == "TestView")
		{
			// horizontal container
			ViewBox layout (ClassID::AnchorLayoutView, bounds, StyleFlags (Styles::kHorizontal, Styles::kLayoutWrap));
			layout.setSizeMode (IView::kAttachAll);

			Font font (getStandardFont ());
			float fontSizes[] = {32, 24, 20, 16, 14, 12, 10, 8, 6};

			VectorForEach (ConstVector<float> (fontSizes, ARRAY_COUNT (fontSizes)), float, fontSize)
				Font f (font);
				f.setSize (fontSize);
				layout.getChildren ().add (*NEW TextAlignView (f, customText));
			EndFor
			return layout;
		}
		return nullptr;
	}

protected:
	IParameter* customText;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("Graphics", "Graphics 2D", GraphicsDemo)
REGISTER_DEMO ("Graphics", "Text Alignment", TextAlignDemo)
