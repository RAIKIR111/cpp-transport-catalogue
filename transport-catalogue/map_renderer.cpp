#include "map_renderer.h"

using namespace map_renderer;
using namespace std;

bool map_renderer::IsZero(double value) {
    return std::abs(value) < EPSILON;
}

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */