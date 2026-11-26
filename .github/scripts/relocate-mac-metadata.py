"""Point CMake and pkg-config metadata from a CI archive at this checkout and SDK."""
from pathlib import Path
import re
import subprocess
import sys

root = Path(sys.argv[1]).resolve()
sdk = subprocess.check_output(["xcrun", "--sdk", "macosx", "--show-sdk-path"], text=True).strip()
developer = subprocess.check_output(["xcode-select", "-p"], text=True).strip()
for path in (root / "deps/build").glob("*/BambuStudio_deps/usr/local/**/*"):
    if path.is_symlink() or not path.is_file():
        continue
    if path.suffix not in {".cmake", ".pc", ".la"} and not path.name.endswith("-config") and "wx/config/" not in path.as_posix():
        continue
    original = path.read_text()
    updated = original.replace("/Users/runner/work/BambuStudio/BambuStudio/deps/", str(root / "deps") + "/")
    updated = re.sub(r"/Applications/Xcode[^/]*\.app/Contents/Developer/Platforms/MacOSX\.platform/Developer/SDKs/MacOSX[^/;\" ]*\.sdk", sdk, updated)
    updated = re.sub(r"/Applications/Xcode[^/]*\.app/Contents/Developer(?=/Toolchains/)", developer, updated)
    if original != updated:
        path.write_text(updated)
