# Development Guide

## Docker local development setup

1. You should have docker installed in your system, if not click [here](https://docs.docker.com/get-docker/).

2. Build the docker image:
    ```shell
    docker build -t clion/ubuntu/cpp-env:1.0 -f Dockerfile .
    ```
3. Set up CLion:
    - Navigate to Settings > Build, Execution, Deployment > Toolchains
    - Select _+_ then choose _Docker_ from the dropdown
    - Select the newly created clion container in the dropdown field
      marked with the _Image_ label. Allow a minute for CLion to complete the configuration
    - Click the gear icon on the right side of the _Container Settings_
      field
    - In the new popup, map the absolute path of your Host's project
      root to `/app` for the container's path. You can find the options
      under the Volumes section by clicking the triangle to expand the
      section labeled _Volumes_ and clicking the _+_ on the top left of
      the sub-section. i.e.:
        `/Users/ilovesynervoz/code/SimpleDJ` -> `/app`
      *NOTE*: It has to be an abolute path

4. After configuring a Docker toolchain, you can select it in CMake
   profiles or in Makefile settings. Alternatively, move the toolchain
   to the top of the list to make it default.

5. The project folder will be mounted to the Docker container, and
   build/run/debug will be performed inside it.
    *NOTE*: By default, the project folder is mounted into the /tmp
    folder in the container. However, if there are path mappings
    specified in the toolchain, CLion will use them instead.

