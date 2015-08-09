cmake_minimum_required(VERSION 3.2)
project(trunk)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    base/appconn/appclient.hpp
    base/appconn/appconn.hpp
    base/appconn/appmessages.hpp
    base/appconn/compositeie.hpp
    base/appconn/conn.hpp
    base/appconn/ie.hpp
    base/appconn/message.hpp
    base/appconn/primitiveie.hpp
    base/cqueue.hpp
    base/exception.hpp
    base/executor.hpp
    base/logger.hpp
    base/properties.hpp
    base/socket.hpp
    base/thread.hpp
    base/timer.hpp
    base/utils.hpp
    cf/cap/appconn/cfie.hpp
    cf/cap/appconn/cfmessage.hpp
    cf/cap/block.hpp
    cf/cap/capdialog.hpp
    cf/cap/cftask.hpp
    cf/cap/dialog.hpp
    cf/cap/gmapapp.hpp
    gmap/gblocksm.hpp
    gmap/gmapconn.hpp
    gmap/gblock
    properties/gmap.ati.10.properties
    properties/start_gmap.ati.10.sh
    main.cpp)

    include_directories("/Users/asalazar/dev/cpp/ulcm/include")

add_executable(trunk ${SOURCE_FILES} gmap/gblock.hpp)