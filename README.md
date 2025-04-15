# Verilator on Kubernetes (k8s)

## Legacy Meets Modernization

Companies operating in the semiconductor industry often rely on a "legacy" technology stack — typically CentOS 6 or 7 with outdated package bases, aging infrastructure, and workflows dating back to the mid-2000s. In this space, every mistake can cost millions, so risk aversion is the norm. However, that doesn’t mean modernization should be ignored. Sooner or later, contemporary infrastructure practices and SDLC (Software Development Life Cycle) approaches will find their way into even the most conservative environments.

## Why Run Verilator on Kubernetes?

Running **Verilator** on **Kubernetes (k8s)** provides a **scalable**, **automated**, and **reproducible** approach to hardware simulation and verification.

### Benefits:
- **Parallel Execution**: Distribute simulations across multiple nodes to speed up regression testing.
- **Dynamic Scaling**: Automatically allocate compute resources based on job demand.
- **Fault Tolerance**: Recover from failures without manual intervention.
- **DevOps Integration**: Seamless alignment with CI/CD pipelines.
- **Reproducible Environments**: Use containers to ensure consistency across development, testing, and production stages.
- **Collaboration**: Bridging the gap between hardware and software teams with modern infrastructure.

Verilator converts Verilog/SystemVerilog into C++/SystemC models, and k8s ensures those simulations are executed efficiently and reliably.

---

Packaging with nfpm

nfpm is a simple tool to create Linux packages like .deb (Debian/Ubuntu) and .rpm (CentOS/Fedora). Instead of manually managing package creation with complex scripts, nfpm lets you define your package metadata and contents in a YAML file.

This is ideal for developers who want to distribute their applications easily across Linux systems without diving deep into low-level packaging tools.

Create nfpm.yaml configuration:
```yaml
name: verilator
arch: amd64
platform: linux
version: 5.035
section: devel
priority: optional
maintainer: Your Name <your.email@example.com>
description: |
  Verilator is a fast, free Verilog HDL simulator.
  It converts Verilog to a cycle-accurate behavioral model in C++ or SystemC.
  Verilator is not a traditional simulator, but a compiler.
vendor: Verilator Project
homepage: https://www.veripool.org/wiki/verilator
license: LGPL-3.0

contents:
  - src: bin/verilator
    dst: /usr/bin/verilator

depends:
  - g++
  - make
  - perl
  - libfl2
  - libfl-dev
  - zlib1g
  - zlib1g-dev

conflicts:
  - verilator

replaces:
  - verilator
```

Build the package:
```
nfpm package --config nfpm.yaml --packager deb --target .
```

Install the package:
```
apt-get install ./verilator_5.035_amd64.deb
```

---

## Build & Push the Docker Image

1. **Build the Docker image:**
   ```bash
   docker build -t verilator-k8s:latest .
   ```

2. **Run a local Docker registry:**
   ```bash
   docker run -d -p 5000:5000 --name registry registry:2
   ```

3. **Tag the image for the local registry:**
   ```bash
   docker tag verilator-k8s:latest localhost:5000/verilator-k8s:latest
   ```

4. **Push the image:**
   ```bash
   docker push localhost:5000/verilator-k8s:latest
   ```

---

## Deploy to Kubernetes

1. **Deploy using a Kubernetes manifest:**
   ```bash
   kubectl apply -f deployment.yaml
   ```

2. **Check deployment status:**
   ```bash
   kubectl get all
   ```

   Example output:
   ```
   NAME                                        READY   STATUS    RESTARTS   AGE
   pod/verilator-deployment-64cd8fdc55-6v724   1/1     Running   0          108s
   pod/verilator-deployment-64cd8fdc55-cjznw   1/1     Running   0          108s
   ...

   NAME                                   READY   UP-TO-DATE   AVAILABLE   AGE
   deployment.apps/verilator-deployment   5/5     5            5           108s

   NAME                                              DESIRED   CURRENT   READY   AGE
   replicaset.apps/verilator-deployment-64cd8fdc55   5         5         5       108s
   ```

---

## 📄 View Verilator Logs

1. **Check logs from a specific pod:**
   ```bash
   kubectl logs -f pod/verilator-deployment-64cd8fdc55-zfpdz
   ```

2. **Example log output:**
   ```
   - V e r i l a t i o n   R e p o r t: Verilator 5.034 2025-02-24 rev v5.034-47-gac3f30ed6
   - Verilator: Built from 0.022 MB sources in 2 modules, into 0.024 MB in 6 C++ files needing 0.000 MB
   - Verilator: Walltime 0.091 s (elab=0.000, cvt=0.082, bld=0.000); cpu 0.014 s on 1 threads; alloced 9.059 MB
   ```

---

Let me know if you'd like to add sections like CI/CD integration examples, Helm support, or monitoring/logging tools.
