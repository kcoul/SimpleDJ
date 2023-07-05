# Development Guide

## Docker local development setup

1. You should have docker installed in your system, if not click [here](https://docs.docker.com/get-docker/).

2. Build the docker image:
    ```shell
    docker build -t clion/ubuntu/cpp-env:1.0 -f Dockerfile .
    ```
3. Set up CLion:
    - Navigate to Settings > Build, Execution, Deployment
      > Toolchains
    - Select _+_ then choose _Docker_ from the dropdown
    - Click the gear icon on the right side of the server field
    - Select the docker image  from the dropdown field
      labeled _Container Settings_ and wait for CLion to
      complete the configuration.
    - Use the container setttings to mount the project root as a volume with the path `/tmp/SimpleDJ`
		*NOTE*: It has to be an abolute path

4. After configuring a Docker toolchain, you can select it in CMake profiles or in Makefile settings. Alternatively, move the toolchain to the top of the list to make it default.
5. The project folder will be mounted to the Docker container, and build/run/debug will be performed inside it.
	*NOTE*: By default, the project folder is mounted into the /tmp folder in the container. However, if there are path mappings specified in the toolchain, CLion will use them instead.

