// Copyright Sanya Larsson 2020

#pragma once

// KBM_Default includes
#include "CoreMinimal.h"
#include "PicrossBlock.generated.h"

UENUM()
enum class EBlockState : uint8
{
	Clear	UMETA(DisplayName = "Clear"),
	Crossed	UMETA(DisplayName = "Crossed"),
	Filled	UMETA(DisplayName = "Filled")
};

USTRUCT(BlueprintType)
struct PICROSS_API FPicrossBlock
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	EBlockState State;
	UPROPERTY(VisibleAnywhere)
	FTransform Transform;
	UPROPERTY(VisibleAnywhere)
	int32 MasterIndex;
	UPROPERTY(VisibleAnywhere)
	int32 InstanceIndex;
};
