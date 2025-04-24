# FLT

FLT brings floating point support to C compilers lacking the capability. It is meant as a bridge to allow immediate development of floating point code in lieu of official native support at some future date.

## Usage

FLT requirements:

- `gcc` v9.0 or later (v9.0+); uses the `-fdiagnostics-format=json` option
- `PHP` v7.0 or later (v7.0+)
- the target compiler, e.g. `cc65`, should support an unsigned 32-bit integer type, e.g. `uint32_t`
- the source code should be ANSI C or later; K&R will NOT be supported!

To use FLT in your project:

- clone this repository or download the source code
- (optional) create a FLT library `flt.lib` by compiling the source files `flt-*.c` (see below for an example)
- convert your C `*.c` with floating point code to FLT `*-flt.c` using the PHP script `flt.php`
- compile `*-flt.c` as usual

Example to generate `«flt-repo»/flt/eg/averages`:

```
	cd «flt-repo»/flt
	php flt.php -i eg/averages.c -o eg/averages-flt.c
	gcc -o eg/averages eg/averages-flt.c flt-*.c
```

You can review `«flt-repo»/flt/eg/averages-flt.c` (code will be at the end) and you will see it has no floating point code – all converted to FLT.

For the `cc65` suite, you will probably want to do something like this. Assumes the `«flt-repo»/flt/flt-*.c` files have been compiled to `«flt-repo»/flt/flt.lib`, and the paths of `cc65`, `ca65`, and `ld65` are in `$PATH`.

```
	cd «flt-repo»/flt
	php flt.php -i eg/averages.c -o eg/averages-flt.c -x '-I «cc65-repo»/cc65/include'
	cc65 -t «target» eg/averages-flt.c
	ca65 -t «target» eg/averages-flt.s
	ld65 -o eg/averages -t «target» eg/averages-flt.o flt.lib -L «cc65-repo»/cc65/lib «target».lib
```

Another interesting example is `«flt-repo»/flt/eg/paranoia.c` adapted by [Sumner & Gay](https://people.math.sc.edu/Burkardt/c_src/paranoia/paranoia.html). Building and running this will give you an idea of any remaining defects or flaws in FLT and help you decide whether you want to use it.

To compile `«flt-repo»/flt/eg/paranoia.c` using `gcc` (it is probably too big to run in an 8-bit system):

```
	cd «flt-repo»/flt
	php flt.php -i eg/paranoia.c -o eg/paranoia-flt.c -x '-DNOSIGNAL -DSingle'
	gcc -o eg/paranoia eg/paranoia-flt.c flt-*.c
```

It is recommended to build `«flt-repo»/flt/flt.lib` and link to it to reduce the size of executables. For the `cc65` suite, there is a build script available as `«flt-repo»/flt/build-cc65`. Edit the build script to point `XCC` to where the `cc65` repo is located, revise `TGT` as required, and run the script to build the `«flt-repo»/flt/flt.lib` library.

Other interesting examples:

- `«flt-repo»/flt/eg/flops.c` – Inspired by [flops.c](https://github.com/AMDmi3/flops/blob/master/flops.c), see how blazing fast your machine is! :wink:

- `«flt-repo»/flt/eg/mandelfloat.c` – A fun little program borrowed from [mandelfloat.c](https://github.com/mrdudz/cc65/blob/fptest/samples/mandelfloat.c). Sample output:

```
	***************************************************
	***************************************************
	****************###################****************
	************###########################************
	*********#################################*********
	*******####%%%%%%%%%$$$-=$$%%###############*******
	******##%%%%%%%%$$$$==.%$+=$$$%%#############******
	*****#%%%%%%%$$$==#=*#=  -#+.$$%%%############*****
	****%%%%%$$--==--*           #-$%%%%###########****
	***#$$$$==.*    =.            *$$%%%############***
	***                         $+-$$%%%%###########***
	***#$$$$==.*    =.            *$$%%%############***
	****%%%%%$$--==--*           #-$%%%%###########****
	*****#%%%%%%%$$$==#=*#=  -#+.$$%%%############*****
	******##%%%%%%%%$$$$==.%$+=$$$%%#############******
	*******####%%%%%%%%%$$$-=$$%%###############*******
	*********#################################*********
	************###########################************
	****************###################****************
	***************************************************
	***************************************************
```

For the `cc65` suite, there is a build script available as `«flt-repo»/flt/eg/build-cc65` for the sample programs. Edit the build script to point `XCC` to where the `cc65` repo is located, revise `TGT` and `EXT` as required, then run the script. For the ***target***, it will build executables for `averages`, `mandelfloat`, and `flops`. For the ***host***, it will build executables for `errors` (needs to compare with standard floating point functions), `tests` (too large for 8-bit systems), and `paranoia` (also too large for 8-bit systems).

Be sure to adhere to the licensing terms provided in this and other repositories mentioned here to ensure proper usage and compliance.

## How It Works

Versions of `gcc` v9.0+ have an option `-fdiagnostics-format=json` to output errors and warnings in JSON. The JSON indicates exactly where the issues are including error messages, line numbers, and column positions. With a bit of substitution hocus-pocus to deliberately generate errors, it is possible to use `gcc` and `PHP` to parse the source code and determine where to substitute FLT code!

## Inspiration

The [`cc65`](https://github.com/cc65/cc65) repo for the `cc65` suite of tools has a multi-user multi-year project under development to support [native floating point](https://github.com/mrdudz/cc65/blob/fptest/Floating-point.md), but as they mention "You can not use any of this to write software yet. Don’t bother." There still appears to be a lot of work remaining. I considered contributing to the project but in reviewing the code, I felt the commitment to ramp-up was too much for what I can offer.

I thought instead "Would it be possible to do a light integration with no modifications to the existing compiler?" Say alias a 32-bit integer as a float type, and write functions with that? While writing the floating point routines was easy enough (except for `pow` – so many special cases!), I was stuck on converting floating point code to FLT. I researched many options like: adapting a [C99 parser in Python](https://github.com/eliben/pycparser), or using [CIL via OCaml](https://cil-project.github.io/cil/doc/html/cil/), or a [C++ to C converter](https://www.codeconvert.ai/c++-to-c-converter). But they were all lacking.

On a hunch, I looked at `gcc` v9.0+ and noticed it highlighted the locations of offending code during compilation. I thought, okay, I’ll have to write a clunky parser for the error output. But then I looked through the `man` options and saw `-fdiagnostics-format=json` to output errors in JSON. That was the key! The current version can still have problems with some C code, but it is a very good first step.

While the `cc65` suite of tools was the inspiration, there are a lot of other "tiny" compilers targeting microprocessors without native floating point support which could benefit from FLT.

## Features

- FLT corresponds to IEEE 754 single-precision floating point with one sign bit, eight exponent bits, and 24 significand bits (23 explicitly stored).
- As well as single-precision floating point, double-precision floating point literals, variables, and functions are also converted to `FLT` and `flt_*`. The C standard only specifies that the type `double` provides at LEAST as much precision as the type `float`, so this is permitted!
- Provides NaN, ±infinity, ±zero, and subnormal numbers
- Functions provided, can also use `float` versions, e.g. `sinf`, `cosf`, `tanf`, …
    - mathematical operators: `+`, `-`, `*`, `/`, `+=`, `-=`, `*=`, `/=`, `++`, `--`, `!`
    - comparison operators: `==`, `>`, `<`, `>=`, `<=`, `!=`
    - `ldexp`, `frexp`, `modf`, `fmod`
    - `fabs`, `round`, `trunc`, `ceil`, `floor`
    - `sqrt`, `hypot`
    - `log`, `log10`, `log2`, `exp`, `exp10`, `exp2`, `pow`
    - `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `atan2`
    - `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh`
    - `isinf`, `isnan`, `isnormal`, `issubnormal`, `iszero`, `isfinite`
    - `atof`, `ftoa`, `ltof`, `ultof`, `ftol`, `ftoul`
    - `fmin`, `fmax`, `fsgn`

## Current Limitations

These limitations may be revised as the project evolves:

- The accuracy of FLT is NOT professional grade and should NOT be used in mission-critical applications where errors can have serious consequences!
- As well, FLT is NOT optimized for speed or space. It is basically a temporary solution to provide floating point support in C compilers currently lacking it.
- Currently `gcc -fdiagnostics-format=json` v9.0+ does not provide enough information to parse certain constructions such as a cast spanning multiple lines, or the `scanf` example below. It is recommended to thoroughly test the programs after compiling to ensure correct functionality.
- Direct integer assignments to floating point like: `float f = 0, g = 1, h = 2;` aren't handled. Suggest using explicit floating point literals, for example: `float f = 0.0, g = 1f, h = 2e0;`. This is recommended best practice anyway and avoids conversion warnings in some compilers.
- I/O functions are limited in how many float parameters can be specified in a single function call. For `*printf`, up to 15 `"%e"`/`"%E"` & five `"%f"`/`"%F"` parameters, and for `*scanf`, up to five parameters of any format can be specified.
- Some expressions involving `*scanf` may behave differently in FLT. In particular, constructions like: `if (1 == scanf("%10f", &f)) { … }` will be converted to incorrect code. The `1 ==` is problematic so rather than trying to support this construction, suggest revising to something like: `if (scanf("%10f", &f) == 1) { … }`.
- Also note that FLT parameters in `*scanf` are handled as strings with reduced criteria with respect to what is valid or not. So a call like `sscanf("X Y Z", "%f %f %f", …);` may return 3 indicating three "matches". A workaround is to use `!isnan()` on each variable to confirm if it is valid.
- Similar to above, FLT parameters in `*printf` are also handled as strings, so padding is restricted to spaces for FLT values.
- The variadic functions `vprintf`, `vscanf`, and related are not supported.
- The approximations used in `atan` and `exp2` could display some accuracy issues near boundary conditions. This also includes these dependent functions: `asin`, `acos`, `atan2`, `exp`, `exp10`, `pow`, `sinh`, `cosh`, and `tanh`.

## Precision

The worst-case precisions for the default approximations, in decimal digits, are:

   |  fn  | digits |
   |------|--------|
   | sin  | 7.4<sup>†</sup> |
   | cos  | 7.4<sup>†</sup> |
   | atan | 7.1<sup>†</sup> |
   | exp2 | 6.9 |
   | log2 | 7.2<sup>†‡</sup> |

† The maximum precision for FLT would be expected to be 7.2 decimal digits. Note however the ULP is permitted to be inaccurate in standard floating point functions, so instead of compounding errors by comparing FLT values with those from `float` functions, they were compared with those from `double` functions. This is how precisions greater than 7.2 decimal digits were achieved. In summary, `sin` & `cos` are very good, `log2` is satisfactory, and `atan` is "good enough".

‡ The default `log2` uses a CORDIC routine which requires between zero to 60 floating point operations per invocation (average of 30). Building with `-DPOLY_LOG2` will use a polynomial approximation which requires 20 floating point operations, but accuracy drops to 6.6 decimal digits.

## License

This repository is governed by a dual license – one for individual users and another for commercial users. Please review the respective licenses in the repository for complete details. For commercial distribution rights, a separate Commercial License is required, which can be obtained by contacting the repository owner russell.s.harper@gmail.com.

Your use of this code is subject to these license terms.

---

If you have any questions and/or suggestions, would like to discuss commercial distribution rights, or if you find FLT useful for your project, feel free to contact:

Russell Harper

russell.s.harper@gmail.com

2023-10-16
