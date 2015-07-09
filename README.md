# relative-pattern
This is a tool experimenting a *formal method* for recovering program control flow graph from binaries virtualized by
* [Tigress](http://tigress.cs.arizona.edu/)
* [VMProtect](http://vmpsoft.com/)
* [Code Virtualizer](http://oreans.com/)
* [O-LLVM](https://github.com/obfuscator-llvm/obfuscator)<sup>1</sup>
* Other ad-hoc implementations<sup>2</sup>.

The code is in active development, still buggy and difficult to use. The [underlying concolic execution engine](http://binsec.gforge.inria.fr/) are not published yet, though the current published code can work with any concolic or fuzzing engine. Moreover *the strength of this tool depends only on the strength of the underlying execution engine*, that demonstrates also a rational theoretical limit of the method.

Though this tool follows a mathematical approach, seriously I think the main idea is not new. It may be considered implicitly in many practical "unpack tutorials" of great hackers and crackers. While I am just a newbie, the only original contribution here is to give a more solid *theoretical base* that explains these concrete techniques, and this leads to a "less ad-hoc"<sup>3</sup> deobfuscation method.

The tool is written mostly in C++ and OCaml, and uses the following great softwares:
* [BinSec](http://binsec.gforge.inria.fr/)
* [Pin](https://software.intel.com/en-us/articles/pin-a-dynamic-binary-instrumentation-tool)
* [Boost](http://www.boost.org/)
* [tinyformat](https://github.com/c42f/tinyformat)
* [Protocol Buffers](https://github.com/google/protobuf)
* [ELFIO - ELF](https://github.com/serge1/ELFIO)

![alg tag](demo/code_virtualizer.png)

Currently there is no documentation (please contact me if you are interested in). I try also to prepare a paper on this but there are still a lot of things to do.

<sup>1</sup>[O-LLVM](https://github.com/obfuscator-llvm/obfuscator) does not support yet virtualization transformations, though control-flow-graph flattening can be considered as a "light-weight" virtualization.

<sup>2</sup>Collected from [crackmes.de](http://crackmes.de/).

<sup>3</sup>Hopefully.
