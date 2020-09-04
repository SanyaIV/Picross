// Copyright Sanya Larsson 2020


#include "PicrossPuzzleData.h"

FIntVector UPicrossPuzzleData::GetGridSize() const
{
	return GridSize;
}

void UPicrossPuzzleData::SetGridSize(FIntVector NewGridSize)
{
	GridSize = NewGridSize;
}

const TArray<bool>& UPicrossPuzzleData::GetSolution() const
{
	return PicrossSolution;
}

void UPicrossPuzzleData::SetSolution(const TArray<bool> Solution)
{
	PicrossSolution = Solution;
}

bool UPicrossPuzzleData::ValidatePuzzle() const
{
	if (GridSize.X > 0 && GridSize.Y > 0 && GridSize.Z > 0)
	{
		int32 Size = GridSize.X * GridSize.Y * GridSize.Z;
		if (PicrossSolution.Num() == Size)
		{
			return true;
		}
	}

	return false;
}

FPrimaryAssetId UPicrossPuzzleData::GetPrimaryAssetId() const
{
	FPrimaryAssetId AssetId;
	AssetId.PrimaryAssetName = GetFName();
	AssetId.PrimaryAssetType = TEXT("PicrossPuzzleData");

	return AssetId;
}