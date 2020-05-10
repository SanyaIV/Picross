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

	void CreateGrid(int32 GridSize);
	void ClearGrid() const;
	void DestroyGrid();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	FVector GetLocationForBlockCreation(int32 Index, int32 GridSize) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class APicrossBlock> PicrossBlockBP;

	UPROPERTY(VisibleAnywhere, Category = "Picross")
	TArray<class APicrossBlock*> PicrossGrid;
};
