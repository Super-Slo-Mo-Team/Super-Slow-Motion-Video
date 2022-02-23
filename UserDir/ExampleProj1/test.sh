#!/bin/bash
ROOT="../.."
EXEC="$ROOT/Source/PySrc/slowmotion.py"
python3 $EXEC --input ./akiyo_cif.y4m --output ./Outputs/akiyo_cif.mkv --FPS 25 --slowdown 4
