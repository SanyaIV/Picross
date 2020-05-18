// Copyright Sanya Larsson 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PicrossGrid.generated.h"

UCLASS()
class PICROSS_API APicrossGrid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APicrossGrid();

	bool ValidateGridSize(FIntVector GridSize) const;
	void CreateGrid(FIntVector GridSize);
	void ClearGrid() const;
	void DestroyGrid();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	FVector GetLocationForBlockCreation(int32 Index, FIntVector GridSize) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class APicrossBlock> PicrossBlockBP;

	UPROPERTY(VisibleAnywhere, Category = "Picross")
	TArray<class APicrossBlock*> PicrossGrid;

	UPROPERTY(EditAnywhere, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	FIntVector DefaultGridSize {5,5,1};

	UPROPERTY(EditAnywhere, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	float DistanceBetweenBlocks = 110.f;
};
