\# Texture Requirements for Echoes of Time



This document lists all textures needed for the game. The game includes \*\*procedural fallbacks\*\* for all textures, so it will function without any external assets, but visual quality will be significantly enhanced with proper texture files.



\## Directory Structure



```

assets/



│   ├── floor\_tile.png

│   ├── floor\_concrete.png

│   ├── floor\_wood.png

│   ├── floor\_carpet.png

│   ├── ceiling\_tile.png

│   ├── door\_wood.png

│   ├── door\_metal.png

│   ├── road\_asphalt.png

│   ├── grass.png

│   ├── dirt.png

│   ├── tree\_bark.png

│   ├── tree\_leaves.png

│   ├── building\_exterior.png

│   ├── window\_glass.png

│   ├── roof\_shingles.png

│   ├── bush\_leaves.png

│   ├── flower\_petals.png

│   └── sky.png

└── shaders/

&nbsp;   ├── lighting.vs

&nbsp;   └── lighting.fs

```



\## Texture Specifications



\### Wall Textures



\#### wall\_concrete.png

\- \*\*Size\*\*: 512x512 or 1024x1024 (seamless/tileable)

\- \*\*Description\*\*: Gray concrete wall with subtle imperfections

\- \*\*Style\*\*: Industrial, slightly weathered

\- \*\*Fallback\*\*: Solid gray with border lines



\#### wall\_brick.png

\- \*\*Size\*\*: 512x512 or 1024x1024 (seamless/tileable)

\- \*\*Description\*\*: Red/brown brick pattern with mortar lines

\- \*\*Style\*\*: Standard masonry brick layout

\- \*\*Fallback\*\*: Procedurally generated brick pattern



\#### wall\_metal.png

\- \*\*Size\*\*: 512x512 or 1024x1024 (seamless/tileable)

\- \*\*Description\*\*: Brushed metal or steel panels

\- \*\*Style\*\*: Industrial, slight reflective quality

\- \*\*Fallback\*\*: Gray with horizontal lines



\### Floor Textures



\#### floor\_tile.png

\- \*\*Size\*\*: 512x512 or 1024x1024 (seamless/tileable)

\- \*\*Description\*\*: Clean white/cream floor tiles

\- \*\*Style\*\*: Office/laboratory style

\- \*\*Fallback\*\*: Checkerboard pattern



\#### floor\_concrete.png

\- \*\*Size\*\*: 512x512 or 1024x1024 (seamless/tileable)

\- \*\*Description\*\*: Polished concrete floor

\- \*\*Style\*\*: Modern industrial

\- \*\*Fallback\*\*: Solid gray



\#### floor\_wood.png

\- \*\*Size\*\*: 512x512 or 1024x1024 (seamless/tileable)

\- \*\*Description\*\*: Wooden planks or parquet

\- \*\*Style\*\*: Residential/office

\- \*\*Fallback\*\*: Brown with horizontal lines



\#### floor\_carpet.png

\- \*\*Size\*\*: 512x512 or 1024x1024 (seamless/tileable)

\- \*\*Description\*\*: Office carpet texture

\- \*\*Style\*\*: Dark red/maroon

\- \*\*Fallback\*\*: Solid dark red



\### Ceiling Textures



\#### ceiling\_tile.png

\- \*\*Size\*\*: 512x512 or 1024x1024 (seamless/tileable)

\- \*\*Description\*\*: Suspended ceiling tiles (drop ceiling)

\- \*\*Style\*\*: Commercial building standard

\- \*\*Fallback\*\*: Light gray checkerboard



\### Door Textures



\#### door\_wood.png

\- \*\*Size\*\*: 512x512 or 1024x1024

\- \*\*Description\*\*: Wooden door with panels/grain

\- \*\*Style\*\*: Interior door, brown wood

\- \*\*Fallback\*\*: Brown with rectangular panel



\#### door\_metal.png

\- \*\*Size\*\*: 512x512 or 1024x1024

\- \*\*Description\*\*: Steel door

\- \*\*Style\*\*: Security/industrial door

\- \*\*Fallback\*\*: Solid gray



\### Outdoor Textures



\#### road\_asphalt.png

\- \*\*Size\*\*: 512x512 or 1024x1024 (seamless/tileable)

\- \*\*Description\*\*: Cracked asphalt road surface

\- \*\*Style\*\*: Urban, slightly worn

\- \*\*Fallback\*\*: Dark gray



\#### grass.png

\- \*\*Size\*\*: 512x512 or 1024x1024 (seamless/tileable)

\- \*\*Description\*\*: Green grass texture with variation

\- \*\*Style\*\*: Natural, slightly unkempt

\- \*\*Fallback\*\*: Green with random darker pixels



\#### dirt.png

\- \*\*Size\*\*: 512x512 or 1024x1024 (seamless/tileable)

\- \*\*Description\*\*: Brown dirt/soil

\- \*\*Style\*\*: Natural ground

\- \*\*Fallback\*\*: Brown solid color



\### Nature Textures



\#### tree\_bark.png

\- \*\*Size\*\*: 512x512 or 1024x1024 (seamless/tileable)

\- \*\*Description\*\*: Tree bark texture (oak or similar)

\- \*\*Style\*\*: Natural wood grain

\- \*\*Fallback\*\*: Brown solid color



\#### tree\_leaves.png

\- \*\*Size\*\*: 512x512 or 1024x1024

\- \*\*Description\*\*: Dense foliage texture with alpha channel

\- \*\*Style\*\*: Green leaves, some transparency

\- \*\*Fallback\*\*: Solid green



\#### bush\_leaves.png

\- \*\*Size\*\*: 256x256 or 512x512

\- \*\*Description\*\*: Smaller leaf texture for bushes

\- \*\*Style\*\*: Dense green foliage

\- \*\*Fallback\*\*: Medium green



\#### flower\_petals.png

\- \*\*Size\*\*: 256x256 or 512x512

\- \*\*Description\*\*: Flower petal texture with alpha

\- \*\*Style\*\*: Colorful, delicate

\- \*\*Fallback\*\*: Pink solid color



\### Building Textures



\#### building\_exterior.png

\- \*\*Size\*\*: 1024x1024 or 2048x2048 (seamless/tileable)

\- \*\*Description\*\*: Building facade (concrete or stucco)

\- \*\*Style\*\*: Modern office building

\- \*\*Fallback\*\*: Light gray



\#### window\_glass.png

\- \*\*Size\*\*: 512x512

\- \*\*Description\*\*: Semi-transparent glass with reflections

\- \*\*Style\*\*: Blue-tinted glass, partially transparent

\- \*\*Fallback\*\*: Light blue, semi-transparent



\#### roof\_shingles.png

\- \*\*Size\*\*: 512x512 or 1024x1024 (seamless/tileable)

\- \*\*Description\*\*: Roof shingle pattern

\- \*\*Style\*\*: Asphalt shingles, dark color

\- \*\*Fallback\*\*: Dark brown/gray



\### Sky Textures



\#### sky.png

\- \*\*Size\*\*: 2048x1024 or higher (HDR if possible)

\- \*\*Description\*\*: Sky gradient or skybox texture

\- \*\*Style\*\*: Blue sky with subtle clouds

\- \*\*Fallback\*\*: Blue gradient (light to dark)



\## Sourcing Recommendations



\### Free Texture Resources



1\. \*\*OpenGameArt.org\*\* - Public domain game assets

2\. \*\*Textures.com\*\* (formerly CGTextures) - Free account with limited downloads

3\. \*\*Poly Haven\*\* (polyhaven.com) - CC0 textures, HDRIs

4\. \*\*AmbientCG\*\* (ambientcg.com) - CC0 PBR textures

5\. \*\*3D Textures.me\*\* - Free seamless textures



\### Texture Creation Tools



\- \*\*GIMP\*\* (free) - Basic texture editing

\- \*\*Krita\*\* (free) - Digital painting and textures

\- \*\*Material Maker\*\* (free) - Procedural texture generation

\- \*\*Substance Designer/Painter\*\* (paid) - Professional PBR workflow

\- \*\*Blender\*\* (free) - Baking and procedural textures



\## Texture Format Guidelines



\- \*\*Format\*\*: PNG (for transparency support) or JPG (for solid textures)

\- \*\*Color Space\*\*: sRGB

\- \*\*Mip Maps\*\*: Will be generated automatically by engine

\- \*\*Seamless\*\*: All tileable textures should wrap perfectly

\- \*\*Resolution\*\*: 512x512 minimum, 1024x1024 recommended for quality

\- \*\*Compression\*\*: Keep source files uncompressed; engine handles compression



\## Normal Maps (Optional Enhancement)



For enhanced visual quality, consider adding normal maps:

\- Same filename with `\_normal` suffix (e.g., `wall\_concrete\_normal.png`)

\- Tangent space normal maps

\- Purple/blue tint in base color

\- Same resolution as diffuse texture



\## Performance Considerations



\- Use lower resolution textures (512x512) for:

&nbsp; - Small objects (flowers, bushes)

&nbsp; - Distant objects

&nbsp; - Performance-critical areas



\- Use higher resolution (1024x1024 or 2048x2048) for:

&nbsp; - Close-up surfaces (walls, floors)

&nbsp; - Important visual elements

&nbsp; - Detailed textures (building exteriors)



\## Testing the Game Without Textures



The game is fully functional without any texture files:

1\. All textures have \*\*procedural fallbacks\*\*

2\. Missing textures log warnings but don't crash

3\. Fallback textures maintain visual distinction between surface types

4\. Magenta/purple checkerboard indicates completely missing texture system



\## License Compliance



When sourcing textures, ensure:

\- ✅ License allows use in games

\- ✅ License allows redistribution (if distributing game)

\- ✅ Attribution requirements are met (if required)

\- ✅ Commercial use allowed (if selling game)



Recommended licenses:

\- \*\*CC0\*\* (Public Domain) - No restrictions

\- \*\*CC-BY\*\* - Requires attribution

\- \*\*MIT/Apache\*\* - Permissive licenses



---



\*\*Note\*\*: The game will automatically generate procedural textures for any missing files, ensuring it remains playable even with a completely empty assets folder. External textures simply enhance visual quality.

