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
		TArray<FString> StringArray;
		AssetData.TagsAndValues.FindChecked(TEXT("GridSize")).ParseIntoArray(StringArray, TEXT(","));

		if (StringArray.Num() != 3) return FIntVector::NoneValue;

		StringArray[0].RemoveFromStart(TEXT("(X="));
		StringArray[0].RemoveFromEnd(TEXT(","));
		StringArray[1].RemoveFromStart(TEXT("Y="));
		StringArray[1].RemoveFromEnd(TEXT(","));
		StringArray[2].RemoveFromStart(TEXT("Z="));
		StringArray[2].RemoveFromEnd(TEXT(")"));

		return FIntVector
		{
			StringArray[0].IsNumeric() ? FCString::Atoi(*StringArray[0]) : INDEX_NONE,
			StringArray[1].IsNumeric() ? FCString::Atoi(*StringArray[1]) : INDEX_NONE,
			StringArray[2].IsNumeric() ? FCString::Atoi(*StringArray[2]) : INDEX_NONE
		};
	}
	
	return FIntVector::NoneValue;
}

FString UAssetDataObject::GetGridSizeString() const
{
	FIntVector GridSize = GetGridSize();
	return FString::Printf(TEXT("(%d, %d, %d)"), GridSize.X, GridSize.Y, GridSize.Z);
}
