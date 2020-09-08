// Copyright Sanya Larsson 2020


#include "Blueprint/UserWidget.h"
#include "PicrossPlayerController.h"


APicrossPlayerController::APicrossPlayerController()
{
	InputModeGameOnly.SetConsumeCaptureMouseDown(false);
}

void APicrossPlayerController::BeginPlay()
{
	if (PlayerWidgetClass)
	{
		UUserWidget* PlayerWidget = CreateWidget(this, PlayerWidgetClass, TEXT("Player Widget"));
		if (PlayerWidget)
		{
			PlayerWidget->SetOwningPlayer(this);
			PlayerWidget->AddToPlayerScreen();
		}
	}
}

void APicrossPlayerController::SetInputModeGameOnly()
{
	SetInputMode(InputModeGameOnly);
}

bool APicrossPlayerController::LineTraceSingleByChannelFromCenterOfScreen(FHitResult& OutHit, float DistanceToCheck, ECollisionChannel TraceChannel) const
{
	int32 ViewportSizeX, ViewportSizeY;
	GetViewportSize(ViewportSizeX, ViewportSizeY);

	FVector WorldLocation, WorldDirection;
	DeprojectScreenPositionToWorld(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f, WorldLocation, WorldDirection);

	FCollisionQueryParams Params = FCollisionQueryParams(FName(TEXT("")), false, GetOwner());
	if (GetPawn())
	{
		Params.AddIgnoredActor(GetPawn());
	}

	return GetWorld()->LineTraceSingleByChannel(
		OutHit,
		WorldLocation,
		WorldLocation + WorldDirection * DistanceToCheck,
		TraceChannel,
		Params
	);
}