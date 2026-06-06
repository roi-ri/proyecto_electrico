# macOS Installation

Use this guide only if your computer is a Mac.

## If Nothing Is Installed

Install these first:

1. Apple Command Line Tools.
2. Homebrew.
3. `cmake`.
4. `wxwidgets`.
5. `python`.
6. `pkg-config`.

How to install them:

```bash
xcode-select --install
```

After the Apple installer finishes, install Homebrew if you do not already have it:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Then install the build tools:

```bash
brew install cmake wxwidgets python pkg-config
```

Now run the installer script from the `batView` folder:

```bash
./install_macos.sh
```

If you do not want the app to open at the end, run:

```bash
./install_macos.sh --no-run
```

## If Everything Is Installed

If Apple Command Line Tools, Homebrew, CMake, wxWidgets, Python, and pkg-config are already installed, you can skip the dependency step.

Run:

```bash
./install_macos.sh --skip-deps
```

If you also do not want the app to open, use:

```bash
./install_macos.sh --skip-deps --no-run
```

## What You Should See

When it works, the script ends with `batView full listo.` and the app appears on the Desktop as `batView.app`.
