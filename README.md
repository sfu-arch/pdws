# Path Derived Workload Suite 1.0

A workload suite for accelerators derived from microprocessor workloads by identifying and isolating, recurrent and acceleratable behaviour. 

## Contents

This repository contains LLVM bitcode distributed as part of the Path Derived Workload Suite [1]. The bitcode has been compiled for a x86_64 system using LLVM 3.8.1 [2]. Each bitcode file is self-contained, i.e all the source has been compiled to bitcode and linked together. There are no external dependencies. Each workload has 5 different bitcode files. Each bitcode contains one `path` outlined as a function in the form `__offload_func_XXX` where `XXX` is the Ball-Larus path id of interest.

## Directory Structure
```
x86_64  
\-- sw_undo
\-- hw_undo  
\-- tools  
    \-- pin  
```
## Execution Model

An acyclic path through a control flow graph spans across one more branches. Deviation from the outlined path requires the restoration of program state and re-execution.

Guard Checks :  
Checks to ensure correct path execution are implemented as branches. If a guard check fails the function immediately returns false. Successful execution of the outlined path returns true.

Software Undo :   
Memory state modifications are saved in an undo log global variable during the execution of the workload. If any guard condition in the outlined path evaluates to false, the memory state is restored using a software undo runtime. 

Hardware Undo :  
There is *no* software support for undoing changes to memory state. This version of the bitcode assumes the presence of a hardware store commit buffer. The contents of the buffer are only flushed to memory on the successful completion of the path. A use case for this bitcode is on architectural simulators. This version of bitcode has undefined behaviour without hardware undo support.

## Usage

0. Pre-Requisites : Please download LLVM 3.8.1 binaries from [2].

1. Compile to x86_64 binary  
```
$ clang x86_64/sw_undo/164.gzip/20.bc -o 164.gzip
```

2. Human readable bitcode format
```
$ llvm-dis x86_64/sw_undo/164.gzip/20.bc -o 164.gzip.ll
```

3. Pintool : Requires Pin Rev 71313
```
$ export PIN_HOME=your/pin/install/dir
$ make -C tools/pin 
$ pin -t tools/pin/obj-intel64/MyPintool.so -- 164.gzip
```
## Note : Overlapping Paths

In some cases the top five paths include paths which overlap in part or a path is subsumed in another. In these cases the execution model may not correctly indicate the execution of the intended path due to the greedy nature in which it is invoked. A list of paths where this is observed is provided in info/overlap.txt with the severity of the observed issue. For these paths the recommended approach is to independently account of the execution of the individual paths by ignoring the return type of the offload function.


## Info Files

```
info
\-- idmap.txt : Mapping of path id to path rank   
\-- overlap.txt : List of overlapping paths affected by execution model 
```


## References
[1] SPEC-AX and PARSEC-AX: Extracting Accelerator Benchmarks from Microprocessor Benchmarks  
Snehasish Kumar, William N. Sumner, Arrvindh Shriraman (Simon Fraser University) 
[2] http://llvm.org/releases/download.html 
[3] Ball, Thomas, and James R. Larus. "Efficient path profiling." Proceedings of the 29th annual ACM/IEEE international symposium on Microarchitecture. IEEE Computer Society, 1996.


