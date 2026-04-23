# Download batView on Linux

Follow these steps in order to download and open the app on Linux.

## Step 1: Download the Linux package

Download the batView file prepared for Linux.

Common formats:

- a compressed archive such as `.zip` or `.tar.gz`
- a Debian package such as `.deb`

## Step 2: Open the downloaded file

Go to your `Downloads` folder and find the file you downloaded.

## Step 3: Install or extract the app

If you downloaded an archive:

1. Extract the file.
2. Open the extracted folder.

If you downloaded a `.deb` package on Debian or Ubuntu:

1. Open a terminal in the folder that contains the file.
2. Run:

```bash
sudo apt install ./batView-*.deb
```

## Step 4: Open batView

If you extracted an archive:

1. Open a terminal inside the extracted folder.
2. Run:

```bash
chmod +x batView
./batView
```

If you installed a `.deb` package, open the app from your applications menu or run it from the installed location.

## Step 5: Keep the required files together

If you are using the extracted version, keep the `assets/` folder next to the `batView` executable.
