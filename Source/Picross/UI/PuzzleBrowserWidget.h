// Copyright Sanya Larsson 2020

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AssetData.h"
#include "Containers/Array.h"
#include "PuzzleBrowserWidget.generated.h"

/**
 * UI Widget class for the puzzle browser.
 */
UCLASS()
class PICROSS_API UPuzzleBrowserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetPicrossGrid(class APicrossGrid* Grid);

	UFUNCTION(BlueprintCallable, Category = "Picross")
	class APicrossGrid* GetPicrossGrid() const;

protected:
	UFUNCTION(BlueprintCallable, Category = "Picross")
	TArray<class UAssetDataObject*> GetPuzzles();

private:
	TArray<FAssetData> GetPuzzleDatas() const;

	UPROPERTY()
	class APicrossGrid* PicrossGrid;
	
};
