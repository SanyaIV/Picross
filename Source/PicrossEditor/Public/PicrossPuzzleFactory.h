// Copyright Sanya Larsson 2020

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "PicrossPuzzleFactory.generated.h"

/**
 * Factory for PicrossPuzzleData
 */
UCLASS()
class PICROSSEDITOR_API UPicrossPuzzleFactory : public UFactory
{
	GENERATED_BODY()
	
public:
	UPicrossPuzzleFactory(const FObjectInitializer& ObjectInitializer);
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	class UPicrossPuzzleData* CreatedObjectAsset;
};
