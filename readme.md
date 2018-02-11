Link
====

## Features
## Configuration
## Installation
```
git clone git@github.com:pretty-wise/base.git
mkdir base/build
cd base/build
cmake ..
make
make install
```
## Requirements
- Mac OS 10.13
## Dependencies
- [Base](https://github.com/pretty-wise/base) library
## Project Structure
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
## Usage
## Examples
```code
struct s {
	int m;
};
```
## License
> Copyright (c) 2018, Krzysiek
> All rights reserved.
> 
> Redistribution and use in source and binary forms, with or without
> modification, are permitted provided that the following conditions are met:
> 
> * Redistributions of source code must retain the above copyright notice, this
>   list of conditions and the following disclaimer.
> 
> * Redistributions in binary form must reproduce the above copyright notice,
>   this list of conditions and the following disclaimer in the documentation
>   and/or other materials provided with the distribution.
> 
> THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
> AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
> IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
> DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
> FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
> DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
> SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
> CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
> OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
> OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
