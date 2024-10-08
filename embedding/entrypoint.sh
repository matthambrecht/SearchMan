#!/bin/bash

PORT=$(cat config.json | jq -r ".endpoint.port")
SCRIPT_DIR="$(dirname "$(realpath "$0")")"

cd "$SCRIPT_DIR"
source venv/bin/activate
uvicorn endpoint:app --workers=6 --port $PORT --log-config=log_conf.yaml --reload 