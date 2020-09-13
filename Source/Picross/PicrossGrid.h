// Copyright Sanya Larsson 2020

#pragma once

// Default includes
#include "Components/TextRenderComponent.h"
#include "CoreMinimal.h"
#include "PicrossBlock.h"
#include "GameFramework/Actor.h"
#include "PicrossGrid.generated.h"

// Forward declarations
class ATextRenderActor;
class UMaterialInstance;
class UUserWidget;
class UPicrossPuzzleData;

UENUM()
enum class ESelectionAxis : uint8
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
	void ClearGrid();
	void DestroyGrid();
	bool IsLocked() const;

	void UpdateBlocks(const int32 StartMasterIndex, const int32 EndMasterIndex, const EBlockState Action);
	
	void Cycle2DRotation(const int32 MasterIndexPivot);
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

	void GenerateNumbers();
	void GenerateNumbersForAxis(ESelectionAxis Axis);
	void CreateTextRenderActor(FVector WorldLocation, FRotator RelativeRotation, FText Text, FColor Color, EHorizTextAligment HAlignment, EVerticalTextAligment VAlignment);

	void SetRotationXAxis() const;
	void SetRotationYAxis() const;
	void SetRotationZAxis() const;

	void UpdateBlockState(FPicrossBlock& Block, const EBlockState NewState, const int32 PreviousInstanceIndex);
	void CreateBlockInstance(const FPicrossBlock& Block) const;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void EnableOnlyFilledBlocks() const;
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void EnableAllBlocks() const;
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void DisableAllBlocks() const;

	void Lock();
	void Unlock();
	bool IsSolved() const;
	void TrySolve() ;

	// A 3D array implemented in a 1D array. Use FArray3D to handle translation.
	UPROPERTY()
	TArray<FPicrossBlock> MasterGrid;
	// GridSize for MasterGrid above.
	FIntVector GridSize = FIntVector::ZeroValue;
	// Instanced Static Mesh Components for each EBlockState
	UPROPERTY()
	TMap<EBlockState, UInstancedStaticMeshComponent*> BlockInstances;
	// Meshes for each EBlockState
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Picross Block", meta = (AllowPrivateAccess = "true"))
	TMap<EBlockState, UStaticMesh*> BlockMeshes;
	// Materials for each EBlockState
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Picross Block", meta = (AllowPrivateAccess = "true"))
	TMap<EBlockState, UMaterialInstance*> BlockMaterials;
	UPROPERTY()
	TArray<ATextRenderActor*> TextRenderActors;

	// The distance to use between the blocks when spawning them.
	UPROPERTY(EditAnywhere, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	float DistanceBetweenBlocks = 110.f;

	// The currently loaded Puzzle, if null then we're creating a new puzzle.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	UPicrossPuzzleData* CurrentPuzzle = nullptr;
	// The total amount of filled blocks in the puzzle solution.
	int32 SolutionFilledBlocksCount = -1;
	int32 CurrentlyFilledBlocksCount = 0;
	
	// GridSize to use when we can't load a puzzle. Not constexpr since we change this in editor to create new puzzles.
	UPROPERTY(EditAnywhere, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	FIntVector DefaultGridSize {5,5,5};

	// The material to use for the text that we create for rows and columns.
	UPROPERTY(EditAnywhere, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* NumbersTextMaterial = nullptr;

	// Keeps track of the current axis of selection.
	ESelectionAxis SelectionAxis = ESelectionAxis::All;
	// Caches the last index used to pivot so we can do that operation without a new pivot-index.
	FIntVector LastPivotXYZ = FIntVector::ZeroValue;
	// Lock that we can set when solution has been found so the player can't edit finished puzzles.
	bool bLocked = false;

	// The blueprint widget class that we use to create the puzzle browser widget.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> PuzzleBrowserWidgetClass = nullptr;
	// Pointer to the created puzzle browser widget.
	UPROPERTY()
	UUserWidget* PuzzleBrowserWidget = nullptr;
};
