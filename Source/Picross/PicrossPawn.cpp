// Copyright Sanya Larsson 2020


#include "PicrossPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Classes/Kismet/GameplayStatics.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "PicrossGrid.h"
#include "PicrossPlayerController.h"

// Sets default values
APicrossPawn::APicrossPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->SetSphereRadius(50, false);
	SetRootComponent(Collision);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement Component"));
}

void APicrossPawn::BeginPlay()
{
	Super::BeginPlay();

	PicrossGrid = Cast<APicrossGrid>(UGameplayStatics::GetActorOfClass(GetWorld(), APicrossGrid::StaticClass()));
	ensureMsgf(PicrossGrid, TEXT("PicrossPawn couldn't find any PicrossGrid."));

	InputMode = EInputMode::Default;
}

void APicrossPawn::Tick(float DeltaSeconds)
{
	const int32 CurrentBlockInView = (InputMode == EInputMode::Default ? GetBlockInView() : GetBlockUnderMouse());
	if (CurrentBlockInView != BlockInView)
	{
		if (PicrossGrid)
		{
			PicrossGrid->HighlightBlocks(CurrentBlockInView);
			BlockInView = CurrentBlockInView;
		}
	}
}

// Called to bind functionality to input
void APicrossPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Pause Menu - Set to allow execution even when paused.
	PlayerInputComponent->BindAction("Puzzle Browser", EInputEvent::IE_Pressed, this, &APicrossPawn::TogglePuzzleBrowser).bExecuteWhenPaused = true;

	// Input mode
	PlayerInputComponent->BindAction("Alternative Input Mode", EInputEvent::IE_Pressed, this, &APicrossPawn::EnableAlternativeInputMode);
	PlayerInputComponent->BindAction("Alternative Input Mode", EInputEvent::IE_Released, this, &APicrossPawn::DisableAlternativeInputMode);

	// Actions
	PlayerInputComponent->BindAction("Fill Block", EInputEvent::IE_Pressed, this, &APicrossPawn::SaveStartBlock);
	PlayerInputComponent->BindAction("Fill Block", EInputEvent::IE_Released, this, &APicrossPawn::FillBlocks);
	PlayerInputComponent->BindAction("Cross Block", EInputEvent::IE_Pressed, this, &APicrossPawn::SaveStartBlock);
	PlayerInputComponent->BindAction("Cross Block", EInputEvent::IE_Released, this, &APicrossPawn::CrossBlocks);
	PlayerInputComponent->BindAction("Move Selection Up", EInputEvent::IE_Pressed, this, &APicrossPawn::MoveSelectionUp);
	PlayerInputComponent->BindAction("Move Selection Down", EInputEvent::IE_Pressed, this, &APicrossPawn::MoveSelectionDown);
	PlayerInputComponent->BindAction("Cycle Selection Rotation", EInputEvent::IE_Pressed, this, &APicrossPawn::CycleSelectionRotation);

	// Rotation
	PlayerInputComponent->BindAxis("Rotate Pitch", this, &APicrossPawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Rotate Yaw", this, &APicrossPawn::AddControllerYawInput);

	// Movement
	PlayerInputComponent->BindAxis("Move Forward", this, &APicrossPawn::MoveForward);
	PlayerInputComponent->BindAxis("Move Right", this, &APicrossPawn::MoveRight);
	PlayerInputComponent->BindAxis("Move Up", this, &APicrossPawn::MoveUp);
}

int32 APicrossPawn::GetBlockInView() const
{
	APicrossPlayerController* PlayerController = Cast<APicrossPlayerController>(GetController());
	if (PlayerController)
	{
		FHitResult HitResult;
		if (PlayerController->LineTraceSingleByChannelFromCenterOfScreen(HitResult, ReachDistance, ECollisionChannel::ECC_Visibility))
		{
			UInstancedStaticMeshComponent* ISM = Cast<UInstancedStaticMeshComponent>(HitResult.GetComponent());
			if (ISM && ISM->PerInstanceSMCustomData.IsValidIndex(HitResult.Item * ISM->NumCustomDataFloats))
			{
				return static_cast<int32>(ISM->PerInstanceSMCustomData[HitResult.Item * ISM->NumCustomDataFloats]);
			}
		}
	}

	return INDEX_NONE;
}

int32 APicrossPawn::GetBlockUnderMouse() const
{
	APicrossPlayerController* PlayerController = Cast<APicrossPlayerController>(GetController());
	if (PlayerController)
	{
		FVector Start, Direction, End;
		PlayerController->DeprojectMousePositionToWorld(Start, Direction);
		End = Start + Direction * ReachDistance;

		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility))
		{
			UInstancedStaticMeshComponent* ISM = Cast<UInstancedStaticMeshComponent>(HitResult.GetComponent());
			if (ISM && ISM->PerInstanceSMCustomData.IsValidIndex(HitResult.Item * ISM->NumCustomDataFloats))
			{
				return static_cast<int32>(ISM->PerInstanceSMCustomData[HitResult.Item * ISM->NumCustomDataFloats]);
			}
		}
	}

	return INDEX_NONE;
}

void APicrossPawn::EnableAlternativeInputMode()
{
	InputMode = EInputMode::Alternative;
	if (APicrossPlayerController* PPC = Cast<APicrossPlayerController>(GetController()))
	{
		PPC->bShowMouseCursor = true;
		PPC->bEnableClickEvents = true;
		PPC->SetInputModeGameOnly();
	}
}

void APicrossPawn::DisableAlternativeInputMode()
{
	InputMode = EInputMode::Default;
	if (APicrossPlayerController* PPC = Cast<APicrossPlayerController>(GetController()))
	{
		PPC->bShowMouseCursor = false;
		PPC->bEnableClickEvents = false;
		PPC->SetInputModeGameOnly(); // We set this even though it's already set in order to work around an input issue where rotation won't be possible until left-clicking.
	}
}

void APicrossPawn::SaveStartBlock()
{
	StartBlockIndex = BlockInView;
}

void APicrossPawn::FillBlocks()
{
	if (PicrossGrid)
	{
		PicrossGrid->UpdateBlocks(StartBlockIndex, BlockInView, EBlockState::Filled);
	}
}

void APicrossPawn::CrossBlocks()
{
	if (PicrossGrid)
	{
		PicrossGrid->UpdateBlocks(StartBlockIndex, BlockInView, EBlockState::Crossed);
	}
}

void APicrossPawn::CycleSelectionRotation()
{
	if (!PicrossGrid) return;

	PicrossGrid->Cycle2DRotation(BlockInView);
	PicrossGrid->HighlightBlocks(BlockInView);
}

void APicrossPawn::MoveSelectionUp()
{
	if (!PicrossGrid) return;

	PicrossGrid->Move2DSelectionUp();
}

void APicrossPawn::MoveSelectionDown()
{
	if (!PicrossGrid) return;

	PicrossGrid->Move2DSelectionDown();
}

void APicrossPawn::AddControllerPitchInput(float Value)
{
	switch (InputMode)
	{
		case EInputMode::Default:
			APawn::AddControllerPitchInput(Value);
			break;
	}
}

void APicrossPawn::AddControllerYawInput(float Value)
{
	switch (InputMode)
	{
		case EInputMode::Default:
			APawn::AddControllerYawInput(Value);
			break;
	}
}

void APicrossPawn::MoveForward(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void APicrossPawn::MoveRight(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void APicrossPawn::MoveUp(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		AddMovementInput(FVector::UpVector, Value);
	}
}

void APicrossPawn::TogglePuzzleBrowser()
{
	if (PicrossGrid)
	{
		if (UGameplayStatics::IsGamePaused(this))
		{
			PicrossGrid->ClosePuzzleBrowser();
		}
		else
		{
			PicrossGrid->OpenPuzzleBrowser();
		}
	}
}
