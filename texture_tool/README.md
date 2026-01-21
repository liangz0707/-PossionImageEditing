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

## Features

- load textures into unified, uncompressed layout
- convert between channel formats and pixel types
- optional mipmap generation (2D textures)
- vertical flip on load or save

## Supported input formats

- PNG, JPG/JPEG, BMP, TGA, PSD, GIF, HDR, PIC, PNM (stb_image)
- EXR (tinyexr)

## Supported output formats

- PNG, JPG/JPEG, BMP, TGA (8-bit) and HDR (float32) (stb_image_write)
- EXR (float32) (tinyexr)
- PPM/PGM/PNM (8-bit binary P6/P5)

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
- EXR output is written as float32.
- PPM/PGM/PNM output is 8-bit only.
- Output format may force the pixel type (HDR/EXR use float32, others use uint8).
