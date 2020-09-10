// Copyright Sanya Larsson 2020


#include "PicrossPuzzleFactory.h"
#include "PicrossPuzzleData.h"
#include "UnrealEd.h"


UPicrossPuzzleFactory::UPicrossPuzzleFactory(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UPicrossPuzzleData::StaticClass();
}

UObject* UPicrossPuzzleFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	if (CreatedObjectAsset != nullptr)
	{
		CreatedObjectAsset->SetFlags(Flags | RF_Transactional);
		CreatedObjectAsset->Modify();
		CreatedObjectAsset->Rename(*InName.ToString(), InParent);
	}
	else
	{
		CreatedObjectAsset = NewObject<UPicrossPuzzleData>(InParent, InClass, InName, Flags | RF_Transactional);
	}

	return CreatedObjectAsset;
}
