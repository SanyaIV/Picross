// Copyright Sanya Larsson 2020

#pragma once

// Default includes
#include "Components/TextRenderComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PicrossGrid.generated.h"

// Forward declarations
class APicrossBlock;
class UMaterialInstance;
class UUserWidget;
class UPicrossPuzzleData;
enum class EBlockState : uint8;


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

	void OpenPuzzleBrowser() const;
	void ClosePuzzleBrowser() const;
	TArray<FAssetData> GetAllPuzzles() const;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void CreateGrid();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void ClearGrid() const;
	void DestroyGrid();
	
	void Cycle2DRotation(const APicrossBlock* PivotBlock);
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
	void CreateAndAttachTextToBlock(APicrossBlock* Block, FVector RelativeLocation, FRotator RelativeRotation, FText Text, FColor Color, EHorizTextAligment HAlignment, EVerticalTextAligment VAlignment) const;

	void SetRotationXAxis();
	void SetRotationYAxis();
	void SetRotationZAxis();
	void EnableAllBlocks() const;
	void DisableAllBlocks() const;
	void LockAllBlocks() const;
	bool IsSolved() const;
	void TrySolve() const;

	UFUNCTION()
	void OnBlockStateChanged(EBlockState PreviousState, EBlockState NewState);
	
	// Blueprint for the puzzle blocks we spawn in.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<APicrossBlock> PicrossBlockBP;
	// The distance to use between the blocks when spawning them.
	UPROPERTY(EditAnywhere, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	float DistanceBetweenBlocks = 110.f;

	// The currently loaded Puzzle, if null then we're creating a new puzzle.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	UPicrossPuzzleData* CurrentPuzzle = nullptr;
	// The total amount of filled blocks in the puzzle solution.
	int32 SolutionFilledBlocksCount = -1;
	// Used to keep track of the total amount of filled block count.
	int32 FilledBlocksCount = 0;

	// A 3D array implemented in a 1D array. Use FArray3D to handle translation.
	UPROPERTY(VisibleAnywhere, Category = "Picross")
	TArray<APicrossBlock*> PicrossGrid;
	// GridSize for PicrossGrid above.
	FIntVector GridSize = FIntVector::ZeroValue;

	// GridSize to use when we can't load a puzzle.
	UPROPERTY(EditAnywhere, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	FIntVector DefaultGridSize {5,5,5};

	// The material to use for the text that we create for rows and columns.
	UPROPERTY(EditAnywhere, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* NumbersTextMaterial = nullptr;

	// Keeps track of the current axis of selection.
	ESelectionAxis SelectionAxis = ESelectionAxis::All;
	// Caches the last index used to pivot so we can do that operation without a new pivot-index.
	FIntVector LastPivotXYZ = FIntVector::ZeroValue;

	// The blueprint widget class that we use to create the puzzle browser widget.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> PuzzleBrowserWidgetClass = nullptr;
	// Pointer to the created puzzle browser widget.
	UUserWidget* PuzzleBrowserWidget = nullptr;
};
