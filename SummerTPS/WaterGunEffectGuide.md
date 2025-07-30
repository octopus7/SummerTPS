# 나이아가라: 시원한 물보라 이펙트 제작 가이드

이 가이드는 언리얼 엔진의 나이아가라 시스템을 사용하여 시원한 느낌의 방사형 물보라 이펙트를 만드는 방법을 단계별로 설명합니다. 머티리얼에 대한 사전 지식이 없어도 따라 할 수 있도록 구성되었습니다.

## 컨셉: 시원한 물보라 이펙트

좋은 물보라 이펙트는 여러 요소가 결합되어 만들어집니다.

1.  **중심 물방울 (Core Droplets):** 터지는 순간의 핵심적인 물방울들입니다. 처음에는 컸다가 빠르게 흩어지며 작아집니다.
2.  **물안개 (Mist/Vapor):** 물방울 주변에 부드럽게 퍼지는 요소입니다. 시원하고 청량한 느낌을 더해줍니다.
3.  **반짝이 (Sparkles):** 햇빛이나 조명에 반사되는 작은 물방울들입니다. 생동감과 상쾌함을 줍니다.

이 세 가지 요소를 조합하여 하나의 나이아가라 시스템을 만들겠습니다.

---

## 단계별 제작 가이드

### 1단계: 나이아가라 시스템 생성

1.  콘텐츠 브라우저에서 오른쪽 클릭 > **FX > Niagara System**을 선택합니다.
2.  **New Niagara System from selected emitters**를 선택하고 **Next**를 누릅니다.
3.  템플릿에서 **Fountain**을 선택하고 **+** 버튼을 눌러 추가한 뒤, **Finish**를 누릅니다.
4.  새로 생성된 나이아가라 시스템의 이름을 `NS_WaterGunSplash` 와 같이 지정합니다.

### 2단계: 핵심 물방울 이미터 (Fountain 이미터 수정)

생성된 `NS_WaterGunSplash`를 열고, 기존의 **Fountain** 이미터를 수정하여 핵심 물방울을 만들겠습니다.

1.  **Emitter Spawn (이미터 스폰 설정)**
    *   **Spawn Burst Instantaneous**: 이 모듈을 추가하고, 기존의 Spawn Rate는 비활성화하거나 삭제합니다. `Spawn Count`를 `50` 정도로 설정하여 발사 순간에 한 번에 파티클이 터져 나오게 합니다.

2.  **Particle Spawn (파티클 스폰 설정)**
    *   **Initialize Particle**:
        *   `Lifetime`: `0.5` ~ `1.0` 사이의 랜덤값으로 설정합니다. (Min: 0.5, Max: 1.0)
        *   `Color`: 시원한 느낌을 주는 밝은 하늘색 톤으로 설정합니다.
        *   `Sprite Size`: `Non-Uniform`으로 설정하고, X는 `10`, Y는 `20` 정도로 하여 길쭉한 물방울 모양으로 시작하게 합니다.
    *   **Shape Location**: `Sphere`로 설정하고 `Sphere Radius`를 `5.0`으로 설정하여 중심에서 퍼져나가게 합니다.
    *   **Add Velocity**: `From Point`로 설정하고, `Velocity Speed`를 `1000` ~ `1500` 사이의 랜덤값으로 설정하여 폭발적으로 퍼져나가게 합니다.

3.  **Particle Update (파티클 업데이트 설정 - 움직임과 변화)**
    *   **Scale Sprite Size**: 이 모듈을 추가합니다. 커브를 사용하여 파티클이 시간이 지남에 따라 작아지도록 설정합니다. (시작은 1, 끝은 0)
    *   **Scale Color**: 이 모듈을 추가합니다. `Alpha` 커브를 조정하여 파티클이 사라질 때 부드럽게 사라지도록 합니다. (시작은 1, 끝은 0)
    *   **Gravity Force**: Z값을 `-500` 정도로 설정하여 물방울이 중력의 영향을 받아 아래로 떨어지게 합니다.
    *   **Drag**: `Drag` 값을 `1.0` 정도로 설정하여 파티클이 공기 저항을 받아 서서히 느려지게 합니다.
    *   **Collision**: 이 모듈을 추가하여 파티클이 바닥이나 벽에 닿으면 튕기도록 설정합니다. `Bounce` 값을 조절하여 탄성을 조절할 수 있습니다.

4.  **Render (렌더링 설정)**
    *   **Sprite Renderer**: 여기에 물 느낌이 나는 머티리얼을 할당해야 합니다. 아래 **3단계**에서 간단한 물 머티리얼을 만들어서 여기에 적용하겠습니다.

### 3단계: 간단한 반투명 물 머티리얼 제작

머티리얼 지식이 없어도 따라 할 수 있는 최소한의 노드로 구성합니다.

1.  콘텐츠 브라우저에서 오른쪽 클릭 > **Materials & Textures > Material**을 선택하고 이름을 `M_WaterDroplet`으로 지정합니다.
2.  생성된 머티리얼을 엽니다.
3.  왼쪽 **Details** 패널에서 아래와 같이 설정합니다.
    *   **Blend Mode**: `Translucent` (반투명)
    *   **Shading Model**: `Unlit` (조명 영향 없음)
    *   **Two Sided**: 체크 (양면 렌더링)
4.  머티리얼 그래프에 아래와 같이 노드를 추가하고 연결합니다.
    *   **Particle Color** 노드를 추가합니다.
    *   `Particle Color`의 **맨 위 출력핀(전체 RGBA)**을 `Emissive Color`에 연결합니다.
    *   `Particle Color`의 **알파(A) 출력핀**을 `Opacity`에 연결합니다.
    *   **Constant** 노드를 추가하고 값을 `1.1`로 설정한 뒤, `Refraction`에 연결합니다. (물이 빛을 굴절시키는 효과를 흉내 냅니다)
5.  머티리얼을 **저장**하고 닫습니다.
6.  다시 `NS_WaterGunSplash` 나이아가라 에디터로 돌아와서, **Sprite Renderer**의 `Material` 슬롯에 방금 만든 `M_WaterDroplet`을 할당합니다.

### 4단계: 물안개 이미터 추가

1.  `NS_WaterGunSplash` 시스템 안에서, 기존에 수정했던 **Fountain 이미터**를 오른쪽 클릭하고 **Duplicate**를 선택하여 복제합니다.
2.  복제된 이미터의 이름을 `FX_Mist` 등으로 변경하고 아래와 같이 수정합니다.
    *   **Initialize Particle**:
        *   `Lifetime`: `1.0` ~ `1.5` (물방울보다 조금 더 오래 유지)
        *   `Color`: 더 연하고 투명도가 낮은 색상으로 설정합니다.
        *   `Sprite Size`: `20` ~ `30` 사이의 랜덤값으로 설정하여 더 크고 둥글게 시작합니다.
    *   **Add Velocity**: `Velocity Speed`를 `300` ~ `500`으로 낮춰서 부드럽게 퍼지게 합니다.
    *   **Gravity Force**: Z값을 `-50` 정도로 매우 낮게 설정하여 거의 떠 있는 것처럼 만듭니다.
    *   **Render**: `Material`을 엔진 기본 머티리얼인 `DefaultSpriteMaterial`로 변경하거나, 부드러운 원 모양의 텍스처를 사용하는 다른 머티리얼로 설정합니다.

### 5단계: 반짝이 이미터 추가 (선택 사항이지만 추천)

1.  다시 한번 **Fountain 이미터**를 복제하고 이름을 `FX_Sparkles`로 변경합니다.
2.  아래와 같이 값을 대폭 수정합니다.
    *   **Spawn Burst Instantaneous**: `Spawn Count`를 `10` 정도로 낮춥니다.
    *   **Initialize Particle**:
        *   `Lifetime`: `0.2` ~ `0.5` (매우 짧게 반짝임)
        *   `Color`: 매우 밝은 흰색 또는 하늘색으로 설정합니다.
        *   `Sprite Size`: `1` ~ `3` 사이의 매우 작은 값으로 설정합니다.
    *   **Add Velocity**: `Velocity Speed`를 `800` ~ `1200`으로 설정하여 빠르게 퍼져나가게 합니다.
    *   **Gravity Force**: 이 모듈을 **비활성화**하여 중력의 영향을 받지 않게 합니다.
    *   **Drag**: `Drag` 값을 `3.0` 정도로 높여서 빠르게 퍼졌다가 금방 멈추도록 합니다.
    *   **Render**: `Material`의 **Blend Mode**가 `Additive`인 머티리얼을 사용하면 좋습니다. `M_WaterDroplet`을 복제하여 `Blend Mode`만 `Additive`로 바꾸고 `Refraction`은 제거하여 사용해도 좋습니다.

---

### 6단계: 이펙트 적용하기

이제 완성된 `NS_WaterGunSplash` 나이아가라 시스템을 플레이어 블루프린트에 적용할 차례입니다.

1.  `TPSPlayer` 블루프린트를 엽니다.
2.  **클래스 디폴트(Class Defaults)**를 선택하거나, 컴포넌트 탭에서 `FireEffect` 변수를 찾습니다.
3.  **Fire Effect** 속성에 우리가 만든 `NS_WaterGunSplash`를 할당합니다.

이제 게임을 실행하고 무기를 발사하면, 투사체가 생성되는 위치에 시원한 물보라 이펙트가 터져 나올 것입니다. 각 이미터의 파라미터를 조금씩 조절하면서 원하는 느낌을 찾아보세요.
