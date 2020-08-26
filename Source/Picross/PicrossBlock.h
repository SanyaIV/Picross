// Copyright Sanya Larsson 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PicrossBlock.generated.h"

UENUM()
enum EBlockState
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

	void FillBlock();
	void CrossBlock();
	void ClearBlock();
	bool IsFilled() const;

	void SetIndexInGrid(int32 IndexToSet);
	int32 GetIndexInGrid() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void UpdateMaterial() const;

private:
	EBlockState State = EBlockState::Clear;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Block", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BlockMesh = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Block", meta = (AllowPrivateAccess = "true"))
	TMap<TEnumAsByte<EBlockState>, class UMaterialInstance*> Materials;

	int32 IndexInGrid = -1;
};
