// Copyright Sanya Larsson 2020


#include "Engine/Engine.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "UnrealEd.h"
#include "PicrossBlock.h"
#include "PicrossGrid.h"
#include "PicrossPuzzleData.h"
#include "PicrossPuzzleFactory.h"
#include "FArray3D.h"

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
}

bool APicrossGrid::ValidateGridSize(FIntVector WantedGridSize) const
{
	return WantedGridSize.X > 0 && WantedGridSize.Y > 0 && WantedGridSize.Z > 0;
}

void APicrossGrid::CreateGrid(FIntVector WantedGridSize)
{
	verify(FArray3D::ValidateDimensions(WantedGridSize));

	GridSize = WantedGridSize;

	if (PicrossGrid.Num() > 0)
	{
		DestroyGrid();
	}

	PicrossGrid.SetNumZeroed(FArray3D::Size(GridSize));

	FVector StartPosition = GetActorLocation();
	StartPosition -= GetActorRightVector() * (DistanceBetweenBlocks * (GridSize.Y / 2) - (GridSize.Y % 2 == 0 ? DistanceBetweenBlocks / 2 : 0));
	StartPosition -= GetActorForwardVector() * (DistanceBetweenBlocks * (GridSize.X / 2) - (GridSize.X % 2 == 0 ? DistanceBetweenBlocks / 2 : 0));

	for (int32 Z = 0; Z < GridSize.Z; ++Z)
	{
		float OffsetZ = DistanceBetweenBlocks * Z;
		for (int32 Y = 0; Y < GridSize.Y; ++Y)
		{
			float OffsetY = DistanceBetweenBlocks * Y;
			for (int32 X = 0; X < GridSize.X; ++X)
			{
				float OffsetX = DistanceBetweenBlocks * X;
				FVector BlockPosition = StartPosition;
				BlockPosition += GetActorForwardVector() * OffsetX;
				BlockPosition += GetActorRightVector() * OffsetY;
				BlockPosition += GetActorUpVector() * OffsetZ;

				int32 Index = FArray3D::TranslateTo1D(GridSize, X, Y, Z);
				if (PicrossGrid.IsValidIndex(Index))
				{
					PicrossGrid[Index] = GetWorld()->SpawnActor<APicrossBlock>(PicrossBlockBP, BlockPosition, GetActorRotation());
					if (PicrossGrid[Index])
					{
						PicrossGrid[Index]->SetIndexInGrid(Index);
					}
				}
				
			}
		}
	}
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

void APicrossGrid::SetRotationXAxis(int32 PivotIndex)
{
	DisableAllBlocks();

	LastPivotXYZ = FArray3D::TranslateTo3D(GridSize, PivotIndex);
	for (int32 Z = 0; Z < GridSize.Z; ++Z)
	{
		for (int32 Y = 0; Y < GridSize.Y; ++Y)
		{
			int32 Index = FArray3D::TranslateTo1D(GridSize, LastPivotXYZ.X, Y, Z);
			if (PicrossGrid.IsValidIndex(Index) && PicrossGrid[Index])
			{
				PicrossGrid[Index]->SetEnabled(true);
			}
		}
	}
}

void APicrossGrid::SetRotationYAxis(int32 PivotIndex)
{
	DisableAllBlocks();

	LastPivotXYZ = FArray3D::TranslateTo3D(GridSize, PivotIndex);
	for (int32 Z = 0; Z < GridSize.Z; ++Z)
	{
		for (int32 X = 0; X < GridSize.X; ++X)
		{
			int32 Index = FArray3D::TranslateTo1D(GridSize, X, LastPivotXYZ.Y, Z);
			if (PicrossGrid.IsValidIndex(Index) && PicrossGrid[Index])
			{
				PicrossGrid[Index]->SetEnabled(true);
			}
		}
	}
}

void APicrossGrid::SetRotationZAxis(int32 PivotIndex)
{
	DisableAllBlocks();

	LastPivotXYZ = FArray3D::TranslateTo3D(GridSize, PivotIndex);
	for (int32 Y = 0; Y < GridSize.Y; ++Y)
	{
		for (int32 X = 0; X < GridSize.X; ++X)
		{
			int32 Index = FArray3D::TranslateTo1D(GridSize, X, Y, LastPivotXYZ.Z);
			if (PicrossGrid.IsValidIndex(Index) && PicrossGrid[Index])
			{
				PicrossGrid[Index]->SetEnabled(true);
			}
		}
	}
}

void APicrossGrid::EnableAllBlocks() const
{
	for (APicrossBlock* Block : PicrossGrid)
	{
		Block->SetEnabled(true);
	}
}

void APicrossGrid::DisableAllBlocks() const
{
	for (APicrossBlock* Block : PicrossGrid)
	{
		Block->SetEnabled(false);
	}
}

void APicrossGrid::Move2DSelectionUp()
{
	switch (SelectionAxis)
	{
	case X:
		LastPivotXYZ.X = LastPivotXYZ.X < GridSize.X ? LastPivotXYZ.X + 1 : 0;
		SetRotationXAxis(FArray3D::TranslateTo1D(GridSize, LastPivotXYZ));
		break;
	case Y:
		LastPivotXYZ.Y = LastPivotXYZ.Y < GridSize.Y ? LastPivotXYZ.Y + 1 : 0;
		SetRotationYAxis(FArray3D::TranslateTo1D(GridSize, LastPivotXYZ));
		break;
	case Z:
		LastPivotXYZ.Z = LastPivotXYZ.Z < GridSize.Z ? LastPivotXYZ.Z + 1 : 0;
		SetRotationZAxis(FArray3D::TranslateTo1D(GridSize, LastPivotXYZ));
		break;
	}
}

void APicrossGrid::Move2DSelectionDown()
{
	switch (SelectionAxis)
	{
	case X:
		LastPivotXYZ.X = LastPivotXYZ.X > 0 ? LastPivotXYZ.X - 1 : GridSize.X;
		SetRotationXAxis(FArray3D::TranslateTo1D(GridSize, LastPivotXYZ));
		break;
	case Y:
		LastPivotXYZ.Y = LastPivotXYZ.Y > 0 ? LastPivotXYZ.Y - 1 : GridSize.Y;
		SetRotationYAxis(FArray3D::TranslateTo1D(GridSize, LastPivotXYZ));
		break;
	case Z:
		LastPivotXYZ.Z = LastPivotXYZ.Z > 0 ? LastPivotXYZ.Z - 1 : GridSize.Z;
		SetRotationZAxis(FArray3D::TranslateTo1D(GridSize, LastPivotXYZ));
		break;
	}
}

void APicrossGrid::SavePuzzle() const
{
	UPicrossPuzzleData* ExistingObject = NewObject<UPicrossPuzzleData>();
	UPicrossPuzzleFactory* NewFactory = NewObject<UPicrossPuzzleFactory>();
	NewFactory->CreatedObjectAsset = ExistingObject;
	
	FAssetToolsModule& AssetToolsModule = FAssetToolsModule::GetModule();
	UObject* NewAsset = AssetToolsModule.Get().CreateAssetWithDialog(NewFactory->GetSupportedClass(), NewFactory);
	TArray<UObject*> ObjectsToSync;
	ObjectsToSync.Add(NewAsset);
	GEditor->SyncBrowserToObjects(ObjectsToSync);
}