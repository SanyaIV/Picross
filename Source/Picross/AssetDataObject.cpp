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

