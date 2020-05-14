// Copyright Sanya Larsson 2020


#include "PicrossPlayerController.h"

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