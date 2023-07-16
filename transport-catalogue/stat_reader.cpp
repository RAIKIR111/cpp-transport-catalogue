#include "stat_reader.h"
#include "transport_catalogue.h"

using namespace std;
using namespace requests;

StatReader::StatReader(transport_catalogue::TransportCatalogue* catalogue_obj, data_base::InputReader* input_reader) : catalogue_obj_(catalogue_obj), input_reader_(input_reader) {
    ProccessRequests(cout);
}

void StatReader::ProccessRequests(std::ostream& output) const {
    auto ready_info = input_reader_->ProccessRawDataBase();
    for (const auto& parsed_stop_info : get<0>(ready_info)) {
        string_view main_stopname = get<0>(parsed_stop_info);
        geo::Coordinates coordinates{get<1>(parsed_stop_info), get<2>(parsed_stop_info)};
        catalogue_obj_->AddStop(main_stopname, coordinates);
        for (const auto& [stopname, distance] : get<3>(parsed_stop_info)) {
            catalogue_obj_->SetDistanceBetweenStops(main_stopname, stopname, distance);
        }
    }
    for (const auto& parsed_bus_info : get<1>(ready_info)) {
        string_view busname = get<0>(parsed_bus_info);
        vector<string_view> route = get<1>(parsed_bus_info);
        char sym = get<2>(parsed_bus_info);
        catalogue_obj_->AddBus(busname, route, sym);
    }

    auto requests = input_reader_->ProccessRawRequests();
    for (const auto& [type, request] : requests) {
        if (type == 'B') {
            if (!catalogue_obj_->FindBus(request)) {
                output << "Bus "s << request << ": not found"s << endl;
                continue;
            }

            auto bus_ptr = catalogue_obj_->FindBus(request);

            unordered_set<string_view, transport_catalogue::TransportCatalogue::StringViewHasher> unique_stops;
            int route_length = 0;
            double geo_length = 0;
            for (auto counter = 0; counter < bus_ptr->route.size() - 1; ++counter) {
                route_length += catalogue_obj_->GetDistanceBetweenStops(bus_ptr->route.at(counter)->name, bus_ptr->route.at(counter + 1)->name);
                geo_length += geo::ComputeDistance(bus_ptr->route.at(counter)->coordinates, bus_ptr->route.at(counter + 1)->coordinates);
                unique_stops.insert(bus_ptr->route.at(counter)->name);
            }
            unique_stops.insert(bus_ptr->route.at(bus_ptr->route.size() - 1)->name);
            // route length is ready
            // unique stops is ready

            double curvature = double(route_length) / geo_length;
            // curvature is ready

            output << "Bus "s << bus_ptr->name << ": "s << bus_ptr->route.size() << " stops on route, "s << unique_stops.size() << " unique stops, "s << route_length << " route length, "s << curvature << " curvature"s << endl;
        }
        else if (type == 'S') {
            if (!catalogue_obj_->FindStop(request)) {
                output << "Stop " << request << ": not found" << endl;
                continue;
            }

            if (!catalogue_obj_->FindBusesCrossingStop(catalogue_obj_->FindStop(request)).size()) {
                output << "Stop " << request << ": no buses" << endl;
                continue;
            }
            
            set<string_view> temp_set_buses;
            for (const auto& item : catalogue_obj_->FindBusesCrossingStop(catalogue_obj_->FindStop(request))) {
                temp_set_buses.insert(item->name);
            }
            output << "Stop " << request << ": buses";
            for (const auto& item : temp_set_buses) {
                output << " " << item;
            }
            output << endl;
        }
    }
}