#include "menu.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "data_structures/algorithm.h"
#include "raylib.h"

namespace {

constexpr int ScreenWidth = 1120;
constexpr int ScreenHeight = 720;
constexpr int InputSize = 128;
constexpr float NodeRadius = 6.0f;

enum class ToolPanel {
    None,
    AddLocation,
    DeleteLocation,
    AddPath,
    Search
};

struct TextBox {
    Rectangle bounds;
    char text[InputSize];
    bool active;
    int suggestionIndex = 0;
};

struct NodeView {
    Node* node;
    Vector2 position;
};

struct MapImageConfig {
    std::string imagePath = "assets/Masterplan  KMUTT.jpg";
    int width = 4929;
    int height = 3374;
    int cropX = 1400;
    int cropY = 650;
    int cropW = 3250;
    int cropH = 2600;
    int scalePixels = 409;
    int scaleMeters = 100;
};

struct MapViewState {
    float centerX;
    float centerY;
    float zoom = 1.0f;
};

struct PathResult {
    std::vector<int> nodeIds;
    int distance;
};

Font loadAppFont() {
    Font font = LoadFontEx("/System/Library/Fonts/Supplemental/Trebuchet MS.ttf", 36, nullptr, 0);
    if (font.texture.id == 0) {
        return GetFontDefault();
    }
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
    return font;
}

void drawText(Font font, const char* text, float x, float y, float size, Color color) {
    DrawTextEx(font, text, {x, y}, size, 1.0f, color);
}

float textWidth(Font font, const char* text, float size) {
    return MeasureTextEx(font, text, size, 1.0f).x;
}

bool button(Font font, Rectangle bounds, const char* label, bool active = false) {
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, bounds);
    bool pressed = hovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

    Color fill = active ? Color{30, 98, 166, 255}
                        : hovered ? Color{58, 116, 171, 255} : Color{246, 248, 251, 255};
    Color textColor = active ? RAYWHITE : Color{34, 42, 54, 255};
    Color border = active ? Color{19, 73, 128, 255} : Color{202, 211, 222, 255};

    DrawRectangleRounded(bounds, 0.12f, 8, fill);
    DrawRectangleRoundedLines(bounds, 0.12f, 8, border);

    float size = 17.0f;
    drawText(font, label, bounds.x + (bounds.width - textWidth(font, label, size)) / 2.0f,
             bounds.y + (bounds.height - size) / 2.0f - 1.0f, size, textColor);
    return pressed;
}

bool primaryButton(Font font, Rectangle bounds, const char* label) {
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, bounds);
    bool pressed = hovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
    Color fill = hovered ? Color{34, 112, 188, 255} : Color{28, 95, 164, 255};

    DrawRectangleRounded(bounds, 0.12f, 8, fill);
    DrawRectangleRoundedLines(bounds, 0.12f, 8, Color{17, 73, 128, 255});

    float size = 18.0f;
    drawText(font, label, bounds.x + (bounds.width - textWidth(font, label, size)) / 2.0f,
             bounds.y + (bounds.height - size) / 2.0f - 1.0f, size, RAYWHITE);
    return pressed;
}

char lowerAscii(char value) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(value)));
}

bool containsIgnoreCase(const char* text, const char* query) {
    if (!query[0]) return true;

    std::string haystack = text;
    std::string needle = query;
    std::transform(haystack.begin(), haystack.end(), haystack.begin(), lowerAscii);
    std::transform(needle.begin(), needle.end(), needle.begin(), lowerAscii);
    return haystack.find(needle) != std::string::npos;
}

std::string nodeSearchLabel(Node* node) {
    if (!node) return "";
    if (std::strcmp(node->code, node->name) == 0) return node->name;
    return std::string(node->code) + " - " + node->name;
}

std::vector<Node*> matchingLocations(Graph* g, const char* query) {
    std::vector<Node*> matches;
    if (!g || !query[0]) return matches;

    for (Node* curr = g->head; curr; curr = curr->next) {
        if (containsIgnoreCase(curr->name, query) || containsIgnoreCase(curr->code, query)) {
            matches.push_back(curr);
        }
    }
    std::sort(matches.begin(), matches.end(), [](Node* a, Node* b) {
        return std::strcmp(a->code, b->code) < 0;
    });
    return matches;
}

Node* resolveLocation(Graph* g, const char* input) {
    Node* exact = findNode(g, input);
    if (exact) return exact;
    std::vector<Node*> matches = matchingLocations(g, input);
    return matches.empty() ? nullptr : matches.front();
}

void fillTextBox(TextBox& box, const char* text) {
    std::strncpy(box.text, text, InputSize);
    box.text[InputSize - 1] = '\0';
    box.suggestionIndex = 0;
}

bool textBox(Font font, TextBox& box, Graph* guideGraph = nullptr) {
    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        box.active = CheckCollisionPointRec(mouse, box.bounds);
    }

    if (box.active) {
        int key = GetCharPressed();
        while (key > 0) {
            size_t len = std::strlen(box.text);
            if (key >= 32 && key <= 125 && len < InputSize - 1) {
                box.text[len] = static_cast<char>(key);
                box.text[len + 1] = '\0';
                box.suggestionIndex = 0;
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            size_t len = std::strlen(box.text);
            if (len > 0) {
                box.text[len - 1] = '\0';
                box.suggestionIndex = 0;
            }
        }

        std::vector<Node*> matches = matchingLocations(guideGraph, box.text);
        if (!matches.empty()) {
            if (box.suggestionIndex >= static_cast<int>(matches.size())) {
                box.suggestionIndex = 0;
            }
            if (IsKeyPressed(KEY_DOWN)) {
                box.suggestionIndex = (box.suggestionIndex + 1) % static_cast<int>(matches.size());
            }
            if (IsKeyPressed(KEY_UP)) {
                box.suggestionIndex =
                    (box.suggestionIndex + static_cast<int>(matches.size()) - 1) % static_cast<int>(matches.size());
            }
            if (IsKeyPressed(KEY_TAB)) {
                fillTextBox(box, matches[box.suggestionIndex]->code);
            }
        }
    }

    DrawRectangleRounded(box.bounds, 0.08f, 6, RAYWHITE);
    DrawRectangleRoundedLines(box.bounds, 0.08f, 6,
                              box.active ? Color{28, 95, 164, 255} : Color{190, 199, 210, 255});
    drawText(font, box.text, box.bounds.x + 12.0f, box.bounds.y + 10.0f, 18.0f, Color{28, 32, 38, 255});

    return box.active && IsKeyPressed(KEY_ENTER);
}

void drawSuggestionMenu(Font font, TextBox& box, Graph* guideGraph) {
    std::vector<Node*> matches = matchingLocations(guideGraph, box.text);
    if (!box.active || matches.empty()) return;
    if (box.suggestionIndex >= static_cast<int>(matches.size())) {
        box.suggestionIndex = 0;
    }

    int visibleCount = std::min(4, static_cast<int>(matches.size()));
    Rectangle menu{box.bounds.x, box.bounds.y + box.bounds.height + 6.0f, box.bounds.width,
                   static_cast<float>(visibleCount * 28 + 8)};
    DrawRectangleRounded(menu, 0.08f, 6, Color{255, 255, 255, 248});
    DrawRectangleRoundedLines(menu, 0.08f, 6, Color{166, 184, 204, 255});

    int start = 0;
    if (box.suggestionIndex >= visibleCount) {
        start = box.suggestionIndex - visibleCount + 1;
    }
    for (int i = 0; i < visibleCount; ++i) {
        int index = start + i;
        Rectangle row{menu.x + 4.0f, menu.y + 4.0f + i * 28.0f, menu.width - 8.0f, 26.0f};
        if (index == box.suggestionIndex) {
            DrawRectangleRounded(row, 0.08f, 4, Color{225, 237, 250, 255});
        }
        std::string label = nodeSearchLabel(matches[index]);
        drawText(font, label.c_str(), row.x + 8.0f, row.y + 5.0f, 14.0f, Color{33, 42, 54, 255});
    }

    drawText(font, "Tab selects  Up/Down changes", menu.x + 8.0f, menu.y + menu.height + 6.0f, 12.0f,
             Color{92, 100, 112, 255});
}

void clearText(TextBox& box) {
    box.text[0] = '\0';
    box.active = false;
    box.suggestionIndex = 0;
}

std::vector<Node*> collectNodes(Graph* g) {
    std::vector<Node*> nodes;
    for (Node* curr = g->head; curr; curr = curr->next) {
        nodes.push_back(curr);
    }
    std::sort(nodes.begin(), nodes.end(), [](Node* a, Node* b) {
        return a->id < b->id;
    });
    return nodes;
}

void rebuildHash(Graph* g, HashTable* ht) {
    for (int i = 0; i < ht->size; i++) {
        ht->table[i].occupied = false;
    }
    for (Node* curr = g->head; curr; curr = curr->next) {
        hashInsert(ht, curr->name, curr->id);
        if (std::strcmp(curr->code, curr->name) != 0) {
            hashInsert(ht, curr->code, curr->id);
        }
    }
}

MapImageConfig loadMapImageConfig(const char* filename) {
    MapImageConfig config;
    std::ifstream file(filename);
    if (!file) return config;

    bool inMapSection = false;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;
        if (line == "[MapImage]") {
            inMapSection = true;
            continue;
        }
        if (!line.empty() && line[0] == '[') {
            inMapSection = false;
            continue;
        }
        if (!inMapSection) continue;

        size_t sep = line.find('=');
        if (sep == std::string::npos) continue;
        std::string key = line.substr(0, sep);
        std::string value = line.substr(sep + 1);
        try {
            if (key == "image") config.imagePath = value;
            if (key == "width") config.width = std::stoi(value);
            if (key == "height") config.height = std::stoi(value);
            if (key == "crop_x") config.cropX = std::stoi(value);
            if (key == "crop_y") config.cropY = std::stoi(value);
            if (key == "crop_w") config.cropW = std::stoi(value);
            if (key == "crop_h") config.cropH = std::stoi(value);
            if (key == "scale_pixels") config.scalePixels = std::stoi(value);
            if (key == "scale_meters") config.scaleMeters = std::stoi(value);
        } catch (...) {
            continue;
        }
    }
    return config;
}

Rectangle fitMapRect(Rectangle area, const MapImageConfig& config) {
    float sourceAspect = static_cast<float>(config.cropW) / static_cast<float>(config.cropH);
    float areaAspect = area.width / area.height;
    Rectangle dest = area;
    if (areaAspect > sourceAspect) {
        dest.width = area.height * sourceAspect;
        dest.x = area.x + (area.width - dest.width) / 2.0f;
    } else {
        dest.height = area.width / sourceAspect;
        dest.y = area.y + (area.height - dest.height) / 2.0f;
    }
    return dest;
}

MapViewState defaultMapView(const MapImageConfig& config) {
    return {config.cropX + config.cropW / 2.0f, config.cropY + config.cropH / 2.0f, 1.0f};
}

Rectangle sourceRectForView(const MapImageConfig& config, const MapViewState& view) {
    float zoom = std::max(1.0f, std::min(4.0f, view.zoom));
    float width = config.cropW / zoom;
    float height = config.cropH / zoom;
    float minX = static_cast<float>(config.cropX);
    float minY = static_cast<float>(config.cropY);
    float maxX = static_cast<float>(config.cropX + config.cropW);
    float maxY = static_cast<float>(config.cropY + config.cropH);
    float x = view.centerX - width / 2.0f;
    float y = view.centerY - height / 2.0f;

    if (x < minX) x = minX;
    if (y < minY) y = minY;
    if (x + width > maxX) x = maxX - width;
    if (y + height > maxY) y = maxY - height;
    return {x, y, width, height};
}

void clampMapView(MapViewState& view, const MapImageConfig& config) {
    view.zoom = std::max(1.0f, std::min(4.0f, view.zoom));
    Rectangle source = sourceRectForView(config, view);
    view.centerX = source.x + source.width / 2.0f;
    view.centerY = source.y + source.height / 2.0f;
}

Vector2 screenToImagePoint(Vector2 screen, Rectangle mapDest, Rectangle source) {
    float x = source.x + ((screen.x - mapDest.x) / mapDest.width) * source.width;
    float y = source.y + ((screen.y - mapDest.y) / mapDest.height) * source.height;
    return {x, y};
}

void zoomMapView(MapViewState& view, const MapImageConfig& config, Rectangle mapDest, Vector2 focusScreen,
                 float zoomFactor) {
    Rectangle before = sourceRectForView(config, view);
    Vector2 focusImage = screenToImagePoint(focusScreen, mapDest, before);
    float relX = (focusScreen.x - mapDest.x) / mapDest.width;
    float relY = (focusScreen.y - mapDest.y) / mapDest.height;

    view.zoom = std::max(1.0f, std::min(4.0f, view.zoom * zoomFactor));
    float newWidth = config.cropW / view.zoom;
    float newHeight = config.cropH / view.zoom;
    view.centerX = focusImage.x + newWidth * (0.5f - relX);
    view.centerY = focusImage.y + newHeight * (0.5f - relY);
    clampMapView(view, config);
}

void panMapView(MapViewState& view, const MapImageConfig& config, Rectangle mapDest, Vector2 deltaScreen) {
    Rectangle source = sourceRectForView(config, view);
    view.centerX -= deltaScreen.x * source.width / mapDest.width;
    view.centerY -= deltaScreen.y * source.height / mapDest.height;
    clampMapView(view, config);
}

Vector2 imagePointToScreen(int imageX, int imageY, Rectangle mapDest, Rectangle source) {
    float x = mapDest.x + (static_cast<float>(imageX) - source.x) / source.width * mapDest.width;
    float y = mapDest.y + (static_cast<float>(imageY) - source.y) / source.height * mapDest.height;
    return {x, y};
}

std::vector<NodeView> layoutNodes(Graph* g, Rectangle mapDest, Rectangle source) {
    std::vector<Node*> nodes = collectNodes(g);
    std::vector<NodeView> views;
    if (nodes.empty()) return views;

    int fallbackIndex = 0;
    constexpr int fallbackColumns = 4;
    float fallbackStartX = mapDest.x + mapDest.width * 0.22f;
    float fallbackStartY = mapDest.y + mapDest.height * 0.88f;
    float fallbackGapX = mapDest.width * 0.15f;
    float fallbackGapY = 62.0f;

    for (size_t i = 0; i < nodes.size(); ++i) {
        Vector2 position;
        if (nodes[i]->hasImagePosition) {
            position = imagePointToScreen(nodes[i]->imageX, nodes[i]->imageY, mapDest, source);
        } else {
            int row = fallbackIndex / fallbackColumns;
            int col = fallbackIndex % fallbackColumns;
            position = {fallbackStartX + col * fallbackGapX, fallbackStartY + row * fallbackGapY};
            fallbackIndex++;
        }
        views.push_back({nodes[i], position});
    }
    return views;
}

Vector2 positionFor(const std::vector<NodeView>& views, int nodeId) {
    for (const NodeView& view : views) {
        if (view.node->id == nodeId) return view.position;
    }
    return {0, 0};
}

bool pathContainsNode(const std::vector<int>& path, int nodeId) {
    return std::find(path.begin(), path.end(), nodeId) != path.end();
}

bool nodeMatchesKey(Node* node, const std::string& key) {
    return node && (key == node->code || key == node->name);
}

Node* nodeAtMouse(const std::vector<NodeView>& views) {
    Vector2 mouse = GetMousePosition();
    for (const NodeView& view : views) {
        if (CheckCollisionPointCircle(mouse, view.position, NodeRadius + 8.0f)) {
            return view.node;
        }
    }
    return nullptr;
}

Color nodeColorForType(const char* type) {
    if (std::strcmp(type, "gate") == 0) return Color{218, 64, 43, 255};
    if (std::strcmp(type, "food") == 0) return Color{232, 137, 32, 255};
    if (std::strcmp(type, "parking") == 0) return Color{54, 126, 195, 255};
    if (std::strcmp(type, "shop") == 0) return Color{72, 154, 64, 255};
    if (std::strcmp(type, "health") == 0) return Color{219, 48, 58, 255};
    if (std::strcmp(type, "park") == 0) return Color{62, 166, 75, 255};
    if (std::strcmp(type, "landmark") == 0) return Color{236, 168, 35, 255};
    return Color{35, 101, 164, 255};
}

void drawGraph(Font font, Graph* g, Rectangle area, Rectangle mapDest, Rectangle source,
               Texture2D mapTexture, bool mapLoaded,
               const std::vector<NodeView>& views,
               const std::string& selectedStart, const std::string& selectedEnd, const std::vector<int>& highlightPath,
               int searchedNodeId, Node* hovered) {
    DrawRectangleRounded(area, 0.02f, 8, Color{247, 249, 252, 255});
    DrawRectangleRoundedLines(area, 0.02f, 8, Color{207, 216, 226, 255});
    if (mapLoaded) {
        DrawTexturePro(mapTexture, source, mapDest, {0, 0}, 0.0f, WHITE);
    } else {
        DrawRectangleRec(mapDest, Color{233, 237, 241, 255});
        drawText(font, "Map image not loaded", mapDest.x + 20.0f, mapDest.y + 20.0f, 18.0f,
                 Color{92, 100, 112, 255});
    }
    DrawRectangleLinesEx(mapDest, 1.0f, Color{184, 195, 207, 255});

    if (views.empty()) {
        drawText(font, "No locations loaded", area.x + 28.0f, area.y + 28.0f, 22.0f, Color{92, 100, 112, 255});
        return;
    }

    BeginScissorMode(static_cast<int>(mapDest.x), static_cast<int>(mapDest.y), static_cast<int>(mapDest.width),
                     static_cast<int>(mapDest.height));
    for (size_t i = 1; i < highlightPath.size(); ++i) {
        Vector2 from = positionFor(views, highlightPath[i - 1]);
        Vector2 to = positionFor(views, highlightPath[i]);
        DrawLineEx(from, to, 9.0f, Color{239, 149, 54, 255});
        DrawLineEx(from, to, 4.0f, Color{255, 235, 158, 255});
    }

    for (const NodeView& view : views) {
        bool selected = nodeMatchesKey(view.node, selectedStart) || nodeMatchesKey(view.node, selectedEnd);
        bool onPath = pathContainsNode(highlightPath, view.node->id);
        bool searched = searchedNodeId == view.node->id;
        bool hot = hovered == view.node;
        Color fill = searched ? Color{36, 145, 100, 255}
                              : onPath ? Color{222, 118, 38, 255}
                                       : selected ? Color{214, 124, 54, 255} : nodeColorForType(view.node->type);
        Color ring = searched ? Color{14, 88, 58, 255}
                              : onPath ? Color{118, 61, 18, 255}
                                       : selected ? Color{136, 73, 29, 255} : Color{17, 63, 110, 255};

        if (hot) {
            DrawCircleV(view.position, NodeRadius + 8.0f, Color{207, 230, 252, 210});
        }
        DrawCircleV(view.position, NodeRadius + ((selected || onPath || searched) ? 4.0f : 0.0f),
                    searched ? Color{202, 244, 223, 255} : Color{255, 226, 201, 255});
        DrawCircleV(view.position, NodeRadius, fill);
        DrawCircleLines(static_cast<int>(view.position.x), static_cast<int>(view.position.y),
                        static_cast<int>(NodeRadius), ring);

        const char* code = view.node->code;
        float codeWidth = textWidth(font, code, 9.0f);
        DrawRectangleRounded({view.position.x - codeWidth / 2.0f - 3.0f, view.position.y + 7.0f,
                              codeWidth + 6.0f, 14.0f},
                             0.2f, 6, Color{255, 255, 255, 230});
        drawText(font, code, view.position.x - codeWidth / 2.0f, view.position.y + 10.0f, 9.0f,
                 Color{28, 32, 38, 255});

        bool showFullLabel = selected || searched || hot;
        if (showFullLabel) {
            std::string label = nodeSearchLabel(view.node);
            if (label.size() > 34) {
                label = label.substr(0, 31) + "...";
            }
            float labelWidth = textWidth(font, label.c_str(), 12.0f);
            float labelX = view.position.x + 12.0f;
            if (labelX + labelWidth + 12.0f > mapDest.x + mapDest.width) {
                labelX = view.position.x - labelWidth - 24.0f;
            }
            labelX = std::max(mapDest.x + 4.0f, labelX);
            float labelY = std::max(mapDest.y + 4.0f, view.position.y - 12.0f);
            DrawRectangleRounded({labelX, labelY, labelWidth + 12.0f, 20.0f},
                                 0.18f, 6, Color{255, 255, 255, 245});
            drawText(font, label.c_str(), labelX + 6.0f, labelY + 4.0f, 12.0f, Color{28, 32, 38, 255});
        }
    }
    EndScissorMode();
}

std::string captureDijkstra(Graph* g, const char* start, const char* end) {
    std::ostringstream buffer;
    std::streambuf* previous = std::cout.rdbuf(buffer.rdbuf());
    dijkstra(g, start, end);
    std::cout.rdbuf(previous);

    std::string result = buffer.str();
    while (!result.empty() && (result[0] == '\n' || result[0] == '\r')) {
        result.erase(result.begin());
    }
    return result;
}

PathResult shortestPath(Graph* g, const char* startName, const char* endName) {
    PathResult result{{}, 0};
    Node* startNode = findNode(g, startName);
    Node* endNode = findNode(g, endName);
    if (!startNode || !endNode || g->nodeCount <= 0) {
        return result;
    }

    int n = g->nodeCount;
    std::vector<int> dist(n, INF);
    std::vector<int> parent(n, -1);
    std::vector<bool> visited(n, false);
    dist[startNode->id] = 0;

    for (int i = 0; i < n; ++i) {
        int u = -1;
        int best = std::numeric_limits<int>::max();
        for (int id = 0; id < n; ++id) {
            if (!visited[id] && dist[id] < best) {
                best = dist[id];
                u = id;
            }
        }

        if (u == -1 || u == endNode->id) break;
        visited[u] = true;

        Node* current = nullptr;
        for (Node* node = g->head; node; node = node->next) {
            if (node->id == u) {
                current = node;
                break;
            }
        }
        if (!current) continue;

        for (Edge* edge = current->edges; edge; edge = edge->next) {
            if (!visited[edge->dest] && dist[u] + edge->weight < dist[edge->dest]) {
                dist[edge->dest] = dist[u] + edge->weight;
                parent[edge->dest] = u;
            }
        }
    }

    if (dist[endNode->id] == INF) {
        return result;
    }

    for (int at = endNode->id; at != -1; at = parent[at]) {
        result.nodeIds.push_back(at);
        if (at == startNode->id) break;
    }
    std::reverse(result.nodeIds.begin(), result.nodeIds.end());
    result.distance = dist[endNode->id];
    return result;
}

void drawStatus(Font font, const std::string& text, Rectangle area) {
    DrawRectangleRounded(area, 0.06f, 6, Color{255, 255, 255, 255});
    DrawRectangleRoundedLines(area, 0.06f, 6, Color{207, 216, 226, 255});

    float y = area.y + 14.0f;
    std::istringstream lines(text);
    std::string line;
    while (std::getline(lines, line) && y < area.y + area.height - 16.0f) {
        drawText(font, line.c_str(), area.x + 16.0f, y, 14.0f, Color{28, 32, 38, 255});
        y += 20.0f;
    }
}

}  // namespace

void showMenu(Graph* g, HashTable* ht) {
    InitWindow(ScreenWidth, ScreenHeight, "Smart Campus Navigation");
    SetTargetFPS(60);

    Font font = loadAppFont();
    bool customFontLoaded = font.texture.id != GetFontDefault().texture.id;
    MapImageConfig mapConfig = loadMapImageConfig("data/campus_map.txt");
    Texture2D mapTexture{};
    bool mapLoaded = false;
    if (FileExists(mapConfig.imagePath.c_str())) {
        mapTexture = LoadTexture(mapConfig.imagePath.c_str());
        mapLoaded = mapTexture.id != 0;
        if (mapLoaded) {
            SetTextureFilter(mapTexture, TEXTURE_FILTER_BILINEAR);
        }
    }

    TextBox location{{28, 526, 292, 42}, "", false};
    TextBox deleteName{{28, 526, 292, 42}, "", false};
    TextBox from{{28, 526, 140, 42}, "", false};
    TextBox to{{180, 526, 140, 42}, "", false};
    TextBox distance{{28, 604, 140, 42}, "", false};
    TextBox search{{28, 526, 292, 42}, "", false};
    TextBox pathFrom{{28, 213, 292, 36}, "", false};
    TextBox pathTo{{28, 277, 292, 36}, "", false};

    ToolPanel panel = ToolPanel::None;
    std::string selectedStart;
    std::string selectedEnd;
    std::vector<int> highlightPath;
    int searchedNodeId = -1;
    std::string status = "Click two locations on the map to find the shortest path.";
    MapViewState mapView = defaultMapView(mapConfig);
    bool draggingMap = false;
    Vector2 previousMouse{0, 0};

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(Color{235, 239, 245, 255});

        Rectangle sidebar{0, 0, 360, static_cast<float>(ScreenHeight)};
        Rectangle graphArea{390, 70, 700, 510};
        Rectangle mapDest = fitMapRect(graphArea, mapConfig);
        Vector2 mouse = GetMousePosition();
        bool mouseInMap = CheckCollisionPointRec(mouse, mapDest);
        float wheel = mouseInMap ? GetMouseWheelMove() : 0.0f;
        if (wheel > 0.0f) {
            zoomMapView(mapView, mapConfig, mapDest, mouse, 1.18f);
        } else if (wheel < 0.0f) {
            zoomMapView(mapView, mapConfig, mapDest, mouse, 1.0f / 1.18f);
        }
        if (mouseInMap && IsKeyPressed(KEY_EQUAL)) {
            zoomMapView(mapView, mapConfig, mapDest, {mapDest.x + mapDest.width / 2.0f,
                                                      mapDest.y + mapDest.height / 2.0f}, 1.25f);
        }
        if (mouseInMap && IsKeyPressed(KEY_MINUS)) {
            zoomMapView(mapView, mapConfig, mapDest, {mapDest.x + mapDest.width / 2.0f,
                                                      mapDest.y + mapDest.height / 2.0f}, 1.0f / 1.25f);
        }
        if (mouseInMap && IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            draggingMap = true;
            previousMouse = mouse;
        }
        if (draggingMap && IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
            Vector2 delta{mouse.x - previousMouse.x, mouse.y - previousMouse.y};
            if (delta.x != 0.0f || delta.y != 0.0f) {
                panMapView(mapView, mapConfig, mapDest, delta);
                previousMouse = mouse;
            }
        }
        if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) {
            draggingMap = false;
        }

        Rectangle source = sourceRectForView(mapConfig, mapView);
        std::vector<NodeView> views = layoutNodes(g, mapDest, source);
        Node* hovered = nodeAtMouse(views);
        TextBox* activeGuideBox = nullptr;
        SetMouseCursor(draggingMap ? MOUSE_CURSOR_RESIZE_ALL
                                   : hovered ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);

        DrawRectangleRec(sidebar, Color{248, 250, 253, 255});
        DrawLine(360, 0, 360, ScreenHeight, Color{207, 216, 226, 255});

        drawText(font, "Smart Campus", 28, 28, 31, Color{22, 31, 44, 255});
        drawText(font, "Navigation", 28, 64, 31, Color{22, 31, 44, 255});
        drawText(font, "Raylib GUI", 30, 108, 17, Color{92, 100, 112, 255});

        DrawRectangleRounded({28, 146, 292, 218}, 0.06f, 6, Color{255, 255, 255, 255});
        DrawRectangleRoundedLines({28, 146, 292, 218}, 0.06f, 6, Color{207, 216, 226, 255});
        drawText(font, "Shortest Path", 44, 162, 18, Color{40, 45, 52, 255});
        drawText(font, "Location 1", 28, 192, 13, Color{92, 100, 112, 255});
        drawText(font, "Location 2", 28, 256, 13, Color{92, 100, 112, 255});
        bool pathFromEnter = textBox(font, pathFrom, g);
        bool pathToEnter = textBox(font, pathTo, g);
        if (pathFrom.active) activeGuideBox = &pathFrom;
        if (pathTo.active) activeGuideBox = &pathTo;

        if (button(font, {28, 324, 140, 30}, "Clear")) {
            selectedStart.clear();
            selectedEnd.clear();
            highlightPath.clear();
            searchedNodeId = -1;
            clearText(pathFrom);
            clearText(pathTo);
            status = "Selection cleared.";
        }
        if (button(font, {180, 324, 140, 30}, "Run") || pathFromEnter || pathToEnter) {
            Node* startNode = resolveLocation(g, pathFrom.text);
            Node* endNode = resolveLocation(g, pathTo.text);
            if (!pathFrom.text[0] || !pathTo.text[0]) {
                highlightPath.clear();
                status = "Enter two location names or click two map nodes.";
            } else if (!startNode || !endNode) {
                highlightPath.clear();
                status = "Both shortest-path locations must exist.";
            } else {
                selectedStart = startNode->code;
                selectedEnd = endNode->code;
                fillTextBox(pathFrom, startNode->code);
                fillTextBox(pathTo, endNode->code);
                PathResult path = shortestPath(g, startNode->code, endNode->code);
                highlightPath = path.nodeIds;
                status = captureDijkstra(g, startNode->code, endNode->code);
            }
        }

        drawText(font, "Options", 28, 388, 19, Color{40, 45, 52, 255});
        if (button(font, {28, 420, 140, 42}, "Add", panel == ToolPanel::AddLocation)) panel = ToolPanel::AddLocation;
        if (button(font, {180, 420, 140, 42}, "Delete", panel == ToolPanel::DeleteLocation)) panel = ToolPanel::DeleteLocation;
        if (button(font, {28, 472, 140, 42}, "Path", panel == ToolPanel::AddPath)) panel = ToolPanel::AddPath;
        if (button(font, {180, 472, 140, 42}, "Search", panel == ToolPanel::Search)) panel = ToolPanel::Search;

        bool submit = false;
        if (panel == ToolPanel::AddLocation) {
            drawText(font, "Location name", 28, 502, 15, Color{92, 100, 112, 255});
            submit = textBox(font, location);
            if (primaryButton(font, {28, 586, 292, 42}, "Add Location") || submit) {
                if (!location.text[0]) {
                    status = "Enter a location name.";
                } else if (findNode(g, location.text)) {
                    status = std::string("'") + location.text + "' already exists.";
                } else {
                    addLocation(g, location.text);
                    rebuildHash(g, ht);
                    highlightPath.clear();
                    searchedNodeId = -1;
                    status = std::string("Added location: ") + location.text;
                    clearText(location);
                }
            }
        } else if (panel == ToolPanel::DeleteLocation) {
            drawText(font, "Location name", 28, 502, 15, Color{92, 100, 112, 255});
            submit = textBox(font, deleteName, g);
            if (deleteName.active) activeGuideBox = &deleteName;
            if (primaryButton(font, {28, 586, 292, 42}, "Delete Location") || submit) {
                if (!deleteName.text[0]) {
                    status = "Enter a location name.";
                } else if (!findNode(g, deleteName.text)) {
                    status = std::string("'") + deleteName.text + "' not found.";
                } else {
                    std::string removed = deleteName.text;
                    deleteLocation(g, deleteName.text);
                    rebuildHash(g, ht);
                    if (selectedStart == removed) selectedStart.clear();
                    if (selectedEnd == removed) selectedEnd.clear();
                    if (std::strcmp(pathFrom.text, removed.c_str()) == 0) clearText(pathFrom);
                    if (std::strcmp(pathTo.text, removed.c_str()) == 0) clearText(pathTo);
                    highlightPath.clear();
                    searchedNodeId = -1;
                    status = "Deleted location: " + removed;
                    clearText(deleteName);
                }
            }
        } else if (panel == ToolPanel::AddPath) {
            drawText(font, "From", 28, 502, 15, Color{92, 100, 112, 255});
            drawText(font, "To", 180, 502, 15, Color{92, 100, 112, 255});
            bool fromEnter = textBox(font, from, g);
            bool toEnter = textBox(font, to, g);
            if (from.active) activeGuideBox = &from;
            if (to.active) activeGuideBox = &to;
            drawText(font, "Distance", 28, 580, 15, Color{92, 100, 112, 255});
            bool distanceEnter = textBox(font, distance);
            if (primaryButton(font, {180, 604, 140, 42}, "Connect") || fromEnter || toEnter || distanceEnter) {
                int weight = std::atoi(distance.text);
                if (!from.text[0] || !to.text[0] || weight <= 0) {
                    status = "Enter from, to, and a positive distance.";
                } else if (!findNode(g, from.text) || !findNode(g, to.text)) {
                    status = "Both path locations must exist.";
                } else {
                    addPath(g, from.text, to.text, weight);
                    highlightPath.clear();
                    status = std::string("Added path: ") + from.text + " <-> " + to.text;
                    clearText(from);
                    clearText(to);
                    clearText(distance);
                }
            }
        } else if (panel == ToolPanel::Search) {
            drawText(font, "Location name", 28, 502, 15, Color{92, 100, 112, 255});
            submit = textBox(font, search, g);
            if (search.active) activeGuideBox = &search;
            if (primaryButton(font, {28, 586, 292, 42}, "Search") || submit) {
                Node* found = nullptr;
                int id = hashSearch(ht, search.text);
                if (id != -1) {
                    for (Node* node = g->head; node; node = node->next) {
                        if (node->id == id) {
                            found = node;
                            break;
                        }
                    }
                } else {
                    found = resolveLocation(g, search.text);
                    if (found) id = found->id;
                }
                searchedNodeId = id;
                if (id == -1) {
                    status = std::string("'") + search.text + "' not found.";
                } else {
                    status = "Found and highlighted " + nodeSearchLabel(found) + ".";
                }
            }
        }

        if (activeGuideBox) {
            drawSuggestionMenu(font, *activeGuideBox, g);
        }

        Node* clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON) ? hovered : nullptr;
        if (clicked) {
            if (selectedStart.empty() || (!selectedStart.empty() && !selectedEnd.empty())) {
                selectedStart = clicked->code;
                selectedEnd.clear();
                fillTextBox(pathFrom, clicked->code);
                clearText(pathTo);
                highlightPath.clear();
                status = "Start selected: " + nodeSearchLabel(clicked);
            } else if (nodeMatchesKey(clicked, selectedStart)) {
                selectedStart = clicked->code;
                selectedEnd.clear();
                fillTextBox(pathFrom, clicked->code);
                clearText(pathTo);
                highlightPath.clear();
                status = "Start selected: " + nodeSearchLabel(clicked);
            } else {
                selectedEnd = clicked->code;
                fillTextBox(pathTo, clicked->code);
                PathResult path = shortestPath(g, selectedStart.c_str(), selectedEnd.c_str());
                highlightPath = path.nodeIds;
                status = captureDijkstra(g, selectedStart.c_str(), selectedEnd.c_str());
            }
        }

        drawText(font, "Campus Map", 390, 28, 26, Color{22, 31, 44, 255});
        if (button(font, {808, 26, 42, 34}, "-", false)) {
            zoomMapView(mapView, mapConfig, mapDest, {mapDest.x + mapDest.width / 2.0f,
                                                      mapDest.y + mapDest.height / 2.0f}, 1.0f / 1.25f);
            source = sourceRectForView(mapConfig, mapView);
            views = layoutNodes(g, mapDest, source);
            hovered = nodeAtMouse(views);
        }
        std::string zoomText = std::to_string(static_cast<int>(mapView.zoom * 100.0f + 0.5f)) + "%";
        drawText(font, zoomText.c_str(), 862, 34, 15, Color{73, 82, 92, 255});
        if (button(font, {918, 26, 42, 34}, "+", false)) {
            zoomMapView(mapView, mapConfig, mapDest, {mapDest.x + mapDest.width / 2.0f,
                                                      mapDest.y + mapDest.height / 2.0f}, 1.25f);
            source = sourceRectForView(mapConfig, mapView);
            views = layoutNodes(g, mapDest, source);
            hovered = nodeAtMouse(views);
        }
        if (button(font, {974, 26, 116, 34}, "Reset view", false)) {
            mapView = defaultMapView(mapConfig);
            source = sourceRectForView(mapConfig, mapView);
            views = layoutNodes(g, mapDest, source);
            hovered = nodeAtMouse(views);
        }
        drawGraph(font, g, graphArea, mapDest, source, mapTexture, mapLoaded, views, selectedStart, selectedEnd,
                  highlightPath, searchedNodeId, hovered);
        drawStatus(font, status, {390, 600, 700, 88});

        drawText(font, "Esc or window close exits.", 28, 676, 15, Color{92, 100, 112, 255});
        EndDrawing();
    }

    if (customFontLoaded) {
        UnloadFont(font);
    }
    if (mapLoaded) {
        UnloadTexture(mapTexture);
    }
    CloseWindow();
}
