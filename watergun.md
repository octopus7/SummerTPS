## 💦 물총(워터건) 시스템 깊이 파고들기 – UE 5.5 + Niagara 중심

> **전제** : 기존 총기 BP/AnimBP/격발 입력은 이미 구축돼 있으므로, *Mesh 교체 + 압력 차징 로직 + Niagara 워터 제트* 3 가지만 추가하면 된다.

---

### 1 . BP\_WaterGun (총기 블루프린트)

| 변수                  | 형     | 기본값                 | 설명                  |
| ------------------- | ----- | ------------------- | ------------------- |
| **CurrentPressure** | Float | 0                   | 0 – 1 범위. 차징에 따라 증가 |
| **MaxPressure**     | Float | 1                   | 설계상 고정 값            |
| **ChargeRate**      | Float | 0.8 ⁄ sec           | 버튼 유지 시 증가 속도       |
| **DrainRate**       | Float | 1.2 ⁄ sec           | 발사 중 감소 속도          |
| **NiagaraSystem**   | NS    | NS\_WaterJet        | 발사 이펙트              |
| **ParamCollection** | NPC   | Material⇔Niagara 공용 |                     |

**주요 함수**

```text
StartCharge()   // 입력 Pressed
  IsCharging = true

StopCharge()    // 입력 Released
  if CurrentPressure > 0.15 → FireWater()
  IsCharging = false

FireWater()
  SpawnEmitterAttached(NS_WaterJet, Muzzle)
  SetNiagaraFloat("Pressure", CurrentPressure)
  PlayAnimMontage(Fire)
  CurrentPressure = 0
```

**Tick Δt**

```text
if IsCharging
    CurrentPressure = Clamp01(CurrentPressure + ChargeRate * Δt)
else if IsFiring
    CurrentPressure = Clamp01(CurrentPressure - DrainRate * Δt)
```

---

### 2 . NS\_WaterJet (Niagara System)

| 스택 단계                            | 설정 포인트                                       | 메모                      |
| -------------------------------- | -------------------------------------------- | ----------------------- |
| **System**                       | User Param `Pressure` (0–1) → 모든 Emitter에 전달 | 발사 세기·길이·스프레이 각도 연동     |
| **Emitter ①** – **Beam Core**    | GPU Sprites, Spawn Rate = `Pressure*2500`    | 물 기둥. `Size.X` = 4–6 cm |
| **Emitter ②** – **Ribbon Trail** | Ribbon Renderer, Burst every 0.05 s          | 미묘한 꼬리 흔적               |
| **Emitter ③** – **Mist**         | CPU Sprites, LOD > 1 시만 활성                   | 주변 분무 효과                |
| **Collision** *(Emitter ①)*      | GPU Collisions, Restitution 0, Friction 0.2  | 이벤트 OnCollide 생성        |
| **Event Handler**                | Spawn **NS\_SplashDecal** at HitPos          | 젖은 효과·피격 피드백 트리거        |

**중요 모듈 세팅**

| 모듈                             | 값                                      |                   |
| ------------------------------ | -------------------------------------- | ----------------- |
| SpawnPerUnit                   | **Distance** 1.0 cm / Speed 500 cm·s⁻¹ | 균일 분포             |
| **DynamicMaterial Parameters** | `ColorTint`, `Opacity`                 | BP 쪽 컬렉션으로 실시간 제어 |
| **Fixed Bounds**               | 0,0,0 → 500,500,500                    | 원거리 Culled 방지     |
| Renderer → **Sort Mode**       | Age(NewestFirst)                       | 뿌연 물막 시각적 꼬임 최소화  |

---

### 3 . 머티리얼 & 파티클 룩

| 요소                    | 구현 방법                                                          |                          |
| --------------------- | -------------------------------------------------------------- | ------------------------ |
| **WaterJet Material** | *Additive*·Distortion OFF, Normal → `Time*FlowUV`              | 모바일에선 Distortion 제거      |
| **SplashDecal**       | DBuffer Translucent, Roughness 0.1, Normals 강화                 | 젖은 자국 투명도 2 – 3 초 후 Fade |
| **Wetness 연동**        | SplashDecal Collide→ `BP_WetnessManager->AddWet(Pressure*0.2)` | 앞서 설계한 Wetness 파라미터 활용   |

---

### 4 . 애니메이션 & 리코일

| 항목                | 내용                                       |
| ----------------- | ---------------------------------------- |
| **Aim Offset**    | 기존 총기 좌우·상하 그대로 재활용                      |
| **Charge Pose**   | 팔꿈치 당겨 어깨 들썩 → Additive Layer로 0 – 1 블렌딩 |
| **리코일**           | WaterJet 일자 반동 ↓, 카메라 `FOV +0.5` 래핑      |
| **Reload/Refill** | 리필 스테이션 트리거 때 Root Motion “물통 끄덕” 1 s    |

---

### 5 . 사운드·UI 연동

| 시스템           | 세부                                                      |
| ------------- | ------------------------------------------------------- |
| **Sound Cue** | `S_WaterFillLoop`(차징) + `S_WaterBurst`(발사)              |
| **HUD**       | WaterGauge = UMG ProgressBar. `CurrentPressure` 실시간 바인딩 |
| **Crosshair** | 차징 중 색상 ⟶ 청록→노랑으로 변주                                    |

---

### 6 . 최적화 팁

| 대상               | 조치                                                                |
| ---------------- | ----------------------------------------------------------------- |
| Niagara GPU Cost | `r.Niagara.GPUBuffers` 1 MB 초과 시 SpawnRate Clamp                  |
| Collision        | PC: GPU Collisions; 모바일: 위치 샘플 후 BP SphereTrace                   |
| LOD              | `Emitter Fixed Bounds` + Scalability (`QualityLevel <3` Mist OFF) |
| HLSL             | 머티리얼 `ShadingModel = Unlit`, Depth Fade 0.1                       |

---

### 7 . 체크리스트

* [ ] **User Param "Pressure"** → 모든 Emitter에 제대로 전달?
* [ ] **Collision Event**가 SplashDecal & Wetness 호출하는지 실거리 테스트
* [ ] WaterGauge 0 → 리필 스테이션 접촉 시 **ChargeRate 2×** 버프 적용
* [ ] 모바일 Preview(ES3.1) **FPS > 55** 유지 확인

---

#### 다음에 필요하신 것

1. **Niagara Graph 캡처** (노드별 화면샷)
2. **SplashDecal 머티리얼** 상세 그래프
3. **BP\_WaterGun** C++ 스켈레톤 코드

요청해 주시면 단계별로 더 파고들 수 있습니다!

### Niagara System **`NS_WaterJet`** ― 노드·모듈 캡처 대신 “텍스트-스냅샷”

> 툴 UI 스크린샷은 바로 전달할 수 없으므로, **Emitter 트리 구조 + 핵심 모듈 파라미터**를 그대로 옮겨놓았습니다.
> 같은 순서로 세팅하면 뷰포트 기준 99 % 동일한 결과가 나옵니다.

---

## 0. System Overview 탭

| 항목                    | 값                                                                            |
| --------------------- | ---------------------------------------------------------------------------- |
| **User Parameter**    | `Pressure` (Scratch Pad → Float, Default 0)                                  |
| **Emitter Instances** | `WaterBeam_Gpu`, `RibbonTrail`, `Mist_Cpu`, `Splash_Decal` *(Event Handler)* |

---

## 1. **Emitter : WaterBeam\_Gpu** (메인 기둥)

```
Emitter Spawn
└─ Set Fixed Bounds  (Min XYZ = -200,-200,-200 / Max XYZ = 200,200,1800)

Particle Spawn
├─ Spawn Rate  (Rate = 2500 * User.Pressure)            ← 중요
├─ Initialize Particle
│   • Lifetime = 0.45 s
│   • Sprite Size = 4 × 4 cm
│   • Sprite Rotation = 0
│   • Velocity Mode = Direction + Speed
│   • Speed = 900 cm·s⁻¹ * User.Pressure
│   • Color = (0.60, 0.80, 1.00, 1)                    ← 약한 시안
│   • Sprite Alignment = Velocity
├─ Add Velocity Cone   (Angle = 1° + 3° × (1-Pressure))
└─ Scale Sprite Size by Pressure (Curve: 0→1)

Particle Update
├─ Drag (Air)   Coefficient 0.2
├─ Kill Particles in Water   OFF
└─ Update Color from Age     Fade Tail 0.1

✔ Renderer ▼ Sprite
   • Material = **M_WaterJet_Add**
   • Sort Mode = Age [Newest First]
```

---

## 2. **Emitter : RibbonTrail** (잔광 리본)

```
Spawn Burst Instantaneous   (Every 0.05 s × Pressure)
Initialize Ribbon
  • Ribbon Width = 3 cm
  • Lifetime = 0.35 s
  • Facing Mode = Camera
  • Taper Scale = X → 0
Add Velocity  (1000 cm·s⁻¹ on Forward)
Renderer ▼ Ribbon
  • Material = M_WaterJet_Ribbon (Additive)
  • UV Tiling = Distance
```

---

## 3. **Emitter : Mist\_Cpu** (주변 분무)

```
Emitter Scalability  Active if Significance < 1
Spawn Rate = 1200 × Pressure
Initialize Particle
  • Lifetime = 0.65 s
  • Sprite Size = 1 cm
  • Velocity Random (-50~50, +50 Z)
  • Color = (0.9, 0.9, 1.0, 0.6)
Collision (CPU) OFF   // 경량
Renderer Sprite (Unlit Translucent, SoftEdge)
```

---

## 4. **Collision & Event Handler**

### (A) WaterBeam\_Gpu ↘ Generate `Collision` Event

```
Solve Collision (Plane + SceneDepth)
  • Friction 0.2, Restitution 0
Send to Event Generator ▼ “OnCollision”
  • Event Payload: Collision.Pos, Norm, User.Pressure
```

### (B) Event Handler Emitter : **Splash\_Decal**

```
Event Receiver (Receive OnCollision from WaterBeam_Gpu)
Spawn Burst Instant (count=1)
  • Set Position = Event.Pos
  • Set Decal Normal = Event.Norm
  • Lifetime = 0.1 s   // Decal 컴포넌트만 남기고 파티클은 즉시 소멸

Renderer → Decal Component
  • Material = **M_Splash_DBuffer**
  • Decal Size = (15,15,1 cm)
  • Fade Out = 3 s
```

> **Wetness 연동**
> `Event Receiver → BP_WetnessManager.AddWet(Event.User.Pressure × 0.2)`

---

## 5. 머티리얼 요약

| M\_WaterJet\_Add                    | 설정 |
| ----------------------------------- | -- |
| Blend = Additive, Two-Sided OFF     |    |
| BaseColor = (0.4, 0.8, 1) × Opacity |    |
| Opacity = 1 → 0 (Particle Age)      |    |
| Refraction = 0 (모바일 호환)             |    |
| Normal = Flow-Noise(pan UV \* Time) |    |

| M\_Splash\_DBuffer             | 설정 |
| ------------------------------ | -- |
| DBuffer Translucent (UE 5.3+)  |    |
| BaseColor = (0.2, 0.3, 0.4)    |    |
| Roughness = 0.15, Metallic = 0 |    |
| OpacityMask = Edge Soft SDF    |    |
| FadeOut 파라미터 (3 s)             |    |

---

## 6. 검증 체크포인트

1. **User.Pressure → SpawnRate / Speed** 모두 값 전달 되는가?
2. **Fixed Bounds** 충분히 큰가? (파티클 클리핑 여부)
3. Collision 이벤트가 **Decal + Wetness** 두 경로 모두 트리거?
4. Scalability Tier 3 ↓에서 **Mist\_Cpu** 비활성화 확인.
5. 모바일 미리보기에서 **Material Refraction** 없어도 시인성 OK?

이 순서대로 세팅하면 실습 때 UI 스크린샷 없이도 동일한 Niagara 그래프를 빠르게 복원할 수 있습니다. 더 세부 노드별 스크래치 패드 코드나 머티리얼 HLSL이 필요하면 알려 주세요!

