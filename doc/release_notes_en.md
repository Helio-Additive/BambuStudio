## Unreleased fixes

### Match the Helio optimization layer range to the preview

- Helio reads the right-hand slider's maximum index via `GetMaxValue()` and maps it to the displayed layer number at that position, rather than deriving the index from the render-record array length. Invalid or uninitialized indices return 0.
- Fixed the Helio optimization upper bound potentially exceeding the preview layer count for multi-material prints. The previous count used render-layer records, which can revisit the same Z height. The default upper bound and input validation now use the preview slider's layer-number mapping.
- Sequential object printing retains its existing layer sequence instead of merging equal heights across objects. Selecting a smaller preview range does not reduce the full optimization range.
- Optimization requests still send the layer range entered by the user. This change does not change sliced layer heights or add print layers.
- Validation: an isolated C++ check exercised the production counting functions for empty data, a single layer, 388 render records mapping to 363 layers, agreement with preview labels, sequential printing, single-material printing, and reloading. Full desktop compilation remains unverified because GUI development libraries are unavailable. The original screenshot's G-code has not been reproduced; the 388-to-363 check uses synthetic data.

---

**New Features**
1. 3D text tool
2. Shared data between an object and its copies
3. Parameter table
4. User manual
5. Arachne feature

**Improvements**
1. Added support for exporting generic 3mf that is compatible with other slicers
2. Optimized the performance of hybrid and tree support
3. Added traditional timelapse mode
4. Added support for Textured PEI plate
5. Added support for export/import preset
6. Added random seam position
7. Added Magyar translations
8. Fixed some known bugs

For details, please check https://github.com/bambulab/BambuStudio/releases
