#!/usr/bin/env sh
set -euo pipefail

# Isabelle
# LLAMACPP_BINARY=${HOME}/usr/src/ml/llama.cpp/zig-out/bin/main
# LLM_DIR=${HOME}/usr/src/ml/text-generation-webui/models

# Holly
LLAMACPP_BINARY=${HOME}/usr/src/ml/llm/llama.cpp/main
LLM_DIR=${HOME}/usr/src/ml/llm/models


# MODEL=wizardlm-13b-v1.2.Q4_K_M.gguf
# MODEL=Wizard-Vicuna-13B-Uncensored.Q5_K_M.gguf
# MODEL=deepseek-coder-6.7b-instruct.Q8_0.gguf

# Deepseek
# MODEL=deepseek-coder-1.3b-instruct.Q8_0.gguf
# CONTEXT_SIZE=4096
# PROMPT=$({
# echo "You are an AI programming assistant, utilizing the Deepseek Coder model, developed by Deepseek Company, and you only answer questions related to computer science. For politically sensitive questions, security and privacy issues, and other non-computer science questions, you will refuse to answer."
# echo "### Instruction:"
# cat
# echo "### Response:"
# })

# Isabelle
# MODEL=codellama-13b-instruct.Q4_K_M.gguf
# CONTEXT_SIZE=4096
# Holly
MODEL=phind-codellama-34b-v2.Q8_0.gguf; GPU_LAYERS=26
CONTEXT_SIZE=8192

PROMPT=$({
echo "[INST]"
cat laser.txt
echo "[/INST]"
cat laser.py
})

${LLAMACPP_BINARY} \
        --n-predict ${CONTEXT_SIZE} \
        --threads $(nproc) \
        --ctx-size ${CONTEXT_SIZE} \
        --color \
        --n-gpu-layers ${GPU_LAYERS} \
        --model ${LLM_DIR}/${MODEL} \
        --temp 0.1 \
        --prompt "${PROMPT}"
