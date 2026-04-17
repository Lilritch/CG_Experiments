#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUT_DIR="${ROOT_DIR}/submission_windows"

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

cat <<EOF
Prepared Windows submission folder:
  ${OUT_DIR}

Included:
  - WINDOWS_RUN_ME.txt
  - README_project.md
  - build_windows.bat
  - models/bird.obj

Optional files copied if found in repo root:
  - bird_scene.exe
  - freeglut.dll

Next step:
  Zip the submission_windows folder and send that zip to your professor.
EOF
