// Copyright Sanya Larsson 2020


#include "Engine/Engine.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "UnrealEd.h"
#include "PicrossBlock.h"
#include "PicrossGrid.h"
#include "PicrossPuzzleData.h"
#include "PicrossPuzzleFactory.h"

// Sets default values
APicrossGrid::APicrossGrid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void APicrossGrid::BeginPlay()
{
	Super::BeginPlay();
	
	CreateGrid(DefaultGridSize);
	SetRotationZAxis(0);
}

bool APicrossGrid::ValidateGridSize(FIntVector WantedGridSize) const
{
	return WantedGridSize.X > 0 && WantedGridSize.Y > 0 && WantedGridSize.Z > 0;
}

void APicrossGrid::CreateGrid(FIntVector WantedGridSize)
{
	if (!ValidateGridSize(WantedGridSize))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid grid size: %s"), *WantedGridSize.ToString())
		return;
	}
	else
	{
		GridSize = WantedGridSize;
	}

	if (PicrossGrid.Num() > 0)
	{
		DestroyGrid();
	}

	for (int32 i = 0; i < GridSize.Y * GridSize.X * GridSize.Z; ++i)
	{
		FVector Location = GetLocationForBlockCreation(i);
		int32 Index = PicrossGrid.Add(GetWorld()->SpawnActor<APicrossBlock>(PicrossBlockBP, Location, GetActorRotation()));
		if (PicrossGrid[Index])
		{
			PicrossGrid[Index]->SetIndexInGrid(Index);
		}
	}
}

FVector APicrossGrid::GetLocationForBlockCreation(int32 Index) const
{
	const int32 GridSizeXY = GridSize.X * GridSize.Y;
	FVector Location = FVector::ZeroVector;
	if (Index == 0) // Place the first block in the top left corner of the bottom-most 2D grid so that the grid actor is in the middle of the bottom-most grid.
	{
		const float OffsetX = DistanceBetweenBlocks * (GridSize.X / 2) - (GridSize.X % 2 == 0 ? DistanceBetweenBlocks / 2 : 0); // Get the offset for the first block in Y-axis.
		const float OffsetY = DistanceBetweenBlocks * (GridSize.Y / 2) - (GridSize.Y % 2 == 0 ? DistanceBetweenBlocks / 2 : 0); // Get the offset for the first block in X-axis.
		Location = GetActorLocation();
		Location -= GetActorRightVector() * OffsetY;
		Location -= GetActorForwardVector() * OffsetX;
	}
	else if (PicrossGrid.IsValidIndex((Index % GridSizeXY == 0 ? Index - GridSizeXY : Index % GridSize.Y == 0 ? Index - GridSize.Y : Index - 1))) // Find the index we intend to check and check if it's valid.
	{
		// Order of operation here is important.
		if (Index % GridSizeXY == 0 && PicrossGrid[Index - GridSizeXY] != nullptr) // First check if the index is the first block in a new 2D array layer.
		{
			APicrossBlock* Block = PicrossGrid[Index - GridSizeXY];
			Location = Block->GetActorLocation() + Block->GetActorUpVector() * DistanceBetweenBlocks;
		}
		else if (Index % GridSize.X == 0 && PicrossGrid[Index - GridSize.X] != nullptr) // Second check if the index is the first block in a row in a 2D array layer
		{
			APicrossBlock* Block = PicrossGrid[Index - GridSize.X];
			Location = Block->GetActorLocation() + Block->GetActorRightVector() * DistanceBetweenBlocks;
		}
		else if (PicrossGrid[Index - 1] != nullptr) // If neither of the previous checks were true it should be an index in the middle of a row.
		{
			APicrossBlock* Block = PicrossGrid[Index - 1];
			Location = Block->GetActorLocation() + Block->GetActorForwardVector() * DistanceBetweenBlocks;
		}
		else // If none of the previous checks were true then we had the index but the pointer at that index poitned to nullptr which should never happen.
		{
			UE_LOG(LogTemp, Error, TEXT("PicrossGrid contained nullptr!"))
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Grid attempted to get location for a block out-of-bounds! %i %i"), PicrossGrid.Num(), Index)
	}

	return Location;
}

void APicrossGrid::ClearGrid() const
{
	for (const APicrossBlock* Block : PicrossGrid)
	{
		if (Block)
		{
			Block->ResetBlock();
		}
	}
}

void APicrossGrid::DestroyGrid()
{
	for (APicrossBlock* Block : PicrossGrid)
	{
		if (Block)
		{
			Block->Destroy();
		}
	}

	PicrossGrid.Empty();
}

void APicrossGrid::Cycle2DRotation(const APicrossBlock* PivotBlock)
{
	SelectionAxis = (SelectionAxis == ESelectionAxis::Z ? ESelectionAxis::Y : SelectionAxis == ESelectionAxis::Y ? ESelectionAxis::X : ESelectionAxis::Z);

	if (PivotBlock)
	{
		int32 PivotIndex = PivotBlock->GetIndexInGrid();

		switch (SelectionAxis)
		{
		case X:		SetRotationXAxis(PivotIndex);	break;
		case Y:		SetRotationYAxis(PivotIndex);	break;
		case Z:		SetRotationZAxis(PivotIndex);	break;
		default:	SetRotationZAxis(PivotIndex);	break;
		}
	}
}

void APicrossGrid::SetRotationXAxis(int32 PivotIndex) const
{
	DisableAllBlocks();

	int32 StartIndex = PivotIndex % GridSize.X;
	int32 EndIndex = StartIndex + ((GridSize.Y * GridSize.Z) - 1) * GridSize.X;

	for (int32 i = StartIndex; i <= EndIndex; i += GridSize.X)
	{
		if (PicrossGrid.IsValidIndex(i) && PicrossGrid[i])
		{
			PicrossGrid[i]->SetEnabled(true);
		}
	}
}

void APicrossGrid::SetRotationYAxis(int32 PivotIndex) const
{
	DisableAllBlocks();

	int32 XY = GridSize.X * GridSize.Y;
	int32 LeftMostBlockInRowIndex = PivotIndex - PivotIndex % GridSize.X;
	int32 IndexesToStartIndex = FMath::FloorToInt(static_cast<float>(LeftMostBlockInRowIndex) / XY) * XY;
	int32 StartIndex = LeftMostBlockInRowIndex - IndexesToStartIndex;

	for (int32 i = 0; i < GridSize.Z; ++i)
	{
		for (int32 j = StartIndex + XY * i; j < (StartIndex + XY * i) + GridSize.X; ++j)
		{
			if (PicrossGrid.IsValidIndex(j) && PicrossGrid[j])
			{
				PicrossGrid[j]->SetEnabled(true);
			}
		}
	}
}

void APicrossGrid::SetRotationZAxis(int32 PivotIndex) const
{
	DisableAllBlocks();

	int32 XY = GridSize.X * GridSize.Y;
	int32 StartIndex = FMath::FloorToInt(static_cast<float>(PivotIndex) / XY) * XY;
	int32 EndIndex = StartIndex + XY - 1;

	for (int32 i = StartIndex; i <= EndIndex; ++i)
	{
		if (PicrossGrid.IsValidIndex(i) && PicrossGrid[i])
		{
			PicrossGrid[i]->SetEnabled(true);
		}
	}
}

void APicrossGrid::DisableAllBlocks() const
{
	for (APicrossBlock* Block : PicrossGrid)
	{
		Block->SetEnabled(false);
	}
}

void APicrossGrid::Move2DSelection() const
{
	// TODO: Implement function
}

void APicrossGrid::SavePuzzle() const
{
	UPicrossPuzzleData* ExistingObject = NewObject<UPicrossPuzzleData>();
	UPicrossPuzzleFactory* NewFactory = NewObject<UPicrossPuzzleFactory>();
	NewFactory->CreatedObjectAsset = ExistingObject;
	
	FAssetToolsModule& AssetToolsModule = FAssetToolsModule::GetModule();
	UObject* NewAsset = AssetToolsModule.Get().CreateAsset(NewFactory->GetSupportedClass(), NewFactory);
	TArray<UObject*> ObjectsToSync;
	ObjectsToSync.Add(NewAsset);
	GEditor->SyncBrowserToObjects(ObjectsToSync);
}