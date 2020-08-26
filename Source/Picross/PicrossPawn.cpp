// Copyright Sanya Larsson 2020


#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Classes/Kismet/GameplayStatics.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "PicrossBlock.h"
#include "PicrossGrid.h"
#include "PicrossPawn.h"
#include "PicrossPlayerController.h"

// Sets default values
APicrossPawn::APicrossPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

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
}

// Called to bind functionality to input
void APicrossPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Actions
	PlayerInputComponent->BindAction("Fill Block", EInputEvent::IE_Pressed, this, &APicrossPawn::FillBlock);
	PlayerInputComponent->BindAction("Cross Block", EInputEvent::IE_Pressed, this, &APicrossPawn::CrossBlock);
	PlayerInputComponent->BindAction("Move Selection Up", EInputEvent::IE_Pressed, this, &APicrossPawn::MoveSelectionUp);
	PlayerInputComponent->BindAction("Move Selection Down", EInputEvent::IE_Pressed, this, &APicrossPawn::MoveSelectionDown);
	PlayerInputComponent->BindAction("Cycle Selection Rotation", EInputEvent::IE_Pressed, this, &APicrossPawn::CycleSelectionRotation);

	// Rotation
	PlayerInputComponent->BindAxis("Rotate Pitch", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Rotate Yaw", this, &APawn::AddControllerYawInput);

	// Movement
	PlayerInputComponent->BindAxis("Move Forward", this, &APicrossPawn::MoveForward);
	PlayerInputComponent->BindAxis("Move Right", this, &APicrossPawn::MoveRight);
	PlayerInputComponent->BindAxis("Move Up", this, &APicrossPawn::MoveUp);
}

APicrossBlock* APicrossPawn::GetPicrossBlockInView() const
{
	APicrossBlock* BlockToReturn = nullptr;

	APicrossPlayerController* PlayerController = Cast<APicrossPlayerController>(GetController());
	if (PlayerController)
	{
		FHitResult HitResult;
		if (PlayerController->LineTraceSingleByChannelFromCenterOfScreen(HitResult, ReachDistance, ECollisionChannel::ECC_Visibility))
		{
			BlockToReturn = Cast<APicrossBlock>(HitResult.GetActor());
		}
	}

	return BlockToReturn;
}

void APicrossPawn::FillBlock()
{
	APicrossBlock* Block = GetPicrossBlockInView();
	if (Block)
	{
		Block->FillBlock();
	}
}

void APicrossPawn::CrossBlock()
{
	APicrossBlock* Block = GetPicrossBlockInView();
	if (Block)
	{
		Block->CrossBlock();
	}
}

void APicrossPawn::CycleSelectionRotation()
{
	APicrossBlock* Block = GetPicrossBlockInView();
	if (Block)
	{
		PicrossGrid->Cycle2DRotation(Block);
	}
}

void APicrossPawn::MoveSelectionUp()
{
	PicrossGrid->Move2DSelectionUp();
}

void APicrossPawn::MoveSelectionDown()
{
	PicrossGrid->Move2DSelectionDown();
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

