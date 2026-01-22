# texture_tool

A small C++17 project that loads texture files into a unified, uncompressed
resource layout. It supports reading, format conversion, and writing back to
common image formats.

## Unified texture data

- texture dimension (1D / 2D / 3D / cube)
- width, height, depth
- channel layout (R/RG/RGB/RGBA/Unknown) + channel count
- pixel type (uint8, uint16, float32)
- channel names (if the format provides them)
- mip level count
- per-mip pixel data

## Features

- OOP-based core `TextureResource` for unified data
- codec registry to load/save multiple formats
- convert between channel counts and pixel types
- optional mipmap generation (2D textures) with pluggable generators
- vertical flip on load or save

## Supported input formats

- PNG, JPG/JPEG, BMP, TGA, PSD, GIF, HDR, PIC (stb_image)
- PNM/PPM/PGM (built-in)
- QOI (built-in)
- EXR (tinyexr)
- Optional: OpenImageIO (if found) for more formats (TIFF, DDS, DPX, etc.)

## Supported output formats

- PNG, JPG/JPEG, BMP, TGA (8-bit) and HDR (float32) (stb_image_write)
- EXR (float32) (tinyexr)
- PPM/PGM/PNM (8-bit binary P6/P5)
- QOI (8-bit)

Writing full mip chains is not supported by these formats.

## Build

```
mkdir build
cd build
cmake ..
cmake --build .
```

## Library output

- `texture_lib` shared library (DLL/SO) for external integration
- `texture_tool` standalone CLI executable

To enable the optional OpenImageIO codec (if installed on your system):

```
cmake -DTEXTURE_USE_OIIO=ON ..
```

## Usage

```
./texture_tool list
./texture_tool info ../data/0240020025.jpg
./texture_tool convert ../data/0240020025.jpg out.png --format=rgba8
./texture_tool convert ../data/0240020025.jpg out.hdr --format=rgba32f --mips=0
./texture_tool convert ../data/0240020025.jpg out.exr --format=rgba32f --compression=zip
```

## Notes

- HDR input is loaded as float32 automatically.
- EXR output is written as float32.
- PPM/PGM/PNM output is 8-bit only.
- QOI output is 8-bit only.
- Output format may force the pixel type (HDR/EXR use float32, others use uint8).
