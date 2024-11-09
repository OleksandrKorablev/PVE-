#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"


#include "ProjectileMachine.generated.h"

UCLASS()
class MYPROJECT_API AProjectileMachine : public AActor
{
	GENERATED_BODY()

public:
	AProjectileMachine();
	virtual void PostInitializeComponents() override;

	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (AllowPrivateAccess = "true"))
	float Damage = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UDamageType> DamageType;

private:
	bool bHasDealtDamage = false;

	void StopDeathEffects();

	UPROPERTY(EditAnywhere, Category = "Audio")
	float TimeDeathEffects = 0.5f;

	FTimerHandle UnusedHandle;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* BoxCollision;

	UPROPERTY(VisibleAnywhere, Category = "Category")
	UStaticMeshComponent* MeshComponent = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "ProjectileMovementComponent")
	UProjectileMovementComponent* ProjectileMovementComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	UAudioComponent* ExplosionSound = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
	UParticleSystemComponent* ExplosionEffectVFX = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Category")
	USphereComponent* ExplosionRadius = nullptr;

	float CalculateDamage(float Distance);
};