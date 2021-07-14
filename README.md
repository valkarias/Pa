# PCrap
![maintenance](https://img.shields.io/maintenance/yes/2021?style=flat-square)
![License](https://img.shields.io/badge/license-MIT-green?style=flat-square)
![Version](https://img.shields.io/badge/version-0.1-green?style=flat-square)

You are welcomed to the holy lands of PCrap.  

Pcrap is a hobby project, its designed to be small. The goal was to make a language that i can use in my daily life.

## Documentation
If you are completely new, Please head to the [Pcrap guide](https://cdn.flipsnack.com/widget/v2/widget.html?hash=dcs6n9hgvu).  
The documentation is a future plan.  
**The Guide and the Documentation are in progress, Expect Major Changes.**  

Examples are all in the [examples folder](https://github.com/valkarias/PCrap/tree/master/examples).

## Up & Running
- PCrap uses a tool called [PBuild](https://github.com/valkarias/PCrap/releases) to download & build.  
Building manually is possible.

### PBuild
Assuming you downloaded PBuild from [releases](https://github.com/valkarias/PCrap/releases)  
- You need to extract the zip.  

The executable is located in `PBuild_{platform} -> pbuild -> pbuild`  

PBuild have only 2 commands as of now:
```bash
pbuild.exe download
```
Which clones the PCrap repo into the user's home directory.  
- the build command will not work if the download command was not executed.
```bash
pbuild.exe build --cc-type=<compiler name>
```
This command builds the cloned PCrap repo. It takes a required option specifying the compiler to build with.  

There are two compilers supported as of now.  
[Gnu Compiler Collection](https://gcc.gnu.org) (gcc) & [Tiny C Compiler](https://bellard.org/tcc/) (tcc).

## Example
Here is a split string function algorithm implemented in pcrap.
```js
define split > st,dl > {

    let word = "";
    let number = 0;

    let full = st + dl;
    let length = full.length();

    let temp = [];

    for let i = 0; i < length; i++ {
        if full[i] != dl {
            word = word + full[i];
        } else {
            if word.length() != 0 {
                temp.append(word);
            }

            word = "";
        }
    }

    return temp;
}
```

## Thanks to
- [Dictu](https://github.com/dictu-lang/Dictu) By Jason Hall.
- [Gwion](https://github.com/Gwion/Gwion) By Jérémie Astor.  
 
- [Crafting Interpreters](https://github.com/munificent/craftinginterpreters) By Bob Nystrom.
- [Wren](https://github.com/wren-lang/wren) By Wren-lang Organization.
- [PocketLang](https://github.com/ThakeeNathees/pocketlang) By ThakeeNathees.

# License

Licensed under the [MIT License](https://github.com/valkarias/PCrap/blob/master/LICENSE).  
Please look into [NOTICE](https://github.com/valkarias/PCrap/blob/master/NOTICE.txt) for important details regarding references and credits.
