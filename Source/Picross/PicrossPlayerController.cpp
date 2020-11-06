// Copyright Sanya Larsson 2020


#include "PicrossPlayerController.h"
#include "Blueprint/UserWidget.h"


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

	CreateMainMenu();
	SetMainMenuEnabled(true);
}

void APicrossPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// MainMenu
	InputComponent->BindAction("Main Menu", EInputEvent::IE_Pressed, this, &APicrossPlayerController::ToggleMainMenu).bExecuteWhenPaused = true;
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

void APicrossPlayerController::CreateMainMenu()
{
	if (MainMenuClass)
	{
		MainMenu = CreateWidget(GetWorld(), MainMenuClass, TEXT("Main Menu"));
		if (MainMenu)
		{
			MainMenu->SetOwningPlayer(this);
		}
	}
}

void APicrossPlayerController::SetMainMenuEnabled(bool bEnabled)
{
	if (MainMenu)
	{
		if (bEnabled)
		{
			MainMenu->AddToPlayerScreen();
		}
		else
		{
			MainMenu->RemoveFromViewport();
		}
	}
}

void APicrossPlayerController::ToggleMainMenu()
{
	if (MainMenu)
	{
		const bool bEnable = (MainMenu->IsInViewport() ? false : true);
		SetMainMenuEnabled(bEnable);
	}
}