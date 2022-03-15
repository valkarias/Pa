# PCrap
<img src="https://user-images.githubusercontent.com/70243457/156932366-71635391-789e-4c5d-8b79-d17beef5d387.png" alt="logo" height="300" style="display:block;margin-left: auto;margin-right: auto;">

<img src="https://img.shields.io/github/workflow/status/valkarias/PCrap/Release-builds?style=flat-square" alt="logo" style="display:block;margin-left: auto;margin-right: auto;margin-bottom:10px">
<img src="https://img.shields.io/maintenance/yes/2022?style=flat-square" alt="logo" style="display:block;margin-left: auto;margin-right: auto;margin-bottom:15px">



<p style="text-align: center;margin-bottom:30px">You are welcomed to the holy lands of PCrap.</p>  

Pcrap is a hobby project designed around & specifically for quick-prototyping and that was done by making the language familiar enough but also unique by balancing various features and making them fit in.  

Huge thanks to Jérémie Astor and his language [Gwion](https://github.com/Gwion/Gwion) for helping PCrap!

## Documentation
If you are completely new, Please head to the [Pcrap Documentation](https://valkarias.github.io/contents/toc.html).
The standard library documentation will be gradually added.

Examples are all in the [examples folder](https://github.com/valkarias/PCrap/tree/master/examples).  

Check the [Update Log](https://github.com/valkarias/PCrap/blob/master/LOG.md) page for any major or minor changes added!

## Up & Running
- PCrap uses a tool called [PBuild](https://github.com/valkarias/PCrap/releases) to download & build.  
Building manually is possible.

### PBuild
Assuming you downloaded PBuild from [releases](https://github.com/valkarias/PCrap/releases)  
- You need to extract the zip.  

The executable is located in `PBuild_(platform) -> pbuild -> pbuild(.exe)`  

PBuild have 4 commands as of now: 


#### download 
Which clones the PCrap repo into the user's home directory.  
```bash
pbuild download
```
- the build command will not work if the download command was not executed.     

#### build
This command builds the cloned PCrap repo and modifies the Path environment variable to the executable. It takes a required option specifying the compiler to build with. 
```bash
pbuild build --cc-type=<compiler name>
``` 
There are two compilers supported as of now:  
[Gnu Compiler Collection](https://gcc.gnu.org) (gcc) & [Tiny C Compiler](https://bellard.org/tcc/) (tcc). 

#### uninstall 
This command erases and uninstalls the pcrap repo from you computer :(
```bash
pbuild uninstall
```  

#### version
And this one displays the latest release version from [here](https://github.com/valkarias/PCrap/releases)
```bash
pbuild version
```

### Tooling
- A more versatile tooling is planned! 

At the moment there is only a syntax highlighter for the vscode text editor.  
Avaliable in the marketplace [here](https://marketplace.visualstudio.com/items?itemName=PCrap.pcrap-syntax-highlighter)  


## Example
Here is a tail-call fibonacci function implemented in pcrap.
```js
define fib (n, a, b) {
    if n == 0 {
        return a;
    }
    
    if n == 1 {
        return b;
    }

    return fib(n - 1, b, a + b);
}
```

## Performance  
Thanks [Gwion-Benchmarks](https://github.com/Gwion/gwion-benchmark) for benchmarking PCrap.  
Here are the results of a full benchmark suite for pcrap against some other languages -> [benchmarks](https://gwion.github.io/Gwion/Benchmarks.html).

## Thanks to
- [Dictu](https://github.com/dictu-lang/Dictu) By Jason Hall.  
 
- [Crafting Interpreters](https://github.com/munificent/craftinginterpreters) By Bob Nystrom.
- [Wren](https://github.com/wren-lang/wren) By Wren-lang Organization.
- [PocketLang](https://github.com/ThakeeNathees/pocketlang) By ThakeeNathees.

# License

Licensed under the [MIT License](https://github.com/valkarias/PCrap/blob/master/LICENSE).  
Please look into [NOTICE](https://github.com/valkarias/PCrap/blob/master/NOTICE.txt) for important details regarding references and credits.
