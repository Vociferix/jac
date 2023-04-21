#ifndef JAC_MACROS_HPP
#define JAC_MACROS_HPP

/// @file

/// @mainpage
/// # Jack's Awesome Containers!
///
/// `jac` is a header-only C++20 library of various generally useful and niche container data strcutures.
///
/// ## Smart Pointers (Single Object Containers)
///  Type | Brief  
/// ------|-------
///  @ref jac::holder "holder<T, Tag=void>" | @copybrief jac::holder
///  @ref jac::option "option<T>" | @copybrief jac::option
///  @ref jac::result "result<T, E>" | @copybrief jac::result
///
/// ## Utility Types
///  Type | Brief
/// ------|-------
///  @ref jac::null_t "null_t" | @copybrief jac::null_t
///  @ref jac::void_t "void_t" | @copybrief jac::void_t
///  @ref jac::error "error" | @copybrief jac::error
/// 
/// ## Constants
///  Constant | Brief
/// ----------|-------
/// @ref jac::null "null" | @copybrief jac::null
/// @ref jac::void_v "void_v" | @copybrief jac::void_v

#if !defined(_MSC_VER) || defined(DOXYGEN)
#define JAC_NO_UNIQ_ADDR [[no_unique_address]]
#else
#define JAC_NO_UNIQ_ADDR [[msvc::no_unique_address]]
#endif

#endif
