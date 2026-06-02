# Verilator On Kubernetes

I once worked in a hardware company, and one underrated
feature of EDA work stayed with me: logs. These tools
produce an incredible amount of logs, and those logs are raw
material for analysis when simulation fails, performance
shifts, or a regression starts behaving differently.

Hardware simulation is easiest to trust when each run is
repeatable, isolated, and boring to operate. This example
takes a Verilator regression and runs it as a Kubernetes
batch job instead of a long-lived shell session on a shared
machine.

The design is intentionally more than a hello world. It is
a two-input, two-output packet router. Each input presents
valid, destination, and data signals. Each output can apply
backpressure. When both inputs target the same output, input
zero wins arbitration for that cycle and input one waits.

The C++ testbench is randomized but deterministic. It keeps
pending packets on each input, drives random output ready
signals, records every accepted packet in a scoreboard, and
checks every delivered packet for source, destination,
order, and payload. A pod is successful only when the model
compiles, the simulation drains, and the scoreboard matches.

## What Is In The Repo

`sim/packet_router.sv` contains the SystemVerilog router.
`sim/sim_main.cpp` contains the randomized scoreboard.
`sim/Makefile` runs Verilator and executes the model.
`deployment.yaml` is a Kubernetes Indexed Job.
`scripts/run-k8s.sh` builds, pushes, applies, waits, and
prints logs.

The Dockerfile uses Ubuntu 24.04 and the packaged Verilator.
That keeps the demo build short. For production, pin the
base image by digest and decide whether your team wants
distro packages or a separately built Verilator package.

## Local Container Proof

Build the image for the usual lab node architecture:

```bash
docker build --platform linux/amd64 \
  -t localhost:5000/verilator-k8s:latest .
```

Run one deterministic seed:

```bash
docker run --rm \
  -e SIM_SEED=1234 \
  localhost:5000/verilator-k8s:latest \
  make -C /sim/sim run
```

A passing run ends like this:

```text
PASS packet_router seed=1234 cycles=2000 \
  accepted=1743 delivered=1743 stalls=1553
```

## Kubernetes Run

For a single-node lab with a node-local registry:

```bash
docker run -d --restart=always \
  -p 5000:5000 --name registry registry:2
docker push localhost:5000/verilator-k8s:latest
```

Then launch the regression:

```bash
kubectl create namespace verilator-sim
kubectl delete job verilator-router-sim \
  -n verilator-sim --ignore-not-found
kubectl apply -n verilator-sim -f deployment.yaml
kubectl wait -n verilator-sim \
  --for=condition=complete \
  job/verilator-router-sim --timeout=300s
kubectl logs -n verilator-sim \
  -l app=verilator-router-sim --prefix
```

The script wraps the same flow:

```bash
./scripts/run-k8s.sh
```

The useful part of the log is compact:

```text
PASS packet_router seed=1000 cycles=2000 \
  accepted=1781 delivered=1781 stalls=1452
PASS packet_router seed=1001 cycles=2000 \
  accepted=1762 delivered=1762 stalls=1444
PASS packet_router seed=1002 cycles=2000 \
  accepted=1788 delivered=1788 stalls=1429
PASS packet_router seed=1003 cycles=2000 \
  accepted=1778 delivered=1778 stalls=1462
```

The full Kubernetes YAML is under the cut:

<details>
<summary>Full Kubernetes YAML</summary>

```yaml
apiVersion: batch/v1
kind: Job
metadata:
  name: verilator-router-sim
  labels:
    app: verilator-router-sim
spec:
  completions: 4
  parallelism: 2
  completionMode: Indexed
  backoffLimit: 0
  ttlSecondsAfterFinished: 600
  template:
    metadata:
      labels:
        app: verilator-router-sim
    spec:
      restartPolicy: Never
      containers:
      - name: verilator
        image: localhost:5000/verilator-k8s:latest
        imagePullPolicy: IfNotPresent
        command:
        - bash
        - -lc
        - |
          set -euo pipefail
          job_index="${HOSTNAME#verilator-router-sim-}"
          job_index="${job_index%%-*}"
          case "${job_index}" in
            ''|*[!0-9]*)
              echo "Cannot derive completion index" \
                "from HOSTNAME=${HOSTNAME}" >&2
              exit 1
              ;;
          esac
          seed=$((1000 + job_index))
          echo "Running packet router with SIM_SEED=${seed}"
          SIM_SEED="${seed}" make -C /sim/sim run
```

</details>

The Job runs four completions with two pods in parallel.
Each completion gets a stable index from Kubernetes and
converts it to a seed, starting at 1000. That gives separate
randomized runs without changing the image.

## Why Use A Job

A Deployment keeps pods alive. That is useful for services
but awkward for verification, where the useful result is
pass or fail. A Job gives a clear lifecycle: schedule work,
run to completion, expose logs, and record status in the API
server.

This pattern maps well to hardware regressions. Add more
seeds by increasing `completions`. Increase `parallelism`
when the cluster has spare CPU. Keep source, toolchain, and
test logic inside the image so every pod runs the same
environment.

Kubernetes does not make Verilator faster by itself. It
makes many Verilator runs easier to schedule, repeat, and
audit. That is the modernization win: not a new simulator,
but a cleaner operating model for the simulations you
already depend on.
