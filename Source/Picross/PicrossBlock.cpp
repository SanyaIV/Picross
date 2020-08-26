// Copyright Sanya Larsson 2020


#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstance.h"
#include "PicrossBlock.h"

// Sets default values
APicrossBlock::APicrossBlock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BlockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Block"));
	SetRootComponent(BlockMesh);
}

// Called when the game starts or when spawned
void APicrossBlock::BeginPlay()
{
	Super::BeginPlay();

	ClearBlock();
}

void APicrossBlock::SetEnabled(bool bEnabled)
{
	SetActorHiddenInGame(!bEnabled);
	SetActorEnableCollision(bEnabled);
}

void APicrossBlock::FillBlock()
{
	State = State == EBlockState::Filled ? EBlockState::Clear : EBlockState::Filled;
	UpdateMaterial();
}

void APicrossBlock::CrossBlock()
{
	State = State == EBlockState::Crossed ? EBlockState::Clear : EBlockState::Crossed;
	UpdateMaterial();
}

void APicrossBlock::ClearBlock()
{
	State = EBlockState::Clear;
	UpdateMaterial();
}

bool APicrossBlock::IsFilled() const
{
	return State == EBlockState::Filled;
}

void APicrossBlock::UpdateMaterial() const
{
	UMaterialInstance*const*const MaterialToSet = Materials.Find(State); // Find returns a pointer to the value where the value is a pointer (pointer-to-pointer)
	if (MaterialToSet && *MaterialToSet && BlockMesh) // Make sure all pointers aren't nullptr in the correct order.
	{
		BlockMesh->SetMaterial(0, *MaterialToSet); // Dereference the pointer-to-pointer to get the pointer to the material instance.
	}
}

void APicrossBlock::SetIndexInGrid(int32 IndexToSet)
{
	IndexInGrid = IndexToSet;
}

int32 APicrossBlock::GetIndexInGrid() const
{
	return IndexInGrid;
}
