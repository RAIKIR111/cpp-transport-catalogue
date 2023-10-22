#include "json_reader.h"

#include <fstream>
#include <iostream>
#include <string_view>
#include <iostream>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    json_reader::JsonReader j_reader(std::cin);
    if (mode == "make_base"sv) {
        j_reader.MakeBase();
        // make base here

    } else if (mode == "process_requests"sv) {
        j_reader.ProccessRequests();
        // process requests here

    } else {
        PrintUsage();
        return 1;
    }
}
