#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <vector>
#include <iostream>
#include <cmath>

using namespace sf;
using namespace std;

int main() {
    RenderWindow window(VideoMode(1280, 720), "Paint");
    ImGui::SFML::Init(window);

    // --- تحميل خط Font Awesome ---
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault(); // الخط الأساسي
    ImFontConfig config;
    config.MergeMode = true;
    config.PixelSnapH = true;
    static const ImWchar icons_ranges[] = { 0xf000, 0xf8ff, 0 };
    ImFont* iconFont = io.Fonts->AddFontFromFileTTF("fa-solid-900.ttf", 24.0f, &config, icons_ranges);
    ImGui::SFML::UpdateFontTexture();

    bool flagDraw = false;
    bool flagErase = false;
    CircleShape circle;
    vector<CircleShape> v;
    v.reserve(100000);

    Clock deltaClock;
    static float brushSize = 5.f;
    static float eraserSize = 20.f;
    static ImVec4 brushColor = ImVec4(0, 0, 0, 1);

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == Event::Closed)
                window.close();
        }

        // الرسم بالماوس
        static bool hasPrev = false;
        static Vector2f prevPos;
        if (Mouse::isButtonPressed(Mouse::Left)) {
            Vector2f currentPos(Mouse::getPosition(window));

            if (flagDraw) {
                if (hasPrev) {
                    Vector2f diff = currentPos - prevPos;
                    float distance = sqrt(diff.x * diff.x + diff.y * diff.y);
                    int steps = static_cast<int>(distance / 2.f);
                    for (int i = 0; i <= steps; ++i) {
                        float t = (steps == 0 ? 0.f : static_cast<float>(i) / steps);
                        Vector2f interp = prevPos + t * diff;

                        circle.setRadius(brushSize);
                        circle.setFillColor(Color(
                            brushColor.x * 255,
                            brushColor.y * 255,
                            brushColor.z * 255
                        ));
                        circle.setPosition(interp);
                        v.push_back(circle);
                    }
                }
                prevPos = currentPos;
                hasPrev = true;
            }
            else if (flagErase) {
                circle.setRadius(eraserSize);
                circle.setFillColor(Color::White);
                circle.setPosition(currentPos);
                v.push_back(circle);
            }
        }
        else hasPrev = false;

        ImGui::SFML::Update(window, deltaClock.restart());

        // --- ✅ شريط القوائم العلوي (Menu Bar) ---
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New")) {
                    v.clear();
                }
                if (ImGui::MenuItem("Save")) {
                    Texture texture;
                    texture.create(window.getSize().x, window.getSize().y);
                    texture.update(window);
                    Image screenshot = texture.copyToImage();
                    if (screenshot.saveToFile("painting.png"))
                        cout << "✅ Saved successfully as painting.png" << endl;
                    else
                        cout << "❌ Failed to save image!" << endl;
                }
                if (ImGui::MenuItem("Exit")) {
                    window.close();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Clear All")) {
                    v.clear();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) {
                    ImGui::OpenPopup("AboutWindow");
                }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        // نافذة "About"
        if (ImGui::BeginPopup("AboutWindow")) {
            ImGui::Text("Simple Paint Program");
            ImGui::Text("Created by Mohamed using SFML + ImGui");
            ImGui::Separator();
            ImGui::Text("Press and hold left mouse to draw.");
            ImGui::EndPopup();
        }

        // --- واجهة الأدوات ---
        ImGui::SetNextWindowPos(ImVec2(10, 40)); // ↓ نزلها تحت المينيو بار
        ImGui::SetNextWindowSize(ImVec2(400, 180));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.95f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.25f, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.3f, 0.3f, 1));

        ImGui::Begin("Paint Tools", nullptr,
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove);

        ImGui::PushFont(iconFont);

        // Draw button (pencil)
        if (ImGui::Button(u8"\uf304", ImVec2(50, 50))) {
            flagDraw = true;
            flagErase = false;
        }
        ImGui::SameLine();

        // Erase button (eraser)
        if (ImGui::Button(u8"\uf12d", ImVec2(50, 50))) {
            flagErase = true;
            flagDraw = false;
        }
        ImGui::SameLine();

        // Clear button (broom)
        if (ImGui::Button(u8"\uf51a", ImVec2(50, 50))) {
            v.clear();
        }
        ImGui::SameLine();

        // Save button (floppy disk)
        if (ImGui::Button(u8"\uf0c7", ImVec2(50, 50))) {
            Texture texture;
            texture.create(window.getSize().x, window.getSize().y);
            texture.update(window);
            Image screenshot = texture.copyToImage();
            if (screenshot.saveToFile("painting.png"))
                cout << "✅ Saved successfully as painting.png" << endl;
            else
                cout << "❌ Failed to save image!" << endl;
        }

        ImGui::PopFont();

        ImGui::Separator();
        ImGui::SliderFloat("Brush Size", &brushSize, 1.f, 30.f);
        ImGui::SliderFloat("Eraser Size", &eraserSize, 5.f, 50.f);
        ImGui::ColorEdit3("Brush Color", (float*)&brushColor);

        ImGui::End();
        ImGui::PopStyleColor(5);

        // --- الرسم على الشاشة ---
        window.clear(Color::White);
        for (auto& c : v)
            window.draw(c);

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}
