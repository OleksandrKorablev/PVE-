#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"

#include "InputAction.h"
#include "NiagaraComponent.h"

#include "MachinePawn.generated.h"

class UBoxComponent;
class UArrowComponent;
class UAudioComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmmoDepletedDelegate, int32, AmmoCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFireDelegate);

UCLASS()
class MYPROJECT_API AMachinePawn : public APawn
{
	GENERATED_BODY()

public:
	AMachinePawn(const FObjectInitializer& ObjectInitializer);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(BlueprintAssignable)
	FOnAmmoDepletedDelegate OnAmmoDepleted; //

	UPROPERTY(BlueprintAssignable)
	FOnFireDelegate OnFire;//

	UPROPERTY(EditAnywhere, Category = "SpawnProjectileBlueprint")
	TSubclassOf<AActor> SpawnProjectileBlueprint;//

	UPROPERTY(EditAnywhere, Category = "Camera Shake")
	TSubclassOf<UCameraShakeBase> FireShake;

	// Skeletal Mesh Component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkeletalMesh")
	USkeletalMeshComponent* SkeletalMesh;

	// Bone Name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone")
	FName BoneName;

	// Rotation Angle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	FRotator RotationAngle;

	// Additional Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* BoxCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* ArrowComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAudioComponent* AudioComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* NiagaraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* SphereMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* ProjectileSpawnPoint = nullptr;

	// Vehicle Movement Component
	UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UChaosVehicleMovementComponent> VehicleMovementComponent;

	// Arrow Component Socket Name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	FName ArrowComponentSocketName;

	//					Кістки
	// Кістки башти
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	FName TurretBoneName = "turret_jnt";

	// Кістки ствола
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	FName GunBoneName = "gun_jnt";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	float TurretRotationYaw;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	float GunRotationPitch;

private:
	bool bIsAccelerating = false;
	bool bIsMovingBackward = false;
	bool bIsFiring = true;


	UPROPERTY(EditAnywhere, Category = "Camera")
	float MinZoomDistance = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float MaxZoomDistance = 1200.0f;

	// Кільіксть амуніції, та швидкість пострілу.
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 MaxAmmo = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 CurrentAmmo = 0;

	float LastFireTime = 0.0f;
	float ShotInterval = 0.2f;


	// нахил ствола
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (AllowPrivateAccess = "true"))
	float MinGunPitch = -10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (AllowPrivateAccess = "true"))
	float MaxGunPitch = 20.0f;

	//швидкість руху башти
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (AllowPrivateAccess = "true"))
	float TurretRotationSpeed = 5.0f;

	// швидкість руху машини
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	int32 MaxSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	int32 CurrentSpeed = 0.0f;

	// швидкість затухання інерції 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	int32 AccelerationRate = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	int32 DecelerationRate = 5.0f;

	UFUNCTION()
	void TurnCamera(const FInputActionInstance& Value);

	UFUNCTION()
	void ZoomCamera(const FInputActionInstance& Value);

	void RotateTurret(float DeltaTime);

	void RotateGun();

	UFUNCTION()
	void MoveTank(const FInputActionInstance& Value);

	UFUNCTION()
	void StopMoveTank(const FInputActionInstance& Value);

	UFUNCTION()
	void TurnTank(const FInputActionInstance& Value);

	UFUNCTION()
	void StopTurnTank(const FInputActionInstance& Value);

	//			Постріл
	void FireTower(const FInputActionInstance& Value);
	void StopFireTower(const FInputActionInstance& Value);

	void FakeReload();
	void ReloadAmmo();
	void Fire();

	virtual bool CanFire() const;

	bool bIsAlive = true;
 
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* IA_FireAxis;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* IA_MoveTankAxis;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* IA_TurnTankAxis;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* IA_TurnCameraAxis;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* IA_ZoomCamera;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* IMC_InputContext = nullptr;


};
