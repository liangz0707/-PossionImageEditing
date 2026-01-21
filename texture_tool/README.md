# texture_tool

A small C++17 project that loads texture files into a unified, uncompressed
resource layout. It supports reading, format conversion, and writing back to
common image formats.

## Unified texture data

- texture dimension (1D / 2D / 3D / cube)
- width, height, depth
- color format (R, RG, RGB, RGBA)
- pixel type (uint8, uint16, float32)
- mip level count
- per-mip pixel data

## Supported input formats (via stb_image)

PNG, JPG/JPEG, BMP, TGA, PSD, GIF, HDR, PIC, PNM.

## Supported output formats (via stb_image_write)

PNG, JPG/JPEG, BMP, TGA (8-bit) and HDR (float32).
Writing full mip chains is not supported by these formats.

## Build

```
mkdir build
cd build
cmake ..
cmake --build .
```

## Usage

```
./texture_tool info ../data/0240020025.jpg
./texture_tool convert ../data/0240020025.jpg out.png --format=rgba8
./texture_tool convert ../data/0240020025.jpg out.hdr --format=rgba32f --mips=0
```

## Notes

- HDR input is loaded as float32 automatically.
- Output format may force the pixel type (HDR uses float32, others use uint8).
