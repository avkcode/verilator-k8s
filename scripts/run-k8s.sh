#!/usr/bin/env bash
set -euo pipefail

IMAGE="${IMAGE:-localhost:5000/verilator-k8s:latest}"
NAMESPACE="${NAMESPACE:-verilator-sim}"
JOB_NAME="verilator-router-sim"
DOCKER_PLATFORM="${DOCKER_PLATFORM:-linux/amd64}"
PUSH_IMAGE="${PUSH_IMAGE:-1}"
SAFE_IMAGE="${IMAGE//&/\\&}"
MANIFEST="$(mktemp)"

trap 'rm -f "${MANIFEST}"' EXIT

docker build --platform "${DOCKER_PLATFORM}" -t "${IMAGE}" .

if [[ "${PUSH_IMAGE}" == "1" ]]; then
  docker push "${IMAGE}"
fi

kubectl create namespace "${NAMESPACE}" \
  --dry-run=client -o yaml | kubectl apply -f -
kubectl delete job "${JOB_NAME}" \
  -n "${NAMESPACE}" --ignore-not-found
sed "s|image: localhost:5000/verilator-k8s:latest|image: ${SAFE_IMAGE}|" \
  deployment.yaml > "${MANIFEST}"
kubectl apply -n "${NAMESPACE}" -f "${MANIFEST}"

if ! kubectl wait -n "${NAMESPACE}" \
  --for=condition=complete "job/${JOB_NAME}" --timeout=300s; then
  kubectl describe -n "${NAMESPACE}" "job/${JOB_NAME}" || true
  kubectl logs -n "${NAMESPACE}" \
    -l "app=${JOB_NAME}" --all-containers --prefix || true
  exit 1
fi

kubectl logs -n "${NAMESPACE}" \
  -l "app=${JOB_NAME}" --all-containers --prefix
