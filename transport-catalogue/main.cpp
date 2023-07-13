#include "transport_catalogue.h"

using namespace std;

int main() {
    transport_catalogue::TransportCatalogue obj(cin);
    obj.ProccessDataBase();
    obj.ProccessRequests();
}