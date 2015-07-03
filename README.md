# relative-pattern
A tool for recovering program control flow graph from binaries obfuscated by
* [Tigress](http://tigress.cs.arizona.edu/)
* [VMProtect](http://vmpsoft.com/)
* [Code Virtualizer](http://oreans.com/)
* [O-LLVM](https://github.com/obfuscator-llvm/obfuscator)

The code is in active development and difficult to use. The modules of the concolic execution engine are not published yet, though the current published code can work with any concolic or fuzzing engine. Currently there is no documentation (please contact me if you are interested in). I try also to prepare a paper on this but there are still lots of thing to do.

The tool is written mostly in C++ and OCaml, and uses the following softwares:
* [ELFIO - ELF](https://github.com/serge1/ELFIO)
* [Type safe printf](https://github.com/c42f/tinyformat)
* [Pin](https://software.intel.com/en-us/articles/pin-a-dynamic-binary-instrumentation-tool)
* [Protocol Buffers](https://github.com/google/protobuf)

Demo:
The following function contains a basic control structure
````C++
int msign(int x) 
{
  if (x > 0) return 1;
  else if (x < 0) return -1;
  else return 0;
}
```
The control flow graph of its binary code is as follow:
![Original](demo/rescfg_bb.pdf)

The virtualization transformation of Tigress deforms this CFG into
![Tigress](demo/rescfg_switch_bb.pdf)

or one of VMProtect gives
![VMProtect](demo/rescfg_vmprotect_bb.pdf)

The tool, given any obfuscated binary, can recover the original CFG as
![Recover](demo/rescfg_virtual.pdf)
