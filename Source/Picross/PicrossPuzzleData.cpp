// Copyright Sanya Larsson 2020


#include "PicrossPuzzleData.h"

FIntVector UPicrossPuzzleData::GetGridSize() const
{
	return GridSize;
}

const TArray<bool>& UPicrossPuzzleData::GetSolution() const
{
	return PicrossSolution;
}

bool UPicrossPuzzleData::CheckIndex(int32 Index) const
{
	if (PicrossSolution.IsValidIndex(Index))
	{
		return PicrossSolution[Index];
	}

	UE_LOG(LogTemp, Error, TEXT("Invalid index while checking PicrossPuzzleData for index: %i"), Index)
	return false;
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