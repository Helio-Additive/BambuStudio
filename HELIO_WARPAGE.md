# Helio warpage and printable G-code

This ports the behavior of OrcaSlicer PR #126 to Bambu Studio 2.8.4's legacy
and advanced renderers.

Enable **Experimental: Enable warping analysis** on the Helio activation success
page or in **Preferences → Other → Helio**. It defaults to off. The client adds
`simulationSettings.enableWarpingAnalysis: true` to simulation and optimization
requests only while enabled. The selected regional backend must support this
optional GraphQL input before users enable it; disabled requests omit the field.

The preview recognizes `wdm`, `wdx`, `wdy`, `wdz`, `wr`, `wtg`, `wts`, `whs`, and
`wls` in inline or immediately following `;helioadditive=` comments. Missing or
non-finite values remain unavailable. Each available metric gets its own view,
legend, and move detail. Risk and hull shrinkage use a fixed 0–1 scale. Positive
`WARPAGE_WDM_P95` headers anchor displacement scales; X/Y are signed while Z is
non-negative. Travel moves retain their travel colors.

The files have separate purposes:

- Original sliced G-code remains untouched.
- Simulation previews use annotated G-code and continue printing the input file.
- Optimizations download both annotation-free printable G-code and annotated
  preview G-code. Export, saved 3MFs, printer upload, and subsequent Helio input
  resolve the printable artifact through `PartPlate::get_gcode_filename()`.
- The preview result's `filename` remains the annotated file so its line offsets
  match the sequential G-code window.
- Per-action artifact names and staged results preserve the last completed result
  when downloads fail or a replacement is canceled. Existing action-generation
  checks continue to reject stale callbacks.

## Validation

Standalone metadata and existing material tests:

```sh
cmake -S tests/helio -B build/helio-tests
cmake --build build/helio-tests
ctest --test-dir build/helio-tests --output-on-failure
```

Full-processor regression fixtures are in `tests/fff_print/test_gcode_helio.cpp`.
They cover inline/following comments, partial backfill, invalid values, no-op/event
boundaries, and percentile headers. They require the normal desktop dependencies.

Manual verification should cover both renderers, switching between plates with
different available fields, successful simulation/optimization, canceled and
failed replacements, the sequential G-code window, export, saved sliced 3MFs,
and printer upload. The printable bytes should match the annotation-free
optimization download and the original slice should remain unchanged.
