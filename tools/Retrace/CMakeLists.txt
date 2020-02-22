project(fuzzilly-retrace)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

add_executable(
        fuzzilly-retrace
        Main.cpp
        fzl/Retracer/TraceReader.cpp
        fzl/Retracer/Retracer.cpp
        ${FZL_COMMON_SOURCES}
        fzl/Retracer/MemoryModel.cpp fzl/Retracer/MemoryModel.h)

llvm_map_components_to_libnames(llvm_libs support core irreader TransformUtils BitWriter)

target_include_directories(fuzzilly-retrace PRIVATE
        .
)
target_link_libraries(fuzzilly-retrace ${llvm_libs})
