// Copyright Sanya Larsson 2020


#include "Engine/AssetManager.h"
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
	
	LoadPuzzle();
	CreateGrid();
}

void APicrossGrid::CreateGrid()
{
	GridSize = CurrentPuzzle ? CurrentPuzzle->GetGridSize() : DefaultGridSize;
	verify(FArray3D::ValidateDimensions(GridSize));

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

	GenerateNumbers();
}

void APicrossGrid::ClearGrid() const
{
	for (APicrossBlock* Block : PicrossGrid)
	{
		if (Block)
		{
			Block->ClearBlock();
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

void APicrossGrid::GenerateNumbers() const
{
	if (!CurrentPuzzle) return;

	const TArray<bool>& Solution = CurrentPuzzle->GetSolution();

	// Generate numbers for X-axis
	for (int32 Y = 0; Y < GridSize.Y; ++Y)
	{
		for (int32 Z = 0; Z < GridSize.Z; ++Z)
		{
			TArray<int32> Numbers;
			int32 Sum = 0;

			for (int32 X = 0; X < GridSize.X; ++X)
			{
				if (Solution[FArray3D::TranslateTo1D(GridSize, X, Y, Z)])
				{
					++Sum;
				}
				else if (Sum > 0)
				{
					Numbers.Add(Sum);
					Sum = 0;
				}
			}
		}
	}

	// Generate numbers for Y-axis
	for (int32 X = 0; X < GridSize.X; ++X)
	{
		for (int32 Z = 0; Z < GridSize.Z; ++Z)
		{
			TArray<int32> Numbers;
			int32 Sum = 0;

			for (int32 Y = 0; Y < GridSize.Y; ++Y)
			{
				if (Solution[FArray3D::TranslateTo1D(GridSize, X, Y, Z)])
				{
					++Sum;
				}
				else if (Sum > 0)
				{
					Numbers.Add(Sum);
					Sum = 0;
				}
			}
		}
	}

	// Generate numbers for Z-axis
	for (int32 X = 0; X < GridSize.X; ++X)
	{
		for (int32 Y = 0; Y < GridSize.Y; ++Y)
		{
			TArray<int32> Numbers;
			int32 Sum = 0;

			for (int32 Z = GridSize.Z - 1; Z >= 0; --Z)
			{
				if (Solution[FArray3D::TranslateTo1D(GridSize, X, Y, Z)])
				{
					++Sum;
				}
				else if (Sum > 0)
				{
					Numbers.Add(Sum);
					Sum = 0;
				}
			}
		}
	}
}

void APicrossGrid::Cycle2DRotation(const APicrossBlock* PivotBlock)
{
	SelectionAxis = (SelectionAxis == ESelectionAxis::All ? ESelectionAxis::Z : SelectionAxis == ESelectionAxis::Z ? ESelectionAxis::Y : SelectionAxis == ESelectionAxis::Y ? ESelectionAxis::X : ESelectionAxis::All);

	if (PivotBlock)
	{
		int32 PivotIndex = PivotBlock->GetIndexInGrid();

		switch (SelectionAxis)
		{
		case X:		SetRotationXAxis(PivotIndex);	break;
		case Y:		SetRotationYAxis(PivotIndex);	break;
		case Z:		SetRotationZAxis(PivotIndex);	break;
		default:	EnableAllBlocks();	break;
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

bool APicrossGrid::IsSolved() const
{
	const TArray<bool>& Solution = CurrentPuzzle->GetSolution();

	if (PicrossGrid.Num() == Solution.Num())
	{
		for (int32 i = 0; i < Solution.Num(); ++i)
		{
			if (PicrossGrid[i]->IsFilled() != Solution[i])
			{
				return false;
			}
		}
	}

	return true;
}

void APicrossGrid::Move2DSelectionUp()
{
	switch (SelectionAxis)
	{
	case X:
		LastPivotXYZ.X = LastPivotXYZ.X < GridSize.X - 1 ? LastPivotXYZ.X + 1 : 0;
		SetRotationXAxis(FArray3D::TranslateTo1D(GridSize, LastPivotXYZ));
		break;
	case Y:
		LastPivotXYZ.Y = LastPivotXYZ.Y < GridSize.Y - 1 ? LastPivotXYZ.Y + 1 : 0;
		SetRotationYAxis(FArray3D::TranslateTo1D(GridSize, LastPivotXYZ));
		break;
	case Z:
		LastPivotXYZ.Z = LastPivotXYZ.Z < GridSize.Z - 1 ? LastPivotXYZ.Z + 1 : 0;
		SetRotationZAxis(FArray3D::TranslateTo1D(GridSize, LastPivotXYZ));
		break;
	}
}

void APicrossGrid::Move2DSelectionDown()
{
	switch (SelectionAxis)
	{
	case X:
		LastPivotXYZ.X = LastPivotXYZ.X > 0 ? LastPivotXYZ.X - 1 : GridSize.X - 1;
		SetRotationXAxis(FArray3D::TranslateTo1D(GridSize, LastPivotXYZ));
		break;
	case Y:
		LastPivotXYZ.Y = LastPivotXYZ.Y > 0 ? LastPivotXYZ.Y - 1 : GridSize.Y - 1;
		SetRotationYAxis(FArray3D::TranslateTo1D(GridSize, LastPivotXYZ));
		break;
	case Z:
		LastPivotXYZ.Z = LastPivotXYZ.Z > 0 ? LastPivotXYZ.Z - 1 : GridSize.Z - 1;
		SetRotationZAxis(FArray3D::TranslateTo1D(GridSize, LastPivotXYZ));
		break;
	}
}

void APicrossGrid::SavePuzzle() const
{
	if (!FArray3D::ValidateDimensions(GridSize)) return;

	TArray<bool> Solution;
	for (APicrossBlock* Block : PicrossGrid)
	{
		Solution.Add(Block->IsFilled());
	}

	UPicrossPuzzleData* ExistingObject = NewObject<UPicrossPuzzleData>();
	ExistingObject->SetGridSize(GridSize);
	ExistingObject->SetSolution(Solution);

	UPicrossPuzzleFactory* NewFactory = NewObject<UPicrossPuzzleFactory>();
	NewFactory->CreatedObjectAsset = ExistingObject;
	
	FAssetToolsModule& AssetToolsModule = FAssetToolsModule::GetModule();
	UObject* NewAsset = AssetToolsModule.Get().CreateAssetWithDialog(NewFactory->GetSupportedClass(), NewFactory);
	TArray<UObject*> ObjectsToSync;
	ObjectsToSync.Add(NewAsset);
	GEditor->SyncBrowserToObjects(ObjectsToSync);
}

bool APicrossGrid::LoadPuzzle()
{
	UObjectLibrary* ObjectLibrary = nullptr;
	if (!ObjectLibrary)
	{
		UAssetManager& AssetManager = UAssetManager::Get();
		TArray<FAssetData> AssetDatas;
		AssetManager.GetPrimaryAssetDataList(TEXT("PicrossPuzzleData"), AssetDatas);
		UE_LOG(LogTemp, Warning, TEXT("Found %d puzzles"), AssetDatas.Num())
		
		for (FAssetData AssetData : AssetDatas)
		{
			CurrentPuzzle = Cast<UPicrossPuzzleData>(AssetData.GetAsset());
			if (CurrentPuzzle) return true;
		}
	}

	return false;
}