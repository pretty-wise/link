Link
====

## Features
## Configuration
## Installation

Install Base library.

```
git clone https://github.com/pretty-wise/base.git
mkdir base/build
cd base/build
cmake ..
make install
```

Install Google Test framework:

```
git clone https://github.com/google/googletest.git
mkdir googletest/build
cd googletest/build
cmake ..
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
+- common 			<- utility code shared between server and plugins
|  +- include
|  +- src
+- link
|  +- include
|  +- src
|  |  +- common <- plugin interface shared between server and plugins
|  |  +- core   <- server core code
|  |  +- plugin <- plugin interface code
|  |  +- server <- server executable
+- plugins			<- server plugins
|  +- common
|  +- directory
|  +- gate
|  |  +- clientsrc <- interface static library, used by other plugins
|  |  +- pluginsrc <- plugin shared object
|  |  +- commonsrc <- code shared between client interface and plugin
|  |  +- include
|  +- launcher
|  +- monitor
|  +- punch
|  +- rest
+- web <- web control panel
```
## Usage
## Testing

Link uses Google Test Framework. It comes with a suite of tests covering the core server functionality. Plugins come with their own tests.

To run the tests first build the project. After the project is build execute the following command in the build folder:
```
make test
```
## Web Control Panel
```
brew install node
npm install -g @angular/cli
cd web
ng install
ng serve --open
```
## Examples
```code
struct todo_examples {
	int this_is_wip_doc;
};
```
## License
> Copyright (c) 2018, Krzysztof Stasik
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
