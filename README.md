

# Vau

the vau can run with pscm now.

That's one small step for TeXmacs, one giant leap for PikachuHy's Scheme (pscm).

## ScreenShot

![vau pdf preview](http://cdn.pikachu.net.cn/pscm/vau/vau-test_pdf.png)

## How to

- prepare

```
brew install ghostscript
brew install freetype
```

- clone vau

```
git clone https://github.com/PikachuHy/vau.git -b vau_pscm
```

- clone pscm

we use pscm as subproject of vau

```
cd vau
git clone https://github.com/PikachuHy/pscm.git
cd pscm
git submodule update --init
```
- build with cmake

```
mkdir build && cd build
cmake ..
make -j
```

- run 

```
TEXMACS_PATH=/path/to/vau/resources PSCM_LOAD_PATH=/path/to/vau/resources/progs ./Vau
```

then `vau-test.ps` file generated

- convert to pdf

```
ps2pdf vau-test.ps
```



## Note

- Lots and lots of bugs are still in the pscm.

- Only some of vau's features are available.

- Build and run success on macOS only.

**Vau** is an experiment/exercise over the TeXmacs codebase, to learn more about it. The initial goal is to extract enough machinery to be able to read and typeset arbitrary TeXmacs files. So **Vau** will be initially a viewer. This will allow me to understand the code dependencies and extract a minimal typesetting core, abstracted from the UI and the wider organization of the editor. In the meanwhile I plan to experiment about various refactorings.

### Glue code refactoring

I rewrote the Scheme glue code via C++ templating and metaprogramming and make it local to each submodule. Glue code should be defined locally where the relevant C++ function belongs, so that if a module is not linked in the corresponding glue code is not compiled in and will not be visible from scheme. 


## Logbook

Jan 2022 - the program compiles (under Mac) but still does not produce the correct output. I still have to figure out why.



