// Copyright Sanya Larsson 2020


#include "AssetDataObject.h"
#include "PicrossGrid.h"

void UAssetDataObject::SetAssetData(FAssetData NewAssetData)
{
	AssetData = NewAssetData;
}

FAssetData UAssetDataObject::GetAssetData() const
{
	return AssetData;
}

void UAssetDataObject::SetPicrossGrid(APicrossGrid* NewPicrossGrid)
{
	PicrossGrid = NewPicrossGrid;
}

APicrossGrid* UAssetDataObject::GetPicrossGrid() const
{
	return PicrossGrid;
}

FIntVector UAssetDataObject::GetGridSize() const
{
	if (AssetData.TagsAndValues.Contains(TEXT("GridSize")))
	{
		FString GridSizeString = AssetData.TagsAndValues.FindChecked(TEXT("GridSize"));
		FIntVector GridSize;
		swscanf_s(*GridSizeString, TEXT("(X=%d,Y=%d,Z=%d)"), &GridSize.X, &GridSize.Y, &GridSize.Z);
		return GridSize;
	}
	
	return FIntVector::NoneValue;
}

FString UAssetDataObject::GetGridSizeString() const
{
	FIntVector GridSize = GetGridSize();
	return FString::Printf(TEXT("(%d, %d, %d)"), GridSize.X, GridSize.Y, GridSize.Z);
}
