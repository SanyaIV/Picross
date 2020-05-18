// Copyright Sanya Larsson 2020


#include "PicrossBlock.h"
#include "PicrossGrid.h"

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

bool APicrossGrid::ValidateGridSize(FIntVector GridSize) const
{
	return (GridSize.X > 0 && GridSize.Y > 0 && GridSize.Z > 0);
}

void APicrossGrid::CreateGrid(FIntVector GridSize)
{
	if (!ValidateGridSize(GridSize))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid grid size: %s"), *GridSize.ToString())
		return;
	}

	if (PicrossGrid.Num() > 0)
	{
		DestroyGrid();
	}

	for (int32 i = 0; i < GridSize.X * GridSize.Y * GridSize.Z; ++i)
	{
		FVector Location = GetLocationForBlockCreation(i, GridSize);
		PicrossGrid.Add(GetWorld()->SpawnActor<APicrossBlock>(PicrossBlockBP, Location, GetActorRotation()));
	}
}

FVector APicrossGrid::GetLocationForBlockCreation(int32 Index, FIntVector GridSize) const
{
	const int32 GridSizeXY = GridSize.X * GridSize.Y;
	FVector Location = FVector::ZeroVector;
	if (Index == 0) // Place the first block in the top left corner of the bottom-most 2D grid so that the grid actor is in the middle of the bottom-most grid.
	{
		const float OffsetY = DistanceBetweenBlocks * (GridSize.X / 2) - (GridSize.X % 2 == 0 ? DistanceBetweenBlocks / 2 : 0); // Get the offset for the first block in Y-axis.
		const float OffsetX = DistanceBetweenBlocks * (GridSize.Y / 2) - (GridSize.Y % 2 == 0 ? DistanceBetweenBlocks / 2 : 0); // Get the offset for the first block in X-axis.
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
		else if (Index % GridSize.Y == 0 && PicrossGrid[Index - GridSize.Y] != nullptr) // Second check if the index is the first block in a row in a 2D array layer
		{
			APicrossBlock* Block = PicrossGrid[Index - GridSize.Y];
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