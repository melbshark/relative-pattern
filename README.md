# relative-pattern
A tool for recovering program control flow graph from binaries obfuscated by
* [Tigress](http://tigress.cs.arizona.edu/)
* [VMProtect](http://vmpsoft.com/)
* [Code Virtualizer](http://oreans.com/)
* [O-LLVM](https://github.com/obfuscator-llvm/obfuscator)

The code is in active development and difficult to use. The modules of the concolic execution engine are not published yet, though the current published code can work with any concolic or fuzzing engine. Currently there is no documentation (please contact me if you are interested in). I try also to prepare a paper on this but there are still lots of thing to do.

The tool is written mostly in C++ and OCaml, and uses the following softwares:
* [ELFIO](https://github.com/serge1/ELFIO)
* [tinyformat](https://github.com/c42f/tinyformat)
* [Pin](https://software.intel.com/en-us/articles/pin-a-dynamic-binary-instrumentation-tool)
* [Protocol Buffers](https://github.com/google/protobuf)
