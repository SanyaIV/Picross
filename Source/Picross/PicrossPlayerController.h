// Copyright Sanya Larsson 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PicrossPlayerController.generated.h"

/**
 * The Picross Player Controller that controlls the Picross Pawn
 */
UCLASS()
class PICROSS_API APicrossPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	/**
	 * Linetrace from the center of the screen and forwards in the camera direction.
	 * @param OutHit - Reference to a FHitResult which will contain the results of the linetrace.
	 * @param DistanceToCheck - The length of the linetrace.
	 * @param TraceChannel - The channel to check.
	 * @return Whether or not something was hit by the linetrace.
	 */
	bool LineTraceSingleByChannelFromCenterOfScreen(FHitResult& OutHit, float DistanceToCheck, ECollisionChannel TraceChannel = ECollisionChannel::ECC_Visibility) const;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UUserWidget> PlayerWidgetClass = nullptr;
};
