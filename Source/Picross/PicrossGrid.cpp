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
	
	CreateGrid(6);
}

void APicrossGrid::CreateGrid(int32 GridSize)
{
	const float Distance = 110.f;
	for (int32 i = 0; i < GridSize * GridSize; ++i)
	{
		FVector Location = FVector::ZeroVector;
		if (i == 0)
		{
			const float EvenGridOffset = GridSize % 2 == 0 ? Distance / 2 : 0;
			Location = GetActorLocation();
			Location.Y -= Distance * (GridSize / 2) - EvenGridOffset;
			Location.X -= Distance * (GridSize / 2) - EvenGridOffset;
		}
		else
		{
			int32 Mod = i % GridSize;
			if (Mod == 0)
			{
				Location = PicrossGrid[i - GridSize]->GetActorLocation();
				Location.Y += Distance;
			}
			else
			{
				Location = PicrossGrid[i - 1]->GetActorLocation();
				Location.X += Distance;
			}
		}

		PicrossGrid.Add(GetWorld()->SpawnActor<APicrossBlock>(PicrossBlockBP, Location, GetActorRotation()));
	}
}