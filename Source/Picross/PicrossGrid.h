// Copyright Sanya Larsson 2020

#pragma once

// Default includes
#include "Components/TextRenderComponent.h"
#include "CoreMinimal.h"
#include "FArray3D.h"
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
 * Struct representing a 3D collection of FPicrossBlock, has a GridSize and Array.
 */
USTRUCT(BlueprintType)
struct FPicrossBlockGrid
{
	GENERATED_BODY();

public:
	// Constructors & Assignments
	FORCEINLINE FPicrossBlockGrid() : GridSize(INDEX_NONE) {}
	FORCEINLINE FPicrossBlockGrid(FIntVector GridSize) : GridSize(GridSize) { Grid.SetNum(FArray3D::Size(GridSize)); }
	FORCEINLINE FPicrossBlockGrid(const FPicrossBlockGrid&) = default;
	FORCEINLINE FPicrossBlockGrid(FPicrossBlockGrid&&) = default;
	FORCEINLINE FPicrossBlockGrid& operator=(const FPicrossBlockGrid&) = default;
	FORCEINLINE FPicrossBlockGrid& operator=(FPicrossBlockGrid&&) = default;

	// Getters
	FORCEINLINE FIntVector GetGridSize() const { return GridSize; }
	FORCEINLINE int32 X() const { return GridSize.X; }
	FORCEINLINE int32 Y() const { return GridSize.Y; }
	FORCEINLINE int32 Z() const { return GridSize.Z; }
	FORCEINLINE TArray<FPicrossBlock>& GetGrid() { return Grid; }
	FORCEINLINE const TArray<FPicrossBlock>& GetGrid() const { return Grid; }
	FORCEINLINE int32 GetIndex(FIntVector ThreeDimensionalIndex) const { return FArray3D::TranslateTo1D(GridSize, ThreeDimensionalIndex); }
	FORCEINLINE FIntVector GetIndex(int32 OneDimensionalIndex) const { return FArray3D::TranslateTo3D(GridSize, OneDimensionalIndex); }

	// Ranged for redirection
	FORCEINLINE auto begin() { return Grid.begin(); }
	FORCEINLINE auto begin() const { return Grid.begin(); }
	FORCEINLINE auto end() { return Grid.end(); }
	FORCEINLINE auto end() const { return Grid.end(); }

	// Functions to access the array
	FORCEINLINE FPicrossBlock& operator[](int32 OneDimensionalIndex) { return Grid[OneDimensionalIndex]; }
	FORCEINLINE const FPicrossBlock& operator[](int32 OneDimensionalIndex) const { return Grid[OneDimensionalIndex]; }
	FORCEINLINE FPicrossBlock& operator[](FIntVector ThreeDimensionalIndex) { return Grid[FArray3D::TranslateTo1D(GridSize, ThreeDimensionalIndex)]; }
	FORCEINLINE const FPicrossBlock& operator[](FIntVector ThreeDimensionalIndex) const { return Grid[FArray3D::TranslateTo1D(GridSize, ThreeDimensionalIndex)]; }

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Picross Grid", meta = (AllowPrivateAccess = "true"))
	FIntVector GridSize;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Picross Grid", meta = (AllowPrivateAccess = "true"))
	TArray<FPicrossBlock> Grid;
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
	void HighlightBlocks(const int32 MasterIndexPivot);

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
	void HighlightBlocksInAxis(const int32 MasterIndexPivot, const ESelectionAxis AxisToHighlight);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void EnableOnlyFilledBlocks() const;
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void EnableAllBlocks() const;
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void DisableAllBlocks() const;
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void Solve();

	void Lock();
	void Unlock();
	bool IsSolved() const;
	void TrySolve() ;

	// A 3D array implemented in a 1D array. Use FArray3D to handle translation.
	UPROPERTY()
	FPicrossBlockGrid MasterGrid;
	// Instanced Static Mesh Components for each EBlockState
	UPROPERTY()
	TMap<EBlockState, UInstancedStaticMeshComponent*> BlockInstances;
	// Meshes for each EBlockState
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Picross Block", meta = (AllowPrivateAccess = "true"))
	TMap<EBlockState, UStaticMesh*> BlockMeshes;
	// Materials for each EBlockState
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Picross Block", meta = (AllowPrivateAccess = "true"))
	TMap<EBlockState, UMaterialInstance*> BlockMaterials;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Picross Block", meta = (AllowPrivateAccess = "true"))
	UStaticMesh* HighlightMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Picross Block", meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* HighlightMaterial = nullptr;
	UPROPERTY()
	UInstancedStaticMeshComponent* HighlightedBlocks = nullptr;
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
