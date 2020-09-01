// Copyright Sanya Larsson 2020

#pragma once

#include "Components/TextRenderComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PicrossGrid.generated.h"


UENUM()
enum ESelectionAxis
{
	Z	UMETA(DisplayName = "Z"),
	Y	UMETA(DisplayName = "Y"),
	X	UMETA(DisplayName = "X"),
	All UMETA(DisplayName = "All")
};

/**
 * The Picross Grid creator which handles creating the Picross Grid.
 */
UCLASS()
class PICROSS_API APicrossGrid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APicrossGrid();

	// TODO: Refactor into Puzzle Browser component/actor.
	void OpenPuzzleBrowser() const;
	void ClosePuzzleBrowser() const;
	TArray<FAssetData> GetAllPuzzles() const;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void CreateGrid();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void ClearGrid() const;
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void TrySolve() const;
	void DestroyGrid();
	
	void Cycle2DRotation(const class APicrossBlock* PivotBlock);
	void Move2DSelectionUp();
	void Move2DSelectionDown();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void SavePuzzle() const;
	UFUNCTION(BlueprintCallable, Category = "Picross")
	void LoadPuzzle(FAssetData PuzzleToLoad); 

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void CreatePuzzleBrowser();

	void GenerateNumbers() const;
	void GenerateNumbersForAxis(ESelectionAxis Axis) const;
	void CreateAndAttachTextToBlock(class APicrossBlock* Block, FVector RelativeLocation, FRotator RelativeRotation, FText Text, FColor Color, EHorizTextAligment HAlignment, EVerticalTextAligment VAlignment) const;

	void SetRotationXAxis(int32 PivotIndex);
	void SetRotationYAxis(int32 PivotIndex);
	void SetRotationZAxis(int32 PivotIndex);
	void EnableAllBlocks() const;
	void DisableAllBlocks() const;
	bool IsSolved() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	class UPicrossPuzzleData* CurrentPuzzle = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class APicrossBlock> PicrossBlockBP;

	// A 3D array implemented in a 1D array. Use FArray3D to handle translation.
	UPROPERTY(VisibleAnywhere, Category = "Picross")
	TArray<class APicrossBlock*> PicrossGrid;
	FIntVector GridSize = FIntVector::ZeroValue;

	UPROPERTY(EditAnywhere, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	FIntVector DefaultGridSize {5,5,5};

	UPROPERTY(EditAnywhere, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	float DistanceBetweenBlocks = 110.f;

	UPROPERTY(EditAnywhere, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	class UMaterialInstance* NumbersTextMaterial = nullptr;

	ESelectionAxis SelectionAxis = ESelectionAxis::All;
	FIntVector LastPivotXYZ = FIntVector::ZeroValue;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UUserWidget> PuzzleBrowserWidgetClass = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UUserWidget> PuzzleBrowserItemWidget = nullptr;
	UUserWidget* PuzzleBrowserWidget = nullptr;
};
