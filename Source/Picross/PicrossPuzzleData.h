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
	bool ValidatePuzzle() const;

	FIntVector GetGridSize() const;
	void SetGridSize(FIntVector NewGridSize);

	const TArray<bool>& GetSolution() const;
	void SetSolution(const TArray<bool> Solution);

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Picross", AssetRegistrySearchable, meta = (AllowPrivateAccess = "true"))
	FIntVector GridSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	TArray<bool> PicrossSolution;
};
