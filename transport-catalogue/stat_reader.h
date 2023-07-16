#pragma once

#include <iostream>
#include <cassert>
#include <algorithm>
#include <vector>
#include <string_view>

#include "input_reader.h"

namespace transport_catalogue {
    class TransportCatalogue;
}

namespace requests {

class StatReader {
public:
    StatReader(transport_catalogue::TransportCatalogue* catalogue_obj, data_base::InputReader* input_reader);

    void ProccessRequests(std::ostream& output) const;

private:
    transport_catalogue::TransportCatalogue* catalogue_obj_ = nullptr;
    data_base::InputReader* input_reader_ = nullptr;
};
}