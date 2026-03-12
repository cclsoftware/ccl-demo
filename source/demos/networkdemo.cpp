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
// Filename    : networkdemo.cpp
// Description : Network Demo
//
//************************************************************************************************

#define DEBUG_LOG 1

#include "../demoitem.h"

#include "ccl/extras/web/webxhroperation.h"

#include "ccl/app/controls/listviewmodel.h"

#include "ccl/base/storage/url.h"
#include "ccl/base/storage/settings.h"
#include "ccl/base/message.h"

#include "ccl/public/base/streamer.h"
#include "ccl/public/text/itextstreamer.h"

#include "ccl/public/network/web/httpstatus.h"
#include "ccl/public/network/web/ixmlhttprequest.h"
#include "ccl/public/network/web/iwebsocket.h"
#include "ccl/public/netservices.h"
#include "ccl/public/plugservices.h"
#include "ccl/public/cclversion.h"

using namespace CCL;

//************************************************************************************************
// RequestOperation
//************************************************************************************************

class RequestOperation: public Web::AsyncXHROperation
{
public:
	using AsyncXHROperation::AsyncXHROperation;

	// AsyncXHROperation
	void onHttpRequestFinished () override
	{
		if(IStream* responseStream = httpRequest->getResponseStream ())
		{
			responseStream->rewind ();

			// TODO: get charset from HTML...
			AutoPtr<ITextStreamer> reader = System::CreateTextStreamer (*responseStream, {Text::kUTF8});
			if(reader)
			{
				int lines = 0;
				String line;
				while(reader->readLine (line))
				{
					if(lines++ > 200)
					{
						getProgressHandler ()->setProgressText ("...");
						break;
					}
					getProgressHandler ()->setProgressText (String ("Response: ") << line);
				}
			}
		}
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////
// Tags
//////////////////////////////////////////////////////////////////////////////////////////////////

namespace Tag
{
	enum XMLHttpRequestDemoTags
	{
		kUrl = 'url ',
		kConnect = 'conn',
		kDisconnect = 'disc',
		kReadyState = 'rdys',
		kMessageText = 'mtxt',
		kSendMessage = 'send'
	};
}

//************************************************************************************************
// XMLHttpRequestDemo
//************************************************************************************************

class XMLHttpRequestDemo: public DemoComponent,
						  public AbstractProgressNotify,
						  public IAsyncCompletionHandler
{
public:
	XMLHttpRequestDemo ()
	{
		System::GetNetwork (); // someone must call something from cclnet to create a dynamic link dependency, otherwise cclnet is not loaded

		IParameter* urlParam = paramList.addString ("url", Tag::kUrl);

		listModel = NEW ListViewModel;
		listModel->getColumns ().addColumn (200, 0, "title", 160);
		addObject ("list", listModel);

		// trigger initial request
		urlParam->setValue (CCLSTR (CCL_PRODUCT_WEBSITE), true);
	}

	void writeLine (StringRef info)
	{
		ListViewItem* item = NEW ListViewItem (info);
		listModel->addItem (item);
		listModel->signal (Message (kChanged));
	}

	void startRequest (UrlRef url)
	{
		listModel->removeAll ();

		auto* httpRequest = ccl_new<Web::IXMLHttpRequest> (CCL::ClassID::XMLHttpRequest);
		auto* operation = NEW RequestOperation (httpRequest);

		operation->setProgressHandler (this);
		operation->setCompletionHandler (this);
		operation->setState (IAsyncInfo::kStarted);

		httpRequest->open (Web::HTTP::kGET, url);
		httpRequest->send ();

		pendingOperation = operation;

		writeLine ("Started");
	}

	tbool CCL_API paramChanged (IParameter* param) override
	{
		if(param->getTag () == Tag::kUrl)
		{
			String string (param->getValue ().asString ());
			if(!string.isEmpty ())
			{
				Url url (string);
				startRequest (url);
			}
		}
		return DemoComponent::paramChanged (param);
	}

	// AbstractProgressNotify
	void CCL_API setProgressText (StringRef text) override
	{
		writeLine (text);
	}

	void CCL_API updateProgress (const State& state) override
	{
		String s ("Progress: ");
		if(state.flags & kIndeterminate)
			s << "...";
		else
			s.appendFloatValue (state.value * 100, 1) << "%";
		writeLine (s);
	}

	// IAsyncCompletionHandler
	void CCL_API onCompletion (IAsyncOperation& operation) override
	{
		int status = operation.getResult ();
		writeLine (String ("Completion: state ") << operation.getState ());
		writeLine (String ("Status: ") << status);

		AsyncOperation::deferDestruction (pendingOperation.detach ());
	}

	CLASS_INTERFACE2 (IProgressNotify, IAsyncCompletionHandler, DemoComponent)

private:
	AutoPtr<ListViewModel> listModel;
	AutoPtr<IAsyncOperation> pendingOperation;
};

//************************************************************************************************
// WebSocketDemo
//************************************************************************************************

class WebSocketDemo: public DemoComponent
{
public:
	WebSocketDemo ()
	: webSocket (ccl_new<Web::IWebSocket> (ClassID::WebSocket))
	{
		paramList.addString ("url", Tag::kUrl)->fromString ("wss://echo.websocket.org");
		paramList.addParam ("connect", Tag::kConnect);
		paramList.addParam ("disconnect", Tag::kDisconnect);
		paramList.addString ("readyState", Tag::kReadyState);
		paramList.addString ("messageText", Tag::kMessageText)->fromString ("Hello!");
		paramList.addParam ("sendMessage", Tag::kSendMessage);
		
		ISubject::addObserver (webSocket, this);		
		updateReadyState ();
	}

	~WebSocketDemo ()
	{
		ISubject::removeObserver (webSocket, this);
		webSocket->close ();
	}

	tbool CCL_API paramChanged (IParameter* param) override
	{
		if(param->getTag () == Tag::kConnect)
		{
			if(webSocket->getReadyState () != Web::IWebSocket::kClosed)
				webSocket->close ();

			String urlString;
			paramList.byTag (Tag::kUrl)->toString (urlString);
			
			Url url (urlString);
			webSocket->open (url, CCLSTR (CCL_PACKAGE_DOMAIN ".chat"));
		}
		else if(param->getTag () == Tag::kDisconnect)
		{
			webSocket->close ();
		}
		else if(param->getTag () == Tag::kSendMessage)
		{
			String text;			
			paramList.byTag (Tag::kMessageText)->toString (text);
			
			webSocket->send (text);
		}
		return DemoComponent::paramChanged (param);
	}

	void CCL_API notify (ISubject* subject, MessageRef msg) override
	{
		CCL_PRINTF ("WebSocket notification %s\n", msg.getID ().str ())

		if(msg == Web::IWebSocket::kOnReadyStateChange)
			updateReadyState ();
	}

	void updateReadyState ()
	{
		auto state = webSocket->getReadyState ();
		
		String stateName;
		switch(state)
		{
		case Web::IWebSocket::kConnecting : stateName = "Connecting"; break;
		case Web::IWebSocket::kOpen : stateName = "Open"; break;
		case Web::IWebSocket::kClosing : stateName = "Closing"; break;
		case Web::IWebSocket::kClosed : stateName = "Closed"; break;
		}
		paramList.byTag (Tag::kReadyState)->fromString (stateName);
		
		bool canDisconnect = state != Web::IWebSocket::kClosed;
		paramList.byTag (Tag::kDisconnect)->enable (canDisconnect);

		bool canSend = state == Web::IWebSocket::kOpen;
		paramList.byTag (Tag::kSendMessage)->enable (canSend);

		signal (Message (kPropertyChanged));
	}

protected:
	AutoPtr<Web::IWebSocket> webSocket;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("Network", "XMLHttpRequest", XMLHttpRequestDemo)
//REGISTER_DEMO ("Network", "WebSocket", WebSocketDemo) work in progress
