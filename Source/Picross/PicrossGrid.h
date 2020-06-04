// Copyright Sanya Larsson 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TArray3D.h"
#include "PicrossGrid.generated.h"


UENUM()
enum ESelectionAxis
{
	Z	UMETA(DisplayName = "Z"),
	Y	UMETA(DisplayName = "Y"),
	X	UMETA(DisplayName = "X")
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

	bool ValidateGridSize(FIntVector WantedGridSize) const;
	void CreateGrid(FIntVector WantedGridSize);
	void ClearGrid() const;
	void DestroyGrid();

	void Cycle2DRotation(const class APicrossBlock* PivotBlock);
	void Move2DSelection() const;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void SavePuzzle() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	FVector GetLocationForBlockCreation(int32 Index) const;
	void SetRotationXAxis(int32 PivotIndex) const;
	void SetRotationYAxis(int32 PivotIndex) const;
	void SetRotationZAxis(int32 PivotIndex) const;
	void DisableAllBlocks() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class APicrossBlock> PicrossBlockBP;

	// A 3D array implemented in a 1D array. Functions handle the 3D aspect of it.
	UPROPERTY(VisibleAnywhere, Category = "Picross")
	TArray<class APicrossBlock*> PicrossGrid;
	FIntVector GridSize = FIntVector::ZeroValue;

	UPROPERTY(EditAnywhere, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	FIntVector DefaultGridSize {5,5,1};

	UPROPERTY(EditAnywhere, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	float DistanceBetweenBlocks = 110.f;

	ESelectionAxis SelectionAxis = ESelectionAxis::Z;
};
