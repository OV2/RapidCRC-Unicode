# RapidCRC Unicode

RapidCRC is an open source CRC/MD5/SHA hashing program. I've extended the current unicode support to allow writing of unicode .sfv/.md5/.sha1/.sha256/.sha512 files, which can be turned on/off through the settings page. Unicode sfv files are written as UTF-16LE with BOM or UTF-8 (with or without BOM), and RCRC now performs a codepage detection. I've also expanded the shell extension that was included in the sourcecode to enable all operations directly from the extension.

List of other new features:

- multithreaded hash calculations with asynchronous I/O
- ed2k hash calculation
- SHA hash calculation
- blake2sp hash calculation
- job queueing
- popup menu to copy the calculated hashes to the clipboard

[Prebuilt installers / Portable releases](https://ov2.eu/programs/rapidcrc-unicode)
