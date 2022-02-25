#!/bin/bash
ROOT="../.."
EXEC="$ROOT/Source/PySrc/slowmotion.py"
#ENV_PATH="/Users/lukanixon/Desktop/ECS193/Super-Slow-Motion-Video/Pipfile"
ENV_PATH="$PWD/$ROOT/Pipfile"
export PIPENV_PIPFILE=$ENV_PATH
pipenv run python3 $EXEC --input ./akiyo_cif.y4m --output ./Outputs/akiyo_cif.mkv --FPS 25 --slowdown 4