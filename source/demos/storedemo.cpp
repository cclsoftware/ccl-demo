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
// Filename    : storeapidemo.cpp
// Description : Platform Store API Demo
//
//************************************************************************************************

#include "../demoitem.h"

#include "ccl/base/message.h"

#include "ccl/app/controls/listviewmodel.h"

#include "ccl/extras/stores/platformstoremanager.h"

using namespace CCL;

//************************************************************************************************
// StoreDemo
//************************************************************************************************

class StoreDemo: public DemoComponent
{
public:
	const String kInAppProductId = "platformlicense";

	StoreDemo ()
	{
		paramList.addParam (CSTR ("query"), 'qery');
		paramList.addParam (CSTR ("purchase"), 'prch');
		paramList.addParam (CSTR ("licenses"), 'lcns');

		listModel = NEW ListViewModel;
		listModel->getColumns ().addColumn (200, 0, "label", 160);
		listModel->getColumns ().addColumn (300, 0, "value", 20);
		addObject ("list", listModel);

		PlatformStoreManager& store = PlatformStoreManager::instance ();
		store.addObserver (this);

		Promise (store.startup ()).then ([&](IAsyncOperation& op)
		{
			if(op.getState () != IAsyncInfo::kCompleted)
				return;

			queryLicenses ();
		});
	}

	~StoreDemo ()
	{
		PlatformStoreManager& store = PlatformStoreManager::instance ();
		store.removeObserver (this);
		store.shutdown ();
	}

	tbool CCL_API paramChanged (IParameter* param) override
	{
		if(param->getTag () == 'qery')
		{
			queryStore (kInAppProductId);
		}
		else if(param->getTag () == 'prch')
		{
			purchaseProduct (kInAppProductId);
		}
		else if(param->getTag () == 'lcns')
		{
				queryLicenses ();
		}
		return true;
	}

	void CCL_API notify (ISubject* subject, MessageRef msg) override
	{
		if(msg.getID () == PlatformStoreManager::kTransactionsChanged)
		{
			queryTransactions ();
		}
		else if(msg.getID () == PlatformStoreManager::kLocalLicensesChanged)
		{
			queryLicenses ();
		}

		DemoComponent::notify (subject, msg);
	}

	void queryStore (StringRef productId)
	{
		PlatformStoreManager& store = PlatformStoreManager::instance ();

		Vector<String> productIds = { productId };

		Promise (store.requestProducts (productIds)).then ([&](IAsyncOperation& op)
		{
			if(op.getState () != IAsyncInfo::kCompleted)
				return;

			if(auto* result = unknown_cast<Container> (op.getResult ()))
			{
				for(auto* product : iterate_as<StoreProduct> (*result))
				{
					addInfo ("Query product ID", product->getID ());
					addInfo ("  Name", product->getName ());
					addInfo ("  Price", product->getPrice ());
				}

				listModel->deferSignal (NEW Message (kChanged));
			}
		});
	}

	void purchaseProduct (StringRef productId)
	{
		addInfo ("Purchase flow started", String::kEmpty);

		listModel->deferSignal (NEW Message (kChanged));

		PlatformStoreManager& store = PlatformStoreManager::instance ();

		Promise (store.purchaseProduct (productId)).then ([&](IAsyncOperation& op)
		{
			if(op.getState () != IAsyncInfo::kCompleted)
				return;

			addInfo ("Purchase flow completed", String::kEmpty);

			listModel->deferSignal (NEW Message (kChanged));
		});
	}

	void queryTransactions ()
	{
		PlatformStoreManager& store = PlatformStoreManager::instance ();

		Promise (store.getTransactions ()).then ([&](IAsyncOperation& op)
		{
			if(op.getState () != IAsyncInfo::kCompleted)
				return;

			if(auto* result = unknown_cast<Container> (op.getResult ()))
			{
				addInfo ("Transactions:", String::kEmpty);

				for(auto* transaction : iterate_as<StoreTransaction> (*result))
				{
					String purchaseStatus = "Failed";
					switch(transaction->getState ())
					{
					case PurchaseState::kInProgress : purchaseStatus = "In progress"; break;
					case PurchaseState::kDeferred : purchaseStatus = "Pending"; break;
					case PurchaseState::kCompleted : purchaseStatus = "Completed"; break;
					case PurchaseState::kCanceled : purchaseStatus = "Canceled"; break;
					}

					addInfo ("  Transaction ID", transaction->getTransactionID ());
					addInfo ("    Product ID", transaction->getProductID ());
					addInfo ("    Purchase status", purchaseStatus);
				}

				listModel->deferSignal (NEW Message (kChanged));
			}
		});
	}

	void queryLicenses ()
	{
		PlatformStoreManager& store = PlatformStoreManager::instance ();

		Promise (store.getLocalLicenses ()).then ([&](IAsyncOperation& op)
		{
			if(op.getState () != IAsyncInfo::kCompleted)
				return;

			if(auto* result = unknown_cast<Container> (op.getResult ()))
			{
				addInfo ("Licenses:", String::kEmpty);

				for(auto* license : iterate_as<StoreLicense> (*result))
				{
					String licenseStatus = "Unverified";
					switch(license->getVerificationResult ())
					{
					case LicenseVerificationResult::kValid : licenseStatus = "Valid"; break;
					case LicenseVerificationResult::kExpired : licenseStatus = "Expired"; break;
					case LicenseVerificationResult::kInvalid : licenseStatus = "Invalid"; break;
					}

					addInfo ("  Product ID", license->getProductID ());
					addInfo ("    License status", licenseStatus);
				}

				listModel->deferSignal (NEW Message (kChanged));
			}
		});
	}

	void addInfo (StringRef label, StringRef value)
	{
		ListViewItem* item = NEW ListViewItem (label);
		item->getDetails ().set ("label", label);
		item->getDetails ().set ("value", value);
		listModel->addItem (item);
	}

private:
	AutoPtr<ListViewModel> listModel;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("System", "Store API", StoreDemo)
