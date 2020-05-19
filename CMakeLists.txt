

#add_executable (${PROJECT_NAME}
#        "src/main.cpp"
#        "src/cmdParser.cpp" "src/Client.cpp" "src/Server.cpp" "src/UdpConnection.cpp" )

#target_include_directories( ${PROJECT_NAME}
#        PRIVATE
#        ${PROJECT_SOURCE_DIR}/include
#        )

cmake_minimum_required(VERSION 3.15)
project(testforlog)

set(CMAKE_CXX_STANDARD 14)

INCLUDE_DIRECTORIES(
        ${PROJECT_SOURCE_DIR}/include
)

add_executable(
        NetTest
        "src/main.cpp"
        "src/cmdParser.cpp" "src/Client.cpp" "src/Server.cpp" "src/UdpConnection.cpp"
)



