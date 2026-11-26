"""Restore a pinned dependency artifact produced by the Helio fork."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
import zipfile

repo = "Helio-Additive/BambuStudio"
artifact_id = os.environ["DEPS_ARTIFACT"]
source = Path.cwd()
metadata = json.loads(subprocess.check_output(["gh", "api", f"repos/{repo}/actions/artifacts/{artifact_id}"]))
head = metadata["workflow_run"]["head_sha"]
tree = subprocess.check_output(["git", "-C", str(source), "rev-parse", "HEAD:deps"], text=True).strip()
expected = "dfcee1c78f98fa9ada89f461a2f4653ec9ad4cca"
if tree != expected:
    raise SystemExit("Candidate dependency recipes changed. Select matching artifacts before running.")
if metadata["expired"]:
    raise SystemExit(f"Dependency artifact {artifact_id} expired. Rebuild it from {head}.")
notes = f"Dependencies: {metadata['name']} (artifact {artifact_id} from {head})."
if os.environ["TARGET"] == "windows-arm64":
    notes += " Native ARM64 uses the fork's FFmpeg download and OpenMeshCraft portability fixes from #71."
with open(os.environ["GITHUB_STEP_SUMMARY"], "a", encoding="utf-8") as summary:
    print(notes, file=summary)
print(notes, flush=True)

with tempfile.TemporaryDirectory() as d:
    archive = Path(d) / "artifact.zip"
    with archive.open("wb") as output:
        subprocess.run(["gh", "api", f"repos/{repo}/actions/artifacts/{artifact_id}/zip"], stdout=output, check=True)
    digest = hashlib.sha256()
    with archive.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    if metadata["digest"] != "sha256:" + digest.hexdigest():
        raise SystemExit("Dependency archive checksum mismatch.")
    with zipfile.ZipFile(archive) as outer:
        members = [m for m in outer.infolist() if not m.is_dir()]
        if len(members) != 1 or Path(members[0].filename).name != members[0].filename:
            raise SystemExit("Unexpected dependency artifact layout.")
        payload = Path(outer.extract(members[0], d))
    target = os.environ["TARGET"]
    if target.startswith("macos"):
        destination = source / "deps"
    elif target.startswith("windows"):
        destination = source / "deps/build/BambuStudio_dep"
    else:
        destination = source / "deps/build"
    destination.mkdir(parents=True, exist_ok=True)
    subprocess.run(["tar", "-xf", str(payload), "-C", str(destination)], check=True)
