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
// Filename    : exampletext.h
// Description : Example Text
//
//************************************************************************************************

#ifndef _exampletext_h
#define _exampletext_h

#include "ccl/public/text/cclstring.h"

#include "ccl/public/gui/graphics/font.h"

namespace CCL {

static const String kExampleText 
(
	"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut "
	"labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco "
	"laboris nisi ut aliquip ex ea commodo consequat."
);

extern FontRef getStandardFont ();

} // namespace CCL

#endif // _exampletext_h