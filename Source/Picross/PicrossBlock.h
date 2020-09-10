// Copyright Sanya Larsson 2020

#pragma once

// Default includes
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

	FPicrossBlock() { State = EBlockState::Clear, Transform = FTransform::Identity, MasterIndex = -1; InstanceIndex = -1; };
	FPicrossBlock(EBlockState NewState, FTransform NewTransform, int32 NewMasterIndex, int32 NewInstanceIndex) : State(NewState), Transform(NewTransform), MasterIndex(NewMasterIndex), InstanceIndex(NewInstanceIndex) {};
	FPicrossBlock(const FPicrossBlock& Other) : State(Other.State), Transform(Other.Transform), MasterIndex(Other.MasterIndex), InstanceIndex(Other.InstanceIndex) {};
	FPicrossBlock& operator=(const FPicrossBlock& Other) { State = Other.State; Transform = Other.Transform; MasterIndex = Other.MasterIndex, InstanceIndex = Other.InstanceIndex; return *this; };
};
