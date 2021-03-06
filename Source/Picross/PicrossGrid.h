// Copyright Sanya Larsson 2020

#pragma once

// KBM_Default includes
#include "Components/TextRenderComponent.h"
#include "CoreMinimal.h"
#include "FArray3D.h"
#include "PicrossBlock.h"
#include "PicrossPuzzleData.h"
#include "GameFramework/Actor.h"
#include "Misc/Optional.h"

// Generated
#include "PicrossGrid.generated.h"

// Forward declarations
class APicrossNumber;
class ATextRenderActor;
class UHierarchicalInstancedStaticMeshComponent;

/**
 * Struct representing the action taken on a single block.
 */
USTRUCT()
struct FPicrossBlockAction
{
	GENERATED_BODY();

	UPROPERTY()
	FIntVector BlockIndex;
	UPROPERTY()
	EBlockState PreviousState;
	UPROPERTY()
	EBlockState NewState;
};

/**
 * Struct representing the action taken on a collection of blocks, used for undo/redo stack.
 */
USTRUCT()
struct FPicrossAction
{
	GENERATED_BODY();

	UPROPERTY()
	TArray<FPicrossBlockAction> Actions;
};

/**
 * Struct representing a 3D collection of FPicrossBlock, has a GridSize and Array.
 */
USTRUCT(BlueprintType)
struct FPicrossPuzzle
{
	GENERATED_BODY();

public:
	// Constructors & Assignments
	FPicrossPuzzle() : Puzzle(nullptr) {}
	FPicrossPuzzle(UPicrossPuzzleData* Puzzle) : Puzzle(Puzzle) { if(Puzzle) Grid.SetNum(FArray3D::Size(Puzzle->GetGridSize())); }
	FPicrossPuzzle(const FPicrossPuzzle&) = default;
	FPicrossPuzzle(FPicrossPuzzle&&) = default;
	FPicrossPuzzle& operator=(const FPicrossPuzzle&) = default;
	FPicrossPuzzle& operator=(FPicrossPuzzle&&) = default;

	// Getters
	UPicrossPuzzleData* GetPuzzleData() { return Puzzle; }
	UPicrossPuzzleData const* const GetPuzzleData() const { return Puzzle; }
	FIntVector GetGridSize() const { return Puzzle ? Puzzle->GetGridSize() : FIntVector(INDEX_NONE); }
	int32 X() const { return Puzzle ? Puzzle->GetGridSize().X : INDEX_NONE; }
	int32 Y() const { return Puzzle ? Puzzle->GetGridSize().Y : INDEX_NONE; }
	int32 Z() const { return Puzzle ? Puzzle->GetGridSize().Z : INDEX_NONE; }
	TArray<FPicrossBlock>& GetGrid() { return Grid; }
	const TArray<FPicrossBlock>& GetGrid() const { return Grid; }
	int32 GetIndex(FIntVector ThreeDimensionalIndex) const { return Puzzle ? FArray3D::TranslateTo1D(Puzzle->GetGridSize(), ThreeDimensionalIndex) : INDEX_NONE; }
	FIntVector GetIndex(int32 OneDimensionalIndex) const { return Puzzle ? FArray3D::TranslateTo3D(Puzzle->GetGridSize(), OneDimensionalIndex) : FIntVector(INDEX_NONE); }
	bool IsValid() const { return (Puzzle != nullptr && FArray3D::ValidateDimensions(Puzzle->GetGridSize())); }

	// Ranged for redirection
	auto begin() { return Grid.begin(); }
	auto begin() const { return Grid.begin(); }
	auto end() { return Grid.end(); }
	auto end() const { return Grid.end(); }

	// Functions to access the array
	FPicrossBlock& operator[](int32 OneDimensionalIndex) { return Grid[OneDimensionalIndex]; }
	const FPicrossBlock& operator[](int32 OneDimensionalIndex) const { return Grid[OneDimensionalIndex]; }
	FPicrossBlock& operator[](FIntVector ThreeDimensionalIndex) { return Grid[GetIndex(ThreeDimensionalIndex)]; }
	const FPicrossBlock& operator[](FIntVector ThreeDimensionalIndex) const { return Grid[GetIndex(ThreeDimensionalIndex)]; }

	float DynamicScale = 1.f;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Picross Grid", meta = (AllowPrivateAccess = "true"))
	UPicrossPuzzleData* Puzzle;
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
	
	bool IsLocked() const;

	void UpdateBlocks(const int32 StartMasterIndex, const int32 EndMasterIndex, const EBlockState Action);
	void UpdateBlocks(const int32 StartMasterIndex, const int32 EndMasterIndex, const EBlockState PreviousState, const EBlockState NewState);

	void Undo();
	void Redo();

	void Cycle2DRotation();
	void Move2DSelectionUp();
	void Move2DSelectionDown();

	void SetFocusedBlock(const int32 MasterIndex);
	int32 GetFocusedBlockIndex() const { return Puzzle.GetIndex(FocusedBlock); }
	void MoveFocusUp();
	void MoveFocusDown();
	void MoveFocusLeft();
	void MoveFocusRight();

	TOptional<FTransform> GetIdealPawnTransform(const APawn* Pawn) const;

	UFUNCTION(BlueprintCallable, Category = "Picross")
	void LoadPuzzle(FAssetData PuzzleToLoad);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSolvedEvent);
	FSolvedEvent& OnSolved() { return SolvedEvent; }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPuzzleLoaded);
	FPuzzleLoaded& OnPuzzleLoaded() { return PuzzleLoaded; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void CreateGrid();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void ClearGrid();
	void DestroyGrid();

	// The Picross Puzzle that we work with.
	UPROPERTY()
	FPicrossPuzzle Puzzle;

	// The total amount of filled blocks in the puzzle solution.
	int32 SolutionFilledBlocksCount = -1;
	// The total amount of filled blocks in the puzzle.
	int32 CurrentlyFilledBlocksCount = 0;

private:
	void GenerateNumbers();
	void GenerateNumbersForAxis(const EAxis::Type Axis);
	void CreatePicrossNumber(const EAxis::Type Axis, int32 Axis1, int32 Axis2, const FFormatOrderedArguments& Numbers);
	void ForEachPicrossNumber(const TFunctionRef<void(TPair<FIntVector, APicrossNumber*>&)> Func);
	void CleanupNumbers();
	void UpdateNumbersVisibility();

	void SetRotationXAxis();
	void SetRotationYAxis();
	void SetRotationZAxis();

	void UpdateBlockState(FPicrossBlock& Block, const EBlockState NewState);
	void CreateBlockInstance(FPicrossBlock& Block) const;
	void HighlightBlocks();
	void HighlightBlocksInAxis(const EAxis::Type AxisToHighlight);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void EnableOnlyFilledBlocks();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void EnableAllBlocks();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void DisableAllBlocks();

	void Lock();
	void Unlock();
	bool IsSolved() const;
	void TrySolve() ;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void SaveGame() const;
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void LoadGame();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void DeleteSaveGame() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Picross Block", meta = (AllowPrivateAccess = "true"))
	TMap<EBlockState, UStaticMesh*> BlockMeshes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Picross Block", meta = (AllowPrivateAccess = "true"))
	TMap<EBlockState, UMaterialInstance*> BlockMaterials;
	UPROPERTY()
	TMap<EBlockState, UHierarchicalInstancedStaticMeshComponent*> BlockInstances;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Picross Block", meta = (AllowPrivateAccess = "true"))
	UStaticMesh* HighlightMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Picross Block", meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* HighlightMaterial = nullptr;
	UPROPERTY()
	UHierarchicalInstancedStaticMeshComponent* HighlightedBlocks = nullptr;

	UPROPERTY(EditAnywhere, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<APicrossNumber> PicrossNumberClass = nullptr;
	UPROPERTY()
	TMap<FIntVector, APicrossNumber*> NumbersXAxis;
	UPROPERTY()
	TMap<FIntVector, APicrossNumber*> NumbersYAxis;
	UPROPERTY()
	TMap<FIntVector, APicrossNumber*> NumbersZAxis;

	UPROPERTY()
	TArray<FPicrossAction> UndoStack;
	UPROPERTY()
	TArray<FPicrossAction> RedoStack;

	// The distance to use between the blocks when spawning them.
	UPROPERTY(EditAnywhere, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	float DistanceBetweenBlocks = 5.f;

	// Keeps track of the current axis of selection.
	TEnumAsByte<EAxis::Type> SelectionAxis = EAxis::None;
	// Index for focused block, will be used as pivot for example.
	FIntVector FocusedBlock = FIntVector::ZeroValue;
	// Lock that we can set when solution has been found so the player can't edit finished puzzles.
	bool bLocked = false;

	UPROPERTY(BlueprintAssignable, Category = "Picross")
	FSolvedEvent SolvedEvent;

	UPROPERTY(BlueprintAssignable, Category = "Picross")
	FPuzzleLoaded PuzzleLoaded;
};
