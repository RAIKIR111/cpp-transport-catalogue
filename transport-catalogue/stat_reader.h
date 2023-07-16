#pragma once

#include <iostream>
#include <cassert>
#include <algorithm>
#include <vector>
#include <string_view>

namespace transport_catalogue {
    class TransportCatalogue;
}

namespace requests {

class StatReader {
public:
    StatReader(transport_catalogue::TransportCatalogue* catalogue_obj);

    void ProccessRequests() const;

private:
    transport_catalogue::TransportCatalogue* catalogue_obj_ = nullptr;
};
}