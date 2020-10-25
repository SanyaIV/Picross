// Copyright Sanya Larsson 2020

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AssetData.h"
#include "AssetDataObject.generated.h"


UCLASS( Blueprintable, BlueprintType )
class PICROSS_API UAssetDataObject : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AssetData")
	void SetAssetData(FAssetData NewAssetData);
	UFUNCTION(BlueprintCallable, Category = "AssetData")
	FAssetData GetAssetData() const;

	UFUNCTION(BlueprintCallable, Category = "Picross")
	FIntVector GetGridSize() const;
	UFUNCTION(BlueprintCallable, Category = "Picross")
	FString GetGridSizeString() const;

private:
	FAssetData AssetData;
};
