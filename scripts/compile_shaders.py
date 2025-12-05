#!/usr/bin/env python3
import sys
import subprocess
import argparse
from pathlib import Path

SLANGC = "slangc"

STAGE_INFO = {
    "cs": { "profile": "cs_6_0", "suffix": ".cs.spv" },
    "vx": { "profile": "vs_6_0", "suffix": ".vx.spv" },
    "px": { "profile": "ps_6_0", "suffix": ".px.spv" },
}

def parse_args():
    parser = argparse.ArgumentParser(description="Compile Slang shaders to SPIR-V")
    parser.add_argument("--input", required=True, help="Input shader file (.slang)")
    parser.add_argument("--output", required=True, help="Output SPIR-V file (.spv)")
    parser.add_argument("--root", required=True, help="Shader root directory")
    parser.add_argument("--depfile", required=False, help="Optional depfile for incremental build")
    return parser.parse_args()

def detect_stage(shader_path: Path):
    parts = shader_path.name.split(".")
    if len(parts) >= 3:
        stage = parts[-2]
        if stage in STAGE_INFO:
            return stage
    return None

def main():
    if len(sys.argv) < 3:
        print("Usage: shader_compile.py <source_dir> <output_dir>")
        sys.exit(1)

    args = parse_args()

    shader_path = Path(args.input)
    output_file = Path(args.output)
    shader_dir = Path(args.root)
    output_file.parent.mkdir(parents=True, exist_ok=True)

    stage = detect_stage(shader_path)
    if not stage:
        print(f"[Skip] Unknown stage in: {shader_path.name}")
        return

    info = STAGE_INFO[stage]

    cmd = [
        SLANGC,
        str(shader_path),
        "-I", f"{str(shader_dir)}/shared",
        "-target", "spirv",
        "-profile", info["profile"],
        "-entry", "main",
        "-o", str(output_file)
    ]

    if args.depfile:
        cmd += ["-depfile", str(args.depfile)]

    all_outputs = []

    print(f"[Slang] {stage.upper()} -> {output_file}")
    subprocess.run(cmd, check=True)

if __name__ == "__main__":
    main()
