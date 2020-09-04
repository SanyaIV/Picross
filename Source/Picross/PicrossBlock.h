// Copyright Sanya Larsson 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PicrossBlock.generated.h"


UENUM()
enum class EBlockState : uint8
{
	Clear		UMETA(DisplayName = "Clear"),
	Crossed		UMETA(DisplayName = "Crossed"),
	Filled		UMETA(DisplayName = "Filled")
};

/**
 * The Picross Block
 */
UCLASS()
class PICROSS_API APicrossBlock : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APicrossBlock();

	void SetEnabled(bool bEnabled);
	void Lock();

	void FillBlock();
	void CrossBlock();
	void ClearBlock();
	bool IsFilled() const { return State == EBlockState::Filled; };

	void SetIndexInGrid(int32 IndexToSet);
	int32 GetIndexInGrid() const;

	DECLARE_EVENT_TwoParams(APicrossBlock, FStateChangedEvent, EBlockState, EBlockState);
	FStateChangedEvent& OnStateChanged() { return StateChangedEvent; };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void SetState(EBlockState StateToSet);
	void UpdateMaterial() const;

private:
	bool bLocked = false;
	EBlockState State = EBlockState::Clear;
	FStateChangedEvent StateChangedEvent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Block", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BlockMesh = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Block", meta = (AllowPrivateAccess = "true"))
	TMap<TEnumAsByte<EBlockState>, class UMaterialInstance*> Materials;

	int32 IndexInGrid = -1;
};
