<?php
/*
    Contents: "FLT" PHP source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: russell.s.harper@gmail.com
*/
// Usage: php flt.php [-i input-file] [-o output-file] [-d 0|1] [-x extra-gcc-options]

define('FLT_VERSION', '1.0');	// Initial release

define('FLT_TYPE_REGEX', 'FLT');

define('INT_TYPE_REGEX', '(?:(?:unsigned|signed|char|short|int|long|size_t|u?int[0-9]+_t)\s*)+');

define('FLT_LITERAL_REGEX',
	'[0-9]+\.[0-9]+[Ee][-+]?[0-9]+[FfLl]?'
	.'|[0-9]+\.[Ee][-+]?[0-9]+[FfLl]?'
	.'|\.[0-9]+[Ee][-+]?[0-9]+[FfLl]?'
	.'|[0-9]+[Ee][-+]?[0-9]+[FfLl]?'
	.'|[0-9]+\.[0-9]+[FfLl]?'
	.'|\.[0-9]+[FfLl]?'
	.'|[0-9]+\.[FfLl]?'
	.'|\b[0-9]+[Ff]\b');

main($argc, $argv);

function main($argc, $argv) {
	// Retrieve the parameters
	$params = get_parameters($argc, $argv);
	// To keep track of how many passes
	$pass = 0;
	// Get the code
	$code = file_get_contents($params['-i']);
	// Preprocess to convert to FLT and clean up a bit
	$lines = preprocess($code, $pass, $params['-x']);
	// To keep track of any substitutions for postprocessing
	$substitutions = array();
	// Attempt a compile and process the errors until done
	do
		$done = compile($lines, $substitutions, ++$pass, $params['-d']);
	while (!$done && $pass < 50);
	// Final passes to set global and static initializers
	if ($done) {
		do
			$done = compile($lines, $substitutions, ++$pass, $params['-d'], true);
		while (!$done && $pass < 50);
	}
	if ($done) {
		// Postprocess to restore substitutions and convert assignments to literals
		$code = postprocess($lines, $substitutions);
		file_put_contents($params['-o'], $code);
		array_map('unlink', glob('__FLT_TMP_*'));
	} else
		file_put_contents('php://stderr', '*** ERROR: conversion failure ***'.PHP_EOL.
			'Ensure the input has no errors or warnings when compiled normally.'.PHP_EOL.
			'Rerun with debug enabled and review STDERR output.'.PHP_EOL.
			'Also review the generated __FLT_TMP_* files for more information.'.PHP_EOL, FILE_APPEND);
	// Done
	exit($done? 0: 1);
}

// Converts the $argc, $argv values to an associative array for easier lookup, expects key/value pairs as '-<character> <argument>'
function get_parameters($argc, $argv) {
	$params = array(
		'-i' => 'php://stdin',
		'-o' => 'php://stdout',
		'-d' => '0',
		'-x' => ''
	);
	for ($i = 1; $i < $argc; ++$i) {
		$working = $argv[$i];
		if (strlen($working) == 2 && $working[0] == '-') {
			if ($working != '-h') {
				if ($i + 1 < $argc)
					$params[$working] = $argv[++$i];
			} else {
				$base = basename(__FILE__);
				file_put_contents('php://stdout', $base.' v'.FLT_VERSION.' -- convert C code to use FLT and flt_* vs. float/double'.PHP_EOL.
					PHP_EOL.
					'Usage: php '.$base.' -h | [-i input] [-o output] [-d 0|1] [-x extra-gcc-options]'.PHP_EOL.
					"  -h\t\toutput this message to php://stdout and exit".PHP_EOL.
					"  -i FILE\tinput file/stream, default is php://stdin".PHP_EOL.
					"  -o FILE\toutput file/stream, default is php://stdout".PHP_EOL.
					"  -d 0|1\toutput debug info to php://stderr, default is 0".PHP_EOL.
					"  -x OPTIONS\textra options to pass to gcc".PHP_EOL.
					PHP_EOL.
					"Requirements:".PHP_EOL.
					"  - gcc v9.0 or later (v9.0+); uses the '-fdiagnostics-format=json' option".PHP_EOL.
					"  - PHP v7.0 or later (v7.0+)".PHP_EOL.
					"  - the target compiler, e.g. cc65, should support an unsigned 32-bit integer".PHP_EOL.
					"    type, e.g. uint32_t".PHP_EOL.
					"  - the source code should be ANSI C or later; we are NOT supporting K&R!".PHP_EOL.
					PHP_EOL.
					"Limitations:".PHP_EOL.
					"  - FLT corresponds to IEEE 754 single-precision floating point with one sign".PHP_EOL.
					"    bit, eight exponent bits, and 24 significand bits (23 explicitly stored).".PHP_EOL.
					"  - As well as single-precision floating point, double-precision floating point".PHP_EOL.
					"    literals, variables, and functions are also converted to FLT and flt_*. The".PHP_EOL.
					"    C standard only specifies that the type double provides at LEAST as much".PHP_EOL.
					"    precision as the type float, so this is permitted!".PHP_EOL.
					"  - The accuracy of FLT is NOT professional grade and should NOT be used in".PHP_EOL.
					"    mission critical applications where errors can have serious consequences!".PHP_EOL.
					"  - As well, FLT is NOT optimized for speed or space. It is basically a".PHP_EOL.
					"    temporary solution to provide floating point support in C compilers".PHP_EOL.
					"    currently lacking it.".PHP_EOL.
					"  - Currently gcc v9.0+ '-fdiagnostics-format=json' does not provide enough".PHP_EOL.
					"    information to parse certain constructions such as a cast spanning multiple".PHP_EOL.
					"    lines, or the scanf example below. It is recommended to thoroughly test the".PHP_EOL.
					"    programs after compiling to ensure correct functionality.".PHP_EOL.
					"  - I/O functions are limited in how many float parameters can be specified in".PHP_EOL.
					"    a single function call. For *printf, up to 15 \"%e\"/\"%E\" & five \"%f\"/\"%F\"".PHP_EOL.
					"    parameters, and for *scanf, up to five parameters of any format can be".PHP_EOL.
					"    specified.".PHP_EOL.
					"  - Some expressions involving *scanf may behave differently in FLT. In".PHP_EOL.
					"    particular, constructions like:".PHP_EOL.
					"        if (1 == scanf(\"%10f\", &f)) { ... }".PHP_EOL.
					"    will be converted to incorrect code. The \"1 ==\" is problematic so rather".PHP_EOL.
					"    than trying to support this construction, we recommend revising to something".PHP_EOL.
					"    like:".PHP_EOL.
					"        if (scanf(\"%10f\", &f) == 1) { ... }".PHP_EOL.
					"  - Also note that FLT parameters in *scanf are handled as strings with reduced".PHP_EOL.
					"    criteria with respect to what is valid or not. So a call like:".PHP_EOL.
					"        sscanf(\"X Y Z\", \"%f %f %f\", ...);".PHP_EOL.
					"    may return 3 indicating three \"matches\". A workaround is to use !isnan()".PHP_EOL.
					"    on each variable to confirm if it is valid.".PHP_EOL.
					"  - Similar to above, FLT parameters in *printf are also handled as strings, so".PHP_EOL.
					"    padding is restricted to spaces for FLT values.".PHP_EOL.
					"  - The variadic functions vprintf, vscanf, and related are not supported.".PHP_EOL.
					"  - Polynomial approximations are used for sin, cos, atan, exp2, and log2. Near".PHP_EOL.
					"    boundary conditions, these functions, as well as those dependent on them,".PHP_EOL.
					"    could display some accuracy issues.".PHP_EOL.
					PHP_EOL.
					"Examples:".PHP_EOL.
					"  php flt.php -i eg/paranoia.c -o eg/paranoia-flt.c -x '-DNOSIGNAL -DSingle'".PHP_EOL.
					"  php flt.php -i eg/averages.c -o eg/averages-flt.c -x '-I cc65/include'".PHP_EOL, FILE_APPEND);
				exit(1);
			}
		}
	}
	// Done
	return $params;
}

function preprocess($code, $pass, $extra) {
	// List of (mostly) functions to replace
	$fns = array(
		array('float', 'FLT'),		array('double', 'FLT'),			array('long FLT', 'long double'),
		array('DBL_MIN', 'FLT_MIN'),	array('DBL_TRUE_MIN', 'FLT_TRUE_MIN'),	array('DBL_MAX', 'FLT_MAX'),			array('DBL_EPSILON', 'FLT_EPSILON'),
		array('acosf?', 'flt_acos'),	array('acoshf?', 'flt_acosh'),		array('asinf?', 'flt_asin'),			array('asinhf?', 'flt_asinh'),
		array('atan2f?', 'flt_atan2'),	array('atanf?', 'flt_atan'),		array('atanhf?', 'flt_atanh'),			array('atoff?', 'flt_atof'),
		array('ceilf?', 'flt_ceil'),	array('cosf?', 'flt_cos'),		array('coshf?', 'flt_cosh'),			array('exp10f?', 'flt_exp10'),
		array('exp2f?', 'flt_exp2'),	array('expf?', 'flt_exp'),		array('fabsf?', 'flt_fabs'),			array('floorf?', 'flt_floor'),
		array('fmaxf?', 'flt_fmax'),	array('fminf?', 'flt_fmin'),		array('fmodf?', 'flt_fmod'),			array('frexpf?', 'flt_frexp'),
		array('fsgnf?', 'flt_fsgn'),	array('hypotf?', 'flt_hypot'),		array('isfinite', 'flt_isfinite'),		array('isinf', 'flt_isinf'),
		array('isnan', 'flt_isnan'),	array('isnormal', 'flt_isnormal'),	array('issubnormal', 'flt_issubnormal'),	array('iszero', 'flt_iszero'),
		array('ldexpf?', 'flt_ldexp'),	array('log10f?', 'flt_log10'),		array('log2f?', 'flt_log2'),			array('logf?', 'flt_log'),
		array('modff?', 'flt_modf'),	array('powf?', 'flt_pow'),		array('roundf?', 'flt_round'),			array('sinf?', 'flt_sin'),
		array('sinhf?', 'flt_sinh'),	array('sqrtf?', 'flt_sqrt'),		array('tanf?', 'flt_tan'),			array('tanhf?', 'flt_tanh'),
		array('truncf?', 'flt_trunc')
	);
	// First check if gcc is installed and is a supported version
	$version = floatval(shell_exec('which gcc > /dev/null && gcc -dumpfullversion -dumpversion'));
	if ($version < 9.0) {
		$base = basename(__FILE__);
		if ($version)
			file_put_contents('php://stderr', '*** ERROR: gcc version is not supported ***'.PHP_EOL.
				$base.' requires gcc v9.0 or later to be installed and accessible.'.PHP_EOL.
				'Currently installed is gcc v'.$version.'.'.PHP_EOL, FILE_APPEND);
		else
			file_put_contents('php://stderr', '*** ERROR: gcc is not available ***'.PHP_EOL.
				$base.' requires gcc v9.0 or later to be installed and accessible.'.PHP_EOL, FILE_APPEND);
		// Not installed or not supported!
		exit(1);
	}
	// Comment out #include <math.h> and #include <float.h>!
	// The header file flt-parse.h also has code to prevent their inclusion.
	$code = preg_replace('/(#\s*include\s*["<](?:math|float)\.h[">])/', '/* \1 */', $code);
	// Normalize
	$pass = substr('0'.$pass, -2);
	// Files
	$i_file = '__FLT_TMP_'.$pass.'.c';
	$o_file = '__FLT_TMP_'.$pass.'.i';
	$e_file = '__FLT_TMP_'.$pass.'.txt';
	// Preprocess with gcc
	file_put_contents($i_file, $code);
	shell_exec('gcc -fdiagnostics-format=text -E -I. -include flt-parse.h -include flt.h '.$extra.' '.$i_file.' > '.$o_file.' 2> '.$e_file);
	$code = file_get_contents($o_file);
	// Convert backslash and double quote escape sequences
	$code = str_replace(array('\\\\', '\\"'), array('\x5c', '\x22'), $code);
	// Temporarily hide all strings
	preg_match_all('/"[^"]*"/', $code, $matches);
	foreach ($matches[0] as $index => $match) {
		$key = '__FLT_STR_'.substr('00000000'.$index, -9);
		$code = str_replace($match, $key, $code);
	}
	// Replace existing functions
	foreach ($fns as $fn)
		$code = preg_replace('/\b'.$fn[0].'\b/', $fn[1], $code);
	// Substitute floating point literals
	$code = preg_replace('/('.FLT_LITERAL_REGEX.')/', 'flt_atof("\1")', $code);
	// Remove unnecessary FLT casts
	$code = preg_replace('/\(\s*'.FLT_TYPE_REGEX.'\s*\)\s*flt_/', 'flt_', $code);
	// Restore the strings
	foreach ($matches[0] as $index => $match) {
		$key = '__FLT_STR_'.substr('00000000'.$index, -9);
		$code = str_replace($key, $match, $code);
	}
	// Remove any blank lines and directives
	$lines = array_values(array_filter(explode(PHP_EOL, $code), function($line) { return strlen($line) && $line[0] != '#'; }));
	// Done, return an array of lines
	return $lines;
}

function compile(&$lines, &$substitutions, $pass, $debug, $final_passes = false) {
	// Normalize
	$pass = substr('0'.$pass, -2);
	// To keep track of which lines were modified
	$modified = array();
	// Do a compile and get the messages
	$i_file = '__FLT_TMP_'.$pass.'.c';
	$e_file = '__FLT_TMP_'.$pass.'.json';
	file_put_contents($i_file, implode(PHP_EOL, $lines));
	shell_exec('gcc -fdiagnostics-format=json -c '.$i_file.' 2> '.$e_file);
	$messages = json_decode(file_get_contents($e_file));
	// Join any multiline messages
	join_multiline_messages($lines, $messages, $modified);
	// Continue processing
	foreach ($messages as $message) {
		if ($debug) {
			foreach ($message->locations as $location) {
				list($l, $start, $finish) = get_token_extent($location);
				if (!in_array($l, $modified))
					file_put_contents('php://stderr', 'line '.$l.' - '.$message->message.PHP_EOL
						.$lines[$l].PHP_EOL
						.str_repeat('.', $start).str_repeat('*', $finish - $start + 1).PHP_EOL, FILE_APPEND);
			}
		}
		switch ($message->message) {
			case "initializer element is not constant":
				if ($final_passes)
					process_initializer($lines, $message, $modified);
				break;
			case "invalid initializer":
				if (!$final_passes)
					process_unhandled($message);
				break;
			case "two or more data types in declaration specifiers":
				process_non_gcc_extension($lines, $substitutions, $message, $modified);
				break;
			case "wrong type argument to unary minus":
				process_unary_minus($lines, $message, $modified);
				break;
			case "wrong type argument to unary plus":
				process_unary_plus($lines, $message, $modified);
				break;
			case "wrong type argument to increment":
				process_increment_or_decrement($lines, $message, $modified, 'increment');
				break;
			case "wrong type argument to decrement":
				process_increment_or_decrement($lines, $message, $modified, 'decrement');
				break;
			case "conversion to non-scalar type requested":
				process_cast($lines, $message, $modified, FLT_TYPE_REGEX);
				break;
			case "aggregate value used where an integer was expected":
				process_cast($lines, $message, $modified, INT_TYPE_REGEX);
				break;
			case "expected declaration specifiers or ‘...’ before ‘*’ token":
				process_non_gcc_extension($lines, $substitutions, $message, $modified);
				break;
			// Ignoring these cc65 vs. gcc incompatibilities
			case "assignment to ‘FILE *’ {aka ‘struct _FILE *’} from ‘int’ makes pointer from integer without a cast":
			case "cast from pointer to integer of different size":
			case "useless type name in empty declaration":
			case "‘fastcall’ attribute ignored";
				if ($debug)
					process_ignored($message);
				break;
			default:
				if (preg_match("/incompatible types when assigning to type ‘".INT_TYPE_REGEX."’ from type ‘FLT’/", $message->message))
					process_assignment($lines, $message, $modified, 'flt_ftol');
				else if (preg_match("/incompatible types when assigning to type ‘[_a-zA-Z0-9]+’ \{aka ‘".INT_TYPE_REGEX."’\} from type ‘FLT’/", $message->message))
					process_assignment($lines, $message, $modified, 'flt_ftol');
				else if (preg_match("/incompatible types when assigning to type ‘FLT’ from type ‘".INT_TYPE_REGEX."’/", $message->message))
					process_assignment($lines, $message, $modified, 'flt_ltof');
				else if (preg_match("/incompatible types when assigning to type ‘FLT’ from type ‘[_a-zA-Z0-9]+’ \{aka ‘".INT_TYPE_REGEX."’\}/", $message->message))
					process_assignment($lines, $message, $modified, 'flt_ltof');
				else if (preg_match("/invalid operands to binary [-\+\*\/]=? \(have ‘FLT’ and ‘FLT’\)/", $message->message))
					process_arithmetic_binary_operands($lines, $message, $modified);
				else if (preg_match("/invalid operands to binary (?:[<>]=?|[=!]=) \(have ‘FLT’ and ‘FLT’\)/", $message->message))
					process_comparison_binary_operands($lines, $message, $modified);
				else if (preg_match("/invalid operands to binary [-\+\*\/<>=!]+ \(have ‘".INT_TYPE_REGEX."’ and ‘FLT’\)/", $message->message))
					process_first_operand($lines, $message, $modified);
				else if (preg_match("/invalid operands to binary [-\+\*\/<>=!]+ \(have ‘[_a-zA-Z0-9]+’ \{aka ‘".INT_TYPE_REGEX."’\} and ‘FLT’\)/", $message->message))
					process_first_operand($lines, $message, $modified);
				else if (preg_match("/invalid operands to binary [-\+\*\/<>=!]+ \(have ‘struct <anonymous>’ and ‘FLT’\)/", $message->message))
					process_cast($lines, $message, $modified, FLT_TYPE_REGEX);
				else if (preg_match("/invalid operands to binary [-\+\*\/<>=!]+ \(have ‘FLT’ and ‘".INT_TYPE_REGEX."’\)/", $message->message))
					process_second_operand($lines, $message, $modified);
				else if (preg_match("/invalid operands to binary [-\+\*\/<>=!]+ \(have ‘FLT’ and ‘[_a-zA-Z0-9]+’ \{aka ‘".INT_TYPE_REGEX."’\}\)/", $message->message))
					process_second_operand($lines, $message, $modified);
				else if (preg_match("/invalid operands to binary [-\+\*\/<>=!]+ \(have ‘FLT’ and ‘struct <anonymous>’\)/", $message->message))
					process_cast($lines, $message, $modified, FLT_TYPE_REGEX);
				else if (preg_match("/incompatible type for argument [0-9]+ of ‘flt_[^’]+’/", $message->message))
					process_incompatible_type($lines, $message, $modified);
				else if (preg_match("/format ‘%l?[EeFfGg]’ expects argument of type ‘(?:float|double)’, but argument [0-9]+ has type ‘FLT’/", $message->message))
					process_printf_argument($lines, $message, $modified);
				else if (preg_match("/format ‘%l?[EeFfGg]’ expects argument of type ‘(?:float|double) \*’, but argument [0-9]+ has type ‘FLT \*’/", $message->message))
					process_scanf_argument($lines, $message, $modified);
				else if (preg_match("/expected ‘=’, ‘,’, ‘;’, ‘asm’ or ‘__attribute__’ before ‘[^’]+’/", $message->message))
					process_non_gcc_extension($lines, $substitutions, $message, $modified);
				else if (preg_match("/conflicting types for built-in function ‘[^’]+’; expected ‘[^’]+’/", $message->message))
					process_conflicting_types($lines, $substitutions, $message, $modified);
				else if (preg_match("/(?:incompatible)? *implicit declaration of (?:built-in)? *function ‘[^’]+’/", $message->message) && $message->kind != 'error') {
					if ($debug)
						process_ignored($message);
				} else if (preg_match("/unknown type name ‘[^’]+’/", $message->message)) {
					if ($debug)
						process_ignored($message);
				} else if ($message->kind == 'note') {
					if ($debug)
						process_ignored($message);
				}
				else
					process_unhandled($message);
				break;
		}
	}
	// Tidy up
	if ($modified)
		$lines = array_values(array_filter($lines, function($line) { return strlen($line); }));
	// Done if no longer modified
	return !$modified;
}

function postprocess($lines, $substitutions) {
	// Get the code back
	$code = implode(PHP_EOL, $lines);
	// Correct the typedef for FLT
	$code = preg_replace(
		array(
			'/typedef struct \{short h1, h2;\} FLT;/',
			'/(typedef enum \{E_INFINITE = 1, E_NAN, E_NORMAL, E_SUBNORMAL, E_ZERO\} E_CLASS;)/'
		),array(
			'/* Converted to FLT using '.basename(__FILE__).' v'.FLT_VERSION.' on '.gmdate('c').' */',
			'typedef uint32_t FLT;'.PHP_EOL.'\1'
		), $code);
	// Convert flt_atof("###"), flt_ltof(###), and flt_ultof(###) to compatible literals
	$code = preg_replace_callback(
		array(
			'/flt_atof\(\s*"([^"]+)"\s*\)/',
			'/flt_u?ltof\(\s*([-+]?[0-9]+)\s*\)/'
		),
		function ($matches) {
			return '0x'.strtoupper(bin2hex(pack('G', floatval($matches[1])))).' /* '.$matches[1].' */';
		}, $code);
	// Restore substitutions
	foreach (array_reverse($substitutions) as $substitution)
		$code = str_replace($substitution[0], $substitution[1], $code);
	return $code;
}

function get_token_extent($location) {
	$line = 0;
	$min = +getrandmax();
	$max = -$min;
	foreach (array('caret', 'start', 'finish') as $property) {
		if (property_exists($location, $property)) {
			$t = $location->{$property};
			$line = $t->line - 1;
			$c = $t->{'byte-column'} - 1;
			if ($min > $c)
				$min = $c;
			if ($max < $c)
				$max = $c;
		}
	}
	return array($line, $min, $max);
}

// Can get simple tokens, e.g. numbers and variables
function get_previous_token($input, $allow = '') {
	$allow = str_split($allow);
	$token = '';
	$input = trim($input);
	for ($i = strlen($input) - 1; $i >= 0 && (preg_match('/[A-Za-z0-9_]/', $input[$i]) || in_array($input[$i], $allow, true)); --$i)
		$token = $input[$i].$token;
	return $token;
}

// Like explode($separator, $string, $limit = 2) but from the right, e.g. SPT('-', 'a-b-c-d') => ['a-b-c', 'd']
function split_previous_token($token, $input) {
	$working = explode($token, $input);
	$last = array_pop($working);
	return array(implode($token, $working), $last);
}

// Can get simple tokens and flt_* expressions
function get_next_token($input, $fn = 'flt_') {
	$token = '';
	$input = trim($input);
	for ($i = 0, $limit = strlen($input); $i < $limit && preg_match('/[A-Za-z0-9_]/', $input[$i]); ++$i)
		$token .= $input[$i];
	// Function?
	if (strpos($token, $fn) === 0 || ($i < $limit && $input[$i] == '(')) {
		// Append any whitespace
		for ( ; $i < $limit && preg_match('/\s/', $input[$i]); ++$i)
			$token .= $input[$i];
		// Append the function parameters, including any inner function calls
		if ($i < $limit && $input[$i] == '(') {
			for ($level = 0; $i < $limit; ++$i) {
				$token .= $input[$i];
				if ($input[$i] == '(')
					++$level;
				else if ($input[$i] == ')')
					--$level;
				if (!$level)
					break;
			}
			// Didn't find the end, so clear the token
			if ($level)
				$token = '';
		// Didn't find the start, so clear the token
		} else
			$token = '';
	}
	return $token;
}

function merge_next_line(&$lines, &$modified, $i) {
	$j = $i + 1;
	$lines[$i] .= ' '.trim($lines[$j]);
	$modified[] = $i;
	$lines[$j] = '';
	$modified[] = $j;
}

function merge_previous_line(&$lines, &$modified, $i) {
	$j = $i - 1;
	$lines[$j] .= ' '.trim($lines[$i]);
	$modified[] = $j;
	$lines[$i] = '';
	$modified[] = $i;
}

function join_multiline_messages(&$lines, $messages, &$modified) {
	foreach ($messages as $message) {
		$min = +getrandmax();
		$max = -$min;
		foreach ($message->locations as $location) {
			foreach (array('caret', 'start', 'finish') as $property) {
				if (property_exists($location, $property)) {
					$t = $location->{$property};
					$l = $t->line - 1;
					if ($min > $l) $min = $l;
					if ($max < $l) $max = $l;
				}
			}
		}
		// Multiline?
		if ($min < $max) {
			$modified[] = $min;
			for ($l = $min + 1; $l <= $max; ++$l) {
				$lines[$min] .= ' '.trim($lines[$l]);
				$lines[$l] = '';
				$modified[] = $l;
			}
		}
	}
}

// To use in process_initializer
function flt_atof($a) { return floatval($a); }
function flt_ltof($a) { return floatval($a); }
function flt_ultof($a) { return floatval($a); }

function flt_add($a, $b) { return $a + $b; }
function flt_subtract($a, $b) { return $a - $b; }
function flt_multiply($a, $b) { return $a * $b ; }
function flt_divide($a, $b) { return $a / $b; }
function flt_negated($a) { return -$a; }

function process_initializer(&$lines, $message, &$modified) {
	if (count($message->locations) == 1) {
		list($l, $start, $finish) = get_token_extent($message->locations[0]);
		if (!in_array($l, $modified)) {
			$line = $lines[$l];
			$p = substr($line, $start);
			// Evaluate the initializer if possible
			if (($token = get_next_token($p)) != '') {
				try {
					if (is_numeric($result = eval('return '.$token.';'))) {
						$lines[$l] = str_replace($token, '0x'.strtoupper(bin2hex(pack('G', $result))).' /* '.sprintf('%g', $result).' */', $line);
						$modified[] = $l;
					}
				} catch (Throwable $e) {
    					process_unhandled($message);
				}
			} else
				// Can find anything? Might be on the next line.
				merge_next_line($lines, $modified, $l);
		}
	} else
		process_unhandled($message);
}

function process_arithmetic_binary_operands(&$lines, $message, &$modified) {
	$operators = array(
		'+' => 'flt_add',	'-' => 'flt_subtract',		'*' => 'flt_multiply',		'/' => 'flt_divide',
		'+=' => 'flt_add_into',	'-=' => 'flt_subtract_into',	'*=' => 'flt_multiply_into',	'/=' => 'flt_divide_into'
	);
	$o = $p = $q = -1;
	switch (count($message->locations)) {
		case 3:
			list($q, $q_start, $q_finish) = get_token_extent($message->locations[2]);
		case 2:
			list($p, $p_start, $p_finish) = get_token_extent($message->locations[1]);
		case 1:
			list($o, $o_start, $o_finish) = get_token_extent($message->locations[0]);
			if (!in_array($o, $modified)) {
				$op = substr($lines[$o], $o_start, $o_finish - $o_start + 1);
				$fn = $operators[$op];
			}
	}
	// O is the operator, P is the left operand, Q is the right operand
	if ($o >= 0 && $p >= 0 && $q >= 0) {
		if (!in_array($o, $modified) && !in_array($p, $modified) && !in_array($q, $modified)) {
			// Work in reverse right (Q) - operator (O) - left (P), so that the indices are still valid
			$line = $lines[$q];
			$lines[$q] = substr($line, 0, $q_finish + 1).')'.substr($line, $q_finish + 1);
			$modified[] = $q;
			$line = $lines[$o];
			$lines[$o] = substr($line, 0, $o_start).','.substr($line, $o_finish + 1);
			$modified[] = $o;
			$line = $lines[$p];
			$lines[$p] = substr($line, 0, $p_start).$fn.'('.(strlen($op) == 2? '&': '').substr($line, $p_start);
			$modified[] = $p;
		}
	// O is the operator, P is the left or right operand
	} else if ($o >= 0 && $p >= 0) {
		if (!in_array($o, $modified) && !in_array($p, $modified)) {
			// Have the right operand, need the left, expected to be simple
			if ($p > $o || $p_start > $o_start) {
				$left = get_previous_token(substr($lines[$o], 0, $o_start));
				if ($left != '') {
					// Work in reverse right (P) - operator (O) - left, so that the indices are still valid
					$line = $lines[$p];
					$lines[$p] = substr($line, 0, $p_finish + 1).')'.substr($line, $p_finish + 1);
					$modified[] = $p;
					$line = $lines[$o];
					$working = split_previous_token($left, substr($line, 0, $o_start));
					$lines[$o] = $working[0].$fn.'('.(strlen($op) == 2? '&': '').$left.$working[1].','.substr($line, $o_finish + 1);
					$modified[] = $o;
				} else
					// Can't find the left operand, might be on the previous line
					merge_previous_line($lines, $modified, $o);
			// Have the left operand, need the right, expected to be simple
			} else if ($p < $o || $p_start < $o_start) {
				$right = get_next_token(substr($lines[$o], $o_finish + 1));
				if ($right != '') {
					// Work in reverse right - operator (O) - left (P), so that the indices are still valid
					$line = $lines[$o];
					$working = explode($right, substr($line, $o_finish + 1), 2);
					$lines[$o] = substr($line, 0, $o_start).','.$working[0].$right.')'.$working[1];
					$modified[] = $o;
					$line = $lines[$p];
					$lines[$p] = substr($line, 0, $p_start).$fn.'('.(strlen($op) == 2? '&': '').substr($line, $p_start);
					$modified[] = $p;
				} else
					// Can't find the right operand, might be on the next line
					merge_next_line($lines, $modified, $o);
			} else
				process_unhandled($message);
		}
	// O is the operator, both operands are expected to be simple
	} else if ($o >= 0) {
		if (!in_array($o, $modified)) {
			$line = $lines[$o];
			$p1 = substr($line, 0, $o_start);
			$left = get_previous_token($p1);
			$p2 = substr($line, $o_finish + 1);
			$right = get_next_token($p2);
			if ($left != '' && $right != '') {
					$working = split_previous_token($left, $p1);
					$p1 = $working[0].$fn.'('.(strlen($op) == 2? '&': '').$left.$working[1].',';
					$working = explode($right, $p2, 2);
					$p2 = $working[0].$right.')'.$working[1];
					$lines[$o] = $p1.$p2;
					$modified[] = $o;
			} else {
				// Can't find all the operands, might be on the next or the previous line
				if ($right == '')
					merge_next_line($lines, $modified, $o);
				if ($left == '')
					merge_previous_line($lines, $modified, $o);
			}
		}
	} else
		process_unhandled($message);
}

function process_comparison_binary_operands(&$lines, $message, &$modified) {
	$comparisons = array(
		'==' => 'E_EQUAL_TO',
		'<' => 'E_LESS_THAN',
		'>' => 'E_GREATER_THAN',
		'<=' => 'E_LESS_THAN_OR_EQUAL_TO',
		'>=' => 'E_GREATER_THAN_OR_EQUAL_TO',
		'!=' => 'E_NOT_EQUAL_TO'
	);
	$o = $p = $q = -1;
	switch (count($message->locations)) {
		case 3:
			list($q, $q_start, $q_finish) = get_token_extent($message->locations[2]);
		case 2:
			list($p, $p_start, $p_finish) = get_token_extent($message->locations[1]);
		case 1:
			list($o, $o_start, $o_finish) = get_token_extent($message->locations[0]);
			if (!in_array($o, $modified)) {
				$op = substr($lines[$o], $o_start, $o_finish - $o_start + 1);
				$cmp = $comparisons[$op];
			}
	}
	// O is the operator, P is the left operand, Q is the right operand
	if ($o >= 0 && $p >= 0 && $q >= 0) {
		if (!in_array($o, $modified) && !in_array($p, $modified) && !in_array($q, $modified)) {
			// Work in reverse right (Q) - operator (O) - left (P), so that the indices are still valid
			$line = $lines[$q];
			$lines[$q] = substr($line, 0, $q_finish + 1).','.$cmp.')'.substr($line, $q_finish + 1);
			$modified[] = $q;
			$line = $lines[$o];
			$lines[$o] = substr($line, 0, $o_start).','.substr($line, $o_finish + 1);
			$modified[] = $o;
			$line = $lines[$p];
			$lines[$p] = substr($line, 0, $p_start).'flt_compare('.substr($line, $p_start);
			$modified[] = $p;
		}
	// O is the operator, P is the left or right operand
	} else if ($o >= 0 && $p >= 0) {
		if (!in_array($o, $modified) && !in_array($p, $modified)) {
			// Have the right operand, need the left, expected to be simple
			if ($p > $o || $p_start > $o_start) {
				$left = get_previous_token(substr($lines[$o], 0, $o_start));
				if ($left != '') {
					// Work in reverse right (P) - operator (O) - left, so that the indices are still valid
					$line = $lines[$p];
					$lines[$p] = substr($line, 0, $p_finish + 1).','.$cmp.')'.substr($line, $p_finish + 1);
					$modified[] = $p;
					$line = $lines[$o];
					$working = split_previous_token($left, substr($line, 0, $o_start));
					$lines[$o] = $working[0].'flt_compare('.$left.$working[1].','.substr($line, $o_finish + 1);
					$modified[] = $o;
				} else
					// Can't find the left operand, might be on the previous line
					merge_previous_line($lines, $modified, $o);
			// Have the left operand, need the right, expected to be simple
			} else if ($p < $o || $p_start < $o_start) {
				$right = get_next_token(substr($lines[$o], $o_finish + 1));
				if ($right != '') {
					// Work in reverse right - operator (O) - left (P), so that the indices are still valid
					$line = $lines[$o];
					$working = explode($right, substr($line, $o_finish + 1), 2);
					$lines[$o] = substr($line, 0, $o_start).','.$working[0].$right.','.$cmp.')'.$working[1];
					$modified[] = $o;
					$line = $lines[$p];
					$lines[$p] = substr($line, 0, $p_start).'flt_compare('.substr($line, $p_start);
					$modified[] = $p;
				} else
					// Can't find the right operand, might be on the next line
					merge_next_line($lines, $modified, $o);
			} else
				process_unhandled($message);
		}
	// O is the operator, both operands are expected to be simple
	} else if ($o >= 0) {
		if (!in_array($o, $modified)) {
			$line = $lines[$o];
			$p1 = substr($line, 0, $o_start);
			$left = get_previous_token($p1);
			$p2 = substr($line, $o_finish + 1);
			$right = get_next_token($p2);
			if ($left != '' && $right != '') {
					$working = split_previous_token($left, $p1);
					$p1 = $working[0].'flt_compare('.$left.$working[1].',';
					$working = explode($right, $p2, 2);
					$p2 = $working[0].$right.','.$cmp.')'.$working[1];
					$lines[$o] = $p1.$p2;
					$modified[] = $o;
			} else {
				// Can't find all the operands, might be on the next or the previous line
				if ($right == '')
					merge_next_line($lines, $modified, $o);
				if ($left == '')
					merge_previous_line($lines, $modified, $o);
			}
		}
	} else
		process_unhandled($message);
}

function process_unary_minus(&$lines, $message, &$modified) {
	if (count($message->locations) == 1) {
		list($l, $start, $finish) = get_token_extent($message->locations[0]);
		if (!in_array($l, $modified)) {
			$line = $lines[$l];
			$p1 = substr($line, 0, $start);
			$p2 = substr($line, $finish + 1);
			// Update a literal
			if (substr(trim($p2), 0, 10) == 'flt_atof("') {
				$working = explode('flt_atof("', $p2, 2);
				$p2 = $working[0].'flt_atof("-'.$working[1];
				$lines[$l] = $p1.$p2;
				$modified[] = $l;
			// Update an expression
			} else if (($token = get_next_token($p2)) != '') {
				$working = explode($token, $p2, 2);
				$p2 = $working[0].'flt_negated('.$token.')'.$working[1];
				$lines[$l] = $p1.$p2;
				$modified[] = $l;
			} else
				// Can find anything? Might be on the next line.
				merge_next_line($lines, $modified, $l);
		}
	} else
		process_unhandled($message);
}

function process_unary_plus(&$lines, $message, &$modified) {
	if (count($message->locations) == 1) {
		// Just get rid of the plus sign!
		list($l, $start, $finish) = get_token_extent($message->locations[0]);
		if (!in_array($l, $modified)) {
			$line = $lines[$l];
			$lines[$l] = substr($line, 0, $start).substr($line, $finish + 1);
			$modified[] = $l;
		}
	} else
		process_unhandled($message);
}

function process_increment_or_decrement(&$lines, $message, &$modified, $op) {
	if (count($message->locations) == 1) {
		list($l, $start, $finish) = get_token_extent($message->locations[0]);
		if (!in_array($l, $modified)) {
			$line = $lines[$l];
			$p1 = substr($line, 0, $start);
			$p2 = substr($line, $finish + 1);
			if (($token = get_previous_token($p1)) != '') {
				$working = split_previous_token($token, $p1);
				$p1 = $working[0].'flt_post_'.$op.'(&'.$token.')'.$working[1];
				$lines[$l] = $p1.$p2;
				$modified[] = $l;
			} else if (($token = get_next_token($p2)) != '') {
				$working = explode($token, $p2, 2);
				$p2 = $working[0].'flt_pre_'.$op.'(&'.$token.')'.$working[1];
				$lines[$l] = $p1.$p2;
				$modified[] = $l;
			} else
			// Didn't find anything? Likely across multiple lines, gcc doesn't give enough info to address this case
				process_unhandled($message);
		}
	} else
		process_unhandled($message);
}

function process_cast(&$lines, $message, &$modified, $regex) {
	foreach ($message->locations as $location) {
		// Remove the cast!
		list($l, $start, $finish) = get_token_extent($location);
		if (!in_array($l, $modified)) {
			$line = $lines[$l];
			$count = 0;
			$lines[$l] = preg_replace('/\(\s*'.$regex.'\s*\)/', '', $line, 1, $count);
			if ($count)
				$modified[] = $l;
			// Didn't find anything? Likely a cast across multiple lines, gcc doesn't give enough info to address this case
			else
				process_unhandled($message);
		}
	}
}

function process_incompatible_type(&$lines, $message, &$modified) {
	if (count($message->locations) == 1) {
		list($l, $start, $finish) = get_token_extent($message->locations[0]);
		if (!in_array($l, $modified))
			process_complex_operand($lines, $modified, $l, $start, $finish);
	} else
		process_unhandled($message);
}

function process_printf_argument(&$lines, $message, &$modified) {
	if (count($message->locations) == 2) {
		list($l, $f_start, $f_finish) = get_token_extent($message->locations[0]);
		list($l, $s_start, $s_finish) = get_token_extent($message->locations[1]);
		if (!in_array($l, $modified)) {
			// Set up the formats, precision to no more than nine places
			list($left, $padding, $width, $signed, $precision, $specifier)
				= array_values(parse_format(substr($lines[$l], $f_start, $f_finish - $f_start + 1)));
			$fmt1 = '%'.$left.$padding.$width.'s';
			$fmt2 = '%'.$signed.((strlen($precision) > 2)? '.9': $precision).$specifier;
			$line = $lines[$l];
			$src = substr($line, $s_start, $s_finish - $s_start + 1);
			$p1 = substr($line, 0, $f_start);
			$p2 = substr($line, $f_finish + 1, $s_start - $f_finish - 1);
			$p3 = substr($line, $s_finish + 1);
			$lines[$l] = $p1.$fmt1.$p2.'flt_ftoa('.$src.',"'.$fmt2.'")'.$p3;
			$modified[] = $l;
		}
	} else
		process_unhandled($message);
}

function process_scanf_argument(&$lines, $message, &$modified) {
	// Will need these
	$scn = 'scanf';
	$gsb = 'flt_get_scanf_buffer';
	$lsr = 'g_flt_last_scanf_result';
	if (count($message->locations) == 2) {
		list($l, $f_start, $f_finish) = get_token_extent($message->locations[0]);
		list($l, $d_start, $d_finish) = get_token_extent($message->locations[1]);
		if (!in_array($l, $modified)) {
			$line = $lines[$l];
			// Check if scanf is on the same line
			if (is_numeric($scanf = strrpos(substr($line, 0, $f_start), $scn))) {
				// Check for the end of the expression
				if ($expression = get_next_token(substr($line, $scanf), $scn)) {
					// Set up the format, max and default of 50 characters
					$parsed = parse_format(substr($lines[$l], $f_start, $f_finish - $f_start + 1));
					$width = $parsed['wdt'];
					if (!strlen($width) || intval($width) > 50)
						$width = '50';
					$fmt = '%'.$width.'s';
					// Get how many existing scanf parameters
					$count = substr_count($expression, $gsb) + 1;
					// Destination
					$dst = substr($line, $d_start, $d_finish - $d_start + 1);
					$dst = ($dst[0] == '&')? substr($dst, 1): '*('.$dst.')';
					$p1 = substr($line, 0, $f_start);
					$p2 = substr($line, $f_finish + 1, $d_start - $f_finish - 1);
					$p3 = substr($line, $d_finish + 1);
					$line = $p1.$fmt.$p2.$gsb.'(-'.$count.')'.$p3;
					// Get the new expression, work backwards to include *scanf and any "!" operations
					$expression = $scanf? get_previous_token(substr($line, 0, $scanf), " \t!"): '';
					$expression .= get_next_token(substr($line, $scanf), $scn);
					$working = explode($expression, $line, 2);
					$lines[$l] = $working[0].($count == 1? $lsr.'=': '').$expression.','.$dst.'=flt_atof('.$gsb.'('.$count.'))'.($count == 1? ','.$lsr: '').$working[1];
					$modified[] = $l;
				} else
					// Couldn't find end of scanf call, might be on the next line
					merge_next_line($lines, $modified, $l);
			} else
				// Couldn't find scanf, might be on the previous line
				merge_previous_line($lines, $modified, $l);
		}
	} else
		process_unhandled($message);
}

function parse_format($format) {
	for ($i = 0, $j = strlen($format), $mode = 1; $i < $j; ++$i) {
		switch ($char = $format[$i]) {
			case '%':
				$left = $padding = $width = $signed = $precision = $specifier = '';
				break;
			case '-':
				$left .= $char;
				break;
			case '+':
				$signed .= $char;
				break;
			case ' ':
				$padding .= '_'.$char;
				break;
			case '0':
				switch ($mode) {
					case 1:
						$padding .= $char;
						break;
					case 2:
						$width .= $char;
						break;
					case 3:
						$precision .= $char;
						break;
				}
				break;
			case '1': case '2': case '3':
			case '4': case '5': case '6':
			case '7': case '8': case '9':
			case '*':
				switch ($mode) {
					case 1:
						$mode = 2;
					case 2:
						$width .= $char;
						break;
					case 3:
						$precision .= $char;
						break;
				}
				break;
			case '.':
				$mode = 3;
				$precision .= $char;
				break;
			case 'E': case 'e':
			case 'F': case 'f': 
			case 'G': case 'g': 
			case 'l': case 'h': 
				$mode = 3;
				$specifier .= $char;
				break;
		}
	}			
	return array('lft' => $left, 'pdd' => $padding, 'wdt' => $width, 'sgn' => $signed, 'prc' => $precision, 'spc' => $specifier);
}

function process_assignment(&$lines, $message, &$modified, $fn) {
	if (count($message->locations) == 1) {
		list($l, $start, $finish) = get_token_extent($message->locations[0]);
		if (!in_array($l, $modified)) {
			$line = $lines[$l];
			$p1 = substr($line, 0, $start);
			$p2 = substr($line, $start);
			// Update a token
			if (($token = get_next_token($p2)) != '') {
				$working = explode($token, $p2, 2);
				$p2 = $working[0].$fn.'('.$token.')'.$working[1];
				$lines[$l] = $p1.$p2;
				$modified[] = $l;
			} else
				// Can find anything? Might be on the next line.
				merge_next_line($lines, $modified, $l);
		}
	} else
		process_unhandled($message);
}

function process_first_operand(&$lines, $message, &$modified) {
	$o = $p = $q = -1;
	switch (count($message->locations)) {
		case 3:
			list($q, $q_start, $q_finish) = get_token_extent($message->locations[2]);
		case 2:
			list($p, $p_start, $p_finish) = get_token_extent($message->locations[1]);
		case 1:
			list($o, $o_start, $o_finish) = get_token_extent($message->locations[0]);
	}
	// O is the operator, P is the left operand, Q is the right operand
	if ($o >= 0 && $p >= 0 && $q >= 0) {
		if (!in_array($p, $modified))
			process_complex_operand($lines, $modified, $p, $p_start, $p_finish);
	// O is the operator, P is the left or right operand
	} else if ($o >= 0 && $p >= 0) {
		if (!in_array($o, $modified) && !in_array($p, $modified)) {
			// Have the left operand
			if ($p < $o || $p_start < $o_start)
				process_complex_operand($lines, $modified, $p, $p_start, $p_finish);
			// Have the right operand, need the left, expected to be simple
			else if ($p > $o || $p_start > $o_start)
				process_simple_left_operand($lines, $modified, $o, $o_start);
			else
				process_unhandled($message);
		}
	// O is the operator, left operand is expected to be simple
	} else if ($o >= 0) {
		if (!in_array($o, $modified))
			process_simple_left_operand($lines, $modified, $o, $o_start);
	} else
		process_unhandled($message);
}

function process_second_operand(&$lines, $message, &$modified) {
	$o = $p = $q = -1;
	switch (count($message->locations)) {
		case 3:
			list($q, $q_start, $q_finish) = get_token_extent($message->locations[2]);
		case 2:
			list($p, $p_start, $p_finish) = get_token_extent($message->locations[1]);
		case 1:
			list($o, $o_start, $o_finish) = get_token_extent($message->locations[0]);
	}
	// O is the operator, P is the left operand, Q is the right operand
	if ($o >= 0 && $p >= 0 && $q >= 0) {
		if (!in_array($q, $modified))
			process_complex_operand($lines, $modified, $q, $q_start, $q_finish);
	// O is the operator, P is the left or right operand
	} else if ($o >= 0 && $p >= 0) {
		if (!in_array($o, $modified) && !in_array($p, $modified)) {
			// Have the right operand
			if ($p > $o || $p_start > $o_start)
				process_complex_operand($lines, $modified, $p, $p_start, $p_finish);
			// Have the left operand, need the right, expected to be simple
			else if ($p < $o || $p_start < $o_start)
				process_simple_right_operand($lines, $modified, $o, $o_finish);
			else
				process_unhandled($message);
		}
	// O is the operator, right operand is expected to be simple
	} else if ($o >= 0) {
		if (!in_array($o, $modified))
			process_simple_right_operand($lines, $modified, $o, $o_finish);
	} else
		process_unhandled($message);
}

function process_simple_left_operand(&$lines, &$modified, $o, $o_start) {
	$line = $lines[$o];
	$p1 = substr($line, 0, $o_start);
	$left = get_previous_token($p1);
	if ($left != '') {
		$working = split_previous_token($left, $p1);
		$lines[$o] = $working[0].(is_numeric($left)? 'flt_atof("'.$left.'")': 'flt_ltof('.$left.')').$working[1].substr($line, $o_start);
		$modified[] = $o;
	} else
		// Can't find the left operand, might be on the previous line
		merge_previous_line($lines, $modified, $o);
}

function process_simple_right_operand(&$lines, &$modified, $o, $o_finish) {
	$line = $lines[$o];
	$p1 = substr($line, $o_finish + 1);
	$right = get_next_token($p1);
	if ($right != '') {
		$working = explode($right, $p1, 2);
		$lines[$o] = substr($line, 0, $o_finish + 1).$working[0].(is_numeric($right)? 'flt_atof("'.$right.'")': 'flt_ltof('.$right.')').$working[1];
		$modified[] = $o;
	} else
		// Can't find the right operand, might be on the next line
		merge_next_line($lines, $modified, $o);
}

function process_complex_operand(&$lines, &$modified, $l, $start, $finish) {
	$line = $lines[$l];
	$lines[$l] = substr($line, 0, $start).'flt_ltof('.substr($line, $start, $finish - $start + 1).')'.substr($line, $finish + 1);
	$modified[] = $l;
}

function process_non_gcc_extension(&$lines, &$substitutions, $message, &$modified) {
	if (count($message->locations) == 1) {
		list($l, $start, $finish) = get_token_extent($message->locations[0]);
		if (!in_array($l, $modified)) {
			$line = $lines[$l];
			// Revise __fastcall__
			if (str_contains($line, '__fastcall__')) {
				$lines[$l] = str_replace('__fastcall__', '__attribute__ ((fastcall))', $line);
				$modified[] = $l;
				// Undo in post-processing
				$substitutions[] = array($lines[$l], $line);
			// Change _Bool to __Bool
			} else if (str_contains($line, '_Bool')) {
				$lines[$l] = str_replace('_Bool', '__Bool', $line);
				$modified[] = $l;
				// Undo in post-processing
				$substitutions[] = array($lines[$l], $line);
			} else
				// Didn't find anything? Perhaps an extension to add or a declaration across multiple lines
				process_unhandled($message);
		}
	} else
		process_unhandled($message);
}

function process_conflicting_types(&$lines, &$substitutions, $message, &$modified) {
	if (count($message->locations) == 1) {
		list($l, $start, $finish) = get_token_extent($message->locations[0]);
		if (!in_array($l, $modified)) {
			$line = $lines[$l];
			if (str_contains($line, 'size_t') && str_contains($message->message, 'long unsigned int')) {
				$lines[$l] = str_replace('size_t', 'long unsigned int', $line);
				$modified[] = $l;
				// Undo in post-processing
				$substitutions[] = array($lines[$l], $line);
			} else {
				$token = substr($line, $start, $finish - $start + 1);
				// Ignore v* functions, anything else is unhandled
				if (!in_array($token, array('vfprintf', 'vfscanf', 'vprintf', 'vscanf', 'vsnprintf', 'vsprintf', 'vsscanf')))
					process_unhandled($message);
			}
		}
	} else
		process_unhandled($message);
}

function process_unhandled($message) {
	file_put_contents('php://stderr', '*** ERROR: unhandled message ***'.PHP_EOL.print_r($message, true), FILE_APPEND);
}

function process_ignored($message) {
	file_put_contents('php://stderr', '*** WARNING: ignored message ***'.PHP_EOL.print_r($message, true), FILE_APPEND);
}
