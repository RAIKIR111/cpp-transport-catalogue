cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

set(FILE_SOURCES domain.cpp domain.h geo.cpp geo.h graph.h json.cpp json.h json_builder.cpp json_builder.h json_reader.cpp json_reader.h main.cpp map_renderer.cpp map_renderer.h ranges.h request_handler.cpp request_handler.h router.h serialization.cpp serialization.h svg.cpp svg.h transport_catalogue.h transport_catalogue.cpp transport_catalogue.proto transport_router.cpp transport_router.h graph.proto map_renderer.proto svg.proto transport_catalogue.proto transport_router.proto)

add_executable(transport_catalogue ${FILE_SOURCES})
