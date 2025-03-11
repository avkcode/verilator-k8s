# verilator-k8s
Running verilator on k8s

## Rational

Running Verilator on Kubernetes (k8s) offers a scalable, efficient, and modern approach to hardware design verification. Verilator, a high-performance Verilog/SystemVerilog simulator, is often used in hardware development workflows to convert HDL designs into C++ or SystemC models for simulation. By deploying Verilator on Kubernetes, teams can leverage the orchestration capabilities of k8s to manage distributed workloads, automate resource allocation, and parallelize simulations across multiple nodes. This is particularly beneficial for large-scale verification tasks, such as regression testing or running extensive test suites, where computational resources and scalability are critical. Kubernetes enables dynamic scaling of simulation jobs, ensuring optimal utilization of cluster resources while reducing turnaround times. Additionally, k8s provides fault tolerance, ensuring that simulations can recover from failures without manual intervention. By containerizing Verilator and its dependencies, teams can also achieve consistent and reproducible environments across development, testing, and production stages. This approach aligns with modern DevOps practices, enabling seamless integration with CI/CD pipelines and fostering collaboration between hardware and software teams. Overall, running Verilator on Kubernetes enhances productivity, scalability, and reliability in hardware verification workflows.

Building the image:
```
docker build -t verilator-k8s:latest .
```

In this case, we will use a local Docker repository:
```
docker run -d -p 5000:5000 --name registry registry:2
```

Tagging:
```
docker tag verilator-k8s:latest localhost:5000/verilator-k8s:latest
```

Pushing:
```
docker push localhost:5000/verilator-k8s:latest
```

Deploying:
```
kubectl apply -f deployment.yaml
```
Check the logs:
```
kubectl get all
NAME                                        READY   STATUS    RESTARTS   AGE
pod/verilator-deployment-64cd8fdc55-6v724   1/1     Running   0          108s
pod/verilator-deployment-64cd8fdc55-cjznw   1/1     Running   0          108s
pod/verilator-deployment-64cd8fdc55-ll4g5   1/1     Running   0          108s
pod/verilator-deployment-64cd8fdc55-md5xt   1/1     Running   0          108s
pod/verilator-deployment-64cd8fdc55-zfpdz   1/1     Running   0          108s

NAME                 TYPE        CLUSTER-IP   EXTERNAL-IP   PORT(S)   AGE
service/kubernetes   ClusterIP   10.43.0.1    <none>        443/TCP   3h38m

NAME                                   READY   UP-TO-DATE   AVAILABLE   AGE
deployment.apps/verilator-deployment   5/5     5            5           108s

NAME                                              DESIRED   CURRENT   READY   AGE
replicaset.apps/verilator-deployment-64cd8fdc55   5         5         5       108s

kubectl logs -f pod/verilator-deployment-64cd8fdc55-zfpdz
- V e r i l a t i o n   R e p o r t: Verilator 5.034 2025-02-24 rev v5.034-47-gac3f30ed6
- Verilator: Built from 0.022 MB sources in 2 modules, into 0.024 MB in 6 C++ files needing 0.000 MB
- Verilator: Walltime 0.091 s (elab=0.000, cvt=0.082, bld=0.000); cpu 0.014 s on 1 threads; alloced 9.059 MB
```
