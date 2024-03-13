# FA2sp_Remastered
... is the FA2sp source code version. For now it only supports YR but I will try to make it supports TS after I've migrated most features from FA2sp to this repo.

Compared to FA2sp, the open source code give me a chance to finally replace the old ddraw rendering process, I'm now trying to make it run base on `Direct3D11`, `Direct2D` and `DirectWrite` instead of deprecated `DirectDraw`.

Also, I've tried to remove the 3rd party dependencies, so you can simply compile it without the heavy boost library. This work also give us the opportunity to move on x64 compile target, besides the x86 one (forced by XCC lib). I've tried to make it pass compile on x64 and it works as intended for now.

**NOTICE, this repository cannot be used for making a map yet!**

## Links
- [C&C World-Altering Editor(WAE), a open source modern map editor written in C# [**RECOMMENDED**]](https://github.com/Rampastring/WorldAlteringEditor)

- [FA2sp, an extension for the old FA2 which contains bugfixes and QOL features.](https://github.com/secsome/FA2sp)

## TODO
- [X] Filenames
- [ ] Render system reimplement
    - [X] Basic file texture making
        - [X] ShpFile
        - [X] TmpFile
        - [X] VxlFile
        - [X] HvaFile
        - [X] VplFile
    - [ ] Scene rendering
        - [ ] Background tmp drawing
        - [ ] Infanty drawing
        - [ ] Aircrafts drawing
        - [ ] Units drawing
        - [ ] Structures drawing
        - [ ] Overlays drawing
        - [ ] Smudges drawing
        - [ ] Misc map elements drawing
        - [ ] Other UI drawing
