
// Minimum requirements: gcc/clang C++ 11 or MSVC 2015 Update 3.
#if (!defined(_MSC_VER) && defined(__cplusplus) && (__cplusplus < 201103L)) \
 || (defined(_MSVC_LANG) && (_MSVC_LANG < 201103L)) \
 || (defined(_MSC_VER) && (_MSC_FULL_VER < 190024210))
#error "C++ 11 or higher is required"
#endif

#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>

#include <jstd/basic/stddef.h>

////////////////////////////////////////////////////////////////////////////////
//
// Clang Language Extensions
//
// See: http://clang.llvm.org/docs/LanguageExtensions.html#checking_language_features
//
////////////////////////////////////////////////////////////////////////////////

//
// Feature testing (C++20)
// See: https://en.cppreference.com/w/cpp/feature_test
//
#ifndef __has_feature                               // Optional of course.
  #define __has_feature(x)              0           // Compatibility with non-clang compilers.
#endif

#ifndef __has_builtin                               // Optional of course.
  #define __has_builtin(x)              0           // Compatibility with non-clang compilers.
#endif

#ifndef __has_extension
  #define __has_extension               __has_feature   // Compatibility with pre-3.0 compilers.
#endif

#ifndef __has_cpp_attribute                         // Optional of course.
  #define __has_cpp_attribute(x)        0           // Compatibility with non-clang compilers.
#endif

#ifndef __has_c_attribute                           // Optional of course.
  #define __has_c_attribute(x)          0           // Compatibility with non-clang compilers.
#endif

#ifndef __has_attribute                             // Optional of course.
  #define __has_attribute(x)            0           // Compatibility with non-clang compilers.
#endif

#ifndef __has_declspec_attribute                    // Optional of course.
  #define __has_declspec_attribute(x)   0           // Compatibility with non-clang compilers.
#endif

#ifndef __is_identifier                             // Optional of course.
  // It evaluates to 1 if the argument x is just a regular identifier and not a reserved keyword.
  #define __is_identifier(x)            1           // Compatibility with non-clang compilers.
#endif

// Since C++ 17
#ifndef __has_include
  #define __has_include(x)              0
#endif

#if defined(_MSC_VER)
#ifndef __attribute__
  #define __attribute__(x)
#endif
#endif

//
// See: https://en.cppreference.com/w/cpp/feature_test
//

////////////////////////////////////////////////////////////////////////////////

// Change these options to print out only necessary info.
static struct PrintConfig {
    // compiler marco
    constexpr static bool marco_titles          = 1;
    constexpr static bool marco_counters        = 1;

    constexpr static bool compiler_version      = 1;
    constexpr static bool compiler_paltforms    = 1;
    constexpr static bool compiler_arch         = 1;
    constexpr static bool compiler_others       = 1;

    // print style
    constexpr static bool titles                = 1;
    constexpr static bool counters              = 1;
    constexpr static bool sorted_by_value       = 0;

    // category
    constexpr static bool cxx11                 = 1;
    constexpr static bool cxx14                 = 1;
    constexpr static bool cxx17                 = 1;
    constexpr static bool cxx20                 = 1;
    constexpr static bool cxx23                 = 1;

    // content
    constexpr static bool attributes            = 1;
    constexpr static bool general_features      = 1;
    constexpr static bool core_features         = 1;
    constexpr static bool lib_features          = 1;
    constexpr static bool supported_features    = 1;
    constexpr static bool unsupported_features  = 1;
} printConfig;

////////////////////////////////////////////////////////////////////////////////

#ifdef __has_include
  #if __has_include(<version>)
  #include <version>
  #endif
#endif

#ifndef __count_of
#define __count_of(arr)     (sizeof(arr) / sizeof(arr[0]))
#endif

#define COMPILER_MARCO_VALUE(value)     #value
#define COMPILER_MARCO_ENTRY(name)      { #name, COMPILER_MARCO_VALUE(name) }

#define COMPILER_FEATURE_VALUE(value)   #value
#define COMPILER_FEATURE_ENTRY(name)    { #name, COMPILER_FEATURE_VALUE(name) },

#ifdef __has_cpp_attribute
  #define COMPILER_ATTRIBUTE_VALUE(num)             #num
  #define COMPILER_ATTRIBUTE_AS_NUMBER(attr_value)  COMPILER_ATTRIBUTE_VALUE(attr_value)

  #define COMPILER_ATTRIBUTE_ENTRY(attr) \
        { #attr, COMPILER_ATTRIBUTE_AS_NUMBER(__has_cpp_attribute(attr)) },
#else
  #define COMPILER_ATTRIBUTE_ENTRY(attr)    { #attr, "_" },
#endif

struct CompilerMarco {
    CompilerMarco(const char * name = nullptr, const char * value = nullptr)
        : name(name), value(value) {
    }

    const char * name;
    const char * value;
};

struct CompilerFeature {
    CompilerFeature(const char * name = nullptr, const char * value = nullptr)
        : name(name), value(value) {
    }

    const char * name;
    const char * value;
};

static CompilerMarco compiler_version[] = {
#ifdef __cplusplus
    COMPILER_MARCO_ENTRY(__cplusplus),
#endif

/* gcc or g++ */

#ifdef __GNUC__
    COMPILER_MARCO_ENTRY(__GNUC__),
#endif

#ifdef __GNUC_MAJOR__
    COMPILER_MARCO_ENTRY(__GNUC_MAJOR__),
#endif

#ifdef __GNUC_MINOR__
    COMPILER_MARCO_ENTRY(__GNUC_MINOR__),
#endif

#ifdef __GNUC_PATCHLEVEL__
    COMPILER_MARCO_ENTRY(__GNUC_PATCHLEVEL__),
#endif

#ifdef __GNUG__
    COMPILER_MARCO_ENTRY(__GNUG__),
#endif

/* clang */

#ifdef __clang__
    COMPILER_MARCO_ENTRY(__clang__),
#endif

#ifdef __clang_major__
    COMPILER_MARCO_ENTRY(__clang_major__),
#endif

#ifdef __clang_minor__
    COMPILER_MARCO_ENTRY(__clang_minor__),
#endif

#ifdef __clang_patchlevel__
    COMPILER_MARCO_ENTRY(__clang_patchlevel__),
#endif

/* Interl C++ */

#ifdef __ICL
    COMPILER_MARCO_ENTRY(__ICL),
#endif

#ifdef __ICC
    COMPILER_MARCO_ENTRY(__ICC),
#endif

#ifdef __ECC
    COMPILER_MARCO_ENTRY(__ECC),
#endif

#ifdef __ECL
    COMPILER_MARCO_ENTRY(__ECL),
#endif

#ifdef __ICPC
    COMPILER_MARCO_ENTRY(__ICPC),
#endif

#ifdef __INTEL_COMPILER
    COMPILER_MARCO_ENTRY(__INTEL_COMPILER),
#endif

#ifdef __INTEL_CXX_VERSION
    COMPILER_MARCO_ENTRY(__INTEL_CXX_VERSION),
#endif

/* Visual C++ */

#ifdef _MSC_VER
    COMPILER_MARCO_ENTRY(_MSC_VER),
#endif

#ifdef _MSC_FULL_VER
    COMPILER_MARCO_ENTRY(_MSC_FULL_VER),
#endif

#ifdef _MSVC_LANG
    COMPILER_MARCO_ENTRY(_MSVC_LANG),
#endif

/* DMC++ */

#ifdef __DMC__
    COMPILER_MARCO_ENTRY(__DMC__),
#endif

/* ARM C/C++ */

#ifdef __ARMCC_VERSION
    COMPILER_MARCO_ENTRY(__ARMCC_VERSION),
#endif

/* jstd */

#ifdef JSTD_IS_GCC
    COMPILER_MARCO_ENTRY(JSTD_IS_GCC),
#endif

#ifdef JSTD_IS_CLANG
    COMPILER_MARCO_ENTRY(JSTD_IS_CLANG),
#endif

#ifdef JSTD_IS_MSVC
    COMPILER_MARCO_ENTRY(JSTD_IS_MSVC),
#endif

#ifdef JSTD_IS_ICC
    COMPILER_MARCO_ENTRY(JSTD_IS_ICC),
#endif

#ifdef JSTD_GCC_STYLE_ASM
    COMPILER_MARCO_ENTRY(JSTD_GCC_STYLE_ASM),
#endif

#ifdef JSTD_IS_PURE_GCC
    COMPILER_MARCO_ENTRY(JSTD_IS_PURE_GCC),
#endif
};

static CompilerMarco compiler_platforms[] = {
#ifdef _WIN32
    COMPILER_MARCO_ENTRY(_WIN32),
#endif

#ifdef _WIN64
    COMPILER_MARCO_ENTRY(_WIN64),
#endif

/* Windows32 by mingw compiler */
#ifdef __MINGW32__
    COMPILER_MARCO_ENTRY(__MINGW32__),
#endif

/* Cygwin */
#ifdef __CYGWIN__
    COMPILER_MARCO_ENTRY(__CYGWIN__),
#endif

/* linux */
#ifdef __linux__
    COMPILER_MARCO_ENTRY(__linux__),
#endif

/* unix */
#ifdef __unix__
    COMPILER_MARCO_ENTRY(__unix__),
#endif

#ifdef __unix
    COMPILER_MARCO_ENTRY(__unix),
#endif

/* FreeBSD */
#ifdef __FreeBSD__
    COMPILER_MARCO_ENTRY(__FreeBSD__),
#endif

/* NetBSD */
#ifdef __NetBSD__
    COMPILER_MARCO_ENTRY(__NetBSD__),
#endif

/* OpenBSD */
#ifdef __OpenBSD__
    COMPILER_MARCO_ENTRY(__OpenBSD__),
#endif

/* Sun OS */
#ifdef __sun__
    COMPILER_MARCO_ENTRY(__sun__),
#endif

/* Apple */
#ifdef __APPLE__
    COMPILER_MARCO_ENTRY(__APPLE__),
#endif

/* Apple */
#ifdef __apple__
    COMPILER_MARCO_ENTRY(__apple__),
#endif

/* MAC OS X */
#ifdef __MaxOSX__
    COMPILER_MARCO_ENTRY(__MaxOSX__),
#endif

/* Android */
#ifdef __ANDROID__
    COMPILER_MARCO_ENTRY(__ANDROID__),
#endif
};

static CompilerMarco compiler_arch[] = {
#ifdef JSTD_IS_X86
    COMPILER_MARCO_ENTRY(JSTD_IS_X86),
#endif

#ifdef JSTD_IS_X86_64
    COMPILER_MARCO_ENTRY(JSTD_IS_X86_64),
#endif

#ifdef JSTD_IS_X86_I386
    COMPILER_MARCO_ENTRY(JSTD_IS_X86_I386),
#endif

#ifdef JSTD_WORD_LEN
    COMPILER_MARCO_ENTRY(JSTD_WORD_LEN),
#endif

#ifdef __BIG_ENDIAN__
    COMPILER_MARCO_ENTRY(__BIG_ENDIAN__),
#endif

#ifdef __LITTLE_ENDIAN__
    COMPILER_MARCO_ENTRY(__LITTLE_ENDIAN__),
#endif
};

static CompilerMarco compiler_others[] = {
#ifdef __DATE__
    COMPILER_MARCO_ENTRY(__DATE__),
#endif

#ifdef __TIME__
    COMPILER_MARCO_ENTRY(__TIME__),
#endif

#ifdef __FILE__
    COMPILER_MARCO_ENTRY(__FILE__),
#endif

#ifdef _BSD_SOURCE
    COMPILER_MARCO_ENTRY(_BSD_SOURCE),
#endif

#ifdef _POSIX_SOURCE
    COMPILER_MARCO_ENTRY(_POSIX_SOURCE),
#endif

#ifdef _XOPEN_SOURCE
    COMPILER_MARCO_ENTRY(_XOPEN_SOURCE),
#endif

#ifdef _GNU_SOURCE
    COMPILER_MARCO_ENTRY(_GNU_SOURCE),
#endif

#ifdef __VERSION__
    COMPILER_MARCO_ENTRY(__VERSION__),
#endif

#ifdef __cpp_constexpr
    COMPILER_MARCO_ENTRY(__cpp_constexpr),
#endif

#ifdef __cpp_variable_templates
    COMPILER_MARCO_ENTRY(__cpp_variable_templates),
#endif

#ifdef __cpp_lib_integer_sequence
    COMPILER_MARCO_ENTRY(__cpp_lib_integer_sequence),
#endif

#ifdef __cpp_exceptions
    COMPILER_MARCO_ENTRY(__cpp_exceptions),
#endif
};

static CompilerFeature cxx_core[] = {
    COMPILER_FEATURE_ENTRY(__cplusplus)
    COMPILER_FEATURE_ENTRY(__cpp_exceptions)
    COMPILER_FEATURE_ENTRY(__cpp_rtti)
#if 1
    COMPILER_FEATURE_ENTRY(__GNUC__)
    COMPILER_FEATURE_ENTRY(__GNUC_MINOR__)
    COMPILER_FEATURE_ENTRY(__GNUC_PATCHLEVEL__)
    COMPILER_FEATURE_ENTRY(__GNUG__)
    COMPILER_FEATURE_ENTRY(__clang__)
    COMPILER_FEATURE_ENTRY(__clang_major__)
    COMPILER_FEATURE_ENTRY(__clang_minor__)
    COMPILER_FEATURE_ENTRY(__clang_patchlevel__)
#endif
};

static CompilerFeature cxx11_core[] = {
    COMPILER_FEATURE_ENTRY(__cpp_alias_templates)
    COMPILER_FEATURE_ENTRY(__cpp_attributes)
    COMPILER_FEATURE_ENTRY(__cpp_constexpr)
    COMPILER_FEATURE_ENTRY(__cpp_decltype)
    COMPILER_FEATURE_ENTRY(__cpp_delegating_constructors)
    COMPILER_FEATURE_ENTRY(__cpp_inheriting_constructors)
    COMPILER_FEATURE_ENTRY(__cpp_initializer_lists)
    COMPILER_FEATURE_ENTRY(__cpp_lambdas)
    COMPILER_FEATURE_ENTRY(__cpp_nsdmi)
    COMPILER_FEATURE_ENTRY(__cpp_range_based_for)
    COMPILER_FEATURE_ENTRY(__cpp_raw_strings)
    COMPILER_FEATURE_ENTRY(__cpp_ref_qualifiers)
    COMPILER_FEATURE_ENTRY(__cpp_rvalue_references)
    COMPILER_FEATURE_ENTRY(__cpp_static_assert)
    COMPILER_FEATURE_ENTRY(__cpp_threadsafe_static_init)
    COMPILER_FEATURE_ENTRY(__cpp_unicode_characters)
    COMPILER_FEATURE_ENTRY(__cpp_unicode_literals)
    COMPILER_FEATURE_ENTRY(__cpp_user_defined_literals)
    COMPILER_FEATURE_ENTRY(__cpp_variadic_templates)
};

static CompilerFeature cxx14_core[] = {
    COMPILER_FEATURE_ENTRY(__cpp_aggregate_nsdmi)
    COMPILER_FEATURE_ENTRY(__cpp_binary_literals)
    COMPILER_FEATURE_ENTRY(__cpp_constexpr)
    COMPILER_FEATURE_ENTRY(__cpp_decltype_auto)
    COMPILER_FEATURE_ENTRY(__cpp_generic_lambdas)
    COMPILER_FEATURE_ENTRY(__cpp_init_captures)
    COMPILER_FEATURE_ENTRY(__cpp_return_type_deduction)
    COMPILER_FEATURE_ENTRY(__cpp_sized_deallocation)
    COMPILER_FEATURE_ENTRY(__cpp_variable_templates)
};

static CompilerFeature cxx14_lib[] = {
    COMPILER_FEATURE_ENTRY(__cpp_lib_chrono_udls)
    COMPILER_FEATURE_ENTRY(__cpp_lib_complex_udls)
    COMPILER_FEATURE_ENTRY(__cpp_lib_exchange_function)
    COMPILER_FEATURE_ENTRY(__cpp_lib_generic_associative_lookup)
    COMPILER_FEATURE_ENTRY(__cpp_lib_integer_sequence)
    COMPILER_FEATURE_ENTRY(__cpp_lib_integral_constant_callable)
    COMPILER_FEATURE_ENTRY(__cpp_lib_is_final)
    COMPILER_FEATURE_ENTRY(__cpp_lib_is_null_pointer)
    COMPILER_FEATURE_ENTRY(__cpp_lib_make_reverse_iterator)
    COMPILER_FEATURE_ENTRY(__cpp_lib_make_unique)
    COMPILER_FEATURE_ENTRY(__cpp_lib_null_iterators)
    COMPILER_FEATURE_ENTRY(__cpp_lib_quoted_string_io)
    COMPILER_FEATURE_ENTRY(__cpp_lib_result_of_sfinae)
    COMPILER_FEATURE_ENTRY(__cpp_lib_robust_nonmodifying_seq_ops)
    COMPILER_FEATURE_ENTRY(__cpp_lib_shared_timed_mutex)
    COMPILER_FEATURE_ENTRY(__cpp_lib_string_udls)
    COMPILER_FEATURE_ENTRY(__cpp_lib_transformation_trait_aliases)
    COMPILER_FEATURE_ENTRY(__cpp_lib_transparent_operators)
    COMPILER_FEATURE_ENTRY(__cpp_lib_tuple_element_t)
    COMPILER_FEATURE_ENTRY(__cpp_lib_tuples_by_type)
};

static CompilerFeature cxx17_core[] = {
    COMPILER_FEATURE_ENTRY(__cpp_aggregate_bases)
    COMPILER_FEATURE_ENTRY(__cpp_aligned_new)
    COMPILER_FEATURE_ENTRY(__cpp_capture_star_this)
    COMPILER_FEATURE_ENTRY(__cpp_constexpr)
    COMPILER_FEATURE_ENTRY(__cpp_deduction_guides)
    COMPILER_FEATURE_ENTRY(__cpp_enumerator_attributes)
    COMPILER_FEATURE_ENTRY(__cpp_fold_expressions)
    COMPILER_FEATURE_ENTRY(__cpp_guaranteed_copy_elision)
    COMPILER_FEATURE_ENTRY(__cpp_hex_float)
    COMPILER_FEATURE_ENTRY(__cpp_if_constexpr)
    COMPILER_FEATURE_ENTRY(__cpp_inheriting_constructors)
    COMPILER_FEATURE_ENTRY(__cpp_inline_variables)
    COMPILER_FEATURE_ENTRY(__cpp_namespace_attributes)
    COMPILER_FEATURE_ENTRY(__cpp_noexcept_function_type)
    COMPILER_FEATURE_ENTRY(__cpp_nontype_template_args)
    COMPILER_FEATURE_ENTRY(__cpp_nontype_template_parameter_auto)
    COMPILER_FEATURE_ENTRY(__cpp_range_based_for)
    COMPILER_FEATURE_ENTRY(__cpp_static_assert)
    COMPILER_FEATURE_ENTRY(__cpp_structured_bindings)
    COMPILER_FEATURE_ENTRY(__cpp_template_template_args)
    COMPILER_FEATURE_ENTRY(__cpp_variadic_using)
};

static CompilerFeature cxx17_lib[] = {
    COMPILER_FEATURE_ENTRY(__cpp_lib_addressof_constexpr)
    COMPILER_FEATURE_ENTRY(__cpp_lib_allocator_traits_is_always_equal)
    COMPILER_FEATURE_ENTRY(__cpp_lib_any)
    COMPILER_FEATURE_ENTRY(__cpp_lib_apply)
    COMPILER_FEATURE_ENTRY(__cpp_lib_array_constexpr)
    COMPILER_FEATURE_ENTRY(__cpp_lib_as_const)
    COMPILER_FEATURE_ENTRY(__cpp_lib_atomic_is_always_lock_free)
    COMPILER_FEATURE_ENTRY(__cpp_lib_bool_constant)
    COMPILER_FEATURE_ENTRY(__cpp_lib_boyer_moore_searcher)
    COMPILER_FEATURE_ENTRY(__cpp_lib_byte)
    COMPILER_FEATURE_ENTRY(__cpp_lib_chrono)
    COMPILER_FEATURE_ENTRY(__cpp_lib_clamp)
    COMPILER_FEATURE_ENTRY(__cpp_lib_enable_shared_from_this)
    COMPILER_FEATURE_ENTRY(__cpp_lib_execution)
    COMPILER_FEATURE_ENTRY(__cpp_lib_filesystem)
    COMPILER_FEATURE_ENTRY(__cpp_lib_gcd_lcm)
    COMPILER_FEATURE_ENTRY(__cpp_lib_hardware_interference_size)
    COMPILER_FEATURE_ENTRY(__cpp_lib_has_unique_object_representations)
    COMPILER_FEATURE_ENTRY(__cpp_lib_hypot)
    COMPILER_FEATURE_ENTRY(__cpp_lib_incomplete_container_elements)
    COMPILER_FEATURE_ENTRY(__cpp_lib_invoke)
    COMPILER_FEATURE_ENTRY(__cpp_lib_is_aggregate)
    COMPILER_FEATURE_ENTRY(__cpp_lib_is_invocable)
    COMPILER_FEATURE_ENTRY(__cpp_lib_is_swappable)
    COMPILER_FEATURE_ENTRY(__cpp_lib_launder)
    COMPILER_FEATURE_ENTRY(__cpp_lib_logical_traits)
    COMPILER_FEATURE_ENTRY(__cpp_lib_make_from_tuple)
    COMPILER_FEATURE_ENTRY(__cpp_lib_map_try_emplace)
    COMPILER_FEATURE_ENTRY(__cpp_lib_math_special_functions)
    COMPILER_FEATURE_ENTRY(__cpp_lib_memory_resource)
    COMPILER_FEATURE_ENTRY(__cpp_lib_node_extract)
    COMPILER_FEATURE_ENTRY(__cpp_lib_nonmember_container_access)
    COMPILER_FEATURE_ENTRY(__cpp_lib_not_fn)
    COMPILER_FEATURE_ENTRY(__cpp_lib_optional)
    COMPILER_FEATURE_ENTRY(__cpp_lib_parallel_algorithm)
    COMPILER_FEATURE_ENTRY(__cpp_lib_raw_memory_algorithms)
    COMPILER_FEATURE_ENTRY(__cpp_lib_sample)
    COMPILER_FEATURE_ENTRY(__cpp_lib_scoped_lock)
    COMPILER_FEATURE_ENTRY(__cpp_lib_shared_mutex)
    COMPILER_FEATURE_ENTRY(__cpp_lib_shared_ptr_arrays)
    COMPILER_FEATURE_ENTRY(__cpp_lib_shared_ptr_weak_type)
    COMPILER_FEATURE_ENTRY(__cpp_lib_string_view)
    COMPILER_FEATURE_ENTRY(__cpp_lib_to_chars)
    COMPILER_FEATURE_ENTRY(__cpp_lib_transparent_operators)
    COMPILER_FEATURE_ENTRY(__cpp_lib_type_trait_variable_templates)
    COMPILER_FEATURE_ENTRY(__cpp_lib_uncaught_exceptions)
    COMPILER_FEATURE_ENTRY(__cpp_lib_unordered_map_try_emplace)
    COMPILER_FEATURE_ENTRY(__cpp_lib_variant)
    COMPILER_FEATURE_ENTRY(__cpp_lib_void_t)
};

static CompilerFeature cxx20_core[] = {
    COMPILER_FEATURE_ENTRY(__cpp_aggregate_paren_init)
    COMPILER_FEATURE_ENTRY(__cpp_char8_t)
    COMPILER_FEATURE_ENTRY(__cpp_concepts)
    COMPILER_FEATURE_ENTRY(__cpp_conditional_explicit)
    COMPILER_FEATURE_ENTRY(__cpp_consteval)
    COMPILER_FEATURE_ENTRY(__cpp_constexpr)
    COMPILER_FEATURE_ENTRY(__cpp_constexpr_dynamic_alloc)
    COMPILER_FEATURE_ENTRY(__cpp_constexpr_in_decltype)
    COMPILER_FEATURE_ENTRY(__cpp_constinit)
    COMPILER_FEATURE_ENTRY(__cpp_deduction_guides)
    COMPILER_FEATURE_ENTRY(__cpp_designated_initializers)
    COMPILER_FEATURE_ENTRY(__cpp_generic_lambdas)
    COMPILER_FEATURE_ENTRY(__cpp_impl_coroutine)
    COMPILER_FEATURE_ENTRY(__cpp_impl_destroying_delete)
    COMPILER_FEATURE_ENTRY(__cpp_impl_three_way_comparison)
    COMPILER_FEATURE_ENTRY(__cpp_init_captures)
    COMPILER_FEATURE_ENTRY(__cpp_modules)
    COMPILER_FEATURE_ENTRY(__cpp_nontype_template_args)
    COMPILER_FEATURE_ENTRY(__cpp_using_enum)
};

static CompilerFeature cxx20_lib[] = {
    COMPILER_FEATURE_ENTRY(__cpp_lib_array_constexpr)
    COMPILER_FEATURE_ENTRY(__cpp_lib_assume_aligned)
    COMPILER_FEATURE_ENTRY(__cpp_lib_atomic_flag_test)
    COMPILER_FEATURE_ENTRY(__cpp_lib_atomic_float)
    COMPILER_FEATURE_ENTRY(__cpp_lib_atomic_lock_free_type_aliases)
    COMPILER_FEATURE_ENTRY(__cpp_lib_atomic_ref)
    COMPILER_FEATURE_ENTRY(__cpp_lib_atomic_shared_ptr)
    COMPILER_FEATURE_ENTRY(__cpp_lib_atomic_value_initialization)
    COMPILER_FEATURE_ENTRY(__cpp_lib_atomic_wait)
    COMPILER_FEATURE_ENTRY(__cpp_lib_barrier)
    COMPILER_FEATURE_ENTRY(__cpp_lib_bind_front)
    COMPILER_FEATURE_ENTRY(__cpp_lib_bit_cast)
    COMPILER_FEATURE_ENTRY(__cpp_lib_bitops)
    COMPILER_FEATURE_ENTRY(__cpp_lib_bounded_array_traits)
    COMPILER_FEATURE_ENTRY(__cpp_lib_char8_t)
    COMPILER_FEATURE_ENTRY(__cpp_lib_chrono)
    COMPILER_FEATURE_ENTRY(__cpp_lib_concepts)
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_algorithms)
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_complex)
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_dynamic_alloc)
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_functional)
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_iterator)
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_memory)
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_numeric)
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_string)
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_string_view)
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_tuple)
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_utility)
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_vector)
    COMPILER_FEATURE_ENTRY(__cpp_lib_coroutine)
    COMPILER_FEATURE_ENTRY(__cpp_lib_destroying_delete)
    COMPILER_FEATURE_ENTRY(__cpp_lib_endian)
    COMPILER_FEATURE_ENTRY(__cpp_lib_erase_if)
    COMPILER_FEATURE_ENTRY(__cpp_lib_execution)
    COMPILER_FEATURE_ENTRY(__cpp_lib_format)
    COMPILER_FEATURE_ENTRY(__cpp_lib_generic_unordered_lookup)
    COMPILER_FEATURE_ENTRY(__cpp_lib_int_pow2)
    COMPILER_FEATURE_ENTRY(__cpp_lib_integer_comparison_functions)
    COMPILER_FEATURE_ENTRY(__cpp_lib_interpolate)
    COMPILER_FEATURE_ENTRY(__cpp_lib_is_constant_evaluated)
    COMPILER_FEATURE_ENTRY(__cpp_lib_is_layout_compatible)
    COMPILER_FEATURE_ENTRY(__cpp_lib_is_nothrow_convertible)
    COMPILER_FEATURE_ENTRY(__cpp_lib_is_pointer_interconvertible)
    COMPILER_FEATURE_ENTRY(__cpp_lib_jthread)
    COMPILER_FEATURE_ENTRY(__cpp_lib_latch)
    COMPILER_FEATURE_ENTRY(__cpp_lib_list_remove_return_type)
    COMPILER_FEATURE_ENTRY(__cpp_lib_math_constants)
    COMPILER_FEATURE_ENTRY(__cpp_lib_polymorphic_allocator)
    COMPILER_FEATURE_ENTRY(__cpp_lib_ranges)
    COMPILER_FEATURE_ENTRY(__cpp_lib_remove_cvref)
    COMPILER_FEATURE_ENTRY(__cpp_lib_semaphore)
    COMPILER_FEATURE_ENTRY(__cpp_lib_shared_ptr_arrays)
    COMPILER_FEATURE_ENTRY(__cpp_lib_shift)
    COMPILER_FEATURE_ENTRY(__cpp_lib_smart_ptr_for_overwrite)
    COMPILER_FEATURE_ENTRY(__cpp_lib_source_location)
    COMPILER_FEATURE_ENTRY(__cpp_lib_span)
    COMPILER_FEATURE_ENTRY(__cpp_lib_ssize)
    COMPILER_FEATURE_ENTRY(__cpp_lib_starts_ends_with)
    COMPILER_FEATURE_ENTRY(__cpp_lib_string_view)
    COMPILER_FEATURE_ENTRY(__cpp_lib_syncbuf)
    COMPILER_FEATURE_ENTRY(__cpp_lib_three_way_comparison)
    COMPILER_FEATURE_ENTRY(__cpp_lib_to_address)
    COMPILER_FEATURE_ENTRY(__cpp_lib_to_array)
    COMPILER_FEATURE_ENTRY(__cpp_lib_type_identity)
    COMPILER_FEATURE_ENTRY(__cpp_lib_unwrap_ref)
};

static CompilerFeature cxx23_core[] = {
    //< Continue to Populate
    COMPILER_FEATURE_ENTRY(__cpp_constexpr)
    COMPILER_FEATURE_ENTRY(__cpp_explicit_this_parameter)
    COMPILER_FEATURE_ENTRY(__cpp_if_consteval)
    COMPILER_FEATURE_ENTRY(__cpp_multidimensional_subscript)
    COMPILER_FEATURE_ENTRY(__cpp_size_t_suffix)
};

static CompilerFeature cxx23_lib[] = {
    //< Continue to Populate
    COMPILER_FEATURE_ENTRY(__cpp_lib_adaptor_iterator_pair_constructor)
    COMPILER_FEATURE_ENTRY(__cpp_lib_allocate_at_least)
    COMPILER_FEATURE_ENTRY(__cpp_lib_associative_heterogeneous_erasure)
    COMPILER_FEATURE_ENTRY(__cpp_lib_bind_back)
    COMPILER_FEATURE_ENTRY(__cpp_lib_byteswap)
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_cmath)
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_memory)
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_typeinfo)
    COMPILER_FEATURE_ENTRY(__cpp_lib_containers_ranges)
    COMPILER_FEATURE_ENTRY(__cpp_lib_expected)
    COMPILER_FEATURE_ENTRY(__cpp_lib_format)
    COMPILER_FEATURE_ENTRY(__cpp_lib_invoke_r)
    COMPILER_FEATURE_ENTRY(__cpp_lib_is_scoped_enum)
    COMPILER_FEATURE_ENTRY(__cpp_lib_move_only_function)
    COMPILER_FEATURE_ENTRY(__cpp_lib_optional)
    COMPILER_FEATURE_ENTRY(__cpp_lib_out_ptr)
    COMPILER_FEATURE_ENTRY(__cpp_lib_ranges)
    COMPILER_FEATURE_ENTRY(__cpp_lib_ranges_chunk)
    COMPILER_FEATURE_ENTRY(__cpp_lib_ranges_chunk_by)
    COMPILER_FEATURE_ENTRY(__cpp_lib_ranges_iota)
    COMPILER_FEATURE_ENTRY(__cpp_lib_ranges_join_with)
    COMPILER_FEATURE_ENTRY(__cpp_lib_ranges_slide)
    COMPILER_FEATURE_ENTRY(__cpp_lib_ranges_starts_ends_with)
    COMPILER_FEATURE_ENTRY(__cpp_lib_ranges_to_container)
    COMPILER_FEATURE_ENTRY(__cpp_lib_ranges_zip)
    COMPILER_FEATURE_ENTRY(__cpp_lib_reference_from_temporary)
    COMPILER_FEATURE_ENTRY(__cpp_lib_shift)
    COMPILER_FEATURE_ENTRY(__cpp_lib_spanstream)
    COMPILER_FEATURE_ENTRY(__cpp_lib_stacktrace)
    COMPILER_FEATURE_ENTRY(__cpp_lib_stdatomic_h)
    COMPILER_FEATURE_ENTRY(__cpp_lib_string_contains)
    COMPILER_FEATURE_ENTRY(__cpp_lib_string_resize_and_overwrite)
    COMPILER_FEATURE_ENTRY(__cpp_lib_to_underlying)
    COMPILER_FEATURE_ENTRY(__cpp_lib_unreachable)
    COMPILER_FEATURE_ENTRY(__cpp_lib_variant)
};

static CompilerFeature attributes[] = {
    COMPILER_ATTRIBUTE_ENTRY(noreturn)
    COMPILER_ATTRIBUTE_ENTRY(deprecated)
    COMPILER_ATTRIBUTE_ENTRY(maybe_unused)
    COMPILER_ATTRIBUTE_ENTRY(carries_dependency)
    COMPILER_ATTRIBUTE_ENTRY(fallthrough)
    COMPILER_ATTRIBUTE_ENTRY(likely)
    COMPILER_ATTRIBUTE_ENTRY(unlikely)
    COMPILER_ATTRIBUTE_ENTRY(nodiscard)
    COMPILER_ATTRIBUTE_ENTRY(no_unique_address)
};

static constexpr int max_name_length = 44; //< Update if necessary

void print_compiler_macro(const CompilerMarco & m)
{
    std::cout << "#define "
              << std::left << std::setw(max_name_length - 8)
              << m.name << " " << m.value << '\n';
}

template <std::size_t N>
void printMarco(const char * title, CompilerMarco (&macros)[N]) {
    if (printConfig.marco_titles) {
        std::cout << std::left << title << " (";
        if (printConfig.marco_counters) {
            std::cout << (N - 1) << '/';
        }
        std::cout << (N - 1) << ")\n";
        std::cout << '\n';
    }
    if (printConfig.sorted_by_value) {
        std::sort(std::begin(macros), std::end(macros),
            [](CompilerMarco const & lhs, CompilerMarco const & rhs) {
                return std::strcmp(lhs.value, rhs.value) < 0;
            });
    }
    for (const CompilerMarco & m : macros) {
        print_compiler_macro(m);
    }
    std::cout << '\n';
}

constexpr bool is_feature_supported(const CompilerFeature & x)
{
    return (x.value[0] != '_') && (x.value[0] != '0');
}

void print_compiler_feature(const CompilerFeature & x)
{
    std::string value{ is_feature_supported(x) ? x.value : "------" };
#if 0
    if (value.back() == 'L') {
        value.pop_back(); //~ 201603L -> 201603
    }
#endif

    // value.insert(4, 1, '-'); //~ 201603 -> 2016-03
    if ((printConfig.supported_features && is_feature_supported(x)) ||
        (printConfig.unsupported_features && !is_feature_supported(x))) {
            std::cout << std::left << std::setw(max_name_length)
                      << x.name << " " << value << '\n';
    }
}

template <std::size_t N>
void printFeature(const char * title, CompilerFeature (&features)[N]) {
    if (printConfig.titles) {
        std::cout << std::left << title << " (";
        if (printConfig.counters) {
            std::cout << std::count_if(
                std::begin(features), std::end(features), is_feature_supported) << '/';
        }
        std::cout << N << ")\n";
        std::cout << '\n';
    }
    if (printConfig.sorted_by_value) {
        std::sort(std::begin(features), std::end(features),
            [](CompilerFeature const & lhs, CompilerFeature const & rhs) {
                return std::strcmp(lhs.value, rhs.value) < 0;
            });
    }
    for (const CompilerFeature & x : features) {
        print_compiler_feature(x);
    }
    std::cout << '\n';
}

int main(int argc, char * argv[])
{
    // Compiler marco
    if (printConfig.compiler_version  ) printMarco("Compiler definitions", compiler_version);
    if (printConfig.compiler_paltforms) printMarco("Platform definitions" , compiler_platforms);
    if (printConfig.compiler_arch     ) printMarco("Architecture definitions", compiler_arch);
    if (printConfig.compiler_others   ) printMarco("Other definitions" , compiler_others);

    // Compiler feature
    if (printConfig.general_features) printFeature("C++ GENERAL", cxx_core);

    if (printConfig.cxx11 && printConfig.core_features) printFeature("C++11 CORE", cxx11_core);

    if (printConfig.cxx14 && printConfig.core_features) printFeature("C++14 CORE", cxx14_core);
    if (printConfig.cxx14 && printConfig.lib_features ) printFeature("C++14 LIB" , cxx14_lib);

    if (printConfig.cxx17 && printConfig.core_features) printFeature("C++17 CORE", cxx17_core);
    if (printConfig.cxx17 && printConfig.lib_features ) printFeature("C++17 LIB" , cxx17_lib);

    if (printConfig.cxx20 && printConfig.core_features) printFeature("C++20 CORE", cxx20_core);
    if (printConfig.cxx20 && printConfig.lib_features ) printFeature("C++20 LIB" , cxx20_lib);

    if (printConfig.cxx23 && printConfig.core_features) printFeature("C++23 CORE", cxx23_core);
    if (printConfig.cxx23 && printConfig.lib_features ) printFeature("C++23 LIB" , cxx23_lib);

    if (printConfig.attributes) printFeature("ATTRIBUTES", attributes);

    return 0;
}
