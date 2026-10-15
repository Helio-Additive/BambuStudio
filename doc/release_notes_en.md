## Unreleased fixes

- Ported onto official Bambu Studio 2.8.4 Public Beta (`v02.08.04.57`); material-name handling uses the new catalog loader while retaining response validation, retries, and caching.
- Helio optimization ranges now use the local G-code exporter's logical layer count, shared with `total_layer_count`, stored on successful export and cleared on result reset. Missing valid slice results require reslicing; single-layer ranges are 1–1 and multi-layer defaults start at layer 2. Preview records and sliders are no longer used. Scope is local slicing only; Helio backend layer-number conventions remain unverified.
- Fixed catalog materials being skipped when Bambu Studio aliases are missing, null, or empty: matching now falls back to the catalog name. Synthetic Xioneer VXL111 response records cover this case; the user's live catalog response has not been verified.
- Helio material matching now removes leading square swatch glyphs (such as `□`, `■`, and `🟧`) and adjacent whitespace from name strings for single-material, multi-material, and token matching. Preset lookup keys, displayed names, menu swatches, and symbols inside names remain unchanged. Menu swatches are separate bitmaps and are not part of name strings.
- Validation: standalone material-name tests cover 20 normalization cases and decorated-name matching. Beta catalog/retry regression tests passed (21 test cases, 345 assertions), as did the extracted slicer layer-count checks. Full desktop and live Helio matching remain unverified.

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
