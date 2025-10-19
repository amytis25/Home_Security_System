# Build the reaction timer application, using the HAL

# Find all source files in current directory
include_directories(include)
file(GLOB MY_SOURCES "*.c")

# Create the reaction_timer executable
add_executable(reaction_timer ${MY_SOURCES})

# Link with the HAL library
target_link_libraries(reaction_timer LINK_PRIVATE hal)

# Link additional libraries needed for timing and math functions
target_link_libraries(reaction_timer LINK_PRIVATE rt m)

# Copy executable to final location for deployment
add_custom_command(TARGET reaction_timer POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:reaction_timer>"
     "~/ensc351/public/as1/app/reaction_timer" 
  COMMENT "Copying ARM executable to public NFS directory")


