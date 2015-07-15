# relative-pattern
This is a tool experimenting a *formal method* to recover program control flow graph from binaries obfuscated by *virtualizing obfuscation*, even when binaries are virtualized **mutliple times**. Currently, it considers the transformations of

* [Tigress](http://tigress.cs.arizona.edu/)
* [VMProtect](http://vmpsoft.com/)
* [Code Virtualizer](http://oreans.com/)
* [O-LLVM](https://github.com/obfuscator-llvm/obfuscator)<sup>1</sup>
* Other ad-hoc implementations<sup>2</sup>.

The code is in active development, still buggy and difficult to use. The [underlying concolic execution engine](http://binsec.gforge.inria.fr/) are not fully published yet<sup>3</sup>, though the current published code can work with any concolic/fuzzing engine. Moreover *the strength of this tool depends only on the execution engine*, that is a rational theoretical limit of the method.

Though I follow a mathematical approach, seriously I think the main idea is simple. It may be considered implicitly in many practical "unpack tutorials" of great hackers and crackers. While I am just a newbie, the only original contribution here is to give a more solid *theoretical base* that explains these concrete techniques, and this leads to a "less ad-hoc"<sup>4</sup> deobfuscation implementation.

The tool is written mostly in C++ and OCaml, and uses the following great softwares:
* [BinSec](http://binsec.gforge.inria.fr/)
* [Pin](https://software.intel.com/en-us/articles/pin-a-dynamic-binary-instrumentation-tool)
* [Boost](http://www.boost.org/)
* [tinyformat](https://github.com/c42f/tinyformat)
* [Protocol Buffers](https://github.com/google/protobuf)
* [ELFIO - ELF](https://github.com/serge1/ELFIO)

Currently there is no documentation (if you are interested in, I am very happy to answer any question). I try also to prepare a paper on this but there are still a lot of things to do.

![alg tag](demo/code_virtualizer.png)

<sup>1</sup>[O-LLVM](https://github.com/obfuscator-llvm/obfuscator) does not support yet virtualization transformations, though control-flow-graph flattening can be considered as a "light-weight" virtualization.

<sup>2</sup>Collected from [crackmes.de](http://crackmes.de/).

<sup>3</sup>[BinSec](http://binsec.gforge.inria.fr/) is in very active development and it will be open when it is ready, some technical documents and (rather old) source codes can be referenced [here](http://sebastien.bardin.free.fr/binsec.html).

<sup>4</sup>Hopefully.
