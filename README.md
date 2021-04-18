# SVG/WMF/EMF WIC Decoder and Thumbnail Provider

Windows Imaging Component (WIC) SVG/WMF/EMF decoders and File Explorer thumbnail provider extension.  Gzip-compressed formats (.svgz, .wmz, .emz) are supported.  
The extension uses [Direct2D](https://docs.microsoft.com/en-us/windows/win32/direct2d/) to render SVG and WMF/EMF graphics.

## System Requirements
Windows 10 Creators Update (1703, build 15063) or newer is required. No additional libraries or frameworks are used.
#### Build Environment
Microsoft Visual Studio Community 2019

## Installation
The extension module architecture must match the architecture of your system. Download [**64-bit** package (English)](Release/x64/en-us/svgwext2021_x64_en-us.msi) for **64-bit** Windows, or [**32-bit** package (English)](Release/x86/en-us/svgwext2021_x86_en-us.msi) for **32-bit** Windows.  
Then just install the downloaded .MSI package.

If you prefer to build the source code by yourself and/or use DLL self-registration, the corresponding routines (`DllRegisterServer`, `DllUnregisterServer`, `DllInstall`) are available.

## Downloads
[Choose](Release) an appropriate package or use these direct links:

- [**64-bit** .msi package (English)](../../raw/main/Release/x64/en-us/svgwext2021_x64_en-us.msi)
- [**64-bit** .msi package (Russian)](../../raw/main/Release/x64/ru-ru/svgwext2021_x64_ru-ru.msi)
- [**32-bit** .msi package (English)](../../raw/main/Release/x86/en-us/svgwext2021_x86_en-us.msi)
- [**32-bit** .msi package (Russian)](../../raw/main/Release/x86/ru-ru/svgwext2021_x86_ru-ru.msi)

\* Note that the binaries are language-neutral, only installation package UI is translated.  

__________
## License
This program is free software: you can redistribute it and/or modify it under the terms of the [GNU General Public License](LICENSE.md) as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
