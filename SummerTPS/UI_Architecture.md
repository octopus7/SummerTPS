UI Architecture / UI 아키텍처 (Subsystem + ViewModel)

Overview / 개요
- EN: Bridge all game→UI updates through a Local Player Subsystem. Gameplay code never touches UMG directly.
- KO: 게임→UI 갱신은 Local Player Subsystem을 통해서만 전달합니다. 게임 코드는 UMG에 직접 접근하지 않습니다.
- EN: UMG binds to lightweight ViewModels (UObjects) that mirror state and fire change events.
- KO: UMG는 상태를 보유한 ViewModel(UObject)에만 바인딩하고, 변경 이벤트를 통해 갱신됩니다.

Key Types / 주요 타입
- EN: `UUISessionSubsystem` (ULocalPlayerSubsystem)
  - Owns: HUD widget instance (optional), `UCombatStateViewModel`, `UHealthViewModel`.
  - API: `EnsureHUD`, `EnsureHUDWithClass`, `PushCombatState`, `PushHealth`, `GetCombatVM`, `GetHealthVM`.
- KO: `UUISessionSubsystem` (로컬 플레이어 서브시스템)
  - 보유: HUD 위젯 인스턴스(선택), `UCombatStateViewModel`, `UHealthViewModel`.
  - API: `EnsureHUD`, `EnsureHUDWithClass`, `PushCombatState`, `PushHealth`, `GetCombatVM`, `GetHealthVM`.
- EN: `UCombatStateViewModel` (UObject) — `CombatState`, `OnCombatStateChanged`, `SetCombatState()`
- KO: `UCombatStateViewModel` (UObject) — `CombatState`, `OnCombatStateChanged`, `SetCombatState()`
- EN: `UHealthViewModel` (UObject) — `Current`, `Max`, `OnHealthChangedVM`, `SetHealth()`
- KO: `UHealthViewModel` (UObject) — `Current`, `Max`, `OnHealthChangedVM`, `SetHealth()`

Gameplay Integration / 게임 연동
- EN: `ATPSPlayer::BeginPlay` ensures HUD and pushes initial CombatState/Health via subsystem.
- KO: `ATPSPlayer::BeginPlay`에서 Subsystem이 HUD를 생성하고 초기 전투상태/체력을 푸시합니다.
- EN: `ATPSPlayer::UpdateCombatStateUI` calls `PushCombatState` instead of touching widgets.
- KO: `ATPSPlayer::UpdateCombatStateUI`는 위젯 직접 접근 대신 `PushCombatState`를 호출합니다.
- EN: `ATPSPlayer::OnHealthChanged` calls `PushHealth(Current, Max)`.
- KO: `ATPSPlayer::OnHealthChanged`는 `PushHealth(Current, Max)`를 호출합니다.

UMG Binding Pattern / UMG 바인딩 패턴
- EN: Widgets do NOT query the player or subsystem. The subsystem injects ViewModels.
- KO: 위젯은 플레이어/서브시스템을 찾지 않습니다. Subsystem이 ViewModel을 주입합니다.
- EN: If the HUD derives from `UCombatStateWidget`, the subsystem calls `InitializeViewModels(CombatVM, HealthVM)` after creation.
- KO: HUD가 `UCombatStateWidget`를 부모로 할 경우 생성 직후 `InitializeViewModels(CombatVM, HealthVM)`가 호출됩니다.
- EN: Health ProgressBar auto-binding: add a ProgressBar named `HPBar` in the HUD BP; C++ auto-updates it on health changes.
- KO: 체력 ProgressBar 자동 바인딩: HUD BP에 이름이 `HPBar`인 ProgressBar를 추가하면 C++이 체력 변경 시 자동으로 갱신합니다.
- EN/KR: Optional logic can go in `OnViewModelsReady` (Blueprint).

How HUD is Created
- Preferred: Configure a HUD widget Blueprint class on the player (e.g., `CombatStateWidgetClass`).
- `UUISessionSubsystem::EnsureHUDWithClass(PC, Class)` creates and adds it to the viewport.
- Alternatively, set `UUISessionSubsystem.HUDClass` in code and call `EnsureHUD(PC)`.

Files
- `Source/SummerTPS/Public/UI/UISessionSubsystem.h`
- `Source/SummerTPS/Private/UI/UISessionSubsystem.cpp`
- `Source/SummerTPS/Public/UI/CombatStateViewModel.h`
- `Source/SummerTPS/Public/UI/HealthViewModel.h`
- `Source/SummerTPS/Private/TPSPlayer.cpp` (refactored to use Subsystem)

Notes / 참고
- EN: The subsystem is local-player–scoped, so it supports split-screen and multiplayer clients.
- KO: Subsystem은 로컬 플레이어 범위이므로 스플릿 스크린/멀티플레이에 안전하게 동작합니다.
- EN: To expand UI, add more ViewModels and Push* methods; UMG remains decoupled from gameplay.
- KO: UI 확장은 ViewModel과 Push* API를 추가하여 수행하며, UMG는 게임 코드와 분리된 상태를 유지합니다.
