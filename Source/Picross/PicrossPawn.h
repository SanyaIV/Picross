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
	virtual void Tick(float DeltaSeconds) override;

protected:
	int32 GetBlockInView() const;
	int32 GetBlockUnderMouse() const;

	// Input mode
	void ToggleInputMode();
	void EnableAlternativeInputMode();
	void DisableAlternativeInputMode();
	
	// Picross Actions
	void SaveStartBlock();
	void FillBlocks();
	void CrossBlocks();
	void CycleSelectionRotation();
	void MoveSelectionUp();
	void MoveSelectionDown();
	void Undo();
	void Redo();

	// Rotation
	virtual void AddControllerPitchInput(float Value) override;
	virtual void AddControllerYawInput(float Value) override;

	// Movement
	void MoveForward(float Value);
	void MoveRight(float Value);
	void MoveUp(float Value);
	void MoveToIdealTransformDelayed();
	void MoveToIdealTransform();

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
	int32 StartBlockIndex = INDEX_NONE;
	int32 BlockInView = INDEX_NONE;

	UPROPERTY()
	EInputMode InputMode = EInputMode::Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn", meta = (AllowPrivateAccess = "true"))
	float ReachDistance = 10000.f;
};
