Santa Claus in Trouble (HD) Pak Tool
====================================

This is a simple program to unpack and repack '.pak' files that are present in the Santa Claus in Trouble (HD) game.
It can be used to modify the game files that are hidden in the .pak file.

Usage
-----

`scit-hd-pak-tool unpak <pak-path> <dir-path>`

Unpacks .pak file into individual files that will be stored in given directory.

`scit-hd-pak-tool repak <dir-path> <pak-path>`

Packs the files in given directory back into .pak file.

Example
-------

1. Run `scit-hd-pak-tool unpak "c:\program files\santa claus in trouble\data.pak" c:\data_pak`
2. Edit files in directory `c:\data_pak`
3. Run `scit-hd-pak-tool repak c:\data_pak "c:\program files\santa claus in trouble\data.pak"`

Requirements
------------

x86-64 Windows 10 machine

Source
------

Source code can be found here: https://github.com/jeysym/scit-hd-pak-tool

Author
------

Created by Jeysym in 2023
