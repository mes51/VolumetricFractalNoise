# Volumetric Fractal Noise

---

## is何

3Dなフラクタルノイズを描画します

## Requirements

* 64bit Windows
    * Macは非対応
* After Effects CC 2017
* OpenGL 4.0以上対応のグラフィックカード

## How to Install / Uninstall

AEのPlug-insディレクトリにコピー、または削除します

## Parameters

### Noise

大体本家のフラクタルノイズと同じような感じ

#### Color

ノイズの色

#### Contrast

ノイズの透明度のコントラスト

#### Density

ノイズの濃さ

#### Transform

ノイズの位置やサイズ、回転

#### Octave

複雑度

#### Evolution

展開

### Fall off

ノイズの表示エリアの端の方の距離減衰の設定

#### Size X / Size Y / Size Z

各方向の減衰が起こる範囲

#### Power

減衰の強さ

### Geometry

表示エリアの変形の設定

### Render

描画設定

#### Cast Resolution

Ray Castingの粒度。小さいほど詳細に描画される

#### Distance Attenuation

カメラの視点からの距離による減衰の強さ

---

## Depenent Libraries

* [glbinding](https://github.com/cginternals/glbinding)
* [glm](https://github.com/g-truc/glm)
* [webgl-noise](https://github.com/ashima/webgl-noise)

## License

MIT
