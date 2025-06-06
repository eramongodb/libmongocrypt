# libmongocrypt #

The companion C library for client side encryption in drivers.

This project uses [Semantic Versioning](https://semver.org/).

# Bugs / Feature Requests #

If you have encountered a bug, or would like to see a new feature in libmongocrypt, please open a case in our issue management tool, JIRA:

- [Create an account and login](https://jira.mongodb.org).
- Navigate to [the MONGOCRYPT project](https://jira.mongodb.org/projects/MONGOCRYPT).
- Click **Create Issue** - Please provide as much information as possible about the issue type and how to reproduce it.

## Documentation ##
See [The Integration Guide](integrating.md) to integrate with your driver.

See [mongocrypt.h](src/mongocrypt.h) for the public API reference.
The documentation can be rendered into HTML with doxygen. Run `doxygen ./doc/Doxygen`, then open `./doc/html/index.html`.

## Building libmongocrypt ##

On Windows and macOS, libmongocrypt can use the platform's default encryption
APIs as its encryption backend. On other systems, one will want to install the
OpenSSL development libraries, which libmongocrypt will use as the default
encryption backend.

Then build libmongocrypt:

```
git clone https://github.com/mongodb/libmongocrypt
cd libmongocrypt
mkdir cmake-build && cd cmake-build
cmake ../
make
```

This builds libmongocrypt.dylib and test-libmongocrypt, in the cmake-build
directory.

## Installing libmongocrypt on macOS ##
Install the latest release of libmongocrypt with the following.
```
brew install mongodb/brew/libmongocrypt
```

To install the latest unstable development version of libmongocrypt, use `brew install mongodb/brew/libmongocrypt --HEAD`. Do not use the unstable version of libmongocrypt in a production environment.

## Building libmongocrypt from source on macOS ##

First install [Homebrew according to its own instructions](https://brew.sh/).

Install the XCode Command Line Tools:
```
xcode-select --install
```

Then clone and build libmongocrypt:
```
git clone https://github.com/mongodb/libmongocrypt.git
cd libmongocrypt
cmake .
cmake --build . --target install
```

Then, libmongocrypt can be used with pkg-config:
```
pkg-config libmongocrypt --libs --cflags
```

Or use cmake's `find_package`:
```
find_package (mongocrypt)
# Then link against mongo::mongocrypt
```

## Installing libmongocrypt on Windows ##
A Windows DLL for x86_64 is available on the Github Releases page. See the [latest release](https://github.com/mongodb/libmongocrypt/releases/latest).

Use `gpg` to verify the signature. The public key for `libmongocrypt` is available on https://pgp.mongodb.com/.


### Testing ###
`test-mongocrypt` mocks all I/O with files stored in the `test/data` and `test/example` directories. Run `test-mongocrypt` from the source directory:

```
cd libmongocrypt
./cmake-build/test-mongocrypt
```

libmongocrypt is continuously built and published on evergreen. Submit patch builds to this evergreen project when making changes to test on supported platforms.
The latest tarball containing libmongocrypt built on all supported variants is [published here](https://s3.amazonaws.com/mciuploads/libmongocrypt/all/master/latest/libmongocrypt-all.tar.gz).

### Troubleshooting ###
If OpenSSL is installed in a non-default directory, pass `-DOPENSSL_ROOT_DIR=/path/to/openssl` to the cmake command for libmongocrypt.

If there are errors with cmake configuration, send the set of steps you performed to the maintainers of this project.

If there are compilation or linker errors, run `make` again, setting `VERBOSE=1` in the environment or on the command line (which shows exact compile and link commands), and send the output to the maintainers of this project.

### Releasing ###

See [releasing](./doc/releasing.md).

## Installing libmongocrypt From Distribution Packages ##
Distribution packages (i.e., .deb/.rpm) are built and published for several Linux distributions.  The installation of these packages for supported platforms is documented here.

### Unstable Development Distribution Packages ###
To install the latest unstable development package, change `1.14` to `development` in the package URLs listed in the subsequent instructions. For example, `https://libmongocrypt.s3.amazonaws.com/apt/ubuntu <release>/libmongocrypt/1.14` in the instructions would become `https://libmongocrypt.s3.amazonaws.com/apt/ubuntu <release>/libmongocrypt/development`. Do not use the unstable version of libmongocrypt in a production environment.

### .deb Packages (Debian and Ubuntu) ###

The repository containing the Debian and Ubuntu .deb packages can be configured automatically, using extrepo, or manually. Once the repository is configured then the packages can be installed.

#### Repository configuration with extrepo ####

Extrepo is available on Debian 11 and newer, as well as Ubuntu 20.04 and newer.

First, install the extrepo package:

```
sudo apt install extrepo
```

If you would like to see the information about the repository, it can be viewed with the search command:

```
extrepo search libmongocrypt
```

In order to enable the repository, execute this command:

```
sudo extrepo enable libmongocrypt
```

Once the repository is configured, continue with package installation.

#### Manual repository configuration ####

First, import the public key used to sign the package repositories:

```
sudo sh -c 'curl -s --location https://pgp.mongodb.com/libmongocrypt.asc | gpg --dearmor >/etc/apt/trusted.gpg.d/libmongocrypt.gpg'
```

Second, create a list entry for the repository.  For Ubuntu systems (be sure to change `<release>` to `xenial`, `bionic`, `focal`, or `jammy`, as appropriate to your system):

```
echo "deb https://libmongocrypt.s3.amazonaws.com/apt/ubuntu <release>/libmongocrypt/1.14 universe" | sudo tee /etc/apt/sources.list.d/libmongocrypt.list
```

For Debian systems (be sure to change `<release>` to `stretch`, `buster`, `bullseye`, or `bookworm` as appropriate to your system):

```
echo "deb https://libmongocrypt.s3.amazonaws.com/apt/debian <release>/libmongocrypt/1.14 main" | sudo tee /etc/apt/sources.list.d/libmongocrypt.list
```

#### Package installation ####

Finally, update the package cache and install the libmongocrypt packages:

```
sudo apt-get update
sudo apt-get install -y libmongocrypt-dev
```

### .rpm Packages (RedHat, Suse, and Amazon) ###

RPMs are available for supported systems running on both x86_64 and AArch64 (also called ARM64) processors. The sections below use `x86_64` in the example repository URLs. Substituting `aarch64` in the place of `x86_64` will permit installation of libmongocrypt packages on systems running on AArch64 processors.

#### RedHat Enterprise Linux ####

Create the file `/etc/yum.repos.d/libmongocrypt.repo` with contents:

```
[libmongocrypt]
name=libmongocrypt repository
baseurl=https://libmongocrypt.s3.amazonaws.com/yum/redhat/$releasever/libmongocrypt/1.14/x86_64
gpgcheck=1
enabled=1
gpgkey=https://pgp.mongodb.com/libmongocrypt.asc
```

Then install the libmongocrypt packages:

```
sudo yum install -y libmongocrypt
```

#### Amazon Linux 2023 ####

Create the file `/etc/yum.repos.d/libmongocrypt.repo` with contents:

```
[libmongocrypt]
name=libmongocrypt repository
baseurl=https://libmongocrypt.s3.amazonaws.com/yum/amazon/2023/libmongocrypt/1.14/x86_64
gpgcheck=1
enabled=1
gpgkey=https://pgp.mongodb.com/libmongocrypt.asc
```

Then install the libmongocrypt packages:

```
sudo yum install -y libmongocrypt
```

#### Amazon Linux 2 ####

Create the file `/etc/yum.repos.d/libmongocrypt.repo` with contents:

```
[libmongocrypt]
name=libmongocrypt repository
baseurl=https://libmongocrypt.s3.amazonaws.com/yum/amazon/2/libmongocrypt/1.14/x86_64
gpgcheck=1
enabled=1
gpgkey=https://pgp.mongodb.com/libmongocrypt.asc
```

Then install the libmongocrypt packages:

```
sudo yum install -y libmongocrypt
```

#### Amazon Linux ####

Create the file `/etc/yum.repos.d/libmongocrypt.repo` with contents:

```
[libmongocrypt]
name=libmongocrypt repository
baseurl=https://libmongocrypt.s3.amazonaws.com/yum/amazon/2013.03/libmongocrypt/1.14/x86_64
gpgcheck=1
enabled=1
gpgkey=https://pgp.mongodb.com/libmongocrypt.asc
```

Then install the libmongocrypt packages:

```
sudo yum install -y libmongocrypt
```

#### Suse ####

First, import the public key used to sign the package repositories:

```
sudo rpm --import https://pgp.mongodb.com/libmongocrypt.asc
```

Second, add the repository (be sure to change `<release>` to `12` or `15`, as appropriate to your system):

```
sudo zypper addrepo --gpgcheck "https://libmongocrypt.s3.amazonaws.com/zypper/suse/<release>/libmongocrypt/1.14/x86_64" libmongocrypt
```

Finally, install the libmongocrypt packages:

```
sudo zypper -n install libmongocrypt
```
