#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PACKAGE_BASENAME="${1:-studentNo_yourName_projects}"
OUT_DIR="${ROOT_DIR}/${PACKAGE_BASENAME}"

rm -rf "${OUT_DIR}"
mkdir -p "${OUT_DIR}/models"

cp "${ROOT_DIR}/README.md" "${OUT_DIR}/README_project.md"
cp "${ROOT_DIR}/WINDOWS_RUN_ME.txt" "${OUT_DIR}/WINDOWS_RUN_ME.txt"
cp "${ROOT_DIR}/build_windows.bat" "${OUT_DIR}/build_windows.bat"
cp "${ROOT_DIR}/models/bird.obj" "${OUT_DIR}/models/bird.obj"

if [[ -f "${ROOT_DIR}/bird_scene.exe" ]]; then
  cp "${ROOT_DIR}/bird_scene.exe" "${OUT_DIR}/bird_scene.exe"
fi

if [[ -f "${ROOT_DIR}/freeglut.dll" ]]; then
  cp "${ROOT_DIR}/freeglut.dll" "${OUT_DIR}/freeglut.dll"
fi

if compgen -G "${ROOT_DIR}"/*.mp4 > /dev/null; then
  cp "${ROOT_DIR}"/*.mp4 "${OUT_DIR}/"
fi

if compgen -G "${ROOT_DIR}"/*.mov > /dev/null; then
  cp "${ROOT_DIR}"/*.mov "${OUT_DIR}/"
fi

if [[ -d "${ROOT_DIR}/source_code" ]]; then
  cp -R "${ROOT_DIR}/source_code" "${OUT_DIR}/source_code"
fi

cat <<EOF
Prepared submission folder:
  ${OUT_DIR}

Included:
  - WINDOWS_RUN_ME.txt
  - README_project.md
  - build_windows.bat
  - models/bird.obj
  - any .mp4/.mov videos found in repo root

Optional files copied if found in repo root:
  - bird_scene.exe
  - freeglut.dll
  - source_code/

Next step:
  1. Rename the folder if needed so it matches:
     studentNo+yourName+"projects"
  2. Confirm the folder contains the Windows .exe, required .dll files,
     and your demo video with audio.
  3. Compress this folder as .zip or .rar for submission.
EOF
