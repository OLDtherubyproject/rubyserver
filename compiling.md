# Compilling

The Ruby Server can be compiled on a variety of operating systems. We currently provide build instructions for the following systems:

* [Compiling on Arch Linux](#Compiling-on-Arch-Linux)
* [Compiling on CentOS](#Compiling-on-CentOS)
* [Compiling on Debian GNU/Linux](#Compiling-on-Debian-GNU/Linux)
* [Compiling on Fedora](#Compiling-on-Fedora)
* [Compiling on FreeBSD](#Compiling-on-FreeBSD)
* [Compiling on Gentoo](#Compiling-on-Gentoo)
* [Compiling on Mac OS X](#ompiling-on-Mac OS X)
* [Compiling on Ubuntu](#Compiling-on-Ubuntu)
* [Compiling on Windows](#Compiling-on-Windows)

No worries if your system is not listed above, chances are that it can be compiled on your system anyway, but you will have to download and install the required software and libraries on your own. If your system has a package manager, you could save a couple of hours by installing them from the package manager instead of manually downloading and compiling the packages from their respective websites.

To download the source code, you will need Git (alternatively download a copy from GitHub and skip this step). Once you have installed Git, run this command to download a copy of the source code:

```$ git clone --recursive https://github.com/RubyServer/rubyserver.git```

To compile the required libraries and The Ruby Server, you will need a compiler and a set of libraries. We recommend 
[GCC](http://gcc.gnu.org/) or [Clang](http://clang.llvm.org/).

You will have to install [CMake](http://www.cmake.org/) to generate build files for your compiler.

The following libraries are required to compile The Ruby Server:

* [Boost](https://boost.org/)
* [GMP](https://gmplib.org/)
* [LUA](http://www.lua.org/)
* [MySQL C connector](https://dev.mysql.com/downloads/connector/c/)
* [PlugiXML](http://pugixml.org/)

Once you have installed all the libraries, create a directory with the name "build" in the root directory of the rubyserver directory that you downloaded using Git. Now target the parent directory from the build directory using CMake to generate the build files. If you are using the CMake GUI, it could look like this:

```
"Where is the source code: ~/rubyserver"
"Where to build the binaries: ~/rubyserver/build/"
```

If you are using a command-line interface it could look like this:

```
user@host:~/projects/rubyserver/build$ cmake ..
```

After successfully running CMake, your compiler should be ready to compile The RubyServer from the build files generated in the build directory.

# Compiling on Arch Linux

**1. Install the required software**

The following command will install Git, CMake, a compiler and the libraries used by The Ruby Server.

Git will be used to download the source code, and CMake will be used to generate the build files.

```shell
$ sudo pacman -Syu
$ sudo pacman -S base-devel git cmake lua gmp boost boost-libs libmariadbclient pugixml
```
**2. Download the source code**

```shell
$ git clone --recursive https://github.com/RubyServer/rubyserver.git
```

**3. Generate the build files**

```shell
$ cd rubyserver
$ mkdir build
$ cd build
$ cmake ..
```

**4. Build**

```shell
$ make
```

# Compiling on CentOS

**1. Install the required software**

The following command will install Git, CMake, a compiler and the libraries used by The Ruby Server.

Git will be used to download the source code, and CMake will be used to generate the build files.

```shell
$ sudo yum install git cmake gcc-c++ boost-devel gmp-devel mariadb-devel lua-devel pugixml-devel
```
**2. Download the source code**

```shell
$ git clone --recursive https://github.com/RubyServer/rubyserver.git
```

**3. Generate the build files**

```shell
$ cd rubyserver
$ mkdir build && cd build
$ cmake ..
```

**4. Build**

```shell
$ make
```

# Compiling on Debian GNU Linux

The Ruby Server requires at least gcc 4.8, which is available in Debian 8 (jessie).

**1. Install the required software**

The following command will install Git, CMake, a compiler and the libraries used by The Ruby Server.

Git will be used to download the source code, and CMake will be used to generate the build files.

```shell
$ apt-get install git cmake build-essential liblua5.2-dev libgmp3-dev libmysqlclient-dev libboost-system-dev libboost-iostreams-dev libpugixml-dev
```

Replace ```libmysqlclient-dev``` with ```libmariadbclient-dev``` on Debian 9 and above.

**2. Download the source code**

```shell
$ git clone --recursive https://github.com/RubyServer/rubyserver.git
```

**3. Generate the build files**

```shell
$ cd rubyserver
$ mkdir build && cd build
$ cmake ..
```

**4. Build**

```shell
$ make
```

# Compiling on Fedora

**1. Install the required software**

The following command will install Git, CMake, a compiler and the libraries used by The Ruby Server.

Git will be used to download the source code, and CMake will be used to generate the build files.

```shell
$ sudo dnf install git cmake gcc-c++ boost-devel gmp-devel community-mysql-devel lua-devel pugixml-devel
```

**2. Download the source code**

```shell
$ git clone --recursive https://github.com/RubyServer/rubyserver.git
```

**3. Generate the build files**

```shell
$ cd rubyserver
$ mkdir build && cd build
$ cmake ..
```

**4. Build**

```shell
$ make
```

# Compiling on FreeBSD

**1. Install the required software**

The following command will install Git, CMake, a compiler and the libraries used by The Ruby Server.

Git will be used to download the source code, and CMake will be used to generate the build files.

```shell
$ pkg install git cmake luajit boost-libs gmp mysql-connector-c pugixml
```

**2. Download the source code**

```shell
$ git clone --recursive https://github.com/RubyServer/rubyserver.git
```

**3. Generate the build files**

```shell
$ cd rubyserver
$ mkdir build && cd build
$ cmake ..
```

**4. Build**

```shell
$ make
```

# Compiling on Gentoo

**1. Install the required software**

The following command will install Git, CMake, a compiler and the libraries used by The Ruby Server.

Git will be used to download the source code, and CMake will be used to generate the build files.

```shell
$ emerge dev-vcs/git dev-util/cmake dev-libs/boost dev-libs/gmp dev-db/mysql-connector-c++ dev-lang/luajit dev-libs/pugixml
```

**2. Download the source code**

```shell
$ git clone --recursive https://github.com/RubyServer/rubyserver.git
```

**3. Generate the build files**

```shell
$ cd rubyserver
$ mkdir build && cd build
$ cmake ..
```

**4. Build**

```shell
$ make
```

# Compiling on Mac OS X

**1. Install the required software**

##### 1.1. Installing Homebrew

Homebrew is a package manager for Mac OS X. It uses formulae scripts to compile the software on your system. If you already have Homebrew installed, you can skip this step.

Open a Terminal window and run:

```shell
$ /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

##### 1.2 Installing software and libraries

The following command will install Git, CMake, a compiler and the libraries used by The Ruby Server.

Git will be used to download the source code, and CMake will be used to generate the build files.

```shell
$ brew install git cmake gmp mysql-connector-c luajit boost pugixml pkg-config
```

**2. Download the source code**

```shell
$ git clone --recursive https://github.com/RubyServer/rubyserver.git
```

**3. Generate the build files**

```shell
$ cd rubyserver
$ mkdir build && cd build
$ cmake ..
```

**4. Build**

```shell
$ make
```

# Compiling on Ubuntu

**1. Install the required software**

The following command will install Git, CMake, a compiler and the libraries used by The Ruby Server.

Git will be used to download the source code, and CMake will be used to generate the build files.

```shell
$ sudo apt-get install git cmake build-essential liblua5.2-dev libgmp3-dev libmysqlclient-dev libboost-system-dev libboost-iostreams-dev libpugixml-dev
```

**2. Download the source code**

```shell
$ git clone --recursive https://github.com/RubyServer/rubyserver.git
```

**3. Generate the build files**

```shell
$ cd rubyserver
$ mkdir build && cd build
$ cmake ..
```

**4. Build**

```shell
$ make
```

# Compiling on Windows

**1. Download the required software**

To compile The Ruby Server on Windows, you will need:

* [Visual Studio 2017 Community](https://www.visualstudio.com/) (compiler)
* TRS SDK (libraries)
* sha1sum: 3fb1b140e40e8e8bc90f82f92bd022a50569b185
* sha256sum: 2ffb549f336ad11550da9b9c40716880e13b1cc0ee86e7599022dcacc972a3bf
* Boost C++ libraries for MSVC 14.1 ([32-bit download](https://sourceforge.net/projects/boost/files/boost-binaries/1.66.0/boost_1_66_0-msvc-14.1-32.exe/download), [64-bit download](https://sourceforge.net/projects/boost/files/boost-binaries/1.66.0/boost_1_66_0-msvc-14.1-64.exe/download))

**2. Install the required software**

Once you have downloaded the software listed in the step above, begin by installing Visual Studio and Boost C++ libraries. Extract TRS SDK anywhere on your computer and run the file "register_trssdk_env.bat" to set the PATH environment variable for TRS SDK, so that the compiler can find the libraries once we get to compiling the source code. Move the file "register_boost_env.bat" from TRS SDK to the directory where you installed Boost C++ libraries and run it there (it should be in the directory called boost_1_62_0).

**3. Download the source code**

If you have a Git client installed, you can clone the latest copy with this command:

```shell
git clone --recursive https://github.com/RubyServer/rubyserver.git
```

If you don't have a Git client installed, you can download the latest copy of The Ruby Server from this URL: [https://github.com/RubyServer/rubyserver/archive/master.zip](https://github.com/RubyServer/rubyserver/archive/master.zip)

**4. Build**

Find the directory vc14 in the copy of The Ruby Server that you downloaded, and open rubyserver.sln. This should launch Visual Studio, and you should be good to go.

To configure the build, navigate to Build -> Configuration Manager in the menu. A dialog should pop up where you can choose between Release or Debug build, and 32-bit (Win32) or 64-bit (x64) build.

To start compiling, open the Build menu again and click on Build Solution.
