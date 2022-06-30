<p align="center"> <img src="https://user-images.githubusercontent.com/70243457/156932366-71635391-789e-4c5d-8b79-d17beef5d387.png" alt="logo" height="300" style="display:block;margin-left: auto;margin-right: auto;"></p>
<h1 align="center">Pa</h1>
<p align="center">Balanced as all things should be.</p>

<p align="center">
    <img src="https://img.shields.io/github/workflow/status/valkarias/Pa/Release-builds?style=flat-square" alt="logo">
    <img src="https://img.shields.io/maintenance/yes/2022?style=flat-square" alt="logo">
</p>
<br>

## What
<p style="text-align: center;margin-top:30px">You are welcomed to the holy lands of Pa.</p>  
Pa is a hobby project designed around familiarity & quick-prototyping, it balances various features and makes them fit in one environment for the ease of development and focus.  


## Documentation
![map-marker-home](https://user-images.githubusercontent.com/70243457/170840915-f9427da9-428b-43ad-aacf-8529883dde55.png)&nbsp;
New here? Head straight to [Pa's Documentation](https://valkarias.github.io/contents/toc.html) to learn the basics!

![share-free-icon-font](https://user-images.githubusercontent.com/70243457/170840963-2f0620c4-5488-4565-98ad-c47e593d136b.png)&nbsp;
Examples are all in the [examples folder](https://github.com/valkarias/Pa/tree/master/examples).  

![refresh-free-icon-font](https://user-images.githubusercontent.com/70243457/170841086-63a641d1-f1ad-49b5-9870-fbb05e7cf6e6.png)&nbsp;
And lastly, check the [Update Log](https://github.com/valkarias/Pa/blob/master/LOG.md) page for any major or minor changes added!

<br>

# Up & Running
<p align="center">Pa uses a pre-built command line tool called <a href="https://github.com/valkarias/PBuild">PBuild</a> to automate the process of downloading & building.</p>  
<h3 align="center">Download PBuild</h3>
<p align="center"><a href="https://github.com/valkarias/PBuild"><img src="https://user-images.githubusercontent.com/70243457/170841143-952e0d8a-2ae5-4659-9d88-677fbaa94291.png" alt="download"></a></p>
<br>
<br>

<p align="center">Follow the instructions to install Pa using <a href="https://github.com/valkarias/PBuild">PBuild</a> <a href="https://valkarias.github.io/contents/chapters/getting-started.html">here</a>.</p>

-----------------------

### Tooling
- A more versatile tooling is planned! 

At the moment there is only a syntax highlighter for the vscode text editor.  
Avaliable in the marketplace [here](https://marketplace.visualstudio.com/items?itemName=Pa-lang.pa-syntax-highlighter)  


## Example
Here is a tail-call fibonacci function implemented in Pa.
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
Thanks [Gwion-Benchmarks](https://github.com/Gwion/gwion-benchmark) for benchmarking Pa.  
Here are the results of a full benchmark suite for Pa against some other languages -> [benchmarks](https://gwion.github.io/Gwion/Benchmarks.html).

## Thanks to
- [Dictu](https://github.com/dictu-lang/Dictu) By Jason Hall.  
 
- [Crafting Interpreters](https://github.com/munificent/craftinginterpreters) By Bob Nystrom.
- [Wren](https://github.com/wren-lang/wren) By Wren-lang Organization.
- [PocketLang](https://github.com/ThakeeNathees/pocketlang) By ThakeeNathees.  

Huge thanks to [Jérémie Astor](https://github.com/fennecdjay) for helping Pa!

# License

Licensed under the [MIT License](https://github.com/valkarias/Pa/blob/master/LICENSE).  