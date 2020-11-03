// Copyright Sanya Larsson 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PicrossBlock.h"
#include "PicrossPuzzleSaveGame.generated.h"

/**
 * Class representing a save file for the picross puzzle.
 */
UCLASS()
class PICROSS_API UPicrossPuzzleSaveGame : public USaveGame
{
	GENERATED_BODY()
	

public:
	UPROPERTY(VisibleAnywhere, Category = Basic)
	TArray<EBlockState> PicrossBlockStates;

};
