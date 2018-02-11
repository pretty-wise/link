Link
====

## Features
### Configuration
# Installation
```
git clone git@github.com:pretty-wise/base.git
mkdir base/build
cd base/build
cmake ..
make
make install
```
# Requirements
- Mac OS 10.13
# Dependencies
- [Base](https://github.com/pretty-wise/base) library
# Project Structure
```
.
+- extern
+- common
|  +- include
|  +- src
+- link
|  +- include
|  +- src
|  |  +- common
|  |  +- core
|  |  +- plugin
|  |  +- server <- server executable
+- plugins
|  +- common
|  +- directory
|  +- gate
|  |  +- clientsrc
|  |  +- pluginsrc
|  |  +- commonsrc
|  |  +- include
|  +- launcher
|  +- monitor
|  +- punch
|  +- rest
```
# Usage
# Examples
```code
struct s {
	int m;
};
```
# License
