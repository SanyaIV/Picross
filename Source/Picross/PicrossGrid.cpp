// Copyright Sanya Larsson 2020


#include "PicrossGrid.h"

// Sets default values
APicrossGrid::APicrossGrid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APicrossGrid::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APicrossGrid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

