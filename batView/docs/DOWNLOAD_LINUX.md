# Download batView on Linux

Follow these simple steps to download and open batView on Linux.

## 1. Download batView

Download the Linux file made for batView.

Common formats:

- a compressed archive such as `.zip` or `.tar.gz`
- a Debian package such as `.deb`

## 2. Open the file

Go to your `Downloads` folder and open the file you downloaded.

## 3. Install or unzip it

If you downloaded an archive:

1. Extract the file.
2. Open the folder that was created.
3. If you see `batView.desktop`, keep it with the app folder.

If you downloaded a `.deb` package on Debian or Ubuntu:

1. Open a terminal in the folder with the file.
2. Run this command:

```bash
sudo apt install ./batView-*.deb
```

## 4. Start batView

If you extracted an archive:

1. Open a terminal inside the folder.
2. Run:

```bash
chmod +x batView
./batView
```

If you installed a `.deb` package, open batView from your applications menu.

The Linux package uses the batView image for the icon.
The `.deb` package also adds batView to your applications menu.

## 5. Keep the files together

If you are using the extracted version, keep the `assets/` folder next to the `batView` file.
