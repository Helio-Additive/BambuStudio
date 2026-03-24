# Helio Additive for BambuStudio - Release Notes

**Version:** 1.0.0-helio (2026-03-24)
**Base:** BambuStudio v02.04.00.70 (from [bambulab/BambuStudio](https://github.com/bambulab/BambuStudio))

This document describes the changes and additions made in the Helio-Additive fork of BambuStudio compared to the upstream BambuStudio repository.

---

## Overview

The Helio-Additive fork integrates **Helio Additive** functionality into BambuStudio, enabling advanced simulation and optimization features for 3D printing. Helio Additive helps users achieve faster, more reliable, and warp-free prints through AI-powered thermal analysis and print optimization.

---

## New Features

### 🚀 Helio Additive Integration

#### Core Functionality
- **Helio Assess (Simulation)**: Thermal analysis to predict print outcomes and identify potential issues before printing
- **Helio Enhance (Optimization)**: AI-powered optimization to improve print speed, quality, and reliability
- **Thermal Index Visualization**: Color-coded thermal analysis overlaid on the 3D preview

#### User Interface
- **Helio Activation Panel**: Easy onboarding flow with "Enable Helio Additive" button and first-time tutorial
- **Helio Input Dialog**: Modern card-based UI with dark/light mode support for launching simulations and optimizations
- **Simulation Results Dialog**: Displays print outcome predictions, thermal analysis, and potential speed improvements
- **Optimization Results Dialog**: Shows quality improvements, time savings, and "Print Plate" action button
- **History Dialog**: View past 10 optimizations and simulations with download functionality for enhanced GCode files

#### Mode Cards
- **Assess Mode**: Shield icon with simulation-focused features
- **Enhance Mode**: Speed icon with optimization-focused features
- Purple CTA for Assess, Blue CTA for Enhance

---

### 🎨 Material Handling Improvements

#### Fuzzy Material Matching
- **Prefix/suffix matching**: Recognizes materials like "Bambu TPU 95A HF Yellow" or "yellow Bambu PCFR"
- **Token-based matching**: Last-resort matching with user confirmation (e.g., "blueBambu ABS" → "Bambu ABS")
- **Longest match preference**: Selects most specific material when multiple matches exist

#### Unsupported Material Handling
- **Material type-based fallback**: Extracts keywords (ABS, PLA, TPU, etc.) to find similar materials
- **Reference material selection**: Shows dialog with similar materials and prominent warnings about formulation differences
- **Automatic Slicer defaults**: Forces "Slicer default" limits when using reference materials

#### Multi-Material Support
- **Multi-color filament compatibility detection**: Warns when using multiple different materials
- **Mixed supported/unsupported handling**: Special dialog for prints using both supported and unsupported materials
- **Consistent two-option dialogs**: All material dialogs have "Proceed Anyway" (orange) and "Go Back ✓ Recommended" (green) options

---

### 🖨️ Printer Support

#### Printer Matching
- **Robust word-boundary awareness**: Handles modified preset names (e.g., "myBambu Lab H2Dsmells")
- **Longest match preference**: Selects specific variants (H2D Pro over H2D)
- **PrinterSelectionDialog**: Shows when unsupported printer is detected

#### Chamber Temperature Support
- **Three-state logic** based on `heatedChamber` API field:
  - **No chamber** (A1, A1 mini): Shows "Environment Temperature" input
  - **Passive chamber** (X1C, P1S, P2S): Shows "Chamber Temperature" input *(NEW)*
  - **Heated chamber** (H2S, H2D, X1E): Hidden (backend controls temperature)

---

### 📊 Print Priority Selection

- **Outcome-based choices**: Replaced "Optimise Outer Walls (Yes/No)" with user-friendly options
  - "Preserve Surface Finish" - Maintains surface quality
  - "Speed & Strength" - Optimizes for faster, stronger prints
- **Dynamic options from API**: Fetches available print priority options from Helio backend
- **Fallback support**: Uses hard-coded options when API unavailable

---

### 🔔 Notifications & Guidance

#### Tutorial System
- First-time tutorial popup when user clicks "Run Your First Optimization"
- Tutorial shows after successful slicing
- Tutorial shows when HelioInputDialog is displayed
- Uses HintNotificationLevel for prominent visibility (5-minute duration, special styling)

#### Error Handling
- **Detailed G-Code error detection**: Parses `errors` and `errorsV2` fields from API
- **RESTRICTED status support**: Clear error messages when GCode contains forbidden commands
- **Wiki link extraction**: Makes URLs in error notifications clickable
- **Actual backend errors**: Shows real error messages instead of generic failures

#### Daily Tips
- Added 8 Helio-specific daily tips cards
- Custom images for tips 1, 2, and 7

---

### 📋 Simulation Results Enhancements

#### Fix Suggestions
- **Dynamic API-driven suggestions**: Grouped by category (QUICK, ADVANCED, EXPERT)
- **Collapsible sections**: Expandable "Fix suggestions" with preview text
- **Wiki links**: Links to Helio guides for fixing cold prints

#### Results Display
- **API-provided descriptions**: Uses `printOutcomeDescription` and `temperatureDirectionDescription`
- **Potential speed improvement**: Shows estimated time savings with Enhance
- **View Details navigation**: Navigates to Preview panel

---

### 👤 Account & Subscription

- **Subscription display**: Shows current plan in Account Status card
- **Free trial support**: 
  - "Start Free Trial" button when eligible
  - "Free Trial" status when active
  - "Free Trial Expired" when claimed but expired
- **Monthly quota**: Shows "Unlimited*" with fair use policy link when > 1000
- **Dynamic tooltips**: Placeholder tooltips while loading

---

### 💾 Multi-Plate Support

- **Per-plate result storage**: Each plate stores its own simulation/optimization results
- **Correct View Summary**: Shows results for the currently selected plate
- **Result preservation**: Only clears results for the plate being re-sliced

---

## UI/UX Improvements

### Dark Mode Support
- Full dark mode theming for all Helio dialogs
- Proper text visibility in dark mode (uses `wxColour(255,255,254)` to bypass color remapping)
- Theme-aware warning boxes that update colors dynamically

### Light Mode Fixes
- White input fields for clear contrast (theme.card2)
- Store icon visibility with hover states
- Entry boxes appear enabled instead of disabled

### Dialog Improvements
- **Upper-third positioning**: Dialogs positioned at 1/3 from top, horizontally centered
- **SetSizerAndFit**: Fixes excess blank space in material dialogs
- **Full-width CTA buttons**: 56 DIP height, 16px corner radius
- **Windows scrollbar fix**: Proper virtual size calculation in accordion sections
- **Clickable accordion headers**: Title and arrow labels respond to clicks

### Button Styling
- White text (255,255,254) for all button states in dark mode
- Green "Enable Helio Additive" and "Run Your First Optimization" buttons
- Proper StateColor with all states (Normal, Hovered, Pressed, Focused, Disabled)

---

## Platform-Specific Fixes

### Windows
- **Garbage text fix**: Unicode-safe wxString literals for bullet, em dash, and arrow
- **Dialog layout fix**: Expand wrapped labels and refit after dark UI updates
- **Scrollbar corruption fix**: FitInside() and scroll position clamping
- **History dialog crash fix**: Memory safety improvements
- **Transparency handling**: Widgets match exact brightened background color

### macOS
- **True transparency**: Uses wxNullColour for transparent backgrounds
- **Dialog icons**: Reverted to .ico file loading (CopyFromBitmap unreliable)
- **Resources symlink**: Fixed symlink creation during build

---

## Internationalization

### Chinese (zh_CN) Translations
- Complete translations for all new Helio features
- Welcome screen translations
- Dialog and tooltip translations
- Fixed translation for time saving context: Use "节省" (to save/conserve) for time-saving displays instead of "保存" (to keep/store) which is used for file saving operations

### Translation Infrastructure
- New translation keys for Helio features
- Proper quote escaping in .pot files

---

## Technical Improvements

### API Integration
- **Retry/backoff logic**: Improved error recovery for locale-related issues
- **Decimal parsing**: Robust parsing for different locale settings
- **GraphQL queries**: Extended queries for new API fields:
  - `printInfo`, `speedFactor`, `suggestedFixes`
  - `heatedChamber`, `feedstock`
  - `freeTrialEligibility`, `isFreeTrialActive`, `isFreeTrialClaimed`
  - `restrictions`, `restrictionsV2`

### Error Recovery
- Early polling termination on ERROR/RESTRICTED status
- Actual API error messages in dialogs
- Proper error logging in on_error handlers

### Memory Safety
- Fixed const-correctness in MediaFilePanel.cpp
- Safe string copies for wxExecute argv arrays
- Memory safety in async callback handlers

### Code Quality
- Extracted duplicate refresh logic into helper functions
- Consistent event binding to header panels
- FindExpandButton() helper for tooltip management

---

## Breaking Changes

None. All changes are additive and maintain compatibility with standard BambuStudio workflows.

---

## Known Limitations

- **Infill combination warning**: When "Infill Combination" is enabled in print settings, Helio shows a non-blocking dialog warning that this feature is unsupported. Users may proceed, but thermal analysis accuracy may be reduced for models with complex infill patterns that use infill combination.
- By-object print sequence is now supported (previous restriction removed)

---

## Files Changed

### New Files
- `src/slic3r/GUI/HelioReleaseNote.cpp` / `.hpp`
- `src/slic3r/GUI/HelioHistoryDialog.cpp` / `.hpp`
- `src/slic3r/Utils/HelioDragon.cpp` / `.hpp`
- `resources/data/helio_hints.ini`
- `resources/images/helio_*.svg` / `.png` (icons and daily tips)

### Modified Files
- `src/slic3r/GUI/GUI_App.cpp` - Helio sync integration
- `src/slic3r/GUI/NotificationManager.cpp` - Helio notifications
- `src/slic3r/GUI/Plater.cpp` / `.hpp` - Helio workflow integration

---

## Credits

Developed by Helio Additive team for integration with BambuStudio.

For more information, visit the [Helio Additive Wiki](https://wiki.helioadditive.com).
