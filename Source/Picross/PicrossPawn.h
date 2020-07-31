// Copyright Sanya Larsson 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PicrossPawn.generated.h"

/**
 * The Picross pawn responsible for interacting with the Picross puzzle.
 */
UCLASS()
class PICROSS_API APicrossPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APicrossPawn();
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

protected:
	class APicrossBlock* GetPicrossBlockInView() const;
	
	// Picross Actions
	void DarkenBlock();
	void CrossBlock();
	void CycleSelectionRotation();
	void MoveSelectionUp();
	void MoveSelectionDown();

	// Movement
	void MoveForward(float Value);
	void MoveRight(float Value);
	void MoveUp(float Value);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* Collision = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true"))
	class UFloatingPawnMovement* MovementComponent = nullptr;

	UPROPERTY()
	class APicrossGrid* PicrossGrid = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Pawn", meta = (AllowPrivateAccess = "true"))
	float ReachDistance = 500.f;
};
