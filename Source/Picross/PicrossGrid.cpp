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

void APicrossGrid::CreateGrid(int32 GridSize)
{
	if (PicrossGrid.Num() > 0)
	{
		DestroyGrid();
	}

	for (int32 i = 0; i < GridSize * GridSize; ++i)
	{
		FVector Location = GetLocationForBlockCreation(i, GridSize);
		PicrossGrid.Add(GetWorld()->SpawnActor<APicrossBlock>(PicrossBlockBP, Location, GetActorRotation()));
	}
}

FVector APicrossGrid::GetLocationForBlockCreation(int32 Index, int32 GridSize) const
{
	const static float Distance = 110.f;
	FVector Location = FVector::ZeroVector;
	if (Index == 0)
	{
		const float Offset = Distance * (GridSize/2) - (GridSize % 2 == 0 ? Distance / 2 : 0); // Place the first block in the top left corner so that the grid actor is in the middle.
		Location = GetActorLocation();
		Location.Y -= Offset;
		Location.X -= Offset;
	}
	else if (PicrossGrid.IsValidIndex((Index % GridSize == 0 ? Index - GridSize : Index - 1)))
	{
		if (Index % GridSize == 0 && PicrossGrid[Index - GridSize] != nullptr)
		{
			Location = PicrossGrid[Index - GridSize]->GetActorLocation();
			Location.Y += Distance;
		}
		else if (PicrossGrid[Index - 1] != nullptr)
		{
			Location = PicrossGrid[Index - 1]->GetActorLocation();
			Location.X += Distance;
		}
		else
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