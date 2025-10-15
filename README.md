# Computer Rendering Coursework Skeleton

This folder contains a minimal scaffold following the requested structure.

## Structure

```
s2897495/
├── System.txt
├── FeatureList.txt
├── Code/
│   ├── Makefile
│   └── raytracer.cpp
├── Blend/
│   └── Export.py
├── Textures/
├── ASCII/
├── Output/
└── Report/
```

## Build & Run

```bash
cd s2897495/Code
make
make run
```

The renderer writes a PPM image to `../Output/output.ppm`.

## Notes
- Add texture `.ppm` files under `Textures/`.
- Place exported ASCII scenes under `ASCII/` (e.g., `scene.txt`).
- Submit your PDF report to `Report/s2897495.pdf`.
- Fill in `System.txt` (system spec) and `FeatureList.txt` (implemented features).
