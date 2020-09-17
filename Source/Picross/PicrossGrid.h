// Copyright Sanya Larsson 2020

#pragma once

// Default includes
#include "Components/TextRenderComponent.h"
#include "CoreMinimal.h"
#include "FArray3D.h"
#include "PicrossBlock.h"
#include "PicrossPuzzleData.h"
#include "GameFramework/Actor.h"
#include "PicrossGrid.generated.h"

// Forward declarations
class ATextRenderActor;
class UHierarchicalInstancedStaticMeshComponent;
class UMaterialInstance;
class UUserWidget;

UENUM()
enum class ESelectionAxis : uint8
{
	Z	UMETA(DisplayName = "Z"),
	Y	UMETA(DisplayName = "Y"),
	X	UMETA(DisplayName = "X"),
	All UMETA(DisplayName = "All")
};

/**
 * Struct representing the action taken on a block, used for undo/redo stack.
 */
USTRUCT()
struct FPicrossAction
{
	GENERATED_BODY();

	int32 StartBlockIndex;
	int32 EndBlockIndex;
	EBlockState PreviousState;
	EBlockState NewState;
};

/**
 * Struct representing a pair of opposing text actors, letting the player read it from both sides.
 */
USTRUCT(BlueprintType)
struct FTextPair
{
	GENERATED_BODY();

	UPROPERTY()
	ATextRenderActor* Text1 = nullptr;
	UPROPERTY()
	ATextRenderActor* Text2 = nullptr;
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
	FORCEINLINE FPicrossPuzzle() : Puzzle(nullptr) {}
	FORCEINLINE FPicrossPuzzle(UPicrossPuzzleData* Puzzle) : Puzzle(Puzzle) { if(Puzzle) Grid.SetNum(FArray3D::Size(Puzzle->GetGridSize())); }
	FORCEINLINE FPicrossPuzzle(const FPicrossPuzzle&) = default;
	FORCEINLINE FPicrossPuzzle(FPicrossPuzzle&&) = default;
	FORCEINLINE FPicrossPuzzle& operator=(const FPicrossPuzzle&) = default;
	FORCEINLINE FPicrossPuzzle& operator=(FPicrossPuzzle&&) = default;

	// Getters
	FORCEINLINE UPicrossPuzzleData* GetPuzzleData() { return Puzzle; }
	FORCEINLINE UPicrossPuzzleData const* const GetPuzzleData() const { return Puzzle; }
	FORCEINLINE FIntVector GetGridSize() const { return Puzzle ? Puzzle->GetGridSize() : FIntVector(INDEX_NONE); }
	FORCEINLINE int32 X() const { return Puzzle ? Puzzle->GetGridSize().X : INDEX_NONE; }
	FORCEINLINE int32 Y() const { return Puzzle ? Puzzle->GetGridSize().Y : INDEX_NONE; }
	FORCEINLINE int32 Z() const { return Puzzle ? Puzzle->GetGridSize().Z : INDEX_NONE; }
	FORCEINLINE TArray<FPicrossBlock>& GetGrid() { return Grid; }
	FORCEINLINE const TArray<FPicrossBlock>& GetGrid() const { return Grid; }
	FORCEINLINE int32 GetIndex(FIntVector ThreeDimensionalIndex) const { return Puzzle ? FArray3D::TranslateTo1D(Puzzle->GetGridSize(), ThreeDimensionalIndex) : INDEX_NONE; }
	FORCEINLINE FIntVector GetIndex(int32 OneDimensionalIndex) const { return Puzzle ? FArray3D::TranslateTo3D(Puzzle->GetGridSize(), OneDimensionalIndex) : FIntVector(INDEX_NONE); }
	FORCEINLINE bool IsValid() const { return (Puzzle != nullptr && FArray3D::ValidateDimensions(Puzzle->GetGridSize())); }

	// Ranged for redirection
	FORCEINLINE auto begin() { return Grid.begin(); }
	FORCEINLINE auto begin() const { return Grid.begin(); }
	FORCEINLINE auto end() { return Grid.end(); }
	FORCEINLINE auto end() const { return Grid.end(); }

	// Functions to access the array
	FORCEINLINE FPicrossBlock& operator[](int32 OneDimensionalIndex) { return Grid[OneDimensionalIndex]; }
	FORCEINLINE const FPicrossBlock& operator[](int32 OneDimensionalIndex) const { return Grid[OneDimensionalIndex]; }
	FORCEINLINE FPicrossBlock& operator[](FIntVector ThreeDimensionalIndex) { return Grid[GetIndex(ThreeDimensionalIndex)]; }
	FORCEINLINE const FPicrossBlock& operator[](FIntVector ThreeDimensionalIndex) const { return Grid[GetIndex(ThreeDimensionalIndex)]; }

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

	void OpenPuzzleBrowser() const;
	void ClosePuzzleBrowser() const;
	TArray<FAssetData> GetAllPuzzles() const;
	
	bool IsLocked() const;

	void UpdateBlocks(const int32 StartMasterIndex, const int32 EndMasterIndex, const EBlockState Action);
	void UpdateBlocks(const int32 StartMasterIndex, const int32 EndMasterIndex, const EBlockState PreviousState, const EBlockState NewState, const bool AddToStack = true);
	void HighlightBlocks(const int32 MasterIndexPivot);

	void Undo();
	void Redo();

	void Cycle2DRotation(const int32 MasterIndexPivot);
	void Move2DSelectionUp();
	void Move2DSelectionDown();

	UFUNCTION(BlueprintCallable, Category = "Picross")
	void LoadPuzzle(FAssetData PuzzleToLoad); 

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void CreateGrid();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void ClearGrid();
	void DestroyGrid();

	// The Picross Puzzle that we work with.
	UPROPERTY()
	FPicrossPuzzle Puzzle;

	// The total amount of filled blocks in the puzzle solution.
	int32 SolutionFilledBlocksCount = -1;
	int32 CurrentlyFilledBlocksCount = 0;

private:
	void CreatePuzzleBrowser();

	void GenerateNumbers();
	void GenerateNumbersForAxis(ESelectionAxis Axis);
	void CreateTextFromNumbersForAxis(ESelectionAxis Axis, int32 Axis1, int32 Axis2, const FFormatOrderedArguments& Numbers);
	ATextRenderActor* CreateTextRenderActor(FVector WorldLocation, FRotator RelativeRotation, FText Text, FColor Color, EHorizTextAligment HAlignment, EVerticalTextAligment VAlignment);
	void ForEachTextActor(const TFunctionRef<void(TPair<FIntVector, FTextPair>&)> Func);
	void CleanupNumbers();
	void UpdateNumbersVisibility();
	void RotateNumbersXAxis();
	void RotateNumbersYAxis();
	void RotateNumbersZAxis();
	void RotateNumbersAllAxis();

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

	void Lock();
	void Unlock();
	bool IsSolved() const;
	void TrySolve() ;

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
	UMaterialInstance* NumbersTextMaterial = nullptr;
	UPROPERTY()
	TMap<FIntVector, FTextPair> NumbersXAxis;
	UPROPERTY()
	TMap<FIntVector, FTextPair> NumbersYAxis;
	UPROPERTY()
	TMap<FIntVector, FTextPair> NumbersZAxis;

	UPROPERTY()
	TArray<FPicrossAction> UndoStack;
	UPROPERTY()
	TArray<FPicrossAction> RedoStack;

	// The distance to use between the blocks when spawning them.
	UPROPERTY(EditAnywhere, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	float DistanceBetweenBlocks = 102.f;

	// Keeps track of the current axis of selection.
	ESelectionAxis SelectionAxis = ESelectionAxis::All;
	// Caches the last index used to pivot so we can do that operation without a new pivot-index.
	FIntVector LastPivotXYZ = FIntVector::ZeroValue;
	// Lock that we can set when solution has been found so the player can't edit finished puzzles.
	bool bLocked = false;

	// TODO: Move to game mode, alternatively level, shouldn't be in the Grid class.
	// The blueprint widget class that we use to create the puzzle browser widget.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> PuzzleBrowserWidgetClass = nullptr;
	// Pointer to the created puzzle browser widget.
	UPROPERTY()
	UUserWidget* PuzzleBrowserWidget = nullptr;
};
