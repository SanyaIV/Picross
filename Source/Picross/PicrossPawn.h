// Copyright Sanya Larsson 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PicrossPawn.generated.h"


UENUM()
enum class EInputMode : uint8
{
	Default		UMETA(DisplayName = "Default"),
	Alternative	UMETA(DisplayName = "Alternative")
};

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
	class APicrossBlock* GetPicrossBlockUnderMouse() const;

	// Input mode
	void EnableAlternativeInputMode();
	void DisableAlternativeInputMode();
	
	// Picross Actions
	void FillBlock();
	void CrossBlock();
	void CycleSelectionRotation();
	void MoveSelectionUp();
	void MoveSelectionDown();

	// Rotation
	virtual void AddControllerPitchInput(float Value) override;
	virtual void AddControllerYawInput(float Value) override;

	// Movement
	void MoveForward(float Value);
	void MoveRight(float Value);
	void MoveUp(float Value);

	void TogglePuzzleBrowser();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* Collision = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true"))
	class UFloatingPawnMovement* MovementComponent = nullptr;

	UPROPERTY()
	class APicrossGrid* PicrossGrid = nullptr;

	UPROPERTY()
	EInputMode InputMode = EInputMode::Default;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Pawn", meta = (AllowPrivateAccess = "true"))
	float ReachDistance = 500.f;
};
