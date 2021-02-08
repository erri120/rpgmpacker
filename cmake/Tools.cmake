if (USE_CCACHE)
    CPMAddPackage (
            NAME Ccache.cmake
            GITHUB_REPOSITORY TheLartians/Ccache.cmake
            VERSION 1.2.2
    )
endif()