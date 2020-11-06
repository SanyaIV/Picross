// Copyright Sanya Larsson 2020


#include "PuzzleBrowserWidget.h"
#include "../AssetDataObject.h" 
#include "FArray3D.h"
#include "Algo/Sort.h"
#include "Engine/AssetManager.h"
#include "Engine/ObjectLibrary.h"
#include "../PicrossGrid.h"


TArray<UAssetDataObject*> UPuzzleBrowserWidget::GetPuzzles()
{
	TArray<UAssetDataObject*> AssetDataObjects;
	for (FAssetData AssetData : GetPuzzleDatas())
	{
		UAssetDataObject* AssetDataObject = NewObject<UAssetDataObject>(this, UAssetDataObject::StaticClass());
		AssetDataObject->SetAssetData(AssetData);
		AssetDataObjects.Push(AssetDataObject);
	}

	Algo::Sort(AssetDataObjects, [](UAssetDataObject* A, UAssetDataObject* B) { return (A && B) ? (FArray3D::Size(A->GetGridSize()) < FArray3D::Size(B->GetGridSize())) : false; });
	return AssetDataObjects;
}

TArray<FAssetData> UPuzzleBrowserWidget::GetPuzzleDatas() const
{
	UObjectLibrary* ObjectLibrary = nullptr;
	if (!ObjectLibrary)
	{
		UAssetManager& AssetManager = UAssetManager::Get();
		TArray<FAssetData> AssetDatas;
		AssetManager.GetPrimaryAssetDataList(TEXT("PicrossPuzzleData"), AssetDatas);
		return AssetDatas;
	}

	return TArray<FAssetData>();
}