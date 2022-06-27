# @author Shin'ichiro Nakaoka

file(GLOB subdirs "*")
list(SORT subdirs)

foreach(subdir ${subdirs})
  if(EXISTS ${subdir}/CMakeLists.txt)
    add_subdirectory(${subdir})
    message("cache_value: ${subdir}")
  endif()
endforeach()
