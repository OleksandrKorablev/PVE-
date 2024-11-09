#include "MachinePawn.h"

#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

#include "ChaosVehicleMovementComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"

#include "MyProject/Actors/AnimInstance/MyVehicleAnimationInstance.h"

#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"


AMachinePawn::AMachinePawn(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass(TEXT("BaseComponent"), UBoxComponent::StaticClass()))
{
	PrimaryActorTick.bCanEverTick = true;

    // Create the Skeletal Mesh Component
    SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    SkeletalMesh->SetCollisionProfileName(UCollisionProfile::Vehicle_ProfileName);
    SkeletalMesh->BodyInstance.bSimulatePhysics = false;
    SkeletalMesh->BodyInstance.bNotifyRigidBodyCollision = true;
    SkeletalMesh->BodyInstance.bUseCCD = true;
    SkeletalMesh->bBlendPhysics = true;
    SkeletalMesh->SetGenerateOverlapEvents(true);
    SkeletalMesh->SetCanEverAffectNavigation(false);
    RootComponent = SkeletalMesh;

    // Create additional components
    BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
    BoxCollision->SetupAttachment(RootComponent);

    ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
    ArrowComponent->SetupAttachment(SkeletalMesh);

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(ArrowComponent);

    NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
    NiagaraComponent->SetupAttachment(AudioComponent);

    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    StaticMesh->SetupAttachment(ArrowComponent);

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);

    SphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereMesh"));
    SphereMesh->SetupAttachment(SpringArm);

    ProjectileSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSpawnPoint"));
    ProjectileSpawnPoint->SetupAttachment(ArrowComponent);

    VehicleMovementComponent = CreateDefaultSubobject<UChaosVehicleMovementComponent, UChaosWheeledVehicleMovementComponent>(TEXT("VehicleMovementComponent"));
    VehicleMovementComponent->SetIsReplicated(true); 
    VehicleMovementComponent->UpdatedComponent = SkeletalMesh;

    ArrowComponentSocketName = TEXT("gun_jntSocket");
}

void AMachinePawn::BeginPlay()
{
    Super::BeginPlay();

    // Attach ArrowComponent to a specific bone
    ArrowComponent->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::KeepRelativeTransform, ArrowComponentSocketName);

    // Initialize the previous position
    ReloadAmmo();
}

void AMachinePawn::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    // Connects a play controller to the player
    const APlayerController* PC = Cast<APlayerController>(NewController);
    if (IsValid(PC))
    {
        UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
        if (IsValid(Subsystem) && IsValid(IMC_InputContext))
        {
            Subsystem->AddMappingContext(IMC_InputContext, 1);
        }
    }
}

// Called to bind functionality to input
void AMachinePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (IsValid(Input))
    {
        if (IsValid(IA_MoveTankAxis))
        {
            Input->BindAction(IA_MoveTankAxis, ETriggerEvent::Triggered, this, &AMachinePawn::MoveTank);
            Input->BindAction(IA_MoveTankAxis, ETriggerEvent::Completed, this, &AMachinePawn::StopMoveTank);
        }
        if (IsValid(IA_TurnTankAxis))
        {
            Input->BindAction(IA_TurnTankAxis, ETriggerEvent::Triggered, this, &AMachinePawn::TurnTank);
            Input->BindAction(IA_TurnTankAxis, ETriggerEvent::Completed, this, &AMachinePawn::StopTurnTank);
        }
        if (IsValid(IA_TurnCameraAxis))
        {
            Input->BindAction(IA_TurnCameraAxis, ETriggerEvent::Triggered, this, &AMachinePawn::TurnCamera);
        }
        if (IsValid(IA_ZoomCamera))
        {
            Input->BindAction(IA_ZoomCamera, ETriggerEvent::Triggered, this, &AMachinePawn::ZoomCamera);
        }
        if (IsValid(IA_FireAxis))
        {
            Input->BindAction(IA_FireAxis, ETriggerEvent::Triggered, this, &AMachinePawn::FireTower);
            Input->BindAction(IA_FireAxis, ETriggerEvent::Completed, this, &AMachinePawn::StopFireTower);
        }
    }
}

// Called every frame
void AMachinePawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Рух машини W / S
    // Якщо гравець не натискає кнопку руху, зменшуємо швидкість
    if (!bIsAccelerating)
    {
        if (CurrentSpeed > 0.0f)
        {
            CurrentSpeed = FMath::Clamp(CurrentSpeed - DecelerationRate * DeltaTime, 0.0f, MaxSpeed);
        }
        else if (CurrentSpeed < 0.0f)
        {
            CurrentSpeed = FMath::Clamp(CurrentSpeed + DecelerationRate * DeltaTime, -MaxSpeed, 0.0f);
        }
    }
    // Оновлюємо рух транспортного засобу
    if (CurrentSpeed > 0.0f)
    {
        VehicleMovementComponent->SetThrottleInput(CurrentSpeed / MaxSpeed);
        VehicleMovementComponent->SetBrakeInput(0.0f);
    }
    else if (CurrentSpeed < 0.0f)
    {
        VehicleMovementComponent->SetThrottleInput(0.0f);
        VehicleMovementComponent->SetBrakeInput(-CurrentSpeed / MaxSpeed);
    }

    // Обертання башти та ствола відповідно до напрямку камери
    RotateTurret(DeltaTime);
}

// Рух машини W / S
void AMachinePawn::MoveTank(const FInputActionInstance& Value)
{
    const float AxisValue = Value.GetValue().Get<float>();
    if (AxisValue != 0.0f)
    {
        bIsAccelerating = true;
        CurrentSpeed = FMath::Clamp(CurrentSpeed + AxisValue * AccelerationRate, -MaxSpeed, MaxSpeed);
    }

    if (AxisValue > 0.0f)
    {
        VehicleMovementComponent->SetThrottleInput(CurrentSpeed / MaxSpeed);
        VehicleMovementComponent->SetBrakeInput(0.0f);
    }
    else if (AxisValue < 0.0f)
    {
        VehicleMovementComponent->SetThrottleInput(0.0f);
        VehicleMovementComponent->SetBrakeInput(-CurrentSpeed / MaxSpeed);
    }
}

void AMachinePawn::StopMoveTank(const FInputActionInstance& Value)
{
    bIsAccelerating = false;
}

// Рух поворот машини A / D
void AMachinePawn::TurnTank(const FInputActionInstance& Value)
{
    const float AxisValue = Value.GetValue().Get<float>();
    VehicleMovementComponent->SetSteeringInput(AxisValue);
}

void AMachinePawn::StopTurnTank(const FInputActionInstance& Value)
{
    VehicleMovementComponent->SetSteeringInput(0.0f);
}

// Рух камери
void AMachinePawn::TurnCamera(const FInputActionInstance& Value)
{
    const FVector2D MouseInput = Value.GetValue().Get<FVector2D>();

    // Створюємо обертання на основі вхідних значень миші
    FRotator LocalRotation = FRotator(MouseInput.Y, 0.0f, 0.0f);
    FRotator WorldRotation = FRotator(0.0f, MouseInput.X, 0.0f);

    // Додаємо локальне обертання до SpringArm
    SpringArm->AddLocalRotation(LocalRotation);

    // Додаємо світове обертання до SpringArm
    SpringArm->AddWorldRotation(WorldRotation);
}

// Приближення відалення камери 
void AMachinePawn::ZoomCamera(const FInputActionInstance& Value)
{
    const float ZoomInput = Value.GetValue().Get<float>();
    const float NewTargetArmLength = FMath::Clamp(SpringArm->TargetArmLength - ZoomInput * 200.0f, 100.0f, 1200.0f);
    SpringArm->TargetArmLength = NewTargetArmLength;
}
 
void AMachinePawn::RotateTurret(float DeltaTime)
{
    // Отримуємо ротацію камери
    FRotator CameraRotation = Camera->GetComponentRotation();

    // Обчислюємо відносну ротацію між напрямком камери і ротацією танка
    FRotator RelativeRotation = CameraRotation - GetActorRotation();
    RelativeRotation.Yaw = FMath::UnwindDegrees(RelativeRotation.Yaw);

    // Отримуємо поточну ротацію башти
    FRotator CurrentTurretRotation = FRotator(0.0f, TurretRotationYaw, 0.0f);

    // Інтерполяція ротації башти
    FRotator NewTurretRotation = FMath::RInterpTo(CurrentTurretRotation, FRotator(CurrentTurretRotation.Pitch, RelativeRotation.Yaw, CurrentTurretRotation.Roll), DeltaTime, TurretRotationSpeed);
    TurretRotationYaw = NewTurretRotation.Yaw;

    // Обчислюємо новий кут нахилу ствола з обмеженням на градус нахилу
    float NewGunPitch = FMath::Clamp(CameraRotation.Pitch, MinGunPitch, MaxGunPitch);

    // Викликаємо функцію з Interface для передачі значень до AnimBP
    if (UMyVehicleAnimationInstance* AnimInstance = Cast<UMyVehicleAnimationInstance>(SkeletalMesh->GetAnimInstance()))
    {
        AnimInstance->TurretRotationYaws = TurretRotationYaw;
        AnimInstance->GunRotationPitchs = NewGunPitch;
    }
}

void AMachinePawn::RotateGun()
{

}

void AMachinePawn::FireTower(const FInputActionInstance& Value)
{
    if (!bIsAlive || !IsValid(ProjectileSpawnPoint) || CurrentAmmo <= 0)
    {
        FakeReload();
        return;
    }
    const bool BoolValue = Value.GetValue().Get<bool>();
    if (BoolValue && CanFire())
    {
        Fire();

        CurrentAmmo--;
        OnAmmoDepleted.Broadcast(CurrentAmmo);
    }
}

void AMachinePawn::StopFireTower(const FInputActionInstance& Value)
{
    bIsFiring = false;

    // Зупиняємо звуки та ефекти
    if (AudioComponent)
    {
        AudioComponent->Stop();
    }
    if (NiagaraComponent)
    {
        NiagaraComponent->Deactivate();
    }
}

void AMachinePawn::Fire()
{
    if (!IsValid(ProjectileSpawnPoint))
    {
        return;
    }
    if (AudioComponent)
    {
        AudioComponent->Play();
    }
    if (NiagaraComponent)
    {
        NiagaraComponent->Activate(true);
    }
    // Camera shake
    if (FireShake)
    {
        if (APlayerController* PC = GetController<APlayerController>())
        {
            PC->ClientStartCameraShake(FireShake);
        }
    }

    // Отримуємо позицію та ротацію кістки ствола
    const FVector SpawnLocation = SkeletalMesh->GetSocketLocation(GunBoneName);
    const FRotator SpawnRotation = SkeletalMesh->GetSocketRotation(GunBoneName);

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;

    OnFire.Broadcast();

    GetWorld()->SpawnActor<AActor>(SpawnProjectileBlueprint, SpawnLocation, SpawnRotation, SpawnParams);
    LastFireTime = GetWorld()->GetTimeSeconds();
}

bool AMachinePawn::CanFire() const
{
    if (!bIsAlive || !IsValid(ProjectileSpawnPoint))
    {
        return false;
    }
    return GetWorld()->GetTimeSeconds() - LastFireTime >= ShotInterval;
}

void AMachinePawn::FakeReload()
{
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
    if (PlayerStarts.Num() > 0)
    {
        SetActorLocation(PlayerStarts[0]->GetActorLocation(), false, nullptr, ETeleportType::TeleportPhysics);
    }
    ReloadAmmo();
    OnAmmoDepleted.Broadcast(CurrentAmmo);
}

void AMachinePawn::ReloadAmmo()
{
    CurrentAmmo = MaxAmmo;

    //UHealthComponent* HealthComponentMax = FindComponentByClass<UHealthComponent>();
    //if (HealthComponentMax)
    //{
    //    HealthComponentMax->RestoreToMaxHealth();
    //}
}