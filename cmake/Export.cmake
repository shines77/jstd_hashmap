# Only generate .def for dll on MSVC

if (MSVC)
  set_source_files_properties(${JSTD_HASHMAP_DEF_FILE} PROPERTIES GENERATED 1)

  if (NOT DEFINED ARCH)
      set(ARCH_IN "x86_64")
  else()
      set(ARCH_IN ${ARCH})
  endif()

  if (${CORE} STREQUAL "generic")
      set(ARCH_IN "GENERIC")
  endif ()

  add_custom_command(
      OUTPUT ${PROJECT_BINARY_DIR}/jstd_hashmap.def
      # TARGET ${JSTD_LIBNAME} PRE_LINK
      COMMAND perl
      ARGS "${PROJECT_SOURCE_DIR}/exports/gensymbol" "win2k" "${ARCH_IN}" "dummy" "${SYMBOLPREFIX}" "${SYMBOLSUFFIX}" > "${PROJECT_BINARY_DIR}/jstd_hashmap.def"
      COMMENT "Create jstd_hashmap.def file"
      VERBATIM)
endif()
