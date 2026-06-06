# Linux Installation

Use this guide only if your computer runs Linux.

## If Nothing Is Installed

Install these first:

1. CMake.
2. A C++ compiler and build tools.
3. wxWidgets development files.
4. GTK development files.
5. Python 3.
6. Python headers.
7. pip.
8. Packaging helpers such as `dpkg-dev`, `fakeroot`, or `zip`.

On Ubuntu or Debian:

```bash
sudo apt update
sudo apt install -y cmake g++ make pkg-config libwxgtk3.2-dev libgtk-3-dev python3 python3-dev python3-pip dpkg-dev fakeroot zip
```

On Fedora:

```bash
sudo dnf install -y cmake gcc-c++ make pkgconf-pkg-config wxGTK-devel gtk3-devel python3 python3-devel python3-pip rpm-build zip
```

On Arch Linux:

```bash
sudo pacman -S --needed cmake gcc make pkgconf wxwidgets-gtk3 gtk3 python python-pip zip
```

Then run the installer script from the `batView` folder:

```bash
./install_linux.sh
```

If you do not want the app to open at the end, run:

```bash
./install_linux.sh --no-run
```

## If Everything Is Installed

If CMake, the compiler, wxWidgets, GTK, Python, and the packaging helpers are already installed, you can skip the dependency step.

Run:

```bash
./install_linux.sh --skip-deps
```

If you also do not want the app to open, use:

```bash
./install_linux.sh --skip-deps --no-run
```

## What You Should See

When it works, the script ends with `batView full listo.` and creates a Desktop launcher named `batView.desktop`.
