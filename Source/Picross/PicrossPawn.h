// Copyright Sanya Larsson 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Misc/Optional.h"
#include "PicrossPawn.generated.h"


UENUM()
enum class EInputMode : uint8
{
	KBM_Default		UMETA(DisplayName = "KBM Default"),
	KBM_Alternative	UMETA(DisplayName = "KBM Alternative"),
	Gamepad			UMETA(DisplayName = "Gamepad")
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
	TOptional<int32> GetBlockInView() const;
	TOptional<int32> GetBlockUnderMouse() const;

	// Input mode
	DECLARE_DELEGATE_OneParam(FSetInputModeDelegate, EInputMode);
	void SetInputMode(EInputMode NewInputMode);
	void DetectInput(FKey Key);
	void ToggleInputMode();
	
	// Picross Actions
	void SaveStartBlock();
	void FillBlocks();
	void CrossBlocks();
	void CycleSelectionRotation();
	void MoveSelectionUp();
	void MoveSelectionDown();
	void Undo();
	void Redo();
	void MoveFocusUp();
	void MoveFocusDown();
	void MoveFocusLeft();
	void MoveFocusRight();
	void OnPuzzleSolved();

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

	UPROPERTY()
	EInputMode InputMode = EInputMode::KBM_Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn", meta = (AllowPrivateAccess = "true"))
	float ReachDistance = 10000.f;

	FTransform StartTransform = FTransform::Identity;
};
