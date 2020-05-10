// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PicrossBlock.generated.h"

UCLASS()
class PICROSS_API APicrossBlock : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APicrossBlock();

	void DarkenBlock() const;
	void CrossBlock() const;
	bool IsDarkened() const;

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
};
