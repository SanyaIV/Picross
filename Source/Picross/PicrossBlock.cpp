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

	ResetBlock();
}

void APicrossBlock::DarkenBlock() const
{
	if (BlockMesh && DarkenedMaterial)
	{
		UMaterialInstance* CurrentMaterial = Cast<UMaterialInstance>(BlockMesh->GetMaterial(0));
		if (CurrentMaterial && CurrentMaterial->Equivalent(DarkenedMaterial) && DefaultMaterial)
		{
			BlockMesh->SetMaterial(0, DefaultMaterial);
		}
		else
		{
			BlockMesh->SetMaterial(0, DarkenedMaterial);
		}
	}
}

void APicrossBlock::CrossBlock() const
{
	if (BlockMesh && CrossMaterial)
	{
		UMaterialInstance* CurrentMaterial = Cast<UMaterialInstance>(BlockMesh->GetMaterial(0));
		if (CurrentMaterial && CurrentMaterial->Equivalent(CrossMaterial) && DefaultMaterial)
		{
			BlockMesh->SetMaterial(0, DefaultMaterial);
		}
		else
		{
			BlockMesh->SetMaterial(0, CrossMaterial);
		}
	}
}

void APicrossBlock::ResetBlock() const
{
	if (BlockMesh && DefaultMaterial)
	{
		BlockMesh->SetMaterial(0, DefaultMaterial);
	}
}

bool APicrossBlock::IsDarkened() const
{
	if (BlockMesh && DarkenedMaterial)
	{
		UMaterialInstance* CurrentMaterial = Cast<UMaterialInstance>(BlockMesh->GetMaterial(0));
		if (CurrentMaterial->Equivalent(DarkenedMaterial))
		{
			return true;
		}
	}

	return false;
}
