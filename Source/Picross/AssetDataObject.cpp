// Copyright Sanya Larsson 2020


#include "AssetDataObject.h"

void UAssetDataObject::SetAssetData(FAssetData NewAssetData)
{
	AssetData = NewAssetData;
}

FAssetData UAssetDataObject::GetAssetData() const
{
	return AssetData;
}

FIntVector UAssetDataObject::GetGridSize() const
{
	if (AssetData.TagsAndValues.Contains(TEXT("GridSize")))
	{
		FString GridSizeString = AssetData.TagsAndValues.FindChecked(TEXT("GridSize"));
		FIntVector GridSize;
#if !PLATFORM_WINDOWS
		swscanf(TCHAR_TO_WCHAR(*GridSizeString), L"(X=%d,Y=%d,Z=%d)", &GridSize.X, &GridSize.Y, &GridSize.Z);
#else
		swscanf_s(*GridSizeString, TEXT("(X=%d,Y=%d,Z=%d)"), &GridSize.X, &GridSize.Y, &GridSize.Z);
#endif
		return GridSize;
	}
	
	return FIntVector::NoneValue;
}

FString UAssetDataObject::GetGridSizeString() const
{
	FIntVector GridSize = GetGridSize();
	return FString::Printf(TEXT("(%d, %d, %d)"), GridSize.X, GridSize.Y, GridSize.Z);
}
