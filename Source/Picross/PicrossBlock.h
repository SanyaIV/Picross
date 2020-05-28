// Copyright Sanya Larsson 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PicrossBlock.generated.h"

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

	void DarkenBlock() const;
	void CrossBlock() const;
	void ResetBlock() const;
	bool IsDarkened() const;

	void SetIndexInGrid(int32 IndexToSet);
	int32 GetIndexInGrid() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Block", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BlockMesh = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Block", meta = (AllowPrivateAccess = "true"))
	class UMaterialInstance* DefaultMaterial = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Block", meta = (AllowPrivateAccess = "true"))
	class UMaterialInstance* DarkenedMaterial = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Block", meta = (AllowPrivateAccess = "true"))
	class UMaterialInstance* CrossMaterial = nullptr;

	int32 IndexInGrid = -1;
};
