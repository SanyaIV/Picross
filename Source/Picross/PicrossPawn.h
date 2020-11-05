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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	TOptional<int32> GetBlockInView() const;
	TOptional<int32> GetBlockUnderMouse() const;

	// Input mode
	DECLARE_DELEGATE_OneParam(FSetInputModeDelegate, EInputMode);
	UFUNCTION()
	void SetInputMode(EInputMode NewInputMode);
	UFUNCTION()
	void DetectInput(FKey Key);
	UFUNCTION()
	void ToggleInputMode();
	
	// Picross Actions
	UFUNCTION()
	void SaveStartBlock();
	UFUNCTION()
	void FillBlocks();
	UFUNCTION()
	void CrossBlocks();
	UFUNCTION()
	void CycleSelectionRotation();
	UFUNCTION()
	void MoveSelectionUp();
	UFUNCTION()
	void MoveSelectionDown();
	UFUNCTION()
	void Undo();
	UFUNCTION()
	void Redo();
	UFUNCTION()
	void MoveFocusUp();
	UFUNCTION()
	void MoveFocusDown();
	UFUNCTION()
	void MoveFocusLeft();
	UFUNCTION()
	void MoveFocusRight();

	// Rotation
	virtual void AddControllerPitchInput(float Value) override;
	virtual void AddControllerYawInput(float Value) override;

	// Movement
	UFUNCTION()
	void MoveForward(float Value);
	UFUNCTION()
	void MoveRight(float Value);
	UFUNCTION()
	void MoveUp(float Value);
	UFUNCTION()
	void MoveToIdealTransformDelayed();
	UFUNCTION()
	void MoveToIdealTransform();
	UFUNCTION()
	void MoveTo(const FTransform& Transform);
	UFUNCTION()
	void ResetTransform();

	UFUNCTION()
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
};
