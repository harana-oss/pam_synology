find_package(Perl REQUIRED)

function(enable_linting)

  file(DOWNLOAD
        "https://raw.githubusercontent.com/torvalds/linux/master/scripts/checkpatch.pl"
	${CMAKE_CURRENT_BINARY_DIR}/checkpatch.pl)
  file(DOWNLOAD
        "https://raw.githubusercontent.com/torvalds/linux/master/scripts/spelling.txt"
	${CMAKE_CURRENT_BINARY_DIR}/spelling.txt)
  file(DOWNLOAD
	"https://raw.githubusercontent.com/torvalds/linux/master/scripts/const_structs.checkpatch"
	${CMAKE_CURRENT_BINARY_DIR}/const_structs.checkpatch)
  file(GLOB_RECURSE SOURCES_LIST
	${PROJECT_SOURCE_DIR}/src/*.c
	${PROJECT_SOURCE_DIR}/src/*.h
	${PROJECT_SOURCE_DIR}/include/*.h)

  list(REMOVE_DUPLICATES SOURCES_LIST)
  list(SORT SOURCES_LIST)

  foreach(sourcefile ${SOURCES_LIST})
	set (lintstampdir "${CMAKE_CURRENT_BINARY_DIR}/lint-stamp")
  	set (lintname "${sourcefile}")
  	string(REPLACE "${PROJECT_SOURCE_DIR}/" "" lintname ${lintname})
	string(REPLACE "/" "_" lintname ${lintname})
	string(TOLOWER ${lintname} lintname)
	set (lintname "${lintstampdir}/${lintname}")
	file(MAKE_DIRECTORY "${lintstampdir}")

	if (${lintname} MATCHES "_di_types_h")
		continue()
	endif()

	string(REPLACE "${CMAKE_SOURCE_DIR}/" "" filenamedisplay ${sourcefile})
	get_filename_component(sourcefilename ${sourcefile} NAME)

	add_custom_command(OUTPUT ${lintname}
		COMMAND "${CMAKE_COMMAND}" -E chdir
			"${CMAKE_CURRENT_SOURCE_DIR}"
			"${PERL_EXECUTABLE}"
			"${CMAKE_CURRENT_BINARY_DIR}/checkpatch.pl"
			"--root=${CMAKE_CURRENT_SOURCE_DIR}"
			"--no-tree"
			"--terse"
			"--strict"
			"--showfile"
			"--summary-file"
			"--max-line-length=120"
			"--ignore=PREFER_KERNEL_TYPES"
			"--ignore=BIT_MACRO"
			"--ignore=VOLATILE"
			"--ignore=COMPLEX_MACRO"
			"--ignore=NEW_TYPEDEFS"
			"--ignore=MULTISTATEMENT_MACRO_USE_DO_WHILE"
			"--ignore=TRAILING_SEMICOLON"
			"-f"
			${sourcefile}
			"&&"
			"touch"
			"${lintname}"
		DEPENDS ${sourcefile}
		COMMENT "Checkpatch ${filenamedisplay}"
		VERBATIM)

	list (APPEND LINT_SOURCES_LIST ${lintname})
  endforeach(sourcefile)

  add_custom_target(lint
    DEPENDS ${LINT_SOURCES_LIST}
    COMMENT "Linting project ${PROJECT_NAME}"
    VERBATIM)
endfunction()
