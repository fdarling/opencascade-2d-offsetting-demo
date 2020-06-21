# opencascade-2d-offsetting-demo
simple C++ demo console application that uses Open CASCADE Technology to both offset and apply boolean operations to 2D shapes composed of line segments and arc segments.

## Dependencies

For the C++ Demo:

* Open CASCADE Technology 7.4.0 (may work with older versions too, not sure about the "Community Edition"...)

For Debian 9 (stretch), the following dependencies needed to be installed to build Open CACADE 7.4.0:

```
sudo apt-get install tk-dev libxmu-dev libxi-dev
```

For the Python Testbench:

* [Python 3](https://www.python.org/)
* [Shapely](https://pypi.org/project/Shapely/) (for Python 3)
* [matplotlib](https://matplotlib.org/) (for Python 3)

On Ubuntu 18.04 (bionic):

```
sudo apt-get install python3-matplotlib python3-shapely
```

## Building

There is a simple Makefile to build the C++ executable. The library paths may need to be edited depending on the operating system.

```
make
```

## Running

Under Linux (and possibly macOS) there is a `run.sh` script that attempts to compile the project, run the C++ demo on a test case, and then visualize the input/output shapes. The script must have execute permissions:

```
chmod a+x run.sh
./run.sh
```
