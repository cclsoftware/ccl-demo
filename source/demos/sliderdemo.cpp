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
// Filename    : sliderdemo.cpp
// Description : Slider Demo
//
//************************************************************************************************

#include "../demoitem.h"

#include "ccl/app/params.h"

using namespace CCL;

//************************************************************************************************
// SliderDemoCurve
//************************************************************************************************

class SliderDemoCurve: public ParamCurve,
				  	   public ITickScale
{
public:
	// ParamCurve
	double CCL_API displayToNormalized (double linearValue) const override
	{
		return linearValue;
	};

	double CCL_API normalizedToDisplay (double curveValue) const override
	{
		return curveValue;
	};

	// ITickScale
	int CCL_API getNumTicks (int weight) const override
	{
		return 17;
	};
	
	tbool CCL_API getTick (double& tick, String* label, int weight, int index) const override
	{
		if(index >= getNumTicks (weight) || index < 0)
			return false;
		
		tick = double (index) / (getNumTicks(weight) - 1.0f);
		return true;
	};

	tbool CCL_API isHiliteTick (int weight, int index) const override
	{
		return index % 4 == 0;
	};

	CLASS_INTERFACE (ITickScale, ParamCurve)
};


//************************************************************************************************
// SliderDemo
//************************************************************************************************

class SliderDemo: public DemoComponent
{
public:
	SliderDemo ()
	{
		IParameter* testValueParam = paramList.addFloat (0, 2, CSTR ("testValue"), 'dctv');
		AutoPtr<SliderDemoCurve> curve = NEW SliderDemoCurve;
		testValueParam->setCurve (curve);

		paramList.addFloat (0, 1, CSTR ("range1"), 'dcr1')->setValue (0.33);
		paramList.addFloat (0, 1, CSTR ("range2"), 'dcr2')->setValue (0.66);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("Controls", "Slider", SliderDemo, DEMO_PAGE_DEFAULT)
