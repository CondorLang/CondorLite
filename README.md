# <img src='https://github.com/CondorLang/Condor/blob/master/web/images/CondorLogo.png' width='40px' /> Condor <img src='https://github.com/CondorLang/CondorLite/workflows/C/C++%20CI/badge.svg'>

# Experiemental C Creation
The purpose behind this part of the library is to optimize speed. When using the Malloc call, 
it takes about 3,000% more time than just using the stack. For this reason, we are developing this to 
See how close we can get to the nanoseconds as possible. Avoiding Malloc also allows for the compiler
to be portable for any operating system.

## Building
```
./configure [-d|--debug] [-e|--execute] [-c|--clean] [-n|--namespace fileName] [-t|--test] [-s|--debug-syntax]
```

### Arguments
 - Debug: Initiates and runs all the debug prints throughout the code. These could be in any file. Due to the exhaustive amount of debug calls, we created a namespace to filter
 - Namespace: The file name to filter the debugs
 - Execute: Execute after building
 - Clean: Clean before building
 
## Rules
 - Do not use Malloc unless absolutely needed
 - Do not implement a file retriever function. Only char* can be passed directly
 - Always run ./mem which runs Valgrind. No memory leaks allowed.