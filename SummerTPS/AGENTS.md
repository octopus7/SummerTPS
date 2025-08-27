# Repository Guidelines

## Project Structure & Module Organization
- `Source/SummerTPS`: Game module. Use `Public/` for headers and `Private/` for implementation. Module deps are defined in `SummerTPS.Build.cs` (Core, Engine, EnhancedInput, Niagara, AIModule, GameplayTasks, NavigationSystem).
- `Content/`: Unreal assets (Blueprints, FX, Characters, Maps). Example map: `Content/Maps/BasicMap.umap`.
- `Config/`: Project settings (`DefaultEngine.ini`, `DefaultInput.ini`, etc.).
- `SummerTPS.uproject`: Open this in Unreal Editor. `SummerTPS.sln`: C++ solution for IDE builds.
- Generated folders (`Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`) should not be edited.

## Build, Test, and Development Commands
- Open Editor: Double‑click `SummerTPS.uproject` and use PIE to run.
- Build via IDE: Open `SummerTPS.sln` and build the `Development Editor | Win64` target.
- CLI build (example): `Engine/Build/BatchFiles/Build.bat SummerTPSEditor Win64 Development -Project="<path>/SummerTPS.uproject" -WaitMutex -FromMsBuild`.
- Map hint: Load `BasicMap` to validate gameplay and input (Enhanced Input).

## Coding Style & Naming Conventions
- Follow Unreal C++ style. Examples: classes `AEnemyCharacter`, `UHealthComponent`; booleans prefixed `b` (e.g., `bIsAiming`); sockets/IDs as `FName` (e.g., `WeaponSocketName`).
- Match existing indentation (tabs in C++/Build.cs). Keep includes ordered and minimal; prefer forward declarations in headers.
- Expose properties with `UPROPERTY` and functions with `UFUNCTION` only when needed for Editor/Blueprints.

## Testing Guidelines
- No formal unit tests present. Prefer Editor validation: PIE, `Stat`/`ShowFlag` toggles, and `DrawDebug*` helpers (used in `TPSPlayer`).
- If adding automation, place tests under `Source/SummerTPS/Private/Tests` and run with Unreal Automation.
- Keep gameplay changes demonstrable on `BasicMap` or provide a test map.

## Commit & Pull Request Guidelines
- Commits: Imperative mood and scoped (e.g., `feat(player): add sprint camera interp`, `fix(ai): correct patrol radius`). Commit assets with meaningful messages and minimize unrelated asset churn.
- PRs: Include a concise description, linked issues, steps to validate (map, pawn/blueprint names), and screenshots/video for visual changes. Note any `Config/` updates and new plugin/dependency requirements in `SummerTPS.Build.cs`.

## Security & Configuration Tips
- Avoid storing secrets in `Config/`. Keep changes to generated folders out of reviews when possible. Validate added dependencies in `SummerTPS.Build.cs` and the `.uproject` file.

## Documentation Conventions
- UI-related docs should be bilingual (Korean/English). When adding or updating UI documentation (e.g., `UI_Architecture.md`), include Korean and English side-by-side so designers and engineers can both follow the guidance.
