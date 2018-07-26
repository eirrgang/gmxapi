# Clean the staging area.
#file(REMOVE_RECURSE ${CMAKE_BINARY_DIR}/gmx)
#file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/gmx/data)
#
#configure_file(__init__.py ${CMAKE_BINARY_DIR}/gmx/ COPYONLY)
#configure_file(context.py ${CMAKE_BINARY_DIR}/gmx/ COPYONLY)
#configure_file(data.py ${CMAKE_BINARY_DIR}/gmx/ COPYONLY)
#configure_file(exceptions.py ${CMAKE_BINARY_DIR}/gmx/ COPYONLY)
#configure_file(fileio.py ${CMAKE_BINARY_DIR}/gmx/ COPYONLY)
#configure_file(status.py ${CMAKE_BINARY_DIR}/gmx/ COPYONLY)
#configure_file(system.py ${CMAKE_BINARY_DIR}/gmx/ COPYONLY)
#configure_file(util.py ${CMAKE_BINARY_DIR}/gmx/ COPYONLY)
#configure_file(workflow.py ${CMAKE_BINARY_DIR}/gmx/ COPYONLY)
#
#configure_file(data/topol.tpr ${CMAKE_BINARY_DIR}/gmx/data/ COPYONLY)

configure_file(version.in version.py @ONLY)

#file(COPY test DESTINATION ${CMAKE_BINARY_DIR}/gmx/)

#install(DIRECTORY ${CMAKE_BINARY_DIR}/gmx/
#        DESTINATION gmx
#        FILES_MATCHING PATTERN "*.py" PATTERN "*.ini"
#        )

install(FILES
        __init__.py
        context.py
        data.py
        exceptions.py
        fileio.py
        status.py
        system.py
        util.py
        workflow.py
        ${CMAKE_CURRENT_BINARY_DIR}/version.py
        DESTINATION gmx)

install(FILES
        test/conftest.py
        test/test_docstrings.py
        test/test_exceptions.py
        test/test_fileio.py
        test/test_mpiarraycontext.py
        test/test_pymd.py
        test/test_workflow.py
        DESTINATION gmx/test)

install(FILES data/topol.tpr
        DESTINATION gmx/data)

add_subdirectory(core)
