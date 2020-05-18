// Copyright Sanya Larsson 2020

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PicrossPuzzleData.generated.h"

/**
 * Class representing a Picross Puzzle.
 */
UCLASS()
class PICROSS_API UPicrossPuzzleData : public UDataAsset
{
	GENERATED_BODY()

public:
	FIntVector GetGridSize() const;
	TArray<bool> GetPicrossSolution() const;
	bool CheckIndex(int32 Index) const;
	bool ValidatePuzzle() const;
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	FIntVector GridSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	TArray<bool> PicrossSolution;
};
